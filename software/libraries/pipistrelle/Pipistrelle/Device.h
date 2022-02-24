#ifndef PIPISTRELLE_DEVICE_H_
#define PIPISTRELLE_DEVICE_H_

#include <Arduino.h>
#include <ResponsiveAnalogRead.h>

namespace Pipistrelle {

const int clockSpeed = 47972352;
const int inputResolution = 12;
const int outputResolution = 12;
const int numInputs = 7;
const int maxInput = (1 << inputResolution) - 1;
const int maxOutput = (1 << outputResolution) - 1;

class Device {
 public:
  enum Pin {
    AudioOut = 0,
    PotA = 2,
    PotB = 3,
    PotC = 9,
    PotD = 10,
    CV1 = 7,
    CV2 = 5,
    Voct = 4,
    LED = 6
  };

  explicit Device(int sample_rate);
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

  // Write a sample between 0 and 1023
  inline void dacWrite(int sample) {
    DAC->DATA.reg = constrain(sample, 0, 1023);
    DAC->CTRLA.bit.ENABLE = 1;
    // We could/should wait for sync here, but we'll hit this again on the next
    // cycle anyway, and we'd rather lose a sample than lock up.
  }

 private:
  void setup(int sample_rate);
  ResponsiveAnalogRead *analog[numInputs];
  float cal_k, cal_a;
};

}  // namespace Pipistrelle

#endif  // PIPISTRELLE_DEVICE_H_
