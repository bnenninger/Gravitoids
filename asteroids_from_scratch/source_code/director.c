#include "LPC17xx.h"
#include "GLCD.h"
#include "director.h"
//#include "display_engine.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "sound.h"
#include "vector.h"
#include <stdio.h>
#include "string.h"
#include "framebuffer.h"

#define ROCKET_THRUST 0.25
#define ROCKET_ROTATION_RATE 0.002
// mask for the game space, relies on overflows for wraparound
//bitshift of 10 or 11 ideal
#define GAME_SPACE_MASK (((1 << 8) - 1) | 1 << 15)

//game variables
//game object array
struct GAME_OBJECT game_object_array[GAME_OBJECT_NUM];
uint32_t game_object_counter;
char debug_text[64]; //used for debugging
uint8_t debug = 0;
//vector used for summing

//functions
//calculate acceleration due to gravity -- NOT FORCE
struct vector2d calculate_gravity(uint32_t affected_object_index, uint32_t cause_object_index)
{
	struct vector2d radius_vector;
	struct vector2d gravity_vector;
	float gravity_magnitude;
	float radius_magnitude;
	//radius_vector =
	//{
	//	game_object_array[cause_object_index].displacement.x - game_object_array[affected_object_index].displacement.x,
	//	game_object_array[cause_object_index].displacement.y - game_object_array[affected_object_index].displacement.y
	//};
	radius_vector.x = game_object_array[cause_object_index].displacement.x - game_object_array[affected_object_index].displacement.x;
	radius_vector.y = game_object_array[cause_object_index].displacement.y - game_object_array[affected_object_index].displacement.y;
	//calculate magnitude of gravity vector
	gravity_magnitude = GRAVITATIONAL_CONSTANT * game_object_array[cause_object_index].mass / magnitude_vector(&radius_vector);
	//calculate direction of gravity vector
	normalise_vector(&radius_vector);
	multiply_vector(&radius_vector, gravity_magnitude);
	//return radius unit vector multiplied by gravity magnitude
	return radius_vector;
}
//updates the object's acceleration based on the gravity of the objects around it
void update_acceleration(uint32_t object_index)
{
	int i;
	struct vector2d gravity_vector;
	game_object_array[object_index].acceleration.x = 0.0;
	game_object_array[object_index].acceleration.y = 0.0;
	//game_object_array[object_index].acceleration = {0.0,0.0};

	//sum all forces of gravity on the object to get the acceleration
	for (i = 0; i < object_index; i++)
	{
		gravity_vector = calculate_gravity(object_index, i);
		add_vector(&(game_object_array[object_index].acceleration), &gravity_vector);
	}
	//skip the object itself
	for (i = object_index + 1; i < game_object_counter; i++)
	{
		gravity_vector = calculate_gravity(object_index, i);
		add_vector(&(game_object_array[object_index].acceleration), &gravity_vector);
	}
}
//updates the object's velocity based on its current acceleration
void update_velocity(uint32_t object_index)
{
	//add acceleration directly
	add_vector(&(game_object_array[object_index].velocity), &(game_object_array[object_index].acceleration));
}
//displacement calc is broken in some way?
void update_displacement(uint32_t object_index)
{
	//add velocity directly
	add_vector(&(game_object_array[object_index].displacement), &(game_object_array[object_index].velocity));
}
void update_sprite(uint32_t object_index)
{
	//x and y positions for sprites are top, left corner, but for objects are middle
	int x_position;
	int y_position;
	//x_position = (int)round(game_object_array[object_index].displacement.x - check_width(game_object_array[object_index].sprite_index) / 2.0);
	//y_position = (int)round(game_object_array[object_index].displacement.y - check_height(game_object_array[object_index].sprite_index) / 2.0);
	x_position = (int)(game_object_array[object_index].displacement.x + .5);
	y_position = (int)(game_object_array[object_index].displacement.y + .5);
	// if (x_position != check_x_pos(game_object_array[object_index].sprite_index) || y_position != check_y_pos(game_object_array[object_index].sprite_index))
	// {
	// 	clear_sprite(game_object_array[object_index].sprite_index);
	// 	update_sprite_position(game_object_array[object_index].sprite_index, x_position, y_position);
	// }
}
void update_objects(void)
{
	int i;
	//update accelerations
	for (i = 0; i < game_object_counter; i++)
	{
		if (game_object_array[i].movable)
		{
			//update acceleration of this object
			update_acceleration(i);
		}
	}
	//update velocities
	for (i = 0; i < game_object_counter; i++)
	{
		if (game_object_array[i].movable)
		{
			//update the velocity of this object
			update_velocity(i);
		}
	}
	// loops through and adds the rocket's inverse velocity to all other objects, makes rocket appear to sit still
	// rocket is game object index 0
	struct vector2d inverseRocketV = game_object_array[0].velocity;
	multiply_vector(&inverseRocketV, -1);
	for (i = 0; i < game_object_counter; i++)
	{
		add_vector(&game_object_array[i].velocity, &inverseRocketV);
	}
	//update displacements and sprite positions
	int rocketX = (int)game_object_array[0].displacement.x;
	int rocketY = (int)game_object_array[0].displacement.y;
	// int rocketX = MAX_X_COORD / 2;
	// int rocketY = MAX_Y_COORD / 2;
	// skips the rocket (object inde), and applies its reverse displacement to all other objects
	for (i = 0; i < game_object_counter; i++)
	{
		if (game_object_array[i].movable)
		{
			//update the displacement of this object
			update_displacement(i);
		}
		// update location to center the rocket
		// places the rocket at the direct center to avoid bouncing:
		//TODO fix this
		// if (i == 0)
		// {
		// 	game_object_array[i].displacement.x = MAX_X_COORD / 2;
		// 	game_object_array[i].displacement.y = MAX_Y_COORD / 2;
		// }
		// else
		// {
		// 	game_object_array[i].displacement.x = ((int)game_object_array[i].displacement.x - rocketX + MAX_X_COORD / 2) & GAME_SPACE_MASK;
		// 	game_object_array[i].displacement.y = ((int)game_object_array[i].displacement.y - rocketY + MAX_Y_COORD / 2) & GAME_SPACE_MASK;
		// }
		// game_object_array[i].displacement.x = ((int)game_object_array[i].displacement.x) & GAME_SPACE_MASK;
		// game_object_array[i].displacement.y = ((int)game_object_array[i].displacement.y) & GAME_SPACE_MASK;
		update_sprite(i);
	}

	// game_object_array[0].displacement.x = MAX_X_COORD / 2;
	// game_object_array[0].displacement.y = MAX_Y_COORD / 2;
}
void initialize_object(uint32_t sprite_index, float scale, float orientation, struct vector2d *displacement, struct vector2d *velocity, struct vector2d *acceleration, float mass, uint8_t movable)
{
	//assign to sprite
	game_object_array[game_object_counter].sprite_index = sprite_index;
	//assign the scale and orientation fields
	game_object_array[game_object_counter].size = scale;
	game_object_array[game_object_counter].orientation = orientation;
	//assign displacement
	game_object_array[game_object_counter].displacement.x = displacement->x;
	game_object_array[game_object_counter].displacement.y = displacement->y;
	//assign velocity
	game_object_array[game_object_counter].velocity.x = velocity->x;
	game_object_array[game_object_counter].velocity.y = velocity->y;
	//assign acceleration
	game_object_array[game_object_counter].acceleration.x = acceleration->x;
	game_object_array[game_object_counter].acceleration.y = acceleration->y;
	//assign mass
	game_object_array[game_object_counter].mass = mass;
	//select if this object should move or not (should the position be updated?) 0 = don't move, 1 = do move
	game_object_array[game_object_counter].movable = movable;
	//increment game object counter
	game_object_counter++;
}

