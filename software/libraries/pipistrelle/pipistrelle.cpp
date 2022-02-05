#include "pipistrelle.h"
#include "calibration.h"

void DACSetup(uint32_t sampleRate) {
  uint32_t top = CLOCK_SPEED / sampleRate; // overflow interrupt trigger value

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
}

void DACWrite(uint16_t sample) {
  DAC->DATA.reg = sample;
  while (ADC->STATUS.bit.SYNCBUSY);
  DAC->CTRLA.bit.ENABLE = 1;
  while (ADC->STATUS.bit.SYNCBUSY);
}

// scale an ADC reading to 0 to 1
double unipolar(int reading) {
  return reading / 4096.0L;
}

// scale an ADC reading to -1 to 1
double bipolar(int reading) {
  return reading / 2048.0L - 1;
}

int readPotA() {
  return analogRead(POTA);
}

int readPotB() {
  return analogRead(POTB);
}

int readPotC() {
  return analogRead(POTC);
}

int readPotD() {
  return analogRead(POTD);
}

int readCv1() {
  return MAX_ADC - analogRead(CV1);
}

int readCv2() {
  return MAX_ADC - analogRead(CV2);
}

double readVoct() {
  return __cal_a + analogRead(VOCT) / __cal_k;
}

void initialize_hardware() {
  // Use maximum ADC and DAC resolution available to us
  analogReadResolution(12);
  analogWriteResolution(10);

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

  if (isCalibrationHandshake()) {
    performCalibration();
  }

  loadCalibration();
}
