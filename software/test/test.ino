#include "pipistrelle.h"
#include "calibration.h"
#include <math.h>
#define SAMPLE_RATE 32000

double potA, potB, potC, potD, cv1, cv2;
double voct = 0;

void setup() {
  initialize_hardware();

  DACSetup(SAMPLE_RATE);

  analogWrite(LED, 255);
}

void loop() {
  voct = readVoct();
  potA = readPotA();
  potB = readPotB();
  potC = readPotC();
  potD = readPotD();
  cv1 = readCv1();
  cv2 = readCv2();

  /*
  Serial.print("__cal_a = ");
  Serial.print(__cal_a);
  Serial.print(", __cal_k = ");
  Serial.print(__cal_k);
  Serial.print("\n\n");

  Serial.print("A = ");
  Serial.print(potA);
  Serial.print(", B = ");
  Serial.print(potB);
  Serial.print(", C = ");
  Serial.print(potC);
  Serial.print(", D = ");
  Serial.print(potD);
  Serial.print("\nCV1 = ");
  Serial.print(cv1);
  Serial.print("\nCV2 = ");
  Serial.print(cv2);
  Serial.print("\nV/oct = ");
  Serial.print(voct);
  Serial.print("\n");

  Serial.print("\nOut = ");
  Serial.print(potA / 4);
  Serial.print("\n");
  //analogWrite(A0, potA / 4);
  DACWrite(potA / 4);
  */

  delay(10);
}

double sineWave(double pos) {
  return sin(pos * M_PI * 2);
}

double triangleWave(double pos) {
  if (pos < 0.25) return pos * 4;
  if (pos < 0.75) return 1 - (pos - 0.25) * 4;
  return 1 + (pos - 0.75) * 4;
}

double squareWave(double pos) {
  if (pos < 0.5) return 1;
  return -1;
}

double sawWave(double pos) {
  if (pos < 0.5) return pos * 2;
  return pos * 2 - 2;
}

void TC4_Handler() {
  static uint32_t offset = 0;
  static double sample = 0;
  static double time, frequency, period, x, y, z;

  if (offset == 0) {
    frequency = 200 * pow(2, voct + 5 * unipolar(potA));
    period = SAMPLE_RATE / frequency;
    x = unipolar(potB);
    y = unipolar(potC);
    z = unipolar(potD);
  }

  time = (double)offset / SAMPLE_RATE;
  sample = 
    (1 - z) * (
      (1 - x) * sineWave(time * frequency)
      + x * triangleWave(time * frequency)
    ) + z * (
      (1 - y) * squareWave(time * frequency)
      + y * sawWave(time * frequency)
    );
  DACWrite(511.5 + 511.5 * sample);

  offset += 1;
  if (offset > period) offset = 0;

  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
