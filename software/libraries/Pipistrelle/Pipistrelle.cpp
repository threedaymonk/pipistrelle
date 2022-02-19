#include <Arduino.h>
#include "Pipistrelle.h"
#include "calibration.h"
#include "q14.h"

#define ADC_RESOLUTION 12

Pipistrelle::Pipistrelle(int sample_rate) {
  const int inputs[INPUTS] = {POTA, POTB, POTC, POTD, CV1, CV2, VOCT};

  // Use maximum ADC and DAC resolution available to us
  analogReadResolution(ADC_RESOLUTION);

  // Set up inputs and outputs
  // Note: *do not* set AUDIO_OUT to OUTPUT, or the output will be truncated
  // at around 2.2V
  for (int i = 0; i < INPUTS; i++) {
    pinMode(inputs[i], INPUT);
  }
  pinMode(LED, OUTPUT);

  if (calibration_requested()) {
    run_calibration();
  }

  load_calibration();

  for (int i = 0; i < INPUTS; i++) {
    analog[i] = new ResponsiveAnalogRead(inputs[i], true, 0.01);
    analog[i]->setAnalogResolution(1 << ADC_RESOLUTION);
    analog[i]->setActivityThreshold(4); // default is 4 for 10 bits
  }

  dac_setup(sample_rate);

  REG_TC4_CTRLA |= TC_CTRLA_ENABLE;
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
}

void Pipistrelle::dac_setup(int sample_rate) {
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

void Pipistrelle::dac_write(int sample) {
  DAC->DATA.reg = (sample);
  DAC->CTRLA.bit.ENABLE = 1;
}

// Convert a signed Q14 value into a value between 0 and 1023.
// We'll occasionally get 1024 and have to clamp it to 1023, but this
// isn't noticeable and is much quicker than any cleverer method.
void Pipistrelle::q14_dac_write(q14_t sample) {
  DAC->DATA.reg = constrain((sample + Q14_1) >> 5, 0, 1023);
  DAC->CTRLA.bit.ENABLE = 1;
}

int Pipistrelle::pota() {
  analog[0]->update();
  return analog[0]->getValue();
}

int Pipistrelle::potb() {
  analog[1]->update();
  return analog[1]->getValue();
}

int Pipistrelle::potc() {
  analog[2]->update();
  return analog[2]->getValue();
}

int Pipistrelle::potd() {
  analog[3]->update();
  return analog[3]->getValue();
}

int Pipistrelle::cv1() {
  analog[4]->update();
  return MAX_ADC - analog[4]->getValue();
}

int Pipistrelle::cv2() {
  analog[5]->update();
  return MAX_ADC - analog[5]->getValue();
}

float Pipistrelle::voct() {
  analog[6]->update();
  return __cal_a + analog[6]->getValue() / __cal_k;
}

void Pipistrelle::led(int state) {
  digitalWrite(LED, state);
}
