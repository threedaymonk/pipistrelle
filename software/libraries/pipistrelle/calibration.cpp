#include "pipistrelle.h"
#include "calibration.h"
#include <FlashStorage.h>

FlashStorage(__flash_cal_d1, int);
FlashStorage(__flash_cal_d3, int);
double __cal_k, __cal_a;

bool isCalibrationHandshake() {
  // Calibration mode is entered by moving all pots to minimum and connecting
  // Out to CV2

  // Check all pots are low
  if (   readPotA() > POT_LOW
      || readPotB() > POT_LOW
      || readPotC() > POT_LOW
      || readPotD() > POT_LOW) return false;

  // Probe CV1 with the DAC output
  DACWrite(0);
  delay(20);
  if (readCv1() > CV_LOW) return false;
  DACWrite(1023);
  delay(20);
  if (readCv1() < CV_HIGH) return false;

  return true;
}

int sampleVoct() {
  int sum = 0; 
  for (int i = 0; i < 10; i++) {
    sum += analogRead(VOCT);
    delay(20);
  }
  Serial.print("Calibration value: ");
  Serial.print(sum / 10);
  Serial.print("\n");
  return sum / 10;
}

void performCalibration() {
  // Flash LED until pot A is turned clockwise
  while(readPotA() < POT_HIGH) {
    analogWrite(LED, 255);
    delay(500);
    analogWrite(LED, 0);
    delay(500);
  }

  // Store reading for 1V
  __flash_cal_d1.write(sampleVoct());

  // Double-flash LED until pot B is turned clockwise
  while(readPotB() < POT_HIGH) {
    analogWrite(LED, 255);
    delay(167);
    analogWrite(LED, 0);
    delay(167);
    analogWrite(LED, 255);
    delay(166);
    analogWrite(LED, 0);
    delay(500);
  }

  // Store reading for 3V
  __flash_cal_d3.write(sampleVoct());
}

void loadCalibration() {
  int d1, d3;
  // TODO: Remove hard-coded values
  d1 = 3363; // __flash_cal_d1.read();
  d3 = 1978; // __flash_cal_d3.read();
  __cal_k = (d3 - d1) / 2.0L;
  __cal_a = 1.0L - d1 / __cal_k;
}
