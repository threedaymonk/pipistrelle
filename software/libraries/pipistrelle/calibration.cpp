#include <pipistrelle.h>
#include <calibration.h>
#include <FlashStorage.h>

FlashStorage(__flash_cal_d1, int);
FlashStorage(__flash_cal_d3, int);
double __cal_k, __cal_a;

bool calibration_requested() {
  // Calibration mode is entered by moving all pots to minimum and connecting
  // Out to CV2

  // Check all pots are low
  if (   read_pota() > POT_LOW
      || read_potb() > POT_LOW
      || read_potc() > POT_LOW
      || read_potd() > POT_LOW) return false;

  // Probe CV1 with the DAC output
  dac_write(0);
  delay(20);
  if (read_cv1() > CV_LOW) return false;
  dac_write(1023);
  delay(20);
  if (read_cv1() < CV_HIGH) return false;

  return true;
}

int sample_voct() {
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

void run_calibration() {
  // Flash LED until pot A is turned clockwise
  while(read_pota() < POT_HIGH) {
    analogWrite(LED, 255);
    delay(500);
    analogWrite(LED, 0);
    delay(500);
  }

  // Store reading for 1V
  __flash_cal_d1.write(sample_voct());

  // Double-flash LED until pot B is turned clockwise
  while(read_potb() < POT_HIGH) {
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
  __flash_cal_d3.write(sample_voct());
}

void load_calibration() {
  int d1, d3;
  d1 = __flash_cal_d1.read();
  d3 = __flash_cal_d3.read();

  Serial.write("d1 = ");
  Serial.write(d1);
  Serial.write(", d3 = ");
  Serial.write(d3);
  Serial.write("\n");

  // Sensible-ish defaults based on build No. 1
  if (d1 == 0 && d3 == 0) {
    d1 = 3363;
    d3 = 1978;
  }
  __cal_k = (d3 - d1) / 2.0L;
  __cal_a = 1.0L - d1 / __cal_k;
}