void render_gamestate_to_LCD(void)
{
	for (int i = 0; i < game_object_counter; i++)
	{
		struct GAME_OBJECT *obj = &game_object_array[i];
		// struct vector2d displacement = obj->displacement;
		// displacement.x = displacement.x - game_object_array[0].displacement.x + MAX_X_COORD / 2;
		// displacement.y = displacement.y - game_object_array[0].displacement.y + MAX_Y_COORD / 2;
		draw_entity_to_buffer(obj->sprite_index, obj->displacement, obj->size, obj->orientation);
	}
	buffer_to_LCD();
}

//this function will be called whenever you want to draw a new frame
void update_place_space(void)
{
	//if debug is true, print debug info
	if (debug)
	{
		print_object_values();
	}
	//update the objects
	update_objects();
	//update the display
	render_gamestate_to_LCD();
}

void print_object_values(void)
{
	int i;
	float x, y, v_x, v_y, a_x, a_y, mass;
	for (i = 0; i < game_object_counter; i++)
	{
		x = game_object_array[i].displacement.x;
		y = game_object_array[i].displacement.y;
		v_x = game_object_array[i].velocity.x;
		v_y = game_object_array[i].velocity.y;
		a_x = game_object_array[i].acceleration.x;
		a_y = game_object_array[i].acceleration.y;
		mass = game_object_array[i].mass;
		//sprintf(debug_text, "x:%4.2f y:%4.2f v_x:%4.2f v_y:%4.2f a_x:%4.2f a_y:%4.2f mass:%4.2f", x, y, v_x, v_y, a_x, a_y, mass);
		//sprintf(debug_text, "This is debug text %.2f", mass);
		// sprintf(debug_text, "Object %d:", i);
		// LCD_PutText(0, i * 64, (uint8_t *)debug_text, Magenta, Black);
		// sprintf(debug_text, "x:%.2f y:%.2f v_x:%.2f v_y:%.2f", x, y, v_x, v_y);
		// LCD_PutText(0, i * 64 + 16, (uint8_t *)debug_text, Yellow, Black);
		// sprintf(debug_text, "a_x:%.3f a_y:%.3f mass:%.2f", a_x, a_y, mass);
		// LCD_PutText(0, i * 64 + 32, (uint8_t *)debug_text, Yellow, Black);
		// sprintf(debug_text, "s_x:%d s_y:%d", check_x_pos(game_object_array[i].sprite_index), check_y_pos(game_object_array[i].sprite_index));
		// LCD_PutText(0, i * 64 + 48, (uint8_t *)debug_text, Yellow, Black);
	}
}

