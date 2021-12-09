//hw3
//based on code provided by Dr. Cris Ababei at Marquette University
//author: Kassie Povinelli
//Snake game
#include <stdio.h>
//#include "LPC17xx.H"
#include "GLCD.h"
// #include "Serial.h"
#include "uart.h"
#include <string.h>
#include <stdlib.h>
#include "timer.h"
#include "framebuffer.h"
#include "director.h"
#include "nunchuck.h"

#define BLACK_HOLE_CODE "black hole"

// global variables for bluetooth input
extern volatile uint32_t UART2_Count;
extern volatile uint8_t UART2_Buffer[BUFSIZE];

//global variables for timer.c
volatile uint32_t timer0_counter = 0;
volatile uint32_t seconds_counter = 0;
volatile uint32_t half_seconds_counter = 0;
uint32_t prev_half_second = 0;
volatile uint32_t frame_counter = 0;
//to display to LCD
uint32_t high_score = 0;

char bluetooth_input_text[70];
//control-related variables
uint8_t direction;
char keyboard_input;

int play_one_game()
{
    start_game();
    int prev10msCount = timer0_counter;
    int prevSecondCount = seconds_counter;
    while (!is_game_over())
    {
        if (timer0_counter >= prev10msCount + 2)
        {

            NunchuckData ctrlInput = NunChuck_read();
            control_input(ctrlInput.joy_x_axis, ctrlInput.joy_y_axis, !ctrlInput.z_button, !ctrlInput.c_button);
            update_game_space();

            prev10msCount = timer0_counter;
        }
        if (UART2_Count != 0 && UART2_Buffer[UART2_Count - 1] == '\n')
        {
            // read the bluetooth input
            strncpy(bluetooth_input_text, (char *)UART2_Buffer, UART2_Count - 1);
            bluetooth_input_text[UART2_Count - 1] = 0;
            UART2_Count = 0;
            buffer_to_LCD(100, 0, bluetooth_input_text);
            // only compares up to the length of the black hole code
            // if this is correct, the user probably meant it
            // removes issues with what line ending is used
            if (0 == strncmp(bluetooth_input_text, BLACK_HOLE_CODE, strlen(BLACK_HOLE_CODE)))
            {
                int numSpawned = spawn_black_holes();
                // int success = spawn_black_hole();
                if (numSpawned)
                {
                    int len = sprintf(bluetooth_input_text, "%d black holes spawned.\n", numSpawned);
                    UARTSend(2, bluetooth_input_text, len);
                }
                else
                {
                    UARTSend(2, "Maximum black holes already in existence.\n", 42);
                }
            }
            else
            {
                UARTSend(2, "Invalid command.\n", 17);
            }
        }
        if (seconds_counter == prevSecondCount + 5)
        {
            if (is_game_over())
            {
                int len = sprintf(bluetooth_input_text, "Player Score: %d\nGAME OVER\n", get_score());
                UARTSend(2, bluetooth_input_text, len);
            }
            else
            {
                int len = sprintf(bluetooth_input_text, "Player Score: %d\nPlayer Lives: %d/%d\n", get_score(), get_lives(), get_max_lives());
                UARTSend(2, bluetooth_input_text, len);
            }
            prevSecondCount = seconds_counter;
        }
    }
    return get_score();
}

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
    // SER_init(0, 9600);
    // SER_init(2, 9600);
    // SER_putString(0, "ser init\n");
    UARTInit(0, 9600);
    UARTInit(2, 9600);

    //BT RX -> P4.28
    //BT TX -> P4.29
    //BT GND -> gnd
    //BT VCC -> 3.3V
    // SER_init(3, 9600);
    //initialize timer to trigger every 10 ms
    init_timer0(10);
    //initialize sound engine
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

    buffer_text_centered(160, 50, "Press 'Z' to start.");
    buffer_to_LCD();
    //start the game, director handles this, initializes the display engine, game-related variables, and sprites
    // start_game();

    // //render_gamestate_to_LCD();
    // // buffer_text(0, 0, "blah");
    // // buffer_to_LCD();
    // update_game_space();
    // // start_game();

    int prevSecondCount = seconds_counter;
    NunchuckData ctrlInput;
    while (1)
    {
        timer0_counter = 0;
        UARTSend(2, "waiting for input", strlen("waiting for input"));
        ctrlInput = NunChuck_read();
        //wait until z button pressed, then start game
        while (ctrlInput.z_button)
        {
            ctrlInput = NunChuck_read();
            int len = sprintf(bluetooth_input_text, "state: %d\n", ctrlInput.z_button);
            UARTSend(2, bluetooth_input_text, len);
        }
        //seed the randomizer based on how long the user took to press the button
        srand(timer0_counter);
        int score = play_one_game();
        // delay 2 seconds
        prevSecondCount = seconds_counter;
        while (prevSecondCount > seconds_counter + 3)
        {
        }
        if (score > high_score)
        {
            high_score = score;
            buffer_text(160 - 10 * 7, 120 + 8, "NEW HIGH SCORE!");
            int len = sprintf(bluetooth_input_text, "Score: %d", score);
            buffer_text_centered(160, 120 + 24, bluetooth_input_text);
        }
        else
        {
            int len = sprintf(bluetooth_input_text, "Score: %d", score);
            buffer_text_centered(160, 120 + 8, bluetooth_input_text);
            len = sprintf(bluetooth_input_text, "High Score: %d", high_score);
            buffer_text_centered(160, 120 + 24, bluetooth_input_text);
        }
        buffer_text_centered(160, 50, "Press 'Z' to play again.");
        buffer_to_LCD();

        // if (timer0_counter >= prev10msCount + 50)
        // {

        //     NunchuckData ctrlInput = NunChuck_read();
        //     control_input(ctrlInput.joy_x_axis, ctrlInput.joy_y_axis, !ctrlInput.z_button, !ctrlInput.c_button);
        //     update_game_space();

        //     prev10msCount = timer0_counter;
        // }
        // if (UART2_Count != 0 && UART2_Buffer[UART2_Count - 1] == '\n')
        // {
        //     // read the bluetooth input
        //     strncpy(bluetooth_input_text, (char *)UART2_Buffer, UART2_Count - 1);
        //     bluetooth_input_text[UART2_Count - 1] = 0;
        //     UART2_Count = 0;
        //     buffer_to_LCD(100, 0, bluetooth_input_text);
        //     // only compares up to the length of the black hole code
        //     // if this is correct, the user probably meant it
        //     // removes issues with what line ending is used
        //     if (0 == strncmp(bluetooth_input_text, BLACK_HOLE_CODE, strlen(BLACK_HOLE_CODE)))
        //     {
        //         int numSpawned = spawn_black_holes();
        //         // int success = spawn_black_hole();
        //         if (numSpawned)
        //         {
        //             int len = sprintf(bluetooth_input_text, "%d black holes spawned.\n", numSpawned);
        //             UARTSend(2, bluetooth_input_text, len);
        //         }
        //         else
        //         {
        //             UARTSend(2, "Maximum black holes already in existence.\n", 42);
        //         }
        //     }
        //     else
        //     {
        //         UARTSend(2, "Invalid command.\n", 17);
        //     }
        // }
        // if (seconds_counter == prevSecondCount + 5)
        // {
        //     if (is_game_over())
        //     {
        //         int len = sprintf(bluetooth_input_text, "Player Score: %d\nGAME OVER\n", get_score());
        //         UARTSend(2, bluetooth_input_text, len);
        //     }
        //     else
        //     {
        //         int len = sprintf(bluetooth_input_text, "Player Score: %d\nPlayer Lives: %d/%d\n", get_score(), get_lives(), get_max_lives());
        //         UARTSend(2, bluetooth_input_text, len);
        //     }
        //     prevSecondCount = seconds_counter;
        // }
    }
}
