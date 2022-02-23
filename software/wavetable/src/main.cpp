// Wavetable oscillator
//
// Pot A => Coarse tune (C0 + 6 octaves)
// Pot B => Fine tune (+/- 1/2 octave)
// Pot C + CV 1 => X
// Pot D + CV 2 => Y
//
// X-Y position in 8x8 wavetable
//
// theta: position within the wave cycle (0 to Q14_1)
// phi:   phase displacement per sample

#include <Pipistrelle.h>
#include <math.h>
#include <q14.h>
#include <cv.h>
#include "wavetable.h"
#define SAMPLE_RATE 32000
#define C0 16.3516

int xa, xb, ya, yb;
q14_t dx, dy, phi = 1;
Pipistrelle *pip;

void setup() {
  pip = new Pipistrelle(SAMPLE_RATE);
  pip->led(true); // Shows that we got this far
}

q14_t next_sample() {
  static q14_t theta = 0, sample;

  // Blend x-y between the four waveforms
  sample = q14_blend(dy, q14_blend(dx, wt_interpolate(xa, ya, theta),
                                       wt_interpolate(xb, ya, theta)),
                         q14_blend(dx, wt_interpolate(xa, yb, theta),
                                       wt_interpolate(xb, yb, theta)));

  theta = (theta + phi) % Q14_1;

  return sample;
}

void loop() {
  float frequency, voct, x, y;

  voct = pip->voct()
       + 6.0F * unipolar(pip->pota()) // coarse tuning
       + unipolar(pip->potb()) / 2.0F; // fine tuning

  frequency = C0 * pow(2, voct);
  phi = ftoq14(frequency / SAMPLE_RATE);

  x = (WT_ROWS - 1)    * unipolar_with_cv(pip->potc(), pip->cv1());
  y = (WT_COLUMNS - 1) * unipolar_with_cv(pip->potd(), pip->cv2());

  xa = floor(x);
  xb = ceil(x);
  dx = ftoq14(x - xa);

  ya = floor(y);
  yb = ceil(y);
  dy = ftoq14(y - ya);
}

void TC4_Handler() {
  pip->q14DacWrite(next_sample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
