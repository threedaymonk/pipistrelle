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

q14_t x, y;
uint32_t period = 100;

void setup() {
  uint32_t top;
  initialize_hardware(SAMPLE_RATE);
  digitalWrite(LED, 1);
}

void loop() {
  double frequency, voct;
  int led_change_at = 0;

  voct = read_voct()
    + 6.0L * unipolar(read_pota())
    + unipolar(read_potb()) / 6.0L;

  frequency = C0 * pow(2, voct);
  period = SAMPLE_RATE / frequency;

  x = Q14_1 * constrain(unipolar(read_potc()) + bipolar(read_cv1()) / 2, 0, 1);
  y = Q14_1 * constrain(unipolar(read_potd()) + bipolar(read_cv2()) / 2, 0, 1);
}

void TC4_Handler() {
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

  q14_dac_write(sample);

  offset += 1;
  if (offset >= cycle_period) offset = 0;

  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
