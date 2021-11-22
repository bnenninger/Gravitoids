#include "LPC17xx.h"

#define DAC_BIAS (0x1<<16)
#define DATA_LENGTH	0x400

void DACInit( void );
void beep( int x, int y);
void play_sound();
void play_sound_2();