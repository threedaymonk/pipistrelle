#include "pipistrelle.h"
#include "calibration.h"
#include <math.h>
#define SAMPLE_RATE 32000
#define C2 65.4064

double voctA, x, y, z;
double voct = 0;
uint32_t __period = 100;

void setup() {
  uint32_t top;
  initializeHardware(SAMPLE_RATE);

  delay(10);
  analogWrite(LED, 255);

  top = CLOCK_SPEED / 32000;
  REG_TC4_COUNT16_CC0 = top;
}

void loop() {
  static double frequency;
  voct = readVoct() + floor(60.0L * unipolar(readPotA())) / 12.0L;
  frequency = C2 * pow(2, voct);
  __period = SAMPLE_RATE / frequency;

  x = constrain(unipolar(readPotB()) + bipolar(readCv1()) / 2, 0, 1);
  y = constrain(unipolar(readPotC()) + bipolar(readCv2()) / 2, 0, 1);
  z = unipolar(readPotD());



  /*
  Serial.print("voct = ");
  Serial.print(voct);
  Serial.print(" frequency = ");
  Serial.print(frequency);
  Serial.print(" period = ");
  Serial.print(__period);
  Serial.print("\n");
  */

  delay(10);
}

// Waveform functions are expressed as f(t) where 0 <= t < 1

double sineWave(double t) {
  return sin(t * M_PI * 2);
}

double triangleWave(double t) {
  if (t < 0.25) return t * 4;
  if (t < 0.75) return 1 - (t - 0.25) * 4;
  return 1 + (t - 1.25) * 4;
}

double squareWave(double t) {
  if (t < 0.5) return 1;
  return -1;
}

double sawWave(double t) {
  if (t < 0.5) return t * 2;
  return t * 2 - 2;
}

void TC4_Handler() {
  static uint32_t offset = 0, period;
  static double sample = 0;
  static double time, frequency;

  if (offset == 0) {
    period = __period;
  }

  /*
  time = offset * frequency / SAMPLE_RATE;
  sample = 
    (1 - y) * ((1 - x) * sineWave(time)   + x * triangleWave(time))
    + y     * ((1 - x) * squareWave(time) + x * sawWave(time));
  DACWrite(511.5 + 511.5 * sample);
  */
  if (2 * offset > period) DACWrite(1023);
  else DACWrite(0);

  offset += 1;
  if (offset >= period) offset = 0;

  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
