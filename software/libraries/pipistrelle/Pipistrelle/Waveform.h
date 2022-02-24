#ifndef PIPISTRELLE_WAVEFORM_H_
#define PIPISTRELLE_WAVEFORM_H_

#undef round
#undef abs
#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

namespace Pipistrelle::Waveform {

using fixed = fpm::fixed_16_16;

const fixed F1   { 1 },
            F1_4 { F1 / 4 },
            F1_2 { F1 / 2 },
            F3_4 { F1_2 + F1_4 };

inline fixed sine(fixed x) {
  return fpm::sin(x * fixed::two_pi());
}

inline fixed triangle(fixed x) {
  if (x < F1_4) return x * 4;
  if (x < F3_4) return F1 - (x - F1_4) * 4;
  return F1 + (x - F1 - F1_4) * 4;
}

inline fixed square(fixed x, fixed width = F1_2) {
  return (x < width) ? F1 : -F1;
}

inline fixed saw(fixed x) {
  return F1 - x * 2;
}

}  // namespace Pipistrelle::Waveform

#endif  // PIPISTRELLE_WAVEFORM_H_
