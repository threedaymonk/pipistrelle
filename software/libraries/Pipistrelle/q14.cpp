#include <Arduino.h>

#include "q14.h"

#define FRAC_MASK 0x3fff

// Waveform functions take an input value of 0 <= x < Q14_1
// and return a value in the range -Q14_1 <= 0 <= Q14_1

// Quadratic sine wave approximation
// See http://www.coranac.com/2009/07/sines/
q14_t q14_sine(q14_t x) {
  int c, y;
  static const int qN = 13, qA = 12, B = 19900, C = 3516;

  x <<= 1;  // convert to Q15

  c = x << (30 - qN);
  x -= 1 << qN;

  x = x << (31 - qN);
  x = x >> (31 - qN);
  x = x * x >> (2 * qN - 14);

  y = B - (x * C >> 14);
  y = (1 << qA) - (x * y >> 16);

  y <<= 2;  // convert from Q12 to Q14

  return c >= 0 ? y : -y;
}

q14_t q14_triangle(q14_t x) {
  if (x < Q14_1_4) return x * 4;
  if (x < Q14_3_4) return Q14_1 - (x - Q14_1_4) * 4;
  return Q14_1 + (x - Q14_1 - Q14_1_4) * 4;
}

q14_t q14_square(q14_t x) {
  if (x < Q14_1_2) return Q14_1;
  return -Q14_1;
}

q14_t q14_quarter_square(q14_t x) {
  if (x < Q14_1_4) return Q14_1;
  return -Q14_1;
}

q14_t q14_saw(q14_t x) {
  return Q14_1 - 2 * x;
}

q14_t q14_floor(q14_t q) {
  (q >> 14) << 14;
}

q14_t q14_ceil(q14_t q) {
  if (q & FRAC_MASK) return q14_floor(q + Q14_1);
  return q;
}

q14_t ftoq14(float f) {
  return Q14_1 * f;
}

float q14tof(q14_t q) {
  return static_cast<float>(q) / Q14_1;
}

q14_t itoq14(int i) {
  return i << 14;
}

int q14toi(q14_t q) {
  return q >> 14;
}
