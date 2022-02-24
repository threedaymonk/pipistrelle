// Supersaw
//
// Pot A => Coarse tune (C0 + 6 octaves)
// Pot B => Fine tune (+/- 1/2 octave)
// Pot C + CV 1 => Detune
// Pot D + CV 2 => Sub oscillator level

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
const int numSaws = 7;
// Pleasing weighting determined through trial and error
const int sawMix[numSaws] = {3, 4, 5, 6, 5, 4, 3};
const fixed F0 = static_cast<fixed>(0);

fixed sawPhi[numSaws], subPhi, subLevel;
int sawDivisor = 0;

Pipistrelle::Device *pip;

void setup() {
  // Compute the weighting divisor once
  for (int i = 0; i < numSaws; i++) sawDivisor += sawMix[i];
  pip = new Pipistrelle::Device(sampleRate);
  pip->led(true);  // Shows that we got this far
}

fixed nextSample() {
  static fixed sawTheta[numSaws] = {F0, F0, F0, F0, F0, F0, F0},
               subTheta = F0;
  fixed sample = F0, subSample;

  for (int i = 0; i < numSaws; i++) {
    sample += saw(sawTheta[i]) * sawMix[i];
    sawTheta[i] = fpm::fmod(sawTheta[i] + sawPhi[i], static_cast<fixed>(1));
  }

  subSample = square(subTheta);
  subTheta = fpm::fmod(subTheta + subPhi, static_cast<fixed>(1));

  return blend(subLevel, sample / sawDivisor, subSample);
}

void loop() {
  float voct = pip->voct()
             + 6.0F * unipolar(pip->pota())  // coarse tuning
             + unipolar(pip->potb()) / 2.0F;  // fine tuning

  float frequency = C0 * pow(2, voct);

  float detune = frequency / 80
               * unipolarPotWithCV(pip->potc(), pip->cv1());

  subLevel = static_cast<fixed>(0.25f * unipolarPotWithCV(pip->potd(), pip->cv2()));

  for (int i = 0; i < numSaws; i++) {
    float f = frequency + detune * (i - numSaws / 2);
    sawPhi[i] = static_cast<fixed>(f / sampleRate);
  }

  subPhi = static_cast<fixed>(frequency / (2 * sampleRate));
}

void TC4_Handler() {
  pip->dacWrite(nextSample());
  REG_TC4_INTFLAG = TC_INTFLAG_OVF;  // clear interrupt overflow flag
}
