// COEN 4720
// Project
// Gravitoids: Asteroids with Extra Physics and Multiplayer
// Brendan Nenninger, Kassie Povinelli, Carl Sustar
//
// director.c
// handles all gameplay
// gravitational physics, collisions, bullet firing, thrust,
// game score, applying player input, etc

#include "director.h"
#include "LPC17xx.h"
#include "vector.h"
#include "vector_render_engine.h"
#include "framebuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846264

//rocket parameters
#define ROCKET_SCALE 1.0
#define ROCKET_MASS 1.0
#define ROCKET_THRUST 0.5
#define ROCKET_ROTATION_RATE 0.003
#define ROCKET_VELOCITY_DAMPING_FACTOR 0.99
#define ROCKET_DEBRIS_MAX_ADDITIONAL_V 0.1
#define ROCKET_DEBRIS_LIFESPAN_MIN 15
#define ROCKET_DEBRIS_LIFESPAN_MAX 45
#define ROCKET_DEBRIS_MAX_ROTATION_RATE 0.1
// bullet parameters
#define BULLET_VELOCITY 10
#define BULLET_ROTATION_RATE 0.5
#define BULLET_LIFESPAN (180 / (int)BULLET_VELOCITY)
// asteroid parameters
#define ASTEROID_MAX_INIT_VELOCITY 4.0
#define ASTEROID_LARGE_MAX_ROTATION_RATE 0.2
#define ASTEROID_LARGE_MAX_NUM 6
#define ASTEROID_SPAWN_COUNTER_MAX 90 // delay of 90 game cycles
#define ASTEROID_SPAWN_COUNTER_MIN 30 // delay of 30 game cycles
// black hole parameters
#define BLACK_HOLE_SCALE 3
#define BLACK_HOLE_MASS 5
#define BLACK_HOLE_MAX_ROTATION_RATE 0.01
#define BLACK_HOLE_MAX_NUMBER 9
#define BLACK_HOLE_SPAWN_BATCH_NUM 3

// game parameters
#define MAX_LIVES 3
#define GAME_SPACE_MAX_X 500
#define GAME_SPACE_MAX_Y 500
#define EXPLODING_GAME_CYCLES 40
#define RESPAWNING_GAME_CYCLES 60

// asteroid parameters, ordered so the object_type enums for asteroids can be used as the indexes
int asteroid_kill_score[] = {30, 100, 100};
int asteroid_masses[] = {4.0, 2.0, 1.0};
float asteroid_sizes[] = {8.0, 6.0, 3.0};

//game variables
uint32_t score = 0;
uint8_t lives = 3;
volatile game_state state = ALIVE;
uint32_t asteroid_spawn_counter;
// tracks the number of game cycles left in a specific mode
uint16_t mode_countdown = RESPAWNING_GAME_CYCLES;

// game object tracking
// game object array
// NOTE: the rocket is the first element of this array, always
struct GAME_OBJECT gravity_object_array[GAME_OBJECT_NUM];
uint32_t gravity_object_counter = 0;
// tracks the number of large asteroids, used to limit the number so
// as to limit the total number of possible child asteroids to prevent
// gravity object array overflow
uint32_t large_asteroid_counter = 0;
uint32_t black_hole_counter = 0;
// array to store bullets
struct GAME_OBJECT bullet_array[BULLET_NUM];
uint32_t bullet_counter = 0;
// array to store particles
struct GAME_OBJECT particle_array[PARTICLE_NUM];
uint32_t particle_counter = 0;
// array to store stars for the starfield
struct GAME_OBJECT star_array[STAR_NUM];
uint32_t star_counter = 0;
char debug_text[32];
uint8_t debug = 0;
uint8_t fire_button_released = 1;

