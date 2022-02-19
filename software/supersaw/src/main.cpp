// Supersaw
//
// Pot A => Coarse frequency
// Pot B => Fine frequency
// Pot C + CV 1 => Spread
// Pot D + CV 2 => Sub oscillator

#include <pipistrelle.h>
#include <calibration.h>
#include <math.h>
#include <q14.h>
#define SAMPLE_RATE 11025
#define C0 16.3516
#define OSCILLATORS 7
#define BUFSIZE 100

// Avoid dividing by zero before everything is up
uint32_t period[OSCILLATORS] = {1, 1, 1, 1, 1, 1, 1};
uint32_t sub_period = 1;
// Pleasing weighting determined through trial and error
int osc_mix[OSCILLATORS] = {3, 4, 5, 6, 5, 4, 3};
q14_t sub_level = 0;
int osc_divisor = 0;

void setup() {
  // Compute the weighting divisor once
  for (int i = 0; i < OSCILLATORS; i++) osc_divisor += osc_mix[i];

  initialize_hardware(SAMPLE_RATE);
  digitalWrite(LED, 1); // Shows that we got this far
}

q14_t next_sample() {
  static uint32_t offset[OSCILLATORS] = {0, 0, 0, 0, 0, 0, 0},
                  sub_offset = 0,
                  cycle_period[OSCILLATORS],
                  sub_cycle_period;
  static q14_t t[OSCILLATORS], sub_t;
  q14_t sample = 0, sub_sample = 0;

  for (int i = 0; i < OSCILLATORS; i++) {
    // Set the period for this cycle.
    // t == 0 is zero crossing point for all waveforms, so if we only ever
    // change frequency here we won't get any audible artefacts from shifting.
    if (offset[i] == 0) cycle_period[i] = period[i];

    // t is the relative offset within the cycle in Q14 representation
    t[i] = (offset[i] * Q14_1) / cycle_period[i];

    sample += q14_saw(t[i]) * osc_mix[i];

    offset[i] += 1;
    if (offset[i] >= cycle_period[i]) offset[i] = 0;
  }

  if (sub_offset == 0) sub_cycle_period = sub_period;
  sub_t = (sub_offset * Q14_1) / sub_cycle_period;
  sub_sample = q14_square(sub_t);
  sub_offset++;
  if (sub_offset >= sub_cycle_period) sub_offset = 0;

  return q14_blend(sub_level, sample / osc_divisor, sub_sample);
}

void loop() {
  float frequency, voct, detune, subosc;

  voct = read_voct(HIGH_ACCURACY)
    + 6.0L * unipolar(read_pota(HIGH_ACCURACY)) // coarse tuning C0 + 6 octaves
    + unipolar(read_potb(HIGH_ACCURACY)) / 2.0L; // fine tuning +/- 1/2 oct

  frequency = C0 * pow(2, voct);
  detune = frequency / 80
         * constrain(unipolar(read_potc(LOW_ACCURACY))
                     + bipolar(read_cv1(LOW_ACCURACY)) / 2, 0, 1);
  sub_level = Q14_1_4
            * constrain(unipolar(read_potd(LOW_ACCURACY))
                        + bipolar(read_cv2(LOW_ACCURACY)) / 2, 0, 1);

  for (int i = 0; i < OSCILLATORS; i++) {
    period[i] = SAMPLE_RATE / (frequency + detune * (i - OSCILLATORS / 2));
  }
  sub_period = 2 * SAMPLE_RATE / frequency;
}

void TC4_Handler() {
  q14_dac_write(next_sample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
