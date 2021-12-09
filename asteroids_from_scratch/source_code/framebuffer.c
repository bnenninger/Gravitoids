/*
Project
Brendan Nenninger
This file includes code to operate a framebuffer.
This framebuffer uses two alternating frame buffers for the current and previous frame so that it
can only change the pixels that have been changed when writing a new frame, saving rendering time
and avoiding flickering. Each bit in the framebuffer represents the color to save memory, such that 1
is white (or whatever color is LIGHT_COLOR) and 0 is black/DARK_COLOR. This enables the framebuffer to
fit within the memory of the LPC1768.
*/

#include "framebuffer.h"
#include "GLCD.h"
#include "AsciiLib.h"
#include <string.h>

#define BUFFER_ITEM_WIDTH 32

uint32_t framebufferSpace[(MAX_X_COORD * MAX_Y_COORD * 2) / BUFFER_ITEM_WIDTH];
uint32_t *currentFramebuffer;
uint32_t *previousFramebuffer;

// initializes the framebuffer
void init_framebuffer()
{
    // prepares the frame buffer structures
    currentFramebuffer = framebufferSpace;
    previousFramebuffer = &framebufferSpace[(MAX_X_COORD * MAX_Y_COORD) / BUFFER_ITEM_WIDTH + 1];
}

// adds an individual light pixel to the framebuffer
void buffer_pixel(int x, int y)
{
    if (x < 0 || x > MAX_X_COORD - 1 || y < 0 || y > MAX_Y_COORD - 1)
    {
        return;
    }
    int bitIndex = x % 32;
    currentFramebuffer[(y * MAX_X_COORD + x) / BUFFER_ITEM_WIDTH] |= 1 << bitIndex;
}

// adds a line to the framebuffer
// This function is based on a function from GLCD.c written by Dr. Cris Ababei
// it has been modifed to place lines into a framebuffer to be rendered later
// x,y coordinates;
// see additional info:
// http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void buffer_line(int x0, int y0, int x1, int y1)
{
    int dx = x0 < x1 ? (x1 - x0) : (x0 - x1); // basically dx=abs(x1-x0)
    int sx = x0 < x1 ? 1 : -1;
    int dy = y0 < y1 ? (y1 - y0) : (y0 - y1);
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;)
    {
        buffer_pixel(x0, y0);
        //LCD_SetPoint(x0, y0, color); // call of GCLD function;
        if (x0 == x1 && y0 == y1)
            break;
        e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y0 += sy;
        }
    }
}

// adds a character to the framebuffer
// This function is based on a function from GLCD.c, LCD_PutChar()
// it has been modified to place characters into a framebuffer to be rendered later
// has no color, all text will be white
void buffer_char(uint16_t Xpos, uint16_t Ypos, uint8_t ASCI)
{
    uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer, ASCI);
    for (i = 0; i < 16; i++)
    {
        tmp_char = buffer[i];
        for (j = 0; j < 8; j++)
        {
            if ((tmp_char >> 7 - j) & 0x01 == 0x01)
            {
                buffer_pixel(Xpos + j, Ypos + i);
            }
            // else
            // {
            //     buffer_pixel(Xpos + j, Ypos + i, bkColor);
            // }
        }
    }
}

// adds a line of text to the framebuffer
// This function is based on a function from GLCD.c, LCD_PutText()
// it has been modified to place text into a framebuffer to be rendered later
// has no color, all text will be white
void buffer_text(uint16_t Xpos, uint16_t Ypos, uint8_t *str)
{
    uint8_t TempChar;
    do
    {
        TempChar = *str++;
        buffer_char(Xpos, Ypos, TempChar);
        if (Xpos < MAX_X - 8)
        {
            Xpos += 8;
        }
        else if (Ypos < MAX_Y - 16)
        {
            Xpos = 0;
            Ypos += 16;
        }
        else
        {
            Xpos = 0;
            Ypos = 0;
        }
    } while (*str != 0);
}
// adds a line of text to the framebuffer such that it is centered on the Xpos
void buffer_text_centered(uint16_t Xpos, uint16_t Ypos, uint8_t *str)
{
    buffer_text(Xpos - 8 * strlen(str) / 2, Ypos, str);
}

// transfers the contents of the buffer to the LCD screen
void buffer_to_LCD()
{
    // iterate through each y line and each block of x positions
    for (int y = 0; y < MAX_Y_COORD; y++)
    {
        // x refers to the index of each x block of 32
        for (int x = 0; x < MAX_X_COORD / BUFFER_ITEM_WIDTH; x++)
        {
            // find which bits have transitioned to light and which have transistioned to dark
            uint32_t changedBits = currentFramebuffer[y * MAX_X_COORD / BUFFER_ITEM_WIDTH + x] ^ previousFramebuffer[y * MAX_X_COORD / BUFFER_ITEM_WIDTH + x];
            uint32_t blackTransition = changedBits & (~currentFramebuffer[y * MAX_X_COORD / BUFFER_ITEM_WIDTH + x]);
            uint32_t whiteTransition = changedBits & currentFramebuffer[y * MAX_X_COORD / BUFFER_ITEM_WIDTH + x];
            // display each pixel of the block to the LCD
            for (int i = 0; i < BUFFER_ITEM_WIDTH; i++)
            {
                // if the pixel is turning black
                if (blackTransition & 1)
                {
                    LCD_SetPoint(x * BUFFER_ITEM_WIDTH + i, y, DARK_COLOR);
                }
                // if the pixel is turning white
                else if (whiteTransition & 1)
                {
                    LCD_SetPoint(x * BUFFER_ITEM_WIDTH + i, y, LIGHT_COLOR);
                }
                // bit shift out the bit after it has been displayed or considered
                blackTransition >>= 1;
                whiteTransition >>= 1;
            }
        }
    }
    // swap the frame buffers
    uint32_t *temp = previousFramebuffer;
    previousFramebuffer = currentFramebuffer;
    currentFramebuffer = temp;
    // clear the current frame buffer to prepare for a new frame to be written to it
    for (int i = 0; i < MAX_X_COORD * MAX_Y_COORD / BUFFER_ITEM_WIDTH; i++)
    {
        currentFramebuffer[i] = 0;
    }
}