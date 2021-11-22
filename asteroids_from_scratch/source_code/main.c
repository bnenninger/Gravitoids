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
#include "director.h"

//global variables for timer.c
volatile uint32_t timer0_counter = 0;
volatile uint32_t seconds_counter = 0;
volatile uint32_t half_seconds_counter = 0;
//to display to LCD
char lcd_text[50];
//control-related variables
uint8_t direction;
char keyboard_input;
int poll_counter = 8;
int joystick_input;
int push_button_input;
//functions
int read_push_button();
void InitPushButton();

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
  LCD_PutText(72, 112, "Waiting for input", White, Black);
  //enables the timer0 interrupts
  enable_timer0();
  while (SER_getChar_nb(0) != 's')
  {
    //wait until start signal sent
    sprintf(lcd_text, "Seed: %d", timer0_counter);
    LCD_PutText(8, 8, lcd_text, Yellow, Black);
  }
  //seed the randomizer
  srand(timer0_counter);
  //start the game, director handles this, initializes the display engine, game-related variables, and sprites
  start_game();

  while (1)
  {
    //poll the controls
		//keyboard_input = SER_getChar(0);
    keyboard_input = SER_getChar_nb(0);
    if (keyboard_input != 0)
    {
      //note: use python application to translate arrow keys to wasd
      switch (keyboard_input)
      {
      case 'a':
        direction = LEFT;
        break;
      case 's':
        direction = DOWN;
        break;
      case 'd':
        direction = RIGHT;
        break;
      case 'w':
        direction = UP;
      }
    }
    //use timer half-seconds variable to see if you should draw new frame
    if (half_seconds_counter > 0)
    {
      //reset counter to 0
      half_seconds_counter = 0;
      //display new frame
      update_place_space();
    }
  }
}
