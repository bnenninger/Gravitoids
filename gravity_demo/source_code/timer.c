// cristinel.ababei, copyleft 2013-present
//
// this is adapted from the Example/Timer from CD that comes with LandTiger board,
// which I think had some errors: when programmed to measure a second, the example
// was running a bit faster; I think, here with the changes, it does measure just
// 1 second  now;
#define TICKS_PER_FRAME 4
#include "lpc17xx.h"
#include "timer.h"

extern uint32_t timer0_counter;		  // global time in increments of 10 msseconds;
extern uint32_t seconds_counter;	  // global time in seconds;
extern uint32_t half_seconds_counter; // global time in half seconds;
extern uint32_t frame_counter;		  // global time in 50th seconds, for 50fps;
///////////////////////////////////////////////////////////////////////////////
//
// functions related to TIMER0
// TIMER0 is used to measure "passing ofo time"; it implements a timeline
// that is configured to pass in intervals of 10 ms; these intervals are
// counted by timer0_counter; tIMER0 interrupt is enabled and the TIMER0_IRQHandler
// ISR routine is used to count and increment a global "1 second" counter;
//
///////////////////////////////////////////////////////////////////////////////

uint32_t init_timer0(uint32_t delayInMs)
{
	timer0_counter = 0;

	LPC_SC->PCONP |= 1 << 1;		   // Power up Timer0
									   // we set bits 3:2 to "01" to effectively force PCLK_peripheral = CCLK, i.e., CPU Clock
									   // as reported on pages 56-57 of the user manual;
	LPC_SC->PCLKSEL0 &= ~(0x0000000C); // clear bits 3:2
	LPC_SC->PCLKSEL0 |= (1 << 2);	   // then, set them to "01" to have PCLK_peripheral = CCLK
	// CCLK = 100 MHz; clock cycle or clock period is 1/100 MHz = 10 ns
	// how many clock cycles of 10 ns do we need to make up delayInMs,
	// if 1 ms = 100000 * 10 ns
	LPC_TIM0->MR0 = 100000 * delayInMs;
	LPC_TIM0->IR = 0xff;		// reset all interrrupts
	LPC_TIM0->MCR |= 1 << 0;	// Interrupt on Match 0 compare
	LPC_TIM0->MCR |= 1 << 1;	// Reset timer on Match 0
	LPC_TIM0->TCR |= 1 << 1;	// Manually Reset TIMER0 (forced)
	LPC_TIM0->TCR &= ~(1 << 1); // Stop resetting the timer
	LPC_TIM0->TCR |= 1 << 0;	// Start timer (i.e., enable timer)

	NVIC_EnableIRQ(TIMER0_IRQn);
	return 1;
}

void enable_timer0(void)
{
	LPC_TIM0->TCR = 1;
	return;
}

void disable_timer0(void)
{
	LPC_TIM0->TCR = 0;
	return;
}

void reset_timer0(void)
{
	uint32_t regVal;
	regVal = LPC_TIM0->TCR;
	regVal |= 0x02;
	LPC_TIM0->TCR = regVal;
	return;
}

void delay_as_10s_of_ms_timer0(unsigned long millisec)
{
	// introduces a delay of an integer number of 10 ms periods;
	// assumption: argument millisec is always a multiple of 10ms;
	uint32_t num_of_10ms_intervals = millisec / 10;

	// calculate when in the future will have passed that many "millisec" from
	// current time, taken as snapshot of ticks_of_10_ms;
	uint32_t millisec_stop_deadline = timer0_counter + num_of_10ms_intervals;

	while (timer0_counter < millisec_stop_deadline)
	{
		// wait, do nothing; ticks_of_10_ms is concurrently and
		// continuously being modified by the IRQ of TIMER0;
	}
}

void TIMER0_IRQHandler(void)
{
	// declare this as static so that its value is preserved among different calls;
	static uint8_t ticks = 0; // local counter

	LPC_TIM0->IR = 1; // clear interrupt flag
	timer0_counter++;

	// increment the global "seconds_counter" every 1 second;
	ticks++; // local count another passed 10 ms;
	if (ticks == 100)
	{
		ticks = 0;				// reset to start counting another 100 periods of 10 ms;
		seconds_counter++;		// increment global time that measures passing of seconds;
		half_seconds_counter++; //increment half-seconds
	}
	else if (ticks == 50)
	{
		half_seconds_counter++; //increment half-seconds
	}
	if (ticks % TICKS_PER_FRAME == 0)
	{
		frame_counter++;
	}

	return;
}
