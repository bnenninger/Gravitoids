#include "LPC17xx.h"
#include <stdio.h>
#include "GLCD.h"
#include "director.h"
//#include "display_engine.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "sound.h"
#include "vector.h"
#include "string.h"
#include "framebuffer.h"

#define PI 3.14159265358979323846264

#define ROCKET_THRUST 0.25
#define ROCKET_ROTATION_RATE 0.002
#define ROCKET_VELOCITY_DAMPING_FACTOR 0.99
#define ROCKET_DEBRIS_MAX_ADDITIONAL_V 0.1
#define ROCKET_DEBRIS_LIFESPAN_MIN 15
#define ROCKET_DEBRIS_LIFESPAN_MAX 45
#define ROCKET_DEBRIS_MAX_ROTATION_RATE 0.1
#define BULLET_VELOCITY 5
#define BULLET_ROTATION_RATE 0.5
// #define BULLET_LIFESPAN 8
#define BULLET_LIFESPAN (180 / (int)BULLET_VELOCITY)
#define GAME_SPACE_MAX_X 100.0
#define GAME_SPACE_MAX_Y 100.0

int asteroid_kill_score[] = {30, 100, 100};
float asteroid_sizes[] = {8.0, 6.0, 3.0};

//game variables
uint32_t score = 0;
uint8_t lives = 3;
game_state state = ALIVE;
//game object array
struct GAME_OBJECT gravity_object_array[GAME_OBJECT_NUM];
uint32_t gravity_object_counter = 0;
struct GAME_OBJECT bullet_array[BULLET_NUM];
uint32_t bullet_counter = 0;
struct GAME_OBJECT particle_array[PARTICLE_NUM];
uint32_t particle_counter = 0;
char debug_text[64]; //used for debugging
uint8_t debug = 0;
uint8_t fire_button_released = 1;

void kill_game_object(struct GAME_OBJECT *arr, int n, int index);

// checks if the passed collision object has collided with any item within the passed array of game objects
// checks based on radius alone
int check_collisions(struct GAME_OBJECT *collisionObject, struct GAME_OBJECT *arr, int n)
{
	float collisionRadius = get_sprite_radius(collisionObject->sprite_index) * collisionObject->size;
	struct vector2d collisionDisplacement = collisionObject->displacement;
	for (int i = 0; i < n; i++)
	{
		float otherRadius = get_sprite_radius(arr[i].sprite_index) * arr[i].size;
		struct vector2d otherDisplacement = arr[i].displacement;
		multiply_vector(&otherDisplacement, -1.0);
		add_vector(&otherDisplacement, &collisionDisplacement);
		float distance = magnitude_vector(&otherDisplacement);
		// return current index if collision, continue otherwise
		if (distance < collisionRadius + otherRadius)
		{
			return i;
		}
	}
	return -1;
}

int check_bullet_asteroid_collisions()
{
	int asteroidIndex = -1;
	for (int i = 1; i < gravity_object_counter; i++)
	{
		int bulletCollisionIndex = check_collisions(&gravity_object_array[i], bullet_array, bullet_counter);
		if (bulletCollisionIndex > -1)
		{
			//TODO increment score
			score += asteroid_kill_score[gravity_object_array[i].type];
			//kill the bullet
			kill_game_object(bullet_array, bullet_counter, bulletCollisionIndex);
			bullet_counter--;
			//kill the asteroid
			// TODO spawn more if not a tiny asteroid
			kill_game_object(gravity_object_array, gravity_object_counter, i);
			gravity_object_counter--;
			// as the number of game objects has decreased, decrement i to keep the for loop valid
			i--;
		}
	}
}

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
	//	gravity_object_array[cause_object_index].displacement.x - gravity_object_array[affected_object_index].displacement.x,
	//	gravity_object_array[cause_object_index].displacement.y - gravity_object_array[affected_object_index].displacement.y
	//};
	radius_vector.x = gravity_object_array[cause_object_index].displacement.x - gravity_object_array[affected_object_index].displacement.x;
	radius_vector.y = gravity_object_array[cause_object_index].displacement.y - gravity_object_array[affected_object_index].displacement.y;
	//calculate magnitude of gravity vector
	gravity_magnitude = GRAVITATIONAL_CONSTANT * gravity_object_array[cause_object_index].mass / magnitude_vector(&radius_vector);
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
	gravity_object_array[object_index].acceleration.x = 0.0;
	gravity_object_array[object_index].acceleration.y = 0.0;
	//gravity_object_array[object_index].acceleration = {0.0,0.0};

	//sum all forces of gravity on the object to get the acceleration
	for (i = 0; i < object_index; i++)
	{
		gravity_vector = calculate_gravity(object_index, i);
		add_vector(&(gravity_object_array[object_index].acceleration), &gravity_vector);
	}
	//skip the object itself
	for (i = object_index + 1; i < gravity_object_counter; i++)
	{
		gravity_vector = calculate_gravity(object_index, i);
		add_vector(&(gravity_object_array[object_index].acceleration), &gravity_vector);
	}
}

