#ifndef __pipistrelle_h__
#define __pipistrelle_h__

#include <Arduino.h>
#include <stdint.h>

#define AUDIO_OUT 0
#define POTA 2
#define POTB 3
#define POTC 9
#define POTD 10
#define CV1 7
#define CV2 5
#define VOCT 4
#define LED 6

#define CLOCK_SPEED 0x2dc0000
#define MAX_ADC 4095

void DACSetup(uint32_t sampleRate);
void DACWrite(uint16_t sample);
double unipolar(int reading);
double bipolar(int reading);
int readPotA();
int readPotB();
int readPotC();
int readPotD();
int readCv1();
int readCv2();
double readVoct();
void initialize_hardware();

#endif /* __pipistrelle_h__ */
