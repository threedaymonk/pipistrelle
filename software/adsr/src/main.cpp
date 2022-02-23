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

#define SAMPLE_RATE 16000
#define MIN_A 0.0002F
#define MAX_A 12.0F
#define MIN_DR 0.001F
#define MAX_DR 15.0F
#define CURVE 2.0F

Pipistrelle *pip;
ADSR *env;

void setup() {
  env = new ADSR();
  pip = new Pipistrelle(SAMPLE_RATE);
}

void loop() {
  static float sample_rate = SAMPLE_RATE;
  bool gate;
  static bool old_gate = false;

  gate = pip->gate3();

  env->setAttackRate(  sample_rate
                     * pow(unipolar(pip->pota()), CURVE)
                     * MAX_A + MIN_A);
  env->setDecayRate(  sample_rate
                    * pow(unipolar_with_cv(pip->potb(), pip->cv1()), CURVE)
                    * MAX_DR + MIN_DR);
  env->setSustainLevel(unipolar(pip->potc()));
  env->setReleaseRate(  sample_rate
                      * pow(unipolar_with_cv(pip->potd(), pip->cv2()), CURVE)
                      * MAX_DR + MIN_DR);

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
