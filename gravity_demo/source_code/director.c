#include "LPC17xx.h"
#include "GLCD.h"
#include "director.h"
#include "display_engine.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "sound.h"
#include "vector.h"
//game variables
//game object array
struct GAME_OBJECT game_object_array[GAME_OBJECT_NUM];
uint32_t game_object_counter = 0;
//vector used for summing


//functions
//calculate acceleration due to gravity -- NOT FORCE
struct vector2d calculate_gravity(uint32_t affected_object_index, uint32_t cause_object_index)
{
	struct vector2d radius_vector;
	struct vector2d gravity_vector;
	float gravity_magnitude;
	float radius_magnitude;
	radius_vector = 
	{
		game_object_array[cause_object_index].displacement.x - game_object_array[affected_object_index].displacement.x,
		game_object_array[cause_object_index].displacement.y - game_object_array[affected_object_index].displacement.y
	};
	//calculate magnitude of gravity vector
	gravity_magnitude = GRAVITATIONAL_CONSTANT * game_object_array[cause_object_index].mass / magnitude_vector(&radius_vector);
	//calculate direction of gravity vector
	gravity_vector = gravity_magnitude * normalise_vector(&radius_vector);
	return gravity_vector;
}
//updates the object's acceleration based on the gravity of the objects around it
void update_acceleration(uint32_t object_index)
{
	int i;
	game_object_array[object_index].acceleration = {0.0,0.0};
	
	//sum all forces of gravity on the object to get the acceleration
	for (i = 0; i < object_index; i++)
	{
		add_vector(&(game_object_array[object_index].acceleration), &(calculate_gravity(object_index, i)));
	}
	//skip the object itself
	for (i = object_index + 1; i < game_object_counter; i++)
	{
		add_vector(&(game_object_array[object_index].acceleration), &(calculate_gravity(object_index, i)));
	}
}
//updates the object's velocity based on its current acceleration
void update_velocity(uint32_t object_index)
{
	//add acceleration directly
	add_vector(&(game_object_array[object_index].velocity), &(game_object_array[object_index].acceleration));
}
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
	x_position = round(game_object_array[object_index].displacement.x - check_width(game_object_array[object_index].sprite_index) / 2.0);
	y_position = round(game_object_array[object_index].displacement.y - check_height(game_object_array[object_index].sprite_index) / 2.0);
	update_sprite_position(game_object_array[object_index].sprite_index, x_position, y_position);
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
	//update displacements and sprite positions
	for (i = 0; i < game_object_counter; i++)
	{
		if (game_object_array[i].movable)
		{
			//update the displacement of this object
			update_displacement(i);
			update_sprite(i);
		}
	}
}
void initialize_object(uint32_t sprite_index, struct vector2d* displacement, struct vector2d* velocity, struct vector2d* acceleration, uint32_t mass, uint8_t movable)
{
	//assign to sprite
	game_object_array[game_object_counter].sprite_index = sprite_index;
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
//this function will be called whenever you want to draw a new frame
void update_place_space(void)
{
	//update the objects
	update_objects();
	//update the display
	update_display();
}
void start_game(void)
{
	
	//initialize the display engine
	initialize_display_engine();
	//set visibilities for two asteroids
	update_sprite_visibility(0, 1);
	update_sprite_visibility(1, 1);
	//initialize two asteroid objects
	//astroid object 0
	struct vector2d displacement_0 = {30.5, 20.5};
	struct vector2d velocity_0 = {.1, .1};
	struct vector2d acceleration_0 = {0, 0};
	float mass_0 = 1.5;
	uint8_t movable_0 = 1;
	//astroid object 1
	struct vector2d displacement_1 = {60.5, 40.5};
	struct vector2d velocity_1 = {.1, .1};
	struct vector2d acceleration_1 = {0, 0};
	float mass_1 = 0.7;
	uint8_t movable_1 = 1;
	//initialize them using the initialization function
	initialize_object(0, &displacement_0, &velocity_0, &acceleration_0, mass_0, movable_0);
	initialize_object(1, &displacement_1, &velocity_1, &acceleration_1, mass_1, movable_1);
	//ready
}