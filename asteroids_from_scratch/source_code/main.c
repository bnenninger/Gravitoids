// COEN 4720
// Project
// Gravitoids: Asteroids with Extra Physics and Multiplayer
// Brendan Nenninger, Kassie Povinelli, Carl Sustar
//
// main.c
// handles initialization of all the board devices, taking input from the nunchuck,
// timing game updates, and tracking high scores

#include "LPC17xx.H"
#include "GLCD.h"
#include "uart.h"
#include "timer.h"
#include "framebuffer.h"
#include "director.h"
#include "nunchuck.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLACK_HOLE_CODE "black hole"

// global variables for bluetooth input
extern volatile uint32_t UART2_Count;
extern volatile uint8_t UART2_Buffer[BUFSIZE];

//global variables for timer.c
volatile uint32_t timer0_counter = 0;
volatile uint32_t seconds_counter = 0;
volatile uint32_t half_seconds_counter = 0;
volatile uint32_t frame_counter;
uint32_t prev_half_second = 0;

// variable for tracking high score
uint32_t high_score = 0;

// text buffer for bluetooth and screen outputs
char text_buffer[70];

// function plays one game of gravitoids and returns the score
int play_one_game()
{
    UARTSend(0, "New game started\n", 17);
    UARTSend(2, "Enter 'black hole' to spawn black holes\n", 40);
    start_game();
    UART2_Count = 0;
    uint32_t prev10msCount = timer0_counter;
    uint32_t prevSecondCount = seconds_counter;
    // loops until the game is finished
    while (!is_game_over())
    {
        // game cycle tuned to 20 milliseconds, attempting to reach 50 fps
        if (timer0_counter >= prev10msCount + 2)
        {
            // for each game turn, read the nunchuck input, apply the input to the game, and update the game space
            NunchuckData ctrlInput = NunChuck_read();
            control_input(ctrlInput.joy_x_axis, ctrlInput.joy_y_axis, !ctrlInput.z_button, !ctrlInput.c_button);
            update_game_space(); // also includes render calls

            // resets the tracker for the previous count so the next cycle will occur on the appropriate timeline
            prev10msCount = timer0_counter;
        }
        // check if any complete UART messages have been received
        // perform the corresponding action if any have
        if (UART2_Count != 0 && UART2_Buffer[UART2_Count - 1] == '\n')
        {
            // read the bluetooth input
            strncpy(text_buffer, (char *)UART2_Buffer, UART2_Count - 1);
            text_buffer[UART2_Count - 1] = 0;
            UART2_Count = 0;
            buffer_to_LCD(100, 0, text_buffer);
            // only compares up to the length of the black hole code
            // if this is correct, the user probably meant it
            // removes issues with what line ending is used
            if (0 == strncmp(text_buffer, BLACK_HOLE_CODE, strlen(BLACK_HOLE_CODE)))
            {
                // spawns in black holes and transmits the number spawned to the bluetooth device
                // otherwise, sends message that says no black holes could be spawned
                int numSpawned = spawn_black_holes();
                if (numSpawned)
                {
                    int len = sprintf(text_buffer, "%d black holes spawned.\n", numSpawned);
                    UARTSend(2, text_buffer, len);
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
        // periodically transmits the game lives and score if still alive, or the score and a game over message if the game is over
        if (seconds_counter == prevSecondCount + 5)
        {
            if (is_game_over())
            {
                int len = sprintf(text_buffer, "Player Score: %d\nGAME OVER\n", get_score());
                UARTSend(2, text_buffer, len);
            }
            else
            {
                int len = sprintf(text_buffer, "Player Score: %d\nPlayer Lives: %d/%d\n", get_score(), get_lives(), get_max_lives());
                UARTSend(2, text_buffer, len);
            }
            prevSecondCount = seconds_counter;
        }
    }
    return get_score();
}

// main function
// initializes board
// tracks high score and launches new games
int main(void)
{
    int i;
    // (1) initializations;
    SystemInit();
    // uart0 for debugging
    UARTInit(0, 9600);
    // bluetooth for external commands
    UARTInit(2, 9600);

    //initialize timer to trigger every 10 ms
    init_timer0(10);

    // Initialize graphical LCD
    LCD_Initialization();
    // Clear graphical LCD display
    LCD_Clear(Black);
    NunChuck_init();
    init_framebuffer();
    init_vector_render_engine();

    enable_timer0();

    buffer_text_centered(160, 50, "Press 'Z' to start.");
    buffer_to_LCD();

    int prevSecondCount;
    NunchuckData ctrlInput;
    while (1)
    {
        timer0_counter = 0;
        ctrlInput = NunChuck_read();
        //wait until z button pressed, then start game
        while (ctrlInput.z_button)
        {
            ctrlInput = NunChuck_read();
        }
        //seed the randomizer based on how long the user took to press the button
        clear_buffer();
        LCD_Clear(Black);
        srand(timer0_counter);

        // play a game and record the score
        volatile int score = play_one_game();
        UARTSend(0, "Game over\n", 10);

        // display a score message, and the current high score if a new high score has not been set
        // save the new high score if appropriate, and display the score to the user
        if (score > high_score)
        {
            high_score = score;
            buffer_text_centered(160, 120 + 8, "NEW HIGH SCORE!");
            int len = sprintf(text_buffer, "Score: %d", score);
            buffer_text_centered(160, 120 + 24, text_buffer);
            UARTSend(0, text_buffer, len);
        }
        // if a highscore has not been achieved, display the player's score and the current highscore
        else
        {
            int len = sprintf(text_buffer, "Score: %d", score);
            UARTSend(0, text_buffer, len);
            buffer_text_centered(160, 120 + 8, text_buffer);
            len = sprintf(text_buffer, "High Score: %d", high_score);
            buffer_text_centered(160, 120 + 24, text_buffer);
        }
        buffer_to_LCD();

        // delay 3 seconds
        prevSecondCount = seconds_counter;
        while (seconds_counter < prevSecondCount + 3)
        {
        }

        // show message for how to start a new game
        buffer_text_centered(160, 50, "Press 'Z' to play again.");
        buffer_to_LCD();
    }
}
