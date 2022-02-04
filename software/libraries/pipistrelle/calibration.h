#ifndef __calibration_h__
#define __calibration_h__

#include <stdint.h>
#include <Arduino.h>

#define POT_LOW 100
#define POT_HIGH 4000
#define CV_LOW 1500
#define CV_HIGH 2500

// voltage = a + raw value / k
extern double __cal_k, __cal_a;

bool isCalibrationHandshake();
int sampleVoct();
void performCalibration();
void loadCalibration();

#endif /* __calibration_h__ */
