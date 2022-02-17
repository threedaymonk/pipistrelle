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
#define BUFSIZE 1024

uint32_t period[OSCILLATORS];
q14_t oscmix[OSCILLATORS];
q14_t buffer[BUFSIZE];
int rptr = 0, wptr = 1;

void setup() {
  uint32_t top;

  // Anything is fine as long as it's not zero
  for (int i = 0; i < OSCILLATORS; i++) period[i] = 1000;

  // Clear buffer
  for (int i = 0; i < BUFSIZE; i++) buffer[i] = 0;

  initialize_hardware(SAMPLE_RATE);
  digitalWrite(LED, 1); // Shows that we got this far
}

q14_t next_sample() {
  static uint32_t offset[] = {0, 0, 0, 0, 0, 0, 0}, cycle_period[7];
  static q14_t t[7];
  q14_t sample = 0, divisor = 0;

  for (int i = 0; i < OSCILLATORS; i++) {
    // Set the period for this cycle.
    // t == 0 is zero crossing point for all waveforms, so if we only ever
    // change frequency here we won't get any audible artefacts from shifting.
    if (offset[i] == 0) cycle_period[i] = period[i];

    // t is the relative offset within the cycle in Q14 representation
    t[i] = (offset[i] * Q14_1) / cycle_period[i];

    sample += (q14_saw(t[i]) * oscmix[i]) / Q14_1;
    divisor += oscmix[i];

    offset[i] += 1;
    if (offset[i] >= cycle_period[i]) offset[i] = 0;
  }

  return sample / (divisor / Q14_1);
}

void fill_buffer() {
  while ((BUFSIZE + wptr - rptr) % BUFSIZE) {
    buffer[wptr] = next_sample();
    wptr = (wptr + 1) % BUFSIZE;
  }
}

void loop() {
  float frequency, voct, detune, sidemix;
  int led_change_at = 0;

  voct = read_voct(HIGH_ACCURACY)
    + 6.0L * unipolar(read_pota(HIGH_ACCURACY)) // coarse tuning C0 + 6 octaves
    + unipolar(read_potb(HIGH_ACCURACY)) / 2.0L; // fine tuning +/- 1/2 oct

  frequency = C0 * pow(2, voct);
  detune = frequency / 80
         * constrain(unipolar(read_potc(LOW_ACCURACY))
                     + bipolar(read_cv1(LOW_ACCURACY)) / 2, 0, 1);
  sidemix = constrain(unipolar(read_potd(LOW_ACCURACY))
                      + bipolar(read_cv2(LOW_ACCURACY)) / 2, 0, 1);

  for (int i = 0, j; i < OSCILLATORS; i++) {
    period[i] = SAMPLE_RATE / (frequency + detune * (i - OSCILLATORS / 2));
    oscmix[i] = Q14_1 - Q14_1 * pow((1 - sidemix) * 0.75, abs(OSCILLATORS / 2 - i));
  }

  fill_buffer();
}

void TC4_Handler() {
  q14_dac_write(buffer[rptr]);
  rptr = (rptr + 1) % BUFSIZE;
  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