//updates the acceleration of all game objects in the game object array
void update_all_accelerations(void)
{
	//only update rocket acceleration if it is alive
	if (state == ALIVE)
	{
		update_acceleration(0);
	}
	for (int i = 1; i < gravity_object_counter; i++)
	{
		if (gravity_object_array[i].movable)
		{
			//update acceleration of this object
			update_acceleration(i);
		}
	}
}

//updates the object's velocity based on its current acceleration
void update_velocity(uint32_t object_index)
{
	//add acceleration directly
	add_vector(&(gravity_object_array[object_index].velocity), &(gravity_object_array[object_index].acceleration));
}

// updates the velocities of all game objects in the game object array
void update_all_velocities(void)
{
	//update velocities
	for (int i = 0; i < gravity_object_counter; i++)
	{
		if (gravity_object_array[i].movable)
		{
			//update the velocity of this object
			update_velocity(i);
		}
	}
}

// displacement calc is broken in some way?
// i think it's working? Was a false alarm that is wasn't. -Brendan
// void update_displacement(uint32_t object_index)
// {
// 	//add velocity directly
// 	add_vector(&(gravity_object_array[object_index].displacement), &(gravity_object_array[object_index].velocity));
// }

// updates the displacement for the passed game object
void update_displacement(struct GAME_OBJECT *obj)
{
	//add velocity directly
	add_vector(&(obj->displacement), &(obj->velocity));
}

// updates the displacements for all objects in the passed array
void update_all_displacements(struct GAME_OBJECT *arr, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (arr[i].movable)
		{
			update_displacement(arr + i);
		}
	}
}

// updates the displacements for all objects such that the rocket is in the center
void center_around_rocket(struct GAME_OBJECT *arr, int n, struct vector2d rocketDisplacement)
{
	// subtract out the rocket displacement, keeping the rocket at the center
	struct vector2d inverseRocketDisplacement = rocketDisplacement;
	multiply_vector(&inverseRocketDisplacement, -1);
	for (int i = 0; i < n; i++)
	{
		add_vector(&arr[i].displacement, &inverseRocketDisplacement);
	}
}

void rotate_objects(struct GAME_OBJECT *arr, int n)
{
	for (int i = 0; i < n; i++)
	{
		arr[i].orientation += arr[i].rotation_rate;
	}
}

// wraps the game objects around the game space
void wrap_around_gamespace()
{
	for (int i = 0; i < gravity_object_counter; i++)
	{
		// wrap around in x dimension
		if (gravity_object_array[i].displacement.x > GAME_SPACE_MAX_X)
		{
			gravity_object_array[i].displacement.x -= GAME_SPACE_MAX_X * 2;
		}
		else if (gravity_object_array[i].displacement.x < -GAME_SPACE_MAX_X)
		{
			gravity_object_array[i].displacement.x += GAME_SPACE_MAX_X * 2;
		}
		// wrap around in y dimension
		if (gravity_object_array[i].displacement.y > GAME_SPACE_MAX_Y)
		{
			gravity_object_array[i].displacement.y -= GAME_SPACE_MAX_Y * 2;
		}
		else if (gravity_object_array[i].displacement.y < -GAME_SPACE_MAX_Y)
		{
			gravity_object_array[i].displacement.y += GAME_SPACE_MAX_Y * 2;
		}
	}
}

// kills the game object at the passed index in the passed array
void kill_game_object(struct GAME_OBJECT *arr, int n, int index)
{
	for (int i = index + 1; i < n; i++)
	{
		arr[i - 1] = arr[i];
	}
}

