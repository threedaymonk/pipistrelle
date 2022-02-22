#ifndef __Pipistrelle_h__
#define __Pipistrelle_h__

#include <stdint.h>
#include <ResponsiveAnalogRead.h>
#include "q14.h"

#define INPUTS 7
#define AUDIO_OUT 0
#define POTA 2
#define POTB 3
#define POTC 9
#define POTD 10
#define CV1 7
#define CV2 5
#define VOCT 4
#define LED 6
#define HIGH_ACCURACY 32
#define LOW_ACCURACY 1

#define CLOCK_SPEED 47972352
#define MAX_ADC 4095

class Pipistrelle {
public:
  Pipistrelle(int sample_rate);
  void dac_write(int sample);
  void q14_dac_write(q14_t sample);
  int pota();
  int potb();
  int potc();
  int potd();
  int cv1();
  int cv2();
  float voct();
  bool gate1();
  bool gate2();
  bool gate3();
  void led(int state);
  void led(bool state);

private:
  void dac_setup(int sample_rate);
  ResponsiveAnalogRead *analog[INPUTS];
  float cal_k, cal_a;
};

#endif /* __Pipistrelle_h__ */
