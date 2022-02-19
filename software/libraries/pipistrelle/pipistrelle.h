#ifndef __pipistrelle_h__
#define __pipistrelle_h__

#include <Arduino.h>
#include <stdint.h>
#include <q14.h>

#define AUDIO_OUT 0
#define POTA 2
#define POTB 3
#define POTC 9
#define POTD 10
#define CV1 7
#define CV2 5
#define VOCT 4
#define LED 6
#define HIGH_ACCURACY 32
#define LOW_ACCURACY 1

#define CLOCK_SPEED 47972352
#define MAX_ADC 4095
#define END_SLOP 5

void initialize_hardware(int sample_rate);
void dac_setup(int sample_rate);
void dac_write(int sample);
void q14_dac_write(q14_t sample);
float unipolar(int reading);
float bipolar(int reading);
int read_pota(int accuracy);
int read_potb(int accuracy);
int read_potc(int accuracy);
int read_potd(int accuracy);
int read_cv1(int accuracy);
int read_cv2(int accuracy);
float read_voct(int accuracy);

#endif /* __pipistrelle_h__ */
