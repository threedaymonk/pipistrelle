#include "wavetable.h"

q14_t wt_lookup(int x, int y, int z) {
  return wavetable[y][x][z] >> 2;
}

q14_t wt_interpolate(int x, int y, q14_t theta) {
  q14_t a, b, mu;

  a = q14_floor(theta);
  b = q14_ceil(theta);
  mu = theta - a;

  return q14_blend(mu, wt_lookup(x, y, a >> (14 - 8)),
                       wt_lookup(x, y, b >> (14 - 8)));
}
