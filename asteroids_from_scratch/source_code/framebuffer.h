
#include "LPC17xx.h"

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#define MAX_X_COORD 320
#define MAX_Y_COORD 240
#define LIGHT_COLOR White
#define DARK_COLOR Black

void init_framebuffer();
void buffer_pixel(int x, int y);
void buffer_line(int x0, int y0, int x1, int y1);
void buffer_char(uint16_t Xpos, uint16_t Ypos, uint8_t ASCI);
void buffer_text(uint16_t Xpos, uint16_t Ypos, uint8_t *str);
void buffer_text_centered(uint16_t Xpos, uint16_t Ypos, uint8_t *str);
void buffer_to_LCD();

#endif
