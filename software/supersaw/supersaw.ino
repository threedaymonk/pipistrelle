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
#define SAMPLE_RATE 32000
#define C0 16.3516
#define OSCILLATORS 5

uint32_t period[OSCILLATORS];
q14_t subosc = 0;

void setup() {
  uint32_t top;

  // Anything is fine as long as it's not zero
  for (int i = 0; i < OSCILLATORS; i++) period[i] = 1000;

  initialize_hardware(SAMPLE_RATE);
  digitalWrite(LED, 1); // Shows that we got this far
}

void loop() {
  double frequency, voct, spread;
  int led_change_at = 0;

  voct = read_voct()
    + 6.0L * unipolar(read_pota()) // coarse tuning C0 + 6 octaves
    + unipolar(read_potb()) / 6.0L; // fine tuning +/- 2 semitones

  frequency = C0 * pow(2, voct);
  spread = frequency / 80
         * constrain(unipolar(read_potc()) + bipolar(read_cv1()) / 2, 0, 1);
  subosc = Q14_1
         * constrain(unipolar(read_potd()) + bipolar(read_cv2()) / 2, 0, 1);

  for (int i = 0; i < OSCILLATORS; i++) {
    period[i] = SAMPLE_RATE / (frequency + spread * (i - OSCILLATORS / 2));
  }

  // Oscillator 7 is sub
  //period[7] = 4 * SAMPLE_RATE / frequency;
}

void TC4_Handler() {
  static uint32_t offset[] = {0, 0, 0, 0, 0, 0, 0}, cycle_period[7];
  static q14_t t[7];
  q14_t sample = 0;

  for (int i = 0; i < OSCILLATORS; i++) {
    // Set the period for this cycle.
    // t == 0 is zero crossing point for all waveforms, so if we only ever
    // change frequency here we won't get any audible artefacts from shifting.
    if (offset[i] == 0) cycle_period[i] = period[i];

    // t is the relative offset within the cycle in Q14 representation
    t[i] = (offset[i] * Q14_1) / cycle_period[i];

    //if (i == 7)
    //  sample += (subosc * q14_quarter_square(t[i])) / Q14_1;
    //else
      sample += q14_saw(t[i]);

    offset[i] += 1;
    if (offset[i] >= cycle_period[i]) offset[i] = 0;
  }

  q14_dac_write(sample / OSCILLATORS);

  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
