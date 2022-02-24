// Wavetable oscillator
//
// Pot A => Coarse tune (C0 + 6 octaves)
// Pot B => Fine tune (+/- 1/2 octave)
// Pot C + CV 1 => X
// Pot D + CV 2 => Y
//
// X-Y position in 8x8 wavetable

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

#include <Pipistrelle/Device.h>
#include <Pipistrelle/Waveform.h>
#include <Pipistrelle/Modulation.h>

using fixed = fpm::fixed_16_16;
using namespace Pipistrelle::Waveform;
using namespace Pipistrelle::Modulation;

const int sampleRate = 32000;
const float C0 = 16.3516f;

const int wtRows = 8, wtColumns = 8, wtSamples = 256;
extern const int16_t wavetable[wtRows][wtColumns][wtSamples];

fixed muX, muY, phi;
int aX, bX, aY, bY;

Pipistrelle::Device *pip;

void setup() {
  pip = new Pipistrelle::Device(sampleRate);
  pip->led(true);  // Shows that we got this far
}

// Look up the sample at integer offset z into the wave at (x, y)
fixed wtLookup(int x, int y, int z) {
  // Samples are 16-bit signed values, so 15 bits of information.
  // Divide by 1 << 15 to get a -1 to +1 value.
  return static_cast<fixed>(wavetable[y][x][z]) / (1 << 15);
}

// Return the interpolated value from offset theta within the wave at (x, y).
// Each wave is wtSamples long.
fixed wtInterpolate(int x, int y, fixed theta) {
  fixed scaledTheta = theta * wtSamples;

  fixed a = fpm::floor(theta * wtSamples);
  fixed b = fpm::ceil(theta * wtSamples);
  fixed mu = scaledTheta - a;

  return blend(mu, wtLookup(x, y, static_cast<int>(a)),
                   wtLookup(x, y, static_cast<int>(b)));
}

fixed nextSample() {
  static fixed theta { 0 };

  // Blend between the four nearest waves
  fixed sample = blend(muY, blend(muX, wtInterpolate(aX, aY, theta),
                                       wtInterpolate(bX, aY, theta)),
                            blend(muX, wtInterpolate(aX, bY, theta),
                                       wtInterpolate(bX, bY, theta)));

  theta = fpm::fmod(theta + phi, static_cast<fixed>(1));

  return sample;
}

void loop() {
  float voct = pip->voct()
             + 6.0F * unipolar(pip->pota()) // coarse tuning
             + unipolar(pip->potb()) / 2.0F; // fine tuning

  float frequency = C0 * pow(2, voct);
  phi = static_cast<fixed>(frequency / sampleRate);

  // We start off with a pair of values in the range 0-1, and multiply
  // to get an x and y offset into our 2D wavespace
  float x = (wtRows - 1)    * unipolarPotWithCV(pip->potc(), pip->cv1());
  float y = (wtColumns - 1) * unipolarPotWithCV(pip->potd(), pip->cv2());

  // However, the wavespace is discrete, so the continous offset must be
  // converted into the indices aX and bX of the waves around that point and a
  // blend amount muX between them.
  aX = floor(x);
  bX = ceil(x);
  muX = static_cast<fixed>(x - aX);

  // Repeat for y
  aY = floor(y);
  bY = ceil(y);
  muY = static_cast<fixed>(y - aY);
}

void TC4_Handler() {
  pip->dacWrite(nextSample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF;  // clear interrupt overflow flag
}
