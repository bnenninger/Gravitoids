//hw3
//based on code provided by Dr. Cris Ababei at Marquette University
//author: Kassie Povinelli
//Snake game
#include <stdio.h>
//#include "LPC17xx.H"
#include "GLCD.h"
#include "Serial.h"
#include <string.h>
#include <stdlib.h>
#include "timer.h"
#include "framebuffer.h"
#include "director.h"
#include "nunchuck.h"

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
    //SystemInit();
    //initialize serial for inputs from computer keyboard
    SER_init(0, 9600);
    //initialize timer to trigger every 10 ms
    init_timer0(10);
    //initialize sound engine
    //DACInit();
    //initialize LCD, print "waiting for start message"
    // Initialize graphical LCD
    LCD_Initialization();
    // Clear graphical LCD display
    LCD_Clear(Black);
    NunChuck_init();
    init_framebuffer();
    init_vector_render_engine();
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

    // render_gamestate_to_LCD();
    update_game_space();
    start_game();
    int prev10msCount = timer0_counter;
    while (1)
    {
        if (timer0_counter >= prev10msCount + 5)
        {

            NunchuckData ctrlInput = NunChuck_read();
            control_input(ctrlInput.joy_x_axis, ctrlInput.joy_y_axis, !ctrlInput.z_button, !ctrlInput.c_button);
            long start = timer0_counter;
            update_game_space();

            long end = timer0_counter;
            // sprintf(lcd_text, "time: %d", end - start);
            // buffer_text(100, 16, lcd_text);
        }
    }
}
