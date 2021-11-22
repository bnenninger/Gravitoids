#include "LPC17xx.h"
#include "GLCD.h"
#include "director.h"
#include "display_engine.h"
#include <stdlib.h>
#include <time.h>
#include "sound.h"
//game variables
//number of lives left
extern int lives;
//snake body length
extern int length;
//head and previous head positions
int head_position_x;
int head_position_y;
int prev_head_pos_x;
int prev_head_pos_y;
//movement direction, 0 left, 1 up, 2 right, 3 down
extern uint8_t direction;

uint8_t mice_ate_consecutively;

//functions
//see if snake ate mouse
int update_mouse(void)
{
	//check if snake got the mouse
	if (head_position_x == check_x_pos(MOUSE) && head_position_y == check_y_pos(MOUSE))
	{
		//snake ate mouse, increase lives and length of snake, respawn mouse
		mice_ate_consecutively++;
		//if snake got 2 in a row
		if (mice_ate_consecutively > 1)
		{
			//increase lives
			lives++;
			//reset consecutive counter
			mice_ate_consecutively = 0;
			//play life gained sound
			play_sound();
		}
		//increase length
		length++;

		//make another piece of snake visible
		if (length < MOUSE)
		{
			update_sprite_visibility(length, 1);
		}

		//respawn it somewhere else
		update_sprite_position(MOUSE, rand() % (MAX_X_COORD + 1), rand() % (MAX_Y_COORD + 1));
		//snake ate mouse, return 1
		return 1;
	}
	//snake did not eat mouse, return 0
	return 0;
}
//move the snake in the current direction
void move_snake(void)
{
	int i;
	head_position_x = check_x_pos(0);
	head_position_y = check_y_pos(0);
	prev_head_pos_x = head_position_x;
	prev_head_pos_y = head_position_y;
	//move snake in chosen direction if possible, if not, decrease lives
	if (direction == LEFT && head_position_x > 0)
	{
		//move left
		head_position_x--;
		update_sprite_position(0, head_position_x, head_position_y);
	}
	else if (direction == RIGHT && head_position_x < MAX_X_COORD)
	{
		//move right
		head_position_x++;
		update_sprite_position(0, head_position_x, head_position_y);
	}
	else if (direction == UP && head_position_y > 0)
	{
		//move up
		head_position_y--;
		update_sprite_position(0, head_position_x, head_position_y);
	}
	else if (direction == DOWN && head_position_y < MAX_Y_COORD)
	{
		//move left
		head_position_y++;
		update_sprite_position(0, head_position_x, head_position_y);
	}
	else
	{
		//reset mice ate consecutively
		mice_ate_consecutively = 0;
		//lose life
		lives--;
		//beep to signify life lost
		play_sound_2();
		//don't need to update rest of snake, did not move
		return;
	}
	if (!update_mouse())
	{
		//if mouse not eaten
		//update the position of the snake's trail
		update_sprite_position(TRAIL, check_x_pos(length), check_y_pos(length));
	}
	//update the rest of the snake accordingly, minus the last body piece
	for (i = length; i > 1; i--)
	{
		update_sprite_position(i, check_x_pos(i - 1), check_y_pos(i - 1));
	}
	//update the last body piece
	update_sprite_position(1, prev_head_pos_x, prev_head_pos_y);
}
//this function will be called whenever you want to draw a new frame
void update_place_space(void)
{
	if (lives < 1)
	{
		//display "GAME OVER"
		display_lose();
	}
	else if (lives > 5)
	{
		//display "YOU WIN"
		display_win();
	}
	else
	{
		//move the snake, see if it eats mouse
		move_snake();

		//update the display
		update_display();
	}
}
void start_game(void)
{
	//set mice ate consecutively
	mice_ate_consecutively = 0;
	//set lives to initial value
	lives = INITIAL_LIVES;
	//set initial movement direction to up
	direction = UP;
	//initialize the display engine
	initialize_display_engine();
	//set visibilities for head, 3 body parts
	update_sprite_visibility(0, 1);
	update_sprite_visibility(1, 1);
	update_sprite_visibility(2, 1);
	update_sprite_visibility(3, 1);
	//set visibility for mouse
	update_sprite_visibility(MOUSE, 1);
	//set visibility of snake trail
	update_sprite_visibility(TRAIL, 1);
	//set mouse position randomly
	update_sprite_position(MOUSE, rand() % (MAX_X_COORD + 1), rand() % (MAX_Y_COORD + 1));
}