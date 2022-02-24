// ADSR
//
// Pot A => Attack
// Pot B + CV 1 => Decay
// Pot C => Sustain
// Pot D + CV 2 => Release
// V/oct => Gate input

#include <math.h>

#include <Pipistrelle/Device.h>
#include <Pipistrelle/Modulation.h>

#include "ADSR.h"

using fixed = fpm::fixed_16_16;
using namespace Pipistrelle::Modulation;

// Anything over about 1kHz is fine when our output resolution is 10 bits,
// but too low leads to visible LED flicker.
const int sampleRate = 8000;

const double minA = 0.0002L,
             maxA = 12.0L,
             minDR = 0.001L,
             maxDR = 15.0L,
             controlExp = 2.0L;

Pipistrelle::Device *pip;
ADSR *env;

void setup() {
  env = new ADSR();
  pip = new Pipistrelle::Device(sampleRate);
}

void loop() {
  static bool old_gate = false;

  bool gate = pip->gate3();

  // Scale pots using an exponential curve. This allows a wider range while
  // still providing fine control at lower levels.
  // Technically, the range is min to (min + max), but as the max values are
  // so much larger, the difference is insignificant.
  double a = pow(unipolar(pip->pota()), controlExp)
           * maxA + minA;
  double d = pow(unipolarPotWithCV(pip->potb(), pip->cv1()), controlExp)
           * maxDR + minDR;
  double s = unipolar(pip->potc());
  double r = pow(unipolarPotWithCV(pip->potd(), pip->cv2()), controlExp)
           * maxDR + minDR;

  env->setAttackRate(a * sampleRate);
  env->setDecayRate(d * sampleRate);
  env->setSustainLevel(s);
  env->setReleaseRate(r * sampleRate);

  if (gate != old_gate) {
    env->gate(gate);
    old_gate = gate;
  }
}

void TC4_Handler() {
  static int counter = 0;

  double level = env->process();

  int sample = 511 + 512 * level;
  pip->dacWrite(sample);

  // Manually PWM the LED. We don't use analogWrite because that kills
  // operation (reason unknown, possibly due to interrupt use)
  pip->led(counter < ((sample - 512) >> 2));
  counter = (counter + 1) % 128;

  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
