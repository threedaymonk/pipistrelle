// Supersaw
//
// Pot A => Coarse frequency
// Pot B => Fine frequency
// Pot C + CV 1 => Spread
// Pot D + CV 2 => Sub oscillator

#include <Pipistrelle.h>
#include <math.h>
#include <q14.h>
#include <cv.h>
#define SAMPLE_RATE 32000
#define C0 16.3516
#define OSCILLATORS 7
#define BUFSIZE 100

// Avoid dividing by zero before everything is up
uint32_t phase[OSCILLATORS] = {1, 1, 1, 1, 1, 1, 1};
uint32_t sub_phase = 1;
// Pleasing weighting determined through trial and error
int osc_mix[OSCILLATORS] = {3, 4, 5, 6, 5, 4, 3};
q14_t sub_level = 0;
int osc_divisor = 0;
Pipistrelle *pip;

void setup() {
  // Compute the weighting divisor once
  for (int i = 0; i < OSCILLATORS; i++) osc_divisor += osc_mix[i];

  pip = new Pipistrelle(SAMPLE_RATE);
  pip->led(1); // Shows that we got this far
}

q14_t next_sample() {
  static q14_t t[OSCILLATORS] = {0, 0, 0, 0, 0, 0, 0}, sub_t = 0;
  q14_t sample = 0, sub_sample = 0;

  for (int i = 0; i < OSCILLATORS; i++) {
    sample += q14_saw(t[i]) * osc_mix[i];
    t[i] = (t[i] + phase[i]) % Q14_1;
  }

  sub_sample = q14_square(sub_t);
  sub_t = (sub_t + sub_phase) % Q14_1;

  return q14_blend(sub_level, sample / osc_divisor, sub_sample);
}

void loop() {
  float frequency, voct, detune;

  voct = pip->voct()
    + 6.0F * unipolar(pip->pota()) // coarse tuning C0 + 6 octaves
    + unipolar(pip->potb()) / 2.0F; // fine tuning +/- 1/2 oct

  frequency = C0 * pow(2, voct);
  detune = frequency / 80
         * constrain(unipolar(pip->potc())
                     + bipolar(pip->cv1()) / 2, 0, 1);
  sub_level = Q14_1_4
            * constrain(unipolar(pip->potd())
                        + bipolar(pip->cv2()) / 2, 0, 1);

  for (int i = 0; i < OSCILLATORS; i++) {
    phase[i] = Q14_1
             * (frequency + detune * (i - OSCILLATORS / 2))
             / SAMPLE_RATE;
  }
  sub_phase = Q14_1 * frequency / (2 * SAMPLE_RATE);
}

void TC4_Handler() {
  pip->q14_dac_write(next_sample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