// kills objects that are outside the gamespace
// returns the number of objects killed so that they can be subtracted from the appropriate counter
int kill_outside_gamespace(struct GAME_OBJECT *arr, int n)
{
	int numKilled = 0;
	for (int i = 0; i < n; i++)
	{
		// wrap around in x dimension
		if (arr[i].displacement.x > GAME_SPACE_MAX_X ||
			arr[i].displacement.x < -GAME_SPACE_MAX_X ||
			arr[i].displacement.y > GAME_SPACE_MAX_Y ||
			arr[i].displacement.y < -GAME_SPACE_MAX_Y)
		{
			kill_game_object(arr, n, i);
			numKilled++;
			n--;
		}
	}
	return numKilled;
}

void spawn_rocket_particle(sprite_index sprite)
{
	if (particle_counter < PARTICLE_NUM)
	{
		particle_array[particle_counter] = gravity_object_array[0];
		particle_array[particle_counter].lifespan = (uint16_t)(((float)rand()) / RAND_MAX * (ROCKET_DEBRIS_LIFESPAN_MAX - ROCKET_DEBRIS_LIFESPAN_MIN) + ROCKET_DEBRIS_LIFESPAN_MIN);
		particle_array[particle_counter].rotation_rate = (((float)rand()) / RAND_MAX - 0.5) * ROCKET_DEBRIS_MAX_ROTATION_RATE;
		struct vector2d additionalVelocity;
		additionalVelocity.x = ((float)rand()) / RAND_MAX * ROCKET_DEBRIS_MAX_ADDITIONAL_V;
		rotate_vector(&additionalVelocity, ((float)rand()) / RAND_MAX * 360);
		add_vector(&particle_array[particle_counter].velocity, &additionalVelocity);
		particle_array[particle_counter].sprite_index = sprite;
		particle_counter++;
	}
}

void kill_rocket()
{
	struct vector2d zeroVector;
	zeroVector.x = 0;
	zeroVector.y = 0;
	gravity_object_array[0].acceleration = zeroVector;
	// spawn particles
	spawn_rocket_particle(ROCKET_EXPLODE_1_INDEX);
	spawn_rocket_particle(ROCKET_EXPLODE_2_INDEX);
	spawn_rocket_particle(ROCKET_EXPLODE_3_INDEX);
	// hide the rocket
	gravity_object_array[0].visible = 0;
	lives--;
}

// kills objects that have passed through their lifespan, if they are set to decay
// returns the number of objects killed so that they can be subtracted from the appropriate counter
int decay_objects(struct GAME_OBJECT *arr, int n)
{
	int numDecayed = 0;
	for (int i = 0; i < n; i++)
	{
		if (arr[i].lifespan > 0)
		{
			sprintf(debug_text, "check %d", arr[i].lifespan);
			buffer_text(100, i * 16, debug_text);
			arr[i].lifespan--;
			// only checks if the lifespan has been zeroed out after checking it was greater than zero
			// allows this field to be ignored for objects without a lifespan
			if (arr[i].lifespan == 0)
			{
				sprintf(debug_text, "kill %d", i);
				buffer_text(150, 0, debug_text);
				kill_game_object(arr, n, i);
				numDecayed++;
				i--;
				n--;
			}
		}
	}
	return numDecayed;
}

void fire_bullet()
{
	if (bullet_counter < BULLET_NUM)
	{
		struct vector2d displacement;
		displacement.x = 0;
		displacement.y = 15;
		rotate_vector(&displacement, gravity_object_array[0].orientation * 180 / PI);
		add_vector(&displacement, &gravity_object_array[0].displacement);
		struct vector2d velocity;
		velocity.x = 0;
		velocity.y = BULLET_VELOCITY;
		rotate_vector(&velocity, gravity_object_array[0].orientation * 180 / PI);
		// adds the velocity to the rocket velocity, helps make behavior more sane when firing at high velocity
		add_vector(&velocity, &gravity_object_array[0].velocity);
		bullet_array[bullet_counter]
			.displacement = displacement;
		bullet_array[bullet_counter].velocity = velocity;
		bullet_array[bullet_counter].size = 1;
		bullet_array[bullet_counter].sprite_index = BULLET_INDEX;
		bullet_array[bullet_counter].rotation_rate = BULLET_ROTATION_RATE;
		bullet_array[bullet_counter].movable = 1;
		bullet_array[bullet_counter].visible = 1;
		bullet_array[bullet_counter].lifespan = BULLET_LIFESPAN;
		bullet_counter++;
	}
}