//functions
struct vector2d random_vector(float magnitude);
struct vector2d calculate_gravity(uint32_t affected_object_index, uint32_t cause_object_index);
void update_acceleration(uint32_t object_index);
void update_all_accelerations(void);
void update_velocity(uint32_t object_index);
void update_all_velocities(void);
void update_displacement(struct GAME_OBJECT *obj);
void update_all_displacements(struct GAME_OBJECT *arr, int n);
void center_around_rocket(struct GAME_OBJECT *arr, int n, struct vector2d rocketDisplacement);
void rotate_objects(struct GAME_OBJECT *arr, int n);
void wrap_around_gamespace(struct GAME_OBJECT *arr, int n);
void set_state_exploding();
void kill_game_object(struct GAME_OBJECT *arr, int n, int index);
int kill_outside_gamespace(struct GAME_OBJECT *arr, int n);
int decay_objects(struct GAME_OBJECT *arr, int n);
void kill_asteroid(int index);
void kill_rocket();
int check_collisions(struct GAME_OBJECT *collisionObject, struct GAME_OBJECT *arr, int n);
void check_bullet_asteroid_collisions();
void generate_fresh_asteroids();
void spawn_starfield();
void spawn_rocket_particle(sprite_index sprite);
void fire_bullet();
void update_objects(void);
void add_array_to_display_buffer(struct GAME_OBJECT *arr, int n);
void display_lives();
void render_gamestate_to_LCD(void);
void game_state_handler();

void start_game(void)
{
	// initializes all the game counters
	lives = MAX_LIVES;
	score = 0;
	gravity_object_counter = 0;
	black_hole_counter = 0;
	large_asteroid_counter = 0;
	bullet_counter = 0;
	particle_counter = 0;
	star_counter = 0;

	// spawns in the rocket
	spawn_rocket();
	gravity_object_counter++;

	// spawns in the starfield
	spawn_starfield();

	// sets the state to respawning so the player will be temporarily immune to damage and make it blink
	state = RESPAWNING;

	// sets the timer for the first asteroid to spawn
	// was previously random, but this ensures that an asteroid spawns in quickly
	asteroid_spawn_counter = ASTEROID_SPAWN_COUNTER_MIN;
}

// returns the current score of the game
int get_score()
{
	return score;
}

// returns the current number of lives of the player
int get_lives()
{
	return lives;
}

// returns the maximum possible number of lives of the player
int get_max_lives()
{
	return MAX_LIVES;
}

// returns whether the game is over or not. 1 if game is over, 0 otherwise
uint8_t is_game_over()
{
	return state == GAME_OVER;
}

// generates a random vector with the specified magnitude
struct vector2d random_vector(float magnitude)
{
	// creates a vector with the given magnitude and then rotates it by a random angle
	struct vector2d vec = {.x = magnitude, .y = 0};
	rotate_vector(&vec, ((float)rand()) / RAND_MAX * 360);
	return vec;
}

//calculate acceleration due to gravity -- NOT FORCE
struct vector2d calculate_gravity(uint32_t affected_object_index, uint32_t cause_object_index)
{
	struct vector2d radius_vector;
	struct vector2d gravity_vector;
	float gravity_magnitude;
	float radius_magnitude;
	radius_vector.x = gravity_object_array[cause_object_index].displacement.x - gravity_object_array[affected_object_index].displacement.x;
	radius_vector.y = gravity_object_array[cause_object_index].displacement.y - gravity_object_array[affected_object_index].displacement.y;
	// calculate magnitude of gravity vector
	// Note: gravity equation would properly square the magnitude of the radius
	// We elected to not implement it this way, as not squaring it led to better gameplay
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

// updates the acceleration of all game objects in the game object array
void update_all_accelerations(void)
{
	//only update rocket acceleration if it is alive
	if (state == ALIVE)
	{
		update_acceleration(0);
	}
	// always update the gravity for the rest of the gravity objects
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
			//damp the velocity to keep things from getting up to wildly high speeds
			multiply_vector(&gravity_object_array[i].velocity, ROCKET_VELOCITY_DAMPING_FACTOR);
		}
	}
}

// updates the displacement for the passed game object
void update_displacement(struct GAME_OBJECT *obj)
{
	//add velocity directly to the displacement
	add_vector(&(obj->displacement), &(obj->velocity));
}

// updates the displacements for all objects in the passed array
void update_all_displacements(struct GAME_OBJECT *arr, int n)
{
	for (int i = 0; i < n; i++)
	{
		// only move objects that are movable
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

// increments the orientation of each object in the passed array by its rotation rate
void rotate_objects(struct GAME_OBJECT *arr, int n)
{
	for (int i = 0; i < n; i++)
	{
		arr[i].orientation += arr[i].rotation_rate;
	}
}

// wraps the game objects around the game space
void wrap_around_gamespace(struct GAME_OBJECT *arr, int n)
{
	for (int i = 0; i < n; i++)
	{
		// wrap around in x dimension
		if (arr[i].displacement.x > GAME_SPACE_MAX_X)
		{
			arr[i].displacement.x -= GAME_SPACE_MAX_X * 2;
		}
		else if (arr[i].displacement.x < -GAME_SPACE_MAX_X)
		{
			arr[i].displacement.x += GAME_SPACE_MAX_X * 2;
		}
		// wrap around in y dimension
		if (arr[i].displacement.y > GAME_SPACE_MAX_Y)
		{
			arr[i].displacement.y -= GAME_SPACE_MAX_Y * 2;
		}
		else if (arr[i].displacement.y < -GAME_SPACE_MAX_Y)
		{
			arr[i].displacement.y += GAME_SPACE_MAX_Y * 2;
		}
	}
}

// sets the rocket into exploding state
// called when the rocket dies
// does nothing but set the mode, does not kill rocket, decrement lives, etc
void set_state_exploding()
{
	state = EXPLODING;
	mode_countdown = EXPLODING_GAME_CYCLES;
}

// kills the game object at the passed index in the passed array
// does not decrement counter for that array, this must be handled independently
void kill_game_object(struct GAME_OBJECT *arr, int n, int index)
{
	// shifts each higher element in the array down to fill the slot of the killed object
	for (int i = index + 1; i < n; i++)
	{
		arr[i - 1] = arr[i];
	}
}

// kills objects in the passed array that are outside the gamespace
// returns the number of objects killed so that they can be subtracted from the appropriate counter
int kill_outside_gamespace(struct GAME_OBJECT *arr, int n)
{
	int numKilled = 0;
	for (int i = 0; i < n; i++)
	{
		// checks if the location of the object is greater than the maximum coordinates
		// (or the opposite of the maximum coordinates, on the negative side)
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

// kills objects in the passed array that have passed through their lifespan, if they are set to decay
// returns the number of objects killed so that they can be subtracted from the appropriate counter
int decay_objects(struct GAME_OBJECT *arr, int n)
{
	int numDecayed = 0;
	for (int i = 0; i < n; i++)
	{
		// only looks at items with lifespans greater than zero, so that -1 and 0 both code for non-decaying objects
		if (arr[i].lifespan > 0)
		{
			arr[i].lifespan--;
			// only checks if the lifespan has been zeroed out after checking it was greater than zero
			// allows this field to be ignored for objects without a lifespan
			if (arr[i].lifespan == 0)
			{
				kill_game_object(arr, n, i);
				numDecayed++;
				// decrements counter and max to reflect shifted array
				i--;
				n--;
			}
		}
	}
	return numDecayed;
}

// kills the asteroid in the gravity object array at the passed index,
// increments the score appropriately, and spawns smaller asteroids if needed
void kill_asteroid(int index)
{
	// store needed data from the asteroid before killing
	object_type type = gravity_object_array[index].type;
	struct vector2d displacement = gravity_object_array[index].displacement;
	struct vector2d velocity = gravity_object_array[index].velocity;
	// kill the old asteroid
	kill_game_object(gravity_object_array, gravity_object_counter, index);
	gravity_object_counter--;
	// increment the score based on the type of asteroid killed
	score += asteroid_kill_score[type];
	// spawn new asteroids of one size smaller
	if (type != SMALL_ASTEROID)
	{
		// spawn two child asteroids for each parent
		for (int i = 0; i < 2; i++)
		{
			// adds a random displacement offset to prevent gravity problems
			struct vector2d offset = random_vector(15);
			add_vector(&offset, &displacement);
			// adds a random bit of velocity to push the child asteroids around
			struct vector2d additional_velocity = random_vector(ASTEROID_MAX_INIT_VELOCITY / 2);
			add_vector(&additional_velocity, &velocity);
			// spawns the asteroid
			spawn_asteroid(type + 1, offset, additional_velocity);
		}
	}
	// if the asteroid killed was a large asteroid, decrement the large asteroid counter
	// enables new large asteroids to be spawned without number becoming excessive
	if (type == LARGE_ASTEROID)
	{
		large_asteroid_counter--;
	}
}

// kills the rocket and spawns the rocket debris particles
// also decrements lives
void kill_rocket()
{
	// zeroes out the rocket acceleration
	gravity_object_array[0].acceleration = (struct vector2d){.x = 0, .y = 0};
	// spawn particles
	spawn_rocket_particle(ROCKET_EXPLODE_1_INDEX);
	spawn_rocket_particle(ROCKET_EXPLODE_2_INDEX);
	spawn_rocket_particle(ROCKET_EXPLODE_3_INDEX);
	// hide the rocket
	gravity_object_array[0].visible = 0;
	lives--;
}

// checks if the passed collision object has collided with any item within the passed array of game objects
// checks based on radius alone
// returns -1 if no collision
int check_collisions(struct GAME_OBJECT *collisionObject, struct GAME_OBJECT *arr, int n)
{
	float collisionRadius = get_sprite_radius(collisionObject->sprite_index) * collisionObject->size;
	struct vector2d collisionDisplacement = collisionObject->displacement;
	for (int i = 0; i < n; i++)
	{
		float otherRadius = get_sprite_radius(arr[i].sprite_index) * arr[i].size;
		// subtract array object's displacement from the collision object's displacement to find the distance
		struct vector2d otherDisplacement = arr[i].displacement;
		multiply_vector(&otherDisplacement, -1.0);
		add_vector(&otherDisplacement, &collisionDisplacement);
		float distance = magnitude_vector(&otherDisplacement);
		// return current index if collision, continue otherwise
		// checks that the distance between the objects is less than the sum of the radiuses to detect collision
		if (distance < collisionRadius + otherRadius)
		{
			return i;
		}
	}
	return -1;
}

// checks if there have been any collisions between bullets and asteroids
void check_bullet_asteroid_collisions()
{
	// stores the current number of gravity objects, as it could change but we don't care about new objects
	int n = gravity_object_counter;
	// loops through all the gravity objects except the first (which is the rocket)
	for (int i = 1; i < n; i++)
	{
		// only checks destructible objects
		// allows black holes to be indestructible
		if (!gravity_object_array[i].indestructible)
		{
			// checks for collisions and gets the index of the bullet that collided
			int bulletCollisionIndex = check_collisions(&gravity_object_array[i], bullet_array, bullet_counter);
			if (bulletCollisionIndex > -1)
			{
				//kill the bullet
				kill_game_object(bullet_array, bullet_counter, bulletCollisionIndex);
				bullet_counter--;
				//kill the asteroid and spawn more if needed
				kill_asteroid(i);
				n--;
				// as the number of game objects has decreased, decrement i to keep the for loop valid
				// don't need to worry about any possible new asteroids, they are at the end of the array
				i--;
			}
		}
	}
}

// resets the rocket into its spawning state, with zero velocity and acceleration
// note: does not provide any incrementation of gravity_object_counter
// as this is only needed in some uses of spawn_rocket()
void spawn_rocket()
{
	gravity_object_array[0] = (struct GAME_OBJECT){
		.sprite_index = ROCKET_INDEX,
		.size = ROCKET_SCALE,
		.orientation = 0.0,
		.rotation_rate = 0.0,
		.displacement = (struct vector2d){.x = 0.0, .y = 0.0},
		.velocity = (struct vector2d){.x = 0.0, .y = 0.0},
		.acceleration = (struct vector2d){.x = 0.0, .y = 0.0},
		.mass = ROCKET_MASS,
		.movable = 1,
		.visible = 1,
		.lifespan = -1,
		.type = ROCKET};
}

// spawns in the number of black holes specified in constants at random locations
// does not spawn black holes if the maximum number of black holes has been reached
uint8_t spawn_black_holes()
{
	// if the game is finished, refuse to spawn
	if (state == GAME_OVER)
	{
		return 0;
	}
	int numSpawned = 0;
	for (int i = 0; i < BLACK_HOLE_SPAWN_BATCH_NUM; i++)
	{
		// check that there are black hole slots and gravity object slots available before spawning
		if (((int)black_hole_counter < BLACK_HOLE_MAX_NUMBER) && ((int)gravity_object_counter < GAME_OBJECT_NUM))
		{
			gravity_object_array[gravity_object_counter] = (struct GAME_OBJECT){
				.sprite_index = BLACK_HOLE_INDEX,
				.size = BLACK_HOLE_SCALE,
				// randomize the starting orientation and rotation rate
				.orientation = ((float)rand()) / RAND_MAX * 2 * PI,
				.rotation_rate = (((float)rand()) / RAND_MAX - 0.5) * BLACK_HOLE_MAX_ROTATION_RATE,
				// randomize the displacement at a random location outside of the screen
				.displacement = random_vector(((float)rand() / RAND_MAX) * (GAME_SPACE_MAX_X - 240) + 240),
				.velocity = (struct vector2d){.x = 0, .y = 0},
				.acceleration = (struct vector2d){.x = 0, .y = 0},
				.mass = BLACK_HOLE_MASS,
				.movable = 0,
				.visible = 1,
				.lifespan = -1,
				.indestructible = 1};
			gravity_object_counter++;
			black_hole_counter++;
			numSpawned++;
		}
	}
	return numSpawned;
}

// spawns in a new asteroid of the specified type with the specified displacement and velocity
// handles incrementing of gravity object counter and large asteroid counter
void spawn_asteroid(object_type asteroid_type, struct vector2d displacement, struct vector2d velocity)
{
	if (gravity_object_counter < GAME_OBJECT_NUM)
	{
		gravity_object_array[gravity_object_counter] = (struct GAME_OBJECT){
			.sprite_index = ASTEROID_INDEX,
			.size = asteroid_sizes[asteroid_type],
			// spawns at random orientation and rotation rate
			.orientation = ((float)rand()) / RAND_MAX * 2 * PI,
			.rotation_rate = (((float)rand()) / RAND_MAX - 0.5) * ASTEROID_LARGE_MAX_ROTATION_RATE * asteroid_type,
			.displacement = displacement,
			.velocity = velocity,
			.acceleration = (struct vector2d){.x = 0, .y = 0},
			.mass = asteroid_masses[asteroid_type],
			.movable = 1,
			.visible = 1,
			.lifespan = -1,
			.type = asteroid_type};
		gravity_object_counter++;
		if (asteroid_type == LARGE_ASTEROID)
		{
			large_asteroid_counter++;
		}
	}
}

// spawns a large asteroid at a random location just outside the screen if the asteroid spawn counter has reached zero
void generate_fresh_asteroids()
{
	asteroid_spawn_counter--;
	if (asteroid_spawn_counter == 0)
	{
		// if there are large asteroid spots free
		// slots free are equal to the number of game objects divided by 4 due to the possibility of generating 4 more game objects
		// 1 is subtracted for the rocket
		if (gravity_object_counter < ASTEROID_LARGE_MAX_NUM)
		{
			// spawn an asteroid with random velocity just off screen
			struct vector2d displacement = random_vector(240);
			struct vector2d velocity = random_vector(((float)rand()) / RAND_MAX * ASTEROID_MAX_INIT_VELOCITY);
			spawn_asteroid(LARGE_ASTEROID, displacement, velocity);
		}
		// always reset the timer so asteroids will spawn as normal when slots are available
		asteroid_spawn_counter = rand() % (ASTEROID_SPAWN_COUNTER_MAX - ASTEROID_SPAWN_COUNTER_MIN) + ASTEROID_SPAWN_COUNTER_MIN;
	}
}

// spawns in the background stars
void spawn_starfield()
{
	struct vector2d displacement = {.x = 0, .y = 0};
	struct GAME_OBJECT obj =
		{
			.sprite_index = STAR_INDEX,
			.size = 1.0,
			.orientation = 0.0,
			.rotation_rate = 0.0,
			.displacement = displacement,
			.velocity = (struct vector2d){.x = 0.0, .y = 0.0},
			.acceleration = (struct vector2d){.x = 0.0, .y = 0.0},
			.mass = 0,
			.movable = 0,
			.visible = 1,
			.lifespan = -1,
			.type = STAR};
	// place each star in a random location
	for (int i = 0; i < STAR_NUM; i++)
	{
		// generate a random location
		displacement.x = rand() % (2 * GAME_SPACE_MAX_X) - GAME_SPACE_MAX_X;
		displacement.y = rand() % (2 * GAME_SPACE_MAX_Y) - GAME_SPACE_MAX_Y;
		obj.displacement = displacement;
		star_array[i] = obj;
		star_counter++;
	}
}

// spawns debris particles for the exploding rocket
// sprite: the index of the particle sprite for this object
void spawn_rocket_particle(sprite_index sprite)
{
	// only spawn particle if particle slots are available
	if (particle_counter < PARTICLE_NUM)
	{
		// start the particle object with a copy of the rocket, as the velocity and orientation should remain the same
		// this is likely an inefficient way to do this, it would likely be better to just copy the velocity and orientation rather than the entire struct
		particle_array[particle_counter] = gravity_object_array[0];
		// give the particle a random lifespan so the debris field disappears gradually
		particle_array[particle_counter].lifespan = (uint16_t)(((float)rand()) / RAND_MAX * (ROCKET_DEBRIS_LIFESPAN_MAX - ROCKET_DEBRIS_LIFESPAN_MIN) + ROCKET_DEBRIS_LIFESPAN_MIN);
		// set a small random rotation rate
		particle_array[particle_counter].rotation_rate = (((float)rand()) / RAND_MAX - 0.5) * ROCKET_DEBRIS_MAX_ROTATION_RATE;
		// add a little bit of random velocity so the particles move apart
		struct vector2d additionalVelocity;
		additionalVelocity.x = ((float)rand()) / RAND_MAX * ROCKET_DEBRIS_MAX_ADDITIONAL_V;
		rotate_vector(&additionalVelocity, ((float)rand()) / RAND_MAX * 360);
		add_vector(&particle_array[particle_counter].velocity, &additionalVelocity);
		particle_array[particle_counter].sprite_index = sprite;
		particle_counter++;
	}
}

// fires a bullet from the rocket heading in the orientation of the rocket
void fire_bullet()
{
	// only spawn a bullet if free bullet slots are available
	if (bullet_counter < BULLET_NUM)
	{
		// start the bullet at the nose of the rocket
		struct vector2d displacement;
		displacement.x = 0;
		displacement.y = -15;
		rotate_vector(&displacement, gravity_object_array[0].orientation * 180 / PI);
		add_vector(&displacement, &gravity_object_array[0].displacement);
		// start the bullet with a forward velocity
		struct vector2d velocity;
		velocity.x = 0;
		velocity.y = BULLET_VELOCITY;
		rotate_vector(&velocity, gravity_object_array[0].orientation * 180 / PI + 180);
		// adds the velocity to the rocket velocity, helps make behavior more sane when firing while travelling at high velocity
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

// updates all object in the game
// handles gravity, movement, centering of the rocket, collisions, deaths, score, etc
// all operations but rendering, movement, and respawn blink handling
void update_objects(void)
{
	// update the accelerations, velocities, and displacements within the game object array
	// these are the only objects to which gravity is applied
	update_all_accelerations();
	update_all_velocities();
	update_all_displacements(gravity_object_array, gravity_object_counter);
	center_around_rocket(&gravity_object_array[1], gravity_object_counter - 1, gravity_object_array[0].displacement);
	wrap_around_gamespace(&gravity_object_array[1], gravity_object_counter - 1);
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

	// update the starfield
	center_around_rocket(star_array, star_counter, gravity_object_array[0].displacement);
	wrap_around_gamespace(star_array, star_counter);

	//zero out the rocket position
	gravity_object_array[0].displacement.x = 0;
	gravity_object_array[0].displacement.y = 0;
	//damp the rocket velocity

	// check if any bullets hit asteroids
	check_bullet_asteroid_collisions();
	// check for collisions between the rocket and asteroids
	// only check when alive to give a respawn immunity period
	if (state == ALIVE && -1 != check_collisions(&gravity_object_array[0], &gravity_object_array[1], gravity_object_counter - 1))
	{
		kill_rocket();
		set_state_exploding();
		// if no lives are left, declare the game over
		if (lives <= 0)
		{
			state = GAME_OVER;
		}
	}
}

// draws all the game objects in the passed array to the framebuffer
// only adds items to the framebuffer, needs buffer_to_LCD() call afterwards
void add_array_to_display_buffer(struct GAME_OBJECT *arr, int n)
{
	for (int i = 0; i < n; i++)
	{
		struct GAME_OBJECT *obj = arr + i;
		// only render the sprite if the game object is marked as visible
		if (obj->visible)
		{
			draw_entity_to_buffer(obj->sprite_index, obj->displacement, obj->size, obj->orientation);
		}
	}
}

// adds spaceship symbols to the corner of the display to denote the number of lives remaining
// only adds items to the framebuffer, needs buffer_to_LCD() call afterwards
void display_lives()
{
	struct vector2d livesLocation = {
		.x = -MAX_X_COORD / 2 + 10,
		.y = -MAX_Y_COORD / 2 + 34};
	// iterates from left to right, drawing the spaceship symbols for lives
	for (int i = 0; i < lives; i++)
	{
		draw_entity_to_buffer(ROCKET_INDEX, livesLocation, 0.6, 0);
		livesLocation.x += 20;
	}
}

// renders the details of the current game state onto the LCD
void render_gamestate_to_LCD(void)
{
	// places all types of objects into the frame buffer to be shown on the LCD
	add_array_to_display_buffer(gravity_object_array, gravity_object_counter);
	add_array_to_display_buffer(bullet_array, bullet_counter);
	add_array_to_display_buffer(particle_array, particle_counter);
	add_array_to_display_buffer(star_array, star_counter);
	// displays the number of lives remaining
	display_lives();
	// display score info
	sprintf(debug_text, "%d", score);
	buffer_text(5, 5, debug_text);
	buffer_to_LCD();
}

// updates the state of the director to reflect the current mode and any changes that have occurred
void game_state_handler()
{
	// decrement the counter for changing modes
	mode_countdown--;
	// when the counter reaches zero, if the player is still alive, switch mode appropriately
	if (mode_countdown == 0 && lives > 0)
	{
		// if rocket is done respawning, make it alive (and visible, in case it was still invisible)
		if (state == RESPAWNING)
		{
			state = ALIVE;
			gravity_object_array[0].visible = 1;
		}
		// if rocket is done exploding, start it on respawning
		else if (state == EXPLODING)
		{
			state = RESPAWNING;
			spawn_rocket();
			mode_countdown = RESPAWNING_GAME_CYCLES;
			// does not handle anything for game being over, because the game is marked as being over in update_objects()
		}
	}
	// is the game is now over, display the game over message
	else if (state == GAME_OVER)
	{
		buffer_text(160 - 10 * 4, 120 - 8, "GAME OVER");
	}
	// handle blinking in respawning state
	if (state == RESPAWNING && mode_countdown % 5 == 0)
	{
		gravity_object_array[0].visible = !gravity_object_array[0].visible;
	}
}

// this function will be called whenever you want to draw a new frame
// handles all updating of gravity physics, object states, and collisions
void update_game_space(void)
{
	//update the objects
	update_objects();
	//handles any necessary processes based on the game state
	game_state_handler();
	// spawn asteroids if appropriate (checks done in generate_fresh_asteroids)
	generate_fresh_asteroids();
	//update the display
	render_gamestate_to_LCD();
}

// applies control inputs to the in-game rocket
void control_input(int x, int y, uint8_t thrust, uint8_t fire)
{
	// only do something in states where the controls should do anything, those being fully alive and immune respawning
	if (state == ALIVE || state == RESPAWNING)
	{
		// handle joystick input
		// center the input around 0
		x -= 128;
		// allow some dead spot in middle, outside of that range rotate it
		if (x > 30 || x < -30)
		{
			gravity_object_array[0].orientation += x * ROCKET_ROTATION_RATE;
		}

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
			rotate_vector(&deltaV, gravity_object_array[0].orientation * 180 / PI + 180);
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
