
//vector.c

#include <stdio.h>
#include <math.h>
#include "GLCD.h"
#include "display_engine.h"

void update_buffer_and_display(uint16_t* screen, uint16_t* render_buffer, uint32_t buffer_size)
{
	uint_32_t i;
	//for all points in framebuffer
	for (i = 0; i < buffer_size; i++)
	{
		//check if point needs to be changed, if so, change it
		if (screen[i] != render_buffer[i])
		{
			//set point on LCD to this color
			LCD_SetPoint(i % MAX_X, i / MAX_Y, render_buffer[i]); 
			//set screen to match render buffer
			screen[i] = render_buffer[i];
		}
	}
}
