// X-Y oscillator
//
// Pot A => Coarse tune (C0 + 6 octaves)
// Pot B => Fine tune (+/- 1/2 octave)
// Pot C + CV 1 => X
// Pot D + CV 2 => Y
//
// X-Y morph between waveforms thus:
//
//  sine  --> triangle
//   |           |
//   v           v
// square -->   saw
//
// theta: position within the wave cycle (0 to Q14_1)
// phi:   phase displacement per sample

#include <Pipistrelle.h>
#include <math.h>
#include <q14.h>
#include <cv.h>
#define SAMPLE_RATE 32000
#define C0 16.3516
#define BUFSIZE 1024

q14_t x, y, phi = 1;
Pipistrelle *pip;

void setup() {
  pip = new Pipistrelle(SAMPLE_RATE);
  pip->led(true); // Shows that we got this far
}

q14_t next_sample() {
  static q14_t theta = 0, sample;

  // Blend x-y between the four waveforms
  sample = q14_blend(y, q14_blend(x, q14_sine(theta),   q14_triangle(theta)),
                        q14_blend(x, q14_square(theta), q14_saw(theta)));

  theta = (theta + phi) % Q14_1;

  return sample;
}

void loop() {
  float frequency, voct;

  voct = pip->voct()
       + 6.0F * unipolar(pip->pota()) // coarse tuning
       + unipolar(pip->potb()) / 2.0F; // fine tuning

  frequency = C0 * pow(2, voct);
  phi = ftoq14(frequency / SAMPLE_RATE);

  x = ftoq14(constrain(unipolar(pip->potc())
                       + bipolar(pip->cv1()) / 2, 0, 1));
  y = ftoq14(constrain(unipolar(pip->potd())
                       + bipolar(pip->cv2()) / 2, 0, 1));
}

void TC4_Handler() {
  pip->q14_dac_write(next_sample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
