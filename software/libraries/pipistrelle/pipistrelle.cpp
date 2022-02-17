#include <pipistrelle.h>
#include <calibration.h>
#include <q14.h>

void initialize_hardware(int sample_rate) {
  // Use maximum ADC and DAC resolution available to us
  analogReadResolution(12);

  // Set up inputs and outputs
  // Note: *do not* set AUDIO_OUT to OUTPUT, or the output will be truncated
  // at around 2.2V
  pinMode(POTA, INPUT);
  pinMode(POTB, INPUT);
  pinMode(POTC, INPUT);
  pinMode(POTD, INPUT);
  pinMode(CV1, INPUT);
  pinMode(CV2, INPUT);
  pinMode(VOCT, INPUT);
  pinMode(LED, OUTPUT);

  if (calibration_requested()) {
    run_calibration();
  }

  load_calibration();

  dac_setup(sample_rate);

  REG_TC4_CTRLA |= TC_CTRLA_ENABLE;
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
}

void dac_setup(int sample_rate) {
  uint32_t top = CLOCK_SPEED / (sample_rate); // overflow interrupt trigger value

  REG_GCLK_GENDIV
    = GCLK_GENDIV_DIV(1) // divide by 1 = 48MHz clock
    | GCLK_GENDIV_ID(3); // use generic clock GCLK3
  while (GCLK->STATUS.bit.SYNCBUSY); // sync

  REG_GCLK_GENCTRL
    = GCLK_GENCTRL_IDC // 50% clock duty cycle
    | GCLK_GENCTRL_GENEN // enable clock
    | GCLK_GENCTRL_SRC_DFLL48M // use 48MHz source
    | GCLK_GENCTRL_ID(3); // select GCLK
  while (GCLK->STATUS.bit.SYNCBUSY); // sync

  REG_GCLK_CLKCTRL
    = GCLK_CLKCTRL_CLKEN // enable GCLK to TC4/TC5 link
    | GCLK_CLKCTRL_GEN_GCLK3 // select GCLK3
    | GCLK_CLKCTRL_ID_TC4_TC5; // connect GCLK to TC4/TC5
  while (GCLK->STATUS.bit.SYNCBUSY); // sync

  REG_TC4_COUNT16_CC0 = top; // set overflow interrupt trigger value
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY); // sync

  NVIC_SetPriority(TC4_IRQn, 0); // set TC4 to highest (0) priority
  NVIC_EnableIRQ(TC4_IRQn); // connect TC4 to interrupt controller

  REG_TC4_INTFLAG |= TC_INTFLAG_OVF; // clear interrupt flags
  REG_TC4_INTENSET = TC_INTENCLR_OVF; // enable TC4 interrupt

  REG_TC4_CTRLA
    |= TC_CTRLA_PRESCALER_DIV1 // prescaler 1 => 48MHz
    | TC_CTRLA_WAVEGEN_MFRQ; // match frequency mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY); // sync

  // I'm not sure why, but if we try to set the interrupt frequency
  // immediately, it doesn't take. Waiting 10ms solves that.
  delay(10);
  top = CLOCK_SPEED / sample_rate;
  REG_TC4_COUNT16_CC0 = top;
}

// scale an ADC reading to 0 to 1
// Ignore the top and bottom (see END_SLOP) because the end of pot travel isn't
// very reliable
float unipolar(int reading) {
  int clamped = reading - END_SLOP;
  return (float)constrain(clamped, 0, MAX_ADC - 2 * END_SLOP)
    / (MAX_ADC - 2 * END_SLOP);
}

// scale an ADC reading to -1 to 1
float bipolar(int reading) {
  return 2 * unipolar(reading) - 1;
}

int read_stabilised(int pin, int accuracy, int previous) {
  int latest = 0, delta;
  for (int i = 0; i < accuracy; i++) {
    latest += analogRead(pin);
  }
  latest /= accuracy;
  delta = latest - previous;
  if (delta < -2 || delta > 2) return latest;
  return previous;
}

int read_pota(int accuracy) {
  static int reading = 0, t = 0;
  reading = read_stabilised(POTA, accuracy, reading);
  return reading;
}

int read_potb(int accuracy) {
  static int reading = 0;
  reading = read_stabilised(POTB, accuracy, reading);
  return reading;
}

int read_potc(int accuracy) {
  static int reading = 0;
  reading = read_stabilised(POTC, accuracy, reading);
  return reading;
}

int read_potd(int accuracy) {
  static int reading = 0;
  reading = read_stabilised(POTD, accuracy, reading);
  return reading;
}

int read_cv1(int accuracy) {
  static int reading = 0;
  reading = read_stabilised(CV1, accuracy, reading);
  return reading;
  return MAX_ADC - reading;
}

int read_cv2(int accuracy) {
  static int reading = 0;
  reading = read_stabilised(CV2, accuracy, reading);
  return MAX_ADC - reading;
}

float read_voct(int accuracy) {
  static int reading = 0;
  reading = read_stabilised(VOCT, accuracy, reading);
  return __cal_a + reading / __cal_k;
}

// Convert a signed Q14 value into a value between 0 and 1023.
// We'll occasionally get 1024 and have to clamp it to 1023, but this
// isn't noticeable and is much quicker than any cleverer method.
void q14_dac_write(q14_t sample) {
  dac_write(constrain((sample + Q14_1) >> 5, 0, 1023));
}
