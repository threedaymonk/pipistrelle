#ifndef __Calibrator_h__
#define __Calibrator_h__

class Calibrator {
public:
  Calibrator();
  float k, a;

private:
  // voltage = a + raw value / k
  bool requested();
  void run();
  void load();
  int sampleVoct();
};

#endif /* __Calibrator_h__ */
