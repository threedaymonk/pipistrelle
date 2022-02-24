// X-Y oscillator
//
// Pot A => Coarse tune (C0 + 6 octaves)
// Pot B => Fine tune (+/- 1/2 octave)
// Pot C + CV 1 => X
// Pot D + CV 2 => Y
//
// X-Y morph between waveforms thus:
//
//  sine  --> triangle
//   |           |
//   v           v
// square -->   saw

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
fixed x, y, phi;
Pipistrelle::Device *pip;

void setup() {
  x = static_cast<fixed>(0);
  y = static_cast<fixed>(0);
  phi = static_cast<fixed>(0);
  pip = new Pipistrelle::Device(sampleRate);
  pip->led(true);  // Shows that we got this far
}

fixed nextSample() {
  static fixed theta { 0 }, sample;

  // Blend x-y between the four waveforms
  sample = blend(y, blend(x, sine(theta),   triangle(theta)),
                    blend(x, square(theta), saw(theta)));

  theta = fpm::fmod(theta + phi, static_cast<fixed>(1));

  return sample;
}

void loop() {
  float voct = pip->voct()
             + 6.0F * unipolar(pip->pota())  // coarse tuning
             + unipolar(pip->potb()) / 2.0F;  // fine tuning

  float frequency = C0 * pow(2, voct);
  phi = static_cast<fixed>(frequency / sampleRate);

  x = static_cast<fixed>(unipolarPotWithCV(pip->potc(), pip->cv1()));
  y = static_cast<fixed>(unipolarPotWithCV(pip->potd(), pip->cv2()));
}

void TC4_Handler() {
  pip->dacWrite(nextSample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF;  // clear interrupt overflow flag
}
