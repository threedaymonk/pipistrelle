// Supersaw
//
// Pot A => Coarse tune (C0 + 6 octaves)
// Pot B => Fine tune (+/- 1/2 octave)
// Pot C + CV 1 => Detune
// Pot D + CV 2 => Sub oscillator level
//
// theta: position within the wave cycle (0 to Q14_1)
// phi:   phase displacement per sample

#include <Pipistrelle.h>
#include <math.h>
#include <q14.h>
#include <cv.h>
#define SAMPLE_RATE 32000
#define C0 16.3516
#define OSCILLATORS 7
#define BUFSIZE 100

// Avoid dividing by zero before everything is up
uint32_t phi[OSCILLATORS] = {1, 1, 1, 1, 1, 1, 1},
         sub_phi = 1;
// Pleasing weighting determined through trial and error
const int osc_mix[OSCILLATORS] = {3, 4, 5, 6, 5, 4, 3};

q14_t sub_level = 0;
int osc_divisor = 0;
Pipistrelle *pip;

void setup() {
  // Compute the weighting divisor once
  for (int i = 0; i < OSCILLATORS; i++) osc_divisor += osc_mix[i];

  pip = new Pipistrelle(SAMPLE_RATE);
  pip->led(true); // Shows that we got this far
}

q14_t next_sample() {
  static q14_t theta[OSCILLATORS] = {0, 0, 0, 0, 0, 0, 0}, sub_theta = 0;
  q14_t sample = 0, sub_sample = 0;

  for (int i = 0; i < OSCILLATORS; i++) {
    sample += q14_saw(theta[i]) * osc_mix[i];
    theta[i] = (theta[i] + phi[i]) % Q14_1;
  }

  sub_sample = q14_square(sub_theta);
  sub_theta = (sub_theta + sub_phi) % Q14_1;

  return q14_blend(sub_level, sample / osc_divisor, sub_sample);
}

void loop() {
  float frequency, voct, detune;

  voct = pip->voct()
       + 6.0F * unipolar(pip->pota()) // coarse tuning
       + unipolar(pip->potb()) / 2.0F; // fine tuning

  frequency = C0 * pow(2, voct);

  detune = frequency / 80
         * unipolar_with_cv(pip->potc(), pip->cv1());

  sub_level = ftoq14(0.25f * unipolar_with_cv(pip->potd(), pip->cv2()));

  for (int i = 0; i < OSCILLATORS; i++) {
    phi[i] = ftoq14((frequency + detune * (i - OSCILLATORS / 2))
                      / SAMPLE_RATE);
  }

  sub_phi = ftoq14(frequency / (2 * SAMPLE_RATE));
}

void TC4_Handler() {
  pip->q14_dac_write(next_sample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
