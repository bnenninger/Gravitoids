//hw3
//based on code provided by Dr. Cris Ababei at Marquette University
//author: Kassie Povinelli
//Snake game
#include <stdio.h>
#include "LPC17xx.H"
#include "GLCD.h"
#include "Serial.h"
#include <string.h>
#include <stdlib.h>
#include "timer.h"
#include "vector_engine.h"
#include "framebuffer.h"

//global variables for timer.c
volatile uint32_t timer0_counter = 0;
volatile uint32_t seconds_counter = 0;
volatile uint32_t half_seconds_counter = 0;
uint32_t prev_half_second = 0;
volatile uint32_t frame_counter = 0;
//to display to LCD
char lcd_text[50];
//control-related variables
uint8_t direction;
char keyboard_input;

///////////////////////////////////////////////////////////////////////////////
//
// main program
//
///////////////////////////////////////////////////////////////////////////////
int main(void)
{
    int i;
    // (1) initializations;
    SystemInit();
    //initialize serial for inputs from computer keyboard
    SER_init(0, 9600);
    //initialize timer to trigger every 10 ms
    init_timer0(10);
    //initialize sound engine
    DACInit();
    //initialize LCD, print "waiting for start message"
    // Initialize graphical LCD
    LCD_Initialization();
    // Clear graphical LCD display
    LCD_Clear(Black);
    init_framebuffer();
    init_vector_engine();
    //LCD_PutText(72, 112, "Waiting for input", White, Black);
    //enables the timer0 interrupts
    enable_timer0();
    //while (SER_getChar_nb(0) != 's')
    //{
    //  //wait until start signal sent
    //  sprintf(lcd_text, "Seed: %d", timer0_counter);
    //  LCD_PutText(8, 8, lcd_text, Yellow, Black);
    //}
    //seed the randomizer
    srand(timer0_counter);
    //start the game, director handles this, initializes the display engine, game-related variables, and sprites
    //start_game();

    Entity a1;
    a1.sprite = get_asteroid_sprite();
    a1.orientation = 0;
    a1.size = 10;
    a1.to_clear = 0;
    a1.visible = 1;
    a1.x = 100;
    a1.y = 100;
    Entity a2;
    a2.sprite = get_rocket_sprite();
    a2.orientation = 0;
    a2.size = 1;
    a2.to_clear = 0;
    a2.visible = 1;
    a2.x = 200;
    a2.y = 200;

    buffer_pixel(315, 235);
    buffer_to_LCD();

    buffer_pixel(100, 100);

    int prev10msCount = timer0_counter;
    while (1)
    {
        if (timer0_counter >= prev10msCount + 5)
        {
            int start = timer0_counter;
            buffer_to_LCD();
            int end = timer0_counter;
            sprintf(lcd_text, "%d", end - start);
            LCD_PutText(100, 0, lcd_text, White, Black);
            prev_half_second = half_seconds_counter;
            prev10msCount = timer0_counter;
            frame_counter = 0;
            a1.orientation += 0.2;
            a2.orientation -= 0.1;
            a2.x += 2;
            a2.y += 1;
            if (a2.x > MAX_X_COORD)
            {
                a2.x = 0;
            }
            if (a2.y > MAX_Y_COORD)
            {
                a2.y = 0;
            }

            start = timer0_counter;
            draw_entity_to_buffer(&a1);
            draw_entity_to_buffer(&a2);
            buffer_pixel(0, 0);
            end = timer0_counter;
            sprintf(lcd_text, "%d", end - start);
            buffer_text(0, 0, lcd_text);
        }
        //draw_entity(&squareEntity);
        ////poll the controls
        //	//keyboard_input = SER_getChar(0);
        //keyboard_input = SER_getChar_nb(0);
        //if (keyboard_input != 0)
        //{
        //  //note: use python application to translate arrow keys to wasd
        //  switch (keyboard_input)
        //  {
        //  case 'a':
        //    direction = LEFT;
        //    break;
        //  case 's':
        //    direction = DOWN;
        //    break;
        //  case 'd':
        //    direction = RIGHT;
        //    break;
        //  case 'w':
        //    direction = UP;
        //  }
        //}
        ////use timer frame_counter variable to see if you should draw new frame
        //if (frame_counter > 0)
        //{
        //  //reset counter to 0
        //  frame_counter = 0;
        //  //display new frame
        //  update_place_space();
        //}
    }
}