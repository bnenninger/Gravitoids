#ifndef __TIMER_H
#define __TIMER_H

#include "LPC17xx.h"

// TIMER0 functions
extern uint32_t init_timer0(uint32_t delayInMs);
extern void enable_timer0(void);
extern void disable_timer0(void);
extern void reset_timer0(void);
extern void delay_as_10s_of_ms_timer0(unsigned long millisec); // millisec must be a multiple of "10 ms"
//extern void TIMER0_IRQHandler( void);

#endif
