// COEN 4720
// Project
// Gravitoids: Asteroids with Extra Physics and Multiplayer
// Brendan Nenninger, Kassie Povinelli, Carl Sustar
//
// framebuffer.h
// This file includes code to operate a framebuffer.
// This framebuffer uses two alternating frame buffers for the current and previous frame so that it
// can only change the pixels that have been changed when writing a new frame, saving rendering time
// and avoiding flickering. Each bit in the framebuffer represents the color to save memory, such that 1
// is white (or whatever color is LIGHT_COLOR) and 0 is black/DARK_COLOR. This enables the framebuffer to
// fit within the memory of the LPC1768.

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
void clear_buffer();

#endif
