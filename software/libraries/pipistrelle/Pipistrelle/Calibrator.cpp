#include <FlashStorage.h>
#include <Arduino.h>

#include "Pipistrelle/Device.h"
#include "Pipistrelle/Calibrator.h"

namespace Pipistrelle {

FlashStorage(__flash_cal_d1, int);
FlashStorage(__flash_cal_d3, int);

Calibrator::Calibrator() {
  if (requested()) run();
  load();
}

bool Calibrator::requested() {
  // Calibrator mode is entered by moving all pots to minimum and connecting
  // Out to CV2

  // Check all pots are low
  if (   analogRead(Device::Pin::PotA) > kPotLow
      || analogRead(Device::Pin::PotB) > kPotLow
      || analogRead(Device::Pin::PotC) > kPotLow
      || analogRead(Device::Pin::PotD) > kPotLow) return false;

  // Probe CV1 with the DAC output
  // CV inputs are inverted
  analogWrite(Device::Pin::AudioOut, 0);
  delay(20);
  if (analogRead(Device::Pin::CV1) < kCVHigh) return false;

  analogWrite(Device::Pin::AudioOut, 1023);
  delay(20);
  if (analogRead(Device::Pin::CV1) > kCVLow) return false;

  return true;
}

int Calibrator::sampleVoct() {
  const int samples = 10;
  int sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(Device::Pin::Voct);
    delay(20);
  }

  Serial.print("Calibrator value: ");
  Serial.print(sum / samples);
  Serial.print("\n");

  return sum / samples;
}

void Calibrator::run() {
  // Flash LED until pot A is turned clockwise
  while (analogRead(Device::Pin::PotA) < kPotHigh) {
    analogWrite(Device::Pin::LED, 255);
    delay(500);
    analogWrite(Device::Pin::LED, 0);
    delay(500);
  }

  // Store reading for 1V
  __flash_cal_d1.write(sampleVoct());

  // Double-flash LED until pot B is turned clockwise
  while (analogRead(Device::Pin::PotB) < kPotHigh) {
    analogWrite(Device::Pin::LED, 255);
    delay(167);
    analogWrite(Device::Pin::LED, 0);
    delay(167);
    analogWrite(Device::Pin::LED, 255);
    delay(166);
    analogWrite(Device::Pin::LED, 0);
    delay(500);
  }

  // Store reading for 3V
  __flash_cal_d3.write(sampleVoct());
}

void Calibrator::load() {
  int d1 = __flash_cal_d1.read();
  int d3 = __flash_cal_d3.read();

  Serial.write("d1 = ");
  Serial.write(d1);
  Serial.write(", d3 = ");
  Serial.write(d3);
  Serial.write("\n");

  // Sensible-ish defaults based on build No. 1
  // V/oct input is inverted, so 1V is higher than 3V, but this is irrelevant
  // to the calibration algorithm
  if (d1 == 0 && d3 == 0) {
    d1 = 3363;
    d3 = 1978;
  }
  k = (d3 - d1) / 2.0F;
  a = 1.0F - d1 / k;
}

}  // namespace Pipistrelle
