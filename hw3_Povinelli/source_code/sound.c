#include "sound.h"

void DACInit( void )
{
  LPC_PINCON->PINSEL1 = 0x00200000;  // set P0.26 to DAC output;  
  LPC_DAC->DACCNTVAL = 0x00FF;
  LPC_DAC->DACCTRL = (0x1<<1)|(0x1<<2);
  return;
}
void beep( int x, int y) {
  int i = 0;
  while ( x) {
    x--;
    LPC_DAC->DACR = (i << y); // digital value to be converted;
    i++;
  }  
}
void play_sound()
{
  int am=0;
	int k=0;
  uint32_t m;
  
	while ( am < 2000) {
		LPC_DAC->DACR = (k << 13) | DAC_BIAS;
		k++;
		for (m = 800; m>1; m--);
		if ( k == DATA_LENGTH) {
			k = 0;
		}
		am++;
  }
}
void play_sound_2()
{
  int am=0;
	int k=0;
  uint32_t m;
  
	while ( am < 2000) {
		LPC_DAC->DACR = (k << 13) | DAC_BIAS;
		k++;
		for (m = 1000; m>1; m--);
		if ( k == DATA_LENGTH) {
			k = 0;
		}
		am++;
  }
}