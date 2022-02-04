#include "pipistrelle.h"
#include "calibration.h"
#include <math.h>
#define SAMPLE_RATE 48000

void setup() {
  initialize_hardware();

  // DACSetup(SAMPLE_RATE);

  analogWrite(LED, 255);
}

void loop() {
  static int potA, potB, potC, potD, cv1, cv2;
  static double voct;

  potA = readPotA();
  delay(20);
  potB = readPotB();
  delay(20);
  potC = readPotC();
  delay(20);
  potD = readPotD();
  delay(20);
  cv1 = readCv1();
  delay(20);
  cv2 = readCv2();
  delay(20);
  voct = readVoct();

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

  delay(280);
  delay(100);
}

/*
void TC4_Handler() {
  static uint32_t offset = 0;
  static uint16_t sample = 0;

  static double adecay = 1.7
  static double frequency = 10
  static double fdecay = 1.1

  double x;

  sample = (adecay ** (-x * 2 * M_PI)) * \
           sin(x * frequency * (fdecay ** (-x * 2 * pi)) * 2 * M_PI) 



  x = (double)(offset) / (double)4;
  sample = (int)(511.5 + 511.5 * sin(x));
  //DACWrite(sample );
  analogWrite(AUDIO_OUT, sample);

  /*
  x = (offset * 32) % 1024;
  if (x < 512) sample = 2 * x;
  else sample = 2047 - 2 * x;
  DACWrite(sample * 0.7);

  offset += 1;

  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
*/
