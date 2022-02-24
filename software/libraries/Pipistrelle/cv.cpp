#include <Arduino.h>
#include <Device.h>

#include "cv.h"

// scale an ADC reading to 0 to 1
// Ignore the top and bottom (see END_SLOP) because the end of pot travel isn't
// very reliable
float unipolar(int reading) {
  int clamped = reading - END_SLOP;
  return constrain(clamped, 0.0f, Pipistrelle::maxInput - 2.0f * END_SLOP)
    / (Pipistrelle::maxInput - 2.0f * END_SLOP);
}

// scale an ADC reading to -1 to 1
float bipolar(int reading) {
  return 2.0f * unipolar(reading) - 1.0f;
}

// Combine a unipolar pot reading with a bipolar CV input clamped to 0 to 1.
// When the pot is at 12 o'clock, this is equivalent to CV input on its own.
float unipolar_with_cv(int pot, int cv) {
  return constrain(unipolar(pot) + bipolar(cv) / 2.0f, 0.0f, 1.0f);
}
