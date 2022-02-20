// X-Y oscillator
//
// Pot A => Coarse frequency
// Pot B => Fine frequency
// Pot C + CV 1 => X
// Pot D + CV 2 => Y
//
// X-Y morph between waveforms thus:
//
//  sine  --> triangle
//   |           |
//   v           v
// square -->   saw

#include <Pipistrelle.h>
#include <math.h>
#include <q14.h>
#include <cv.h>
#define SAMPLE_RATE 32000
#define C0 16.3516
#define BUFSIZE 1024

q14_t x, y;
q14_t phase = 1;
Pipistrelle *pip;

void setup() {
  pip = new Pipistrelle(SAMPLE_RATE);
  pip->led(1); // Shows that we got this far
}

q14_t next_sample() {
  static q14_t t = 0, sample;

  // Blend x-y between the four waveforms
  sample = q14_blend(y, q14_blend(x, q14_sine(t),   q14_triangle(t)),
                        q14_blend(x, q14_square(t), q14_saw(t)));

  t = (t + phase) % Q14_1;

  return sample;
}

void loop() {
  float frequency, voct;

  voct = pip->voct()
    + 6.0F * unipolar(pip->pota()) // coarse tuning C0 + 6 octaves
    + unipolar(pip->potb()) / 2.0F; // fine tuning +/- 1/2 oct

  frequency = C0 * pow(2, voct);
  phase = Q14_1 * frequency / SAMPLE_RATE;

  x = Q14_1 * constrain(unipolar(pip->potc())
                        + bipolar(pip->cv1()) / 2, 0, 1);
  y = Q14_1 * constrain(unipolar(pip->potd())
                        + bipolar(pip->cv2()) / 2, 0, 1);
}

void TC4_Handler() {
  pip->q14_dac_write(next_sample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
