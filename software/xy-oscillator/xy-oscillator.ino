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

#include <pipistrelle.h>
#include <calibration.h>
#include <math.h>
#include <q14.h>
#define SAMPLE_RATE 48000
#define C0 16.3516
#define BUFSIZE 1024

q14_t x, y;
uint32_t period = 100;
q14_t buffer[BUFSIZE];
int rptr = 0, wptr = 1;

void setup() {
  uint32_t top;
  initialize_hardware(SAMPLE_RATE);
  digitalWrite(LED, 1); // Shows that we got this far
}

q14_t next_sample() {
  static uint32_t offset = 0, cycle_period;
  static q14_t t, sample;

  // Set the period for this cycle.
  // t == 0 is zero crossing point for all waveforms, so if we only ever
  // change frequency here we won't get any audible artefacts from shifting.
  if (offset == 0) cycle_period = period;

  // t is the relative offset within the cycle in Q14 representation
  t = (offset * Q14_1) / cycle_period;

  // Blend x-y between the four waveforms
  sample = q14_blend(y, q14_blend(x, q14_sine(t),   q14_triangle(t)),
                        q14_blend(x, q14_square(t), q14_saw(t)));

  offset += 1;
  if (offset >= cycle_period) offset = 0;

  return sample;
}

void fill_buffer() {
  while ((BUFSIZE + wptr - rptr) % BUFSIZE) {
    buffer[wptr] = next_sample();
    wptr = (wptr + 1) % BUFSIZE;
  }
}

void loop() {
  float frequency, voct;
  int led_change_at = 0;

  voct = read_voct(HIGH_ACCURACY)
    + 6.0L * unipolar(read_pota(HIGH_ACCURACY)) // coarse tuning C0 + 6 octaves
    + unipolar(read_potb(HIGH_ACCURACY)) / 6.0L; // fine tuning +/- 2 semitones

  frequency = C0 * pow(2, voct);
  period = SAMPLE_RATE / frequency;

  x = Q14_1 * constrain(unipolar(read_potc(LOW_ACCURACY))
                        + bipolar(read_cv1(LOW_ACCURACY)) / 2, 0, 1);
  y = Q14_1 * constrain(unipolar(read_potd(LOW_ACCURACY))
                        + bipolar(read_cv2(LOW_ACCURACY)) / 2, 0, 1);

  fill_buffer();
}

void TC4_Handler() {
  q14_dac_write(buffer[rptr]);
  rptr = (rptr + 1) % BUFSIZE;
  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
