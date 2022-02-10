#include "pipistrelle.h"
#include "calibration.h"
#include <math.h>
#define SAMPLE_RATE 32000
#define C2 65.4064

typedef int32_t q14_t;
#define Q14_1    0x4000
#define Q14_1_4  0x1000
#define Q14_1_2  0x2000
#define Q14_3_4  0x3000

#define blend(amt, a, b) (((Q14_1 - (amt)) * (a) + (amt) * (b)) / Q14_1)

q14_t x, y, z;
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
  double frequency, voct;

  // Quantise v/oct to nearest semitone.
  voct = readVoct() + 5.0L * unipolar(readPotA());
  frequency = C2 * pow(2, voct);
  __period = SAMPLE_RATE / frequency;

  x = Q14_1 * constrain(unipolar(readPotB()) + bipolar(readCv1()) / 2, 0, 1);
  y = Q14_1 * constrain(unipolar(readPotC()) + bipolar(readCv2()) / 2, 0, 1);
  z = Q14_1 * unipolar(readPotD());



  /*
  Serial.print("voct = ");
  Serial.print(voct);
  Serial.print(" frequency = ");
  Serial.print(frequency);
  Serial.print(" period = ");
  Serial.print(__period);
  Serial.print("\n");
  */

}

// Waveform functions take an input value of 0 <= x < Q14_1
// and return a value in the range -Q14_1 <= 0 <= Q14_1

// Quadratic sine wave approximation at f = 1/x
// See http://www.coranac.com/2009/07/sines/
q14_t sineWave(q14_t x) {
  int c, x2, y;
  static const int qN= 13, qA= 12, B=19900, C=3516;

  x <<= 1; // convert to Q15

  c = x << (30 - qN);
  x -= 1 << qN;

  x = x << (31 - qN);
  x = x >> (31 - qN);
  x = x * x >> (2 * qN - 14);

  y = B - (x * C >> 14);
  y = (1 << qA) - (x * y >> 16);

  y <<= 2; // convert from Q12 to Q14

  return c >= 0 ? y : -y;
}

// Triangle wave at f = 1/x
q14_t triangleWave(q14_t x) {
  if (x < Q14_1_4) return x * 4;
  if (x < Q14_3_4) return Q14_1 - (x - Q14_1_4) * 4;
  return Q14_1 + (x - Q14_1 - Q14_1_4) * 4;
}

// Square wave at f = 1/x
q14_t squareWave(q14_t x) {
  if (x < Q14_1_2) return Q14_1;
  return -Q14_1;
}

// Saw wave at f = 1/x
q14_t sawWave(q14_t x) {
  if (x < Q14_1_2) return x * 2;
  return x * 2 - 2 * Q14_1;
}

void TC4_Handler() {
  static uint32_t offset = 0, period;
  static q14_t t, sample;

  if (offset == 0) period = __period;

  t = (offset * Q14_1) / period;

  sample = blend(y, blend(x, sineWave(t), triangleWave(t)),
                    blend(x, squareWave(t), sawWave(t)));

  DACWrite(constrain((sample + Q14_1) >> 5, 0, 1023));

  offset += 1;
  if (offset >= period) offset = 0;

  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
