#ifndef PIPISTRELLE_CALIBRATOR_H_
#define PIPISTRELLE_CALIBRATOR_H_

namespace Pipistrelle {

class Calibrator {
 public:
  Calibrator();
  // voltage = a + raw value / k
  float k, a;

 private:
  bool requested();
  void run();
  void load();
  int sampleVoct();

  const unsigned int kPotLow = 100;
  const unsigned int kPotHigh = 4000;
  const unsigned int kCVLow = 1500;
  const unsigned int kCVHigh = 2500;
};

}  // namespace Pipistrelle

#endif  // PIPISTRELLE_CALIBRATOR_H_
