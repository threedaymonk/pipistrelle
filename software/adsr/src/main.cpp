// ADSR
//
// Pot A => Attack
// Pot B + CV 1 => Decay
// Pot C => Sustain
// Pot D + CV 2 => Release
// V/oct => Gate input

#include <Pipistrelle.h>
#include <stdint.h>
#include <math.h>
#include <cv.h>
#include "ADSR.h"

#define SAMPLE_RATE 8000

#define MIN_A 0.0002L
#define MAX_A 12.0L
#define MIN_DR 0.001L
#define MAX_DR 15.0L
#define CURVE 2.0L

Pipistrelle *pip;
ADSR *env;

void setup() {
  env = new ADSR();
  pip = new Pipistrelle(SAMPLE_RATE);
}

void loop() {
  static double sample_rate = SAMPLE_RATE;
  static bool old_gate = false;
  double a, d, s, r;
  bool gate;

  gate = pip->gate3();

  // Scale pots using an exponential curve. This allows a wider range while
  // still providing fine control at lower levels.
  // Technically, the range is MIN to (MIN + MAX), but as the MAX values are
  // so much larger, the difference is insignificant.
  a = pow(unipolar(pip->pota()), CURVE)
    * MAX_A + MIN_A;
  d = pow(unipolar_with_cv(pip->potb(), pip->cv1()), CURVE)
    * MAX_DR + MIN_DR;
  s = unipolar(pip->potc());
  r = pow(unipolar_with_cv(pip->potd(), pip->cv2()), CURVE)
    * MAX_DR + MIN_DR;

  env->setAttackRate(a * sample_rate);
  env->setDecayRate(d * sample_rate);
  env->setSustainLevel(s);
  env->setReleaseRate(r * sample_rate);

  if (gate != old_gate) {
    env->gate(gate);
    old_gate = gate;
  }
}

void TC4_Handler() {
  static int counter = 0;
  int sample;
  double level;

  level = env->process();

  sample = 511 + 512 * level;
  pip->dac_write(sample);

  // PWM the LED
  digitalWrite(LED, counter < ((sample - 512) >> 2));
  counter = (counter + 1) % 128;

  REG_TC4_INTFLAG = TC_INTFLAG_OVF; // clear interrupt overflow flag
}