void update_objects(void)
{
	// update the accelerations, velocities, and displacements within the game object array
	// these are the only objects to which gravity is applied
	update_all_accelerations();
	update_all_velocities();
	update_all_displacements(gravity_object_array, gravity_object_counter);
	center_around_rocket(&gravity_object_array[1], gravity_object_counter - 1, gravity_object_array[0].displacement);
	wrap_around_gamespace();
	rotate_objects(gravity_object_array, gravity_object_counter);
	// update the bullets
	update_all_displacements(bullet_array, bullet_counter);
	center_around_rocket(bullet_array, bullet_counter, gravity_object_array[0].displacement);
	bullet_counter -= decay_objects(bullet_array, bullet_counter);
	bullet_counter -= kill_outside_gamespace(bullet_array, bullet_counter);
	rotate_objects(bullet_array, bullet_counter);
	// update the particles
	update_all_displacements(particle_array, particle_counter);
	center_around_rocket(particle_array, particle_counter, gravity_object_array[0].displacement);
	rotate_objects(particle_array, particle_counter);
	particle_counter -= decay_objects(particle_array, particle_counter);
	particle_counter -= kill_outside_gamespace(particle_array, particle_counter);
	//zero out the rocket position
	gravity_object_array[0].displacement.x = 0;
	gravity_object_array[0].displacement.y = 0;
	//damp the rocket velocity
	multiply_vector(&gravity_object_array[0].velocity, ROCKET_VELOCITY_DAMPING_FACTOR);
	//check for collisions between the rocket and asteroids
	if (state == ALIVE && -1 != check_collisions(&gravity_object_array[0], &gravity_object_array[1], gravity_object_counter - 1))
	{
		kill_rocket();
		state = EXPLODING;
	}
}

void initialize_object(uint32_t sprite_index, float scale, float orientation, float rotation_rate, struct vector2d *displacement, struct vector2d *velocity, struct vector2d *acceleration, float mass, uint8_t movable)
{
	if (gravity_object_counter < GAME_OBJECT_NUM)
	{
		//assign to sprite
		gravity_object_array[gravity_object_counter].sprite_index = sprite_index;
		//assign the scale and orientation fields
		gravity_object_array[gravity_object_counter].size = scale;
		gravity_object_array[gravity_object_counter].orientation = orientation;
		gravity_object_array[gravity_object_counter].rotation_rate = rotation_rate;
		//assign displacement
		gravity_object_array[gravity_object_counter].displacement.x = displacement->x;
		gravity_object_array[gravity_object_counter].displacement.y = displacement->y;
		//assign velocity
		gravity_object_array[gravity_object_counter].velocity.x = velocity->x;
		gravity_object_array[gravity_object_counter].velocity.y = velocity->y;
		//assign acceleration
		gravity_object_array[gravity_object_counter].acceleration.x = acceleration->x;
		gravity_object_array[gravity_object_counter].acceleration.y = acceleration->y;
		//assign mass
		gravity_object_array[gravity_object_counter].mass = mass;
		//select if this object should move or not (should the position be updated?) 0 = don't move, 1 = do move
		gravity_object_array[gravity_object_counter].movable = movable;
		gravity_object_array[gravity_object_counter].lifespan = -1;
		gravity_object_array[gravity_object_counter].visible = 1;
		//increment game object counter
		gravity_object_counter++;
	}
}

void add_array_to_display_buffer(struct GAME_OBJECT *arr, int n)
{
	for (int i = 0; i < n; i++)
	{

		struct GAME_OBJECT *obj = arr + i;
		if (obj->visible)
		{
			// struct vector2d displacement = obj->displacement;
			// displacement.x = displacement.x - gravity_object_array[0].displacement.x + MAX_X_COORD / 2;
			// displacement.y = displacement.y - gravity_object_array[0].displacement.y + MAX_Y_COORD / 2;
			draw_entity_to_buffer(obj->sprite_index, obj->displacement, obj->size, obj->orientation);
		}
	}
}

void render_gamestate_to_LCD(void)
{
	// for (int i = 0; i < gravity_object_counter; i++)
	// {
	// 	struct GAME_OBJECT *obj = &gravity_object_array[i];
	// 	// struct vector2d displacement = obj->displacement;
	// 	// displacement.x = displacement.x - gravity_object_array[0].displacement.x + MAX_X_COORD / 2;
	// 	// displacement.y = displacement.y - gravity_object_array[0].displacement.y + MAX_Y_COORD / 2;
	// 	draw_entity_to_buffer(obj->sprite_index, obj->displacement, obj->size, obj->orientation);
	// }
	add_array_to_display_buffer(gravity_object_array, gravity_object_counter);
	add_array_to_display_buffer(bullet_array, bullet_counter);
	add_array_to_display_buffer(particle_array, particle_counter);
	buffer_to_LCD();
}

