#include <Arduino.h>

#include "Pipistrelle/Device.h"
#include "Pipistrelle/Calibrator.h"

namespace Pipistrelle {

Device::Device(int sample_rate) {
  Calibrator *calibrator;

  // Use maximum ADC resolution available to us
  analogReadResolution(inputResolution);

  int inputs[] = {
    Pin::PotA, Pin::PotB, Pin::PotC, Pin::PotD,
    Pin::CV1, Pin::CV2, Pin::Voct
  };

  // Set up inputs and outputs
  // Note: *do not* set AUDIO_OUT to OUTPUT, or the output will be truncated
  // at around 2.2V
  for (int i = 0; i < 7; i++) {
    pinMode(inputs[i], INPUT);
  }
  pinMode(Pin::LED, OUTPUT);

  // Fetch calibration values, running calibration if requested via handshake
  calibrator = new Calibrator();
  cal_k = calibrator->k;
  cal_a = calibrator->a;

  // Set up denoised analogue inputs
  for (int i = 0; i < 7; i++) {
    analog[i] = new ResponsiveAnalogRead(inputs[i], true, 0.01);
    analog[i]->setAnalogResolution(1 << inputResolution);
    analog[i]->setActivityThreshold(4);  // default is 4 for 10 bits
  }

  // Set up the interrupt that will service the DAC
  setup(sample_rate);
}

void Device::setup(int sample_rate) {
  uint32_t top = clockSpeed / sample_rate;  // interrupt trigger count

  REG_GCLK_GENDIV
    = GCLK_GENDIV_DIV(1)  // divide by 1 = 48MHz clock
    | GCLK_GENDIV_ID(3);  // use generic clock GCLK3
  while (GCLK->STATUS.bit.SYNCBUSY) {}  // sync

  REG_GCLK_GENCTRL
    = GCLK_GENCTRL_IDC  // 50% clock duty cycle
    | GCLK_GENCTRL_GENEN  // enable clock
    | GCLK_GENCTRL_SRC_DFLL48M  // use 48MHz source
    | GCLK_GENCTRL_ID(3);  // select GCLK
  while (GCLK->STATUS.bit.SYNCBUSY) {}  // sync

  REG_GCLK_CLKCTRL
    = GCLK_CLKCTRL_CLKEN  // enable GCLK to TC4/TC5 link
    | GCLK_CLKCTRL_GEN_GCLK3  // select GCLK3
    | GCLK_CLKCTRL_ID_TC4_TC5;  // connect GCLK to TC4/TC5
  while (GCLK->STATUS.bit.SYNCBUSY) {}  // sync

  REG_TC4_COUNT16_CC0 = top;  // set overflow interrupt trigger value
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY) {}  // sync

  NVIC_SetPriority(TC4_IRQn, 0);  // set TC4 to highest (0) priority
  NVIC_EnableIRQ(TC4_IRQn);  // connect TC4 to interrupt controller

  REG_TC4_INTFLAG |= TC_INTFLAG_OVF;  // clear interrupt flags
  REG_TC4_INTENSET = TC_INTENCLR_OVF;  // enable TC4 interrupt

  REG_TC4_CTRLA
    |= TC_CTRLA_PRESCALER_DIV1  // prescaler 1 => 48MHz
    | TC_CTRLA_WAVEGEN_MFRQ;  // match frequency mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY) {}  // sync

  // If we try to set the interrupt frequency immediately, it doesn't take.
  // Waiting 10ms solves that.
  delay(10);
  top = clockSpeed / sample_rate;
  REG_TC4_COUNT16_CC0 = top;

  // Enable the interrupt
  REG_TC4_CTRLA |= TC_CTRLA_ENABLE;
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY) {}
}

int Device::pota() {
  analog[0]->update();
  return analog[0]->getValue();
}

int Device::potb() {
  analog[1]->update();
  return analog[1]->getValue();
}

int Device::potc() {
  analog[2]->update();
  return analog[2]->getValue();
}

int Device::potd() {
  analog[3]->update();
  return analog[3]->getValue();
}

int Device::cv1() {
  analog[4]->update();
  return maxInput - analog[4]->getValue();
}

int Device::cv2() {
  analog[5]->update();
  return maxInput - analog[5]->getValue();
}

float Device::voct() {
  analog[6]->update();
  return cal_a + analog[6]->getValue() / cal_k;
}

bool Device::gate1() {
  return digitalRead(Pin::CV1) == 0;
}

bool Device::gate2() {
  return digitalRead(Pin::CV2) == 0;
}

bool Device::gate3() {
  return digitalRead(Pin::Voct) == 0;
}

void Device::led(bool state) {
  digitalWrite(Pin::LED, state);
}

void Device::led(int state) {
  analogWrite(Pin::LED, constrain(state, 0, 255));
}

}  // namespace Pipistrelle