void control_input(int x, int y, uint8_t thrust, uint8_t fire)
{
	struct vector2d direction;
	// makes zero the center of the joystick
	// direction.x = x - 128;
	// direction.y = y - 128;
	// float magnitude = magnitude_vector(&direction);
	// if (magnitude > 30)
	// {
	// 	game_object_array[0].orientation = tan(direction.x / direction.y);
	// 	if (direction.y > 0)
	// 	{
	// 		game_object_array[0].orientation += 3.14159;
	// 	}
	// }
	x -= 128;
	// allow some dead spot in middle, outside of that range rotate it
	if (x > 30 || x < -30)
	{
		game_object_array[0].orientation += x * ROCKET_ROTATION_RATE;
	}
	//sprintf(debug_text, "%f", magnitude);
	//buffer_text(10, 10, debug_text);
	sprintf(debug_text, "x: %d y:%d", x, y);
	buffer_text(10, 26, debug_text);
	if (thrust)
	{
		//switches sprite to the rocket with the engine on
		game_object_array[0].sprite_index = ROCKET_FIRE_INDEX;
		// applies thrust to the rocket acceleration
		struct vector2d deltaV;
		//create a vector with the magnitude of the thrust
		deltaV.x = 0;
		deltaV.y = ROCKET_THRUST;
		rotate_vector(&deltaV, game_object_array[0].orientation * 180 / 3.14159);
		add_vector(&game_object_array[0].velocity, &deltaV);
		// TODO change this so it changes the velocity FORWARD
		// struct vector2d velocity = game_object_array[0].velocity;
		// float accelMag = magnitude_vector(&velocity) + ROCKET_THRUST;
		// normalise_vector(&velocity);
		// multiply_vector(&velocity, accelMag);
		// game_object_array[0].velocity = velocity;
	}
	else
	{
		game_object_array[0].sprite_index = ROCKET_INDEX;
	}
}

void start_game(void)
{
	struct vector2d displacement_0, velocity_0, acceleration_0, displacement_1, velocity_1, acceleration_1,
		displacement_2, velocity_2, acceleration_2;
	float mass_0, mass_1, mass_2;
	uint8_t movable_0, movable_1, movable_2;
	game_object_counter = 0;
	//astroid object 0
	displacement_0.x = 10.4;
	displacement_0.y = 20.4;
	velocity_0.x = -0.1;
	velocity_0.y = -0.1;
	acceleration_0.x = 0.0;
	acceleration_0.y = 0.0;
	mass_0 = .01;
	movable_0 = 1;
	//astroid object 1
	displacement_1.x = 60.5;
	displacement_1.y = 30.5;
	velocity_1.x = .1;
	velocity_1.y = .1;
	acceleration_1.x = 0.0;
	acceleration_1.y = 0.0;
	mass_1 = .1;
	movable_1 = 1;
	//astroid object 2 (unmovable
	displacement_2.x = MAX_X_COORD / 2;
	displacement_2.y = MAX_Y_COORD / 2;
	velocity_2.x = 0.0;
	velocity_2.y = 0.0;
	acceleration_2.x = 0.0;
	acceleration_2.y = 0.0;
	mass_2 = .1;
	movable_2 = 0;

	//initialize the display engine
	//initialize_display_engine();
	//set visibilities for two asteroids
	// update_sprite_visibility(0, 1);
	// update_sprite_visibility(1, 1);
	// update_sprite_visibility(2, 1);
	//initialize three asteroid objects

	//initialize them using the initialization function
	// initialize_object(0, &displacement_0, &velocity_0, &acceleration_0, mass_0, movable_0);
	// initialize_object(1, &displacement_1, &velocity_1, &acceleration_1, mass_1, movable_1);
	// initialize_object(2, &displacement_2, &velocity_2, &acceleration_2, mass_2, movable_2);
	//ready
}