//this function will be called whenever you want to draw a new frame
void update_game_space(void)
{
	//if debug is true, print debug info
	if (debug)
	{
		print_object_values();
	}
	//update the objects
	update_objects();
	int col = check_collisions(&gravity_object_array[0], &gravity_object_array[1], gravity_object_counter - 1);
	sprintf(debug_text, "collision %d", col);
	buffer_text(0, 0, debug_text);
	check_bullet_asteroid_collisions();
	//update the display
	render_gamestate_to_LCD();
}

void print_object_values(void)
{
	int i;
	float x, y, v_x, v_y, a_x, a_y, mass;
	for (i = 0; i < gravity_object_counter; i++)
	{
		x = gravity_object_array[i].displacement.x;
		y = gravity_object_array[i].displacement.y;
		v_x = gravity_object_array[i].velocity.x;
		v_y = gravity_object_array[i].velocity.y;
		a_x = gravity_object_array[i].acceleration.x;
		a_y = gravity_object_array[i].acceleration.y;
		mass = gravity_object_array[i].mass;
		//sprintf(debug_text, "x:%4.2f y:%4.2f v_x:%4.2f v_y:%4.2f a_x:%4.2f a_y:%4.2f mass:%4.2f", x, y, v_x, v_y, a_x, a_y, mass);
		//sprintf(debug_text, "This is debug text %.2f", mass);
		// sprintf(debug_text, "Object %d:", i);
		// LCD_PutText(0, i * 64, (uint8_t *)debug_text, Magenta, Black);
		// sprintf(debug_text, "x:%.2f y:%.2f v_x:%.2f v_y:%.2f", x, y, v_x, v_y);
		// LCD_PutText(0, i * 64 + 16, (uint8_t *)debug_text, Yellow, Black);
		// sprintf(debug_text, "a_x:%.3f a_y:%.3f mass:%.2f", a_x, a_y, mass);
		// LCD_PutText(0, i * 64 + 32, (uint8_t *)debug_text, Yellow, Black);
		// sprintf(debug_text, "s_x:%d s_y:%d", check_x_pos(gravity_object_array[i].sprite_index), check_y_pos(gravity_object_array[i].sprite_index));
		// LCD_PutText(0, i * 64 + 48, (uint8_t *)debug_text, Yellow, Black);
	}
}

// applies control inputs to the in-game rocket
void control_input(int x, int y, uint8_t thrust, uint8_t fire)
{
	//always adjust the direction, even when respawning
	//doesn't do anything when dead because sprite is hidden
	x -= 128;
	// allow some dead spot in middle, outside of that range rotate it
	if (x > 30 || x < -30)
	{
		gravity_object_array[0].orientation += x * ROCKET_ROTATION_RATE;
	}
	//TODO add another case to start the game when in the respawning state
	if (state == ALIVE)
	{
		// apply thrust if the thrust button is pressed
		if (thrust)
		{
			//switches sprite to the rocket with the engine on
			gravity_object_array[0].sprite_index = ROCKET_FIRE_INDEX;
			// applies thrust to the rocket acceleration
			struct vector2d deltaV;
			//create a vector with the magnitude of the thrust
			deltaV.x = 0;
			deltaV.y = ROCKET_THRUST;
			rotate_vector(&deltaV, gravity_object_array[0].orientation * 180 / PI);
			add_vector(&gravity_object_array[0].velocity, &deltaV);
		}
		// otherwise set/switch back to the regular rocket sprite
		else
		{
			gravity_object_array[0].sprite_index = ROCKET_INDEX;
		}
		// fire bullet if the "fire" button is pressed
		if (fire && fire_button_released)
		{
			fire_bullet();
			fire_button_released = 0;
		}
		// reset when the fire button is released, allow for firing again
		else if (!fire)
		{
			fire_button_released = 1;
		}
	}
}

void start_game(void)
{
	struct vector2d displacement_0, velocity_0, acceleration_0, displacement_1, velocity_1, acceleration_1,
		displacement_2, velocity_2, acceleration_2;
	float mass_0, mass_1, mass_2;
	uint8_t movable_0, movable_1, movable_2;
	gravity_object_counter = 0;
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