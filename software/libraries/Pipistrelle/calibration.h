#ifndef __calibration_h__
#define __calibration_h__

// voltage = a + raw value / k
extern float __cal_k, __cal_a;

bool calibration_requested();
void run_calibration();
void load_calibration();

#endif /* __calibration_h__ */
