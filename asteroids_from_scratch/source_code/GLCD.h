//-----------------------------------------------------------------------------
//
// Cris:
// this is a somewhat cleaned up version of the original GLCD.h file
//
// File name:        GLCD.h
// Created by:      AVRman
// Created date:    2012-3-10
// Version:          1.3
// Descriptions:    The original version
//
//-----------------------------------------------------------------------------

#ifndef __GLCD_H
#define __GLCD_H

//-----------------------------------------------------------------------------
// Includes
#include "LPC17xx.h"

//-----------------------------------------------------------------------------
// Private define

// LCD Interface
#define PIN_EN (1 << 19)
#define PIN_LE (1 << 20)
#define PIN_DIR (1 << 21)
#define PIN_CS (1 << 22)
#define PIN_RS (1 << 23)
#define PIN_WR (1 << 24)
#define PIN_RD (1 << 25)

#define LCD_EN(x) ((x) ? (LPC_GPIO0->FIOSET = PIN_EN) : (LPC_GPIO0->FIOCLR = PIN_EN));
#define LCD_LE(x) ((x) ? (LPC_GPIO0->FIOSET = PIN_LE) : (LPC_GPIO0->FIOCLR = PIN_LE));
#define LCD_DIR(x) ((x) ? (LPC_GPIO0->FIOSET = PIN_DIR) : (LPC_GPIO0->FIOCLR = PIN_DIR));
#define LCD_CS(x) ((x) ? (LPC_GPIO0->FIOSET = PIN_CS) : (LPC_GPIO0->FIOCLR = PIN_CS));
#define LCD_RS(x) ((x) ? (LPC_GPIO0->FIOSET = PIN_RS) : (LPC_GPIO0->FIOCLR = PIN_RS));
#define LCD_WR(x) ((x) ? (LPC_GPIO0->FIOSET = PIN_WR) : (LPC_GPIO0->FIOCLR = PIN_WR));
#define LCD_RD(x) ((x) ? (LPC_GPIO0->FIOSET = PIN_RD) : (LPC_GPIO0->FIOCLR = PIN_RD));

#define DISP_ORIENTATION 90 // angle 0 90 -- changed for snake game to 90

#if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)
#define MAX_X 320
#define MAX_Y 240
#elif (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180)
#define MAX_X 240
#define MAX_Y 320
#endif

// GLCD RGB color definitions;
#define Black 0x0000     /*   0,   0,   0 */
#define Navy 0x000F      /*   0,   0, 128 */
#define DarkGreen 0x03E0 /*   0, 128,   0 */
#define DarkCyan 0x03EF  /*   0, 128, 128 */
#define Maroon 0x7800    /* 128,   0,   0 */
#define Purple 0x780F    /* 128,   0, 128 */
#define Olive 0x7BE0     /* 128, 128,   0 */
#define Grey 0xF7DE
#define LightGrey 0xC618 /* 192, 192, 192 */
#define DarkGrey 0x7BEF  /* 128, 128, 128 */
#define Blue 0x001F      /*   0,   0, 255 */
#define Blue2 0x051F
#define Green 0x07E0   /*   0, 255,   0 */
#define Cyan 0x07FF    /*   0, 255, 255 */
#define Red 0xF800     /* 255,   0,   0 */
#define Magenta 0xF81F /* 255,   0, 255 */
#define Yellow 0xFFE0  /* 255, 255, 0   */
#define White 0xFFFF   /* 255, 255, 255 */

//-----------------------------------------------------------------------------
// Function Name  : RGB565CONVERT
// Description    :
// Input          : - red: R
//                  - green: G
//                  - blue: B
// Output         : None
// Return         : RGB
#define RGB565CONVERT(red, green, blue) \
  (uint16_t)(((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3))

//-----------------------------------------------------------------------------
// Private function prototypes

void LCD_Initialization(void);
void LCD_Clear(uint16_t Color);

uint16_t LCD_GetPoint(uint16_t Xpos, uint16_t Ypos);
void LCD_SetPoint(uint16_t Xpos, uint16_t Ypos, uint16_t color);

void LCD_PutChar(uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor);
void LCD_PutText(uint16_t Xpos, uint16_t Ypos, uint8_t *str, uint16_t Color, uint16_t bkColor);

void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

void LCD_DrawBargraph(unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                      unsigned int val, uint16_t barColor, uint16_t bkColor);

//-----------------------------------------------------------------------------
//
// cristinel ababei
// this is a collection of various "graphics" functions used for
// drawing stuff on the LCD screen of the LandTiger 2.0 board;
// the vast majority of these are put together with a lot
// of open source code;
// Note to myself: in previous editions of this course, I had these in
// separate files CRIS_UTILS.h and CRIS_UTILS.c; now, they are here to
// work with fewer files;
//

void LCD_draw_circle(int x0, int y0, int radius, uint16_t color);
void LCD_draw_circle_full(int x0, int y0, int radius, uint16_t color);
void LCD_draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void LCD_draw_smiley_face(int x0, int y0, int radius, uint16_t color);

#endif
