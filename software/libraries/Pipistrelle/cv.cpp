#include <Pipistrelle.h>
#include <Arduino.h>
#include "cv.h"

// scale an ADC reading to 0 to 1
// Ignore the top and bottom (see END_SLOP) because the end of pot travel isn't
// very reliable
float unipolar(int reading) {
  int clamped = reading - END_SLOP;
  return (float)constrain(clamped, 0, MAX_ADC - 2 * END_SLOP)
    / (MAX_ADC - 2 * END_SLOP);
}

// scale an ADC reading to -1 to 1
float bipolar(int reading) {
  return 2 * unipolar(reading) - 1;
}
