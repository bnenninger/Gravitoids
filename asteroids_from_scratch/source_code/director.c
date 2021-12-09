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
#include "Serial.h"

#define PI 3.14159265358979323846264

//rocket parameters

#define ROCKET_SCALE 1.0
#define ROCKET_MASS 1.0
#define ROCKET_THRUST 0.25
#define ROCKET_ROTATION_RATE 0.002
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
#define ASTEROID_SPAWN_COUNTER_MAX 90 // delay of 4.5 seconds
#define ASTEROID_SPAWN_COUNTER_MIN 30 // delay of 1.5 seconds
// black hole parameters
#define BLACK_HOLE_SCALE 20
#define BLACK_HOLE_MASS 20
#define BLACK_HOLE_MAX_ROTATION_RATE 0.01
#define BLACK_HOLE_MAX_NUMBER 9
#define BLACK_HOLE_SPAWN_BATCH_NUM 3

// game parameters
#define MAX_LIVES 3
#define GAME_SPACE_MAX_X 500
#define GAME_SPACE_MAX_Y 500
#define EXPLODING_GAME_CYCLES 40
#define RESPAWNING_GAME_CYCLES 60

int asteroid_kill_score[] = {30, 100, 100};
int asteroid_masses[] = {4.0, 2.0, 1.0};
float asteroid_sizes[] = {8.0, 6.0, 3.0};

//game variables
uint32_t score = 0;
uint8_t lives = 3;
game_state state = ALIVE;
uint32_t asteroid_spawn_counter;
// tracks the number of game cycles left in a specific mode
uint16_t mode_countdown = RESPAWNING_GAME_CYCLES;

// game object tracking
// game object array
struct GAME_OBJECT gravity_object_array[GAME_OBJECT_NUM];
uint32_t gravity_object_counter = 0;
uint32_t large_asteroid_counter = 0;
uint32_t black_hole_counter = 0;
struct GAME_OBJECT bullet_array[BULLET_NUM];
uint32_t bullet_counter = 0;
struct GAME_OBJECT particle_array[PARTICLE_NUM];
uint32_t particle_counter = 0;
struct GAME_OBJECT star_array[STAR_NUM];
uint32_t star_counter = 0;
char debug_text[64]; //used for debugging
uint8_t debug = 0;
uint8_t fire_button_released = 1;

void kill_game_object(struct GAME_OBJECT *arr, int n, int index);
void spawn_asteroid(object_type asteroid_type, struct vector2d displacement, struct vector2d velocity);
struct vector2d random_vector(float magnitude);

void set_state_exploding()
{
	state = EXPLODING;
	mode_countdown = EXPLODING_GAME_CYCLES;
}

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

// kills an asteroid, increments the score appropriately, and spawns smaller asteroids if needed
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
	if (type == LARGE_ASTEROID)
	{
		large_asteroid_counter--;
	}
}

int check_bullet_asteroid_collisions()
{
	int asteroidIndex = -1;
	// stores the current number of gravity objects, as it could change but we don't care about new objects
	int n = gravity_object_counter;
	for (int i = 1; i < n; i++)
	{
		if (!gravity_object_array[i].indestructible)
		{
			int bulletCollisionIndex = check_collisions(&gravity_object_array[i], bullet_array, bullet_counter);
			if (bulletCollisionIndex > -1)
			{
				//TODO increment score
				// score += asteroid_kill_score[gravity_object_array[i].type];
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
			arr[i].lifespan--;
			// only checks if the lifespan has been zeroed out after checking it was greater than zero
			// allows this field to be ignored for objects without a lifespan
			if (arr[i].lifespan == 0)
			{
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
		displacement.y = -15;
		rotate_vector(&displacement, gravity_object_array[0].orientation * 180 / PI);
		add_vector(&displacement, &gravity_object_array[0].displacement);
		struct vector2d velocity;
		velocity.x = 0;
		velocity.y = BULLET_VELOCITY;
		rotate_vector(&velocity, gravity_object_array[0].orientation * 180 / PI + 180);
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
	multiply_vector(&gravity_object_array[0].velocity, ROCKET_VELOCITY_DAMPING_FACTOR);

	// check if any bullets hit asteroids
	check_bullet_asteroid_collisions();
	//check for collisions between the rocket and asteroids
	if (state == ALIVE && -1 != check_collisions(&gravity_object_array[0], &gravity_object_array[1], gravity_object_counter - 1))
	{
		kill_rocket();
		set_state_exploding();
		if (lives <= 0)
		{
			state = GAME_OVER;
		}
	}
}

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

// resets the rocket into its spawning state
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

void initialize_object(uint32_t spriteIndex, float scale, float orientation, float rotation_rate, struct vector2d *displacement, struct vector2d *velocity, struct vector2d *acceleration, float mass, uint8_t movable)
{
	if (gravity_object_counter < GAME_OBJECT_NUM)
	{
		gravity_object_array[gravity_object_counter] = (struct GAME_OBJECT){
			.sprite_index = spriteIndex,
			.size = scale,
			.orientation = orientation,
			.rotation_rate = rotation_rate,
			.displacement = *displacement,
			.velocity = *velocity,
			.acceleration = *acceleration,
			.mass = mass,
			.movable = movable,
			.visible = 1,
			.lifespan = -1,
			.type = 0};
		gravity_object_counter++;
		// //assign to sprite
		// gravity_object_array[gravity_object_counter].sprite_index = spriteIndex;
		// //assign the scale and orientation fields
		// gravity_object_array[gravity_object_counter].size = scale;
		// gravity_object_array[gravity_object_counter].orientation = orientation;
		// gravity_object_array[gravity_object_counter].rotation_rate = rotation_rate;
		// //assign displacement
		// gravity_object_array[gravity_object_counter].displacement.x = displacement->x;
		// gravity_object_array[gravity_object_counter].displacement.y = displacement->y;
		// //assign velocity
		// gravity_object_array[gravity_object_counter].velocity.x = velocity->x;
		// gravity_object_array[gravity_object_counter].velocity.y = velocity->y;
		// //assign acceleration
		// gravity_object_array[gravity_object_counter].acceleration.x = acceleration->x;
		// gravity_object_array[gravity_object_counter].acceleration.y = acceleration->y;
		// //assign mass
		// gravity_object_array[gravity_object_counter].mass = mass;
		// //select if this object should move or not (should the position be updated?) 0 = don't move, 1 = do move
		// gravity_object_array[gravity_object_counter].movable = movable;
		// gravity_object_array[gravity_object_counter].lifespan = -1;
		// gravity_object_array[gravity_object_counter].visible = 1;
		// //increment game object counter
		// gravity_object_counter++;
	}
}

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
		if (((int)black_hole_counter < BLACK_HOLE_MAX_NUMBER) && ((int)gravity_object_counter < GAME_OBJECT_NUM))
		{
			gravity_object_array[gravity_object_counter] = (struct GAME_OBJECT){
				.sprite_index = ASTEROID_INDEX,
				.size = BLACK_HOLE_SCALE,
				.orientation = ((float)rand()) / RAND_MAX * 2 * PI,
				.rotation_rate = (((float)rand()) / RAND_MAX - 0.5) * BLACK_HOLE_MAX_ROTATION_RATE,
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

void spawn_asteroid(object_type asteroid_type, struct vector2d displacement, struct vector2d velocity)
{
	if (gravity_object_counter < GAME_OBJECT_NUM)
	{
		gravity_object_array[gravity_object_counter] = (struct GAME_OBJECT){
			.sprite_index = ASTEROID_INDEX,
			.size = asteroid_sizes[asteroid_type],
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
		large_asteroid_counter++;
	}
}

// returns a random vector with the specified magnitude
struct vector2d random_vector(float magnitude)
{
	struct vector2d vec = {.x = magnitude, .y = 0};
	rotate_vector(&vec, ((float)rand()) / RAND_MAX * 360);
	return vec;
}

// spawns a large asteroid at a random location just outside the screen if the interval is correct
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
			// initialize_object(ASTEROID_INDEX,
			// 				  asteroid_sizes[LARGE_ASTEROID],
			// 				  ((float)rand()) / RAND_MAX * 2 * PI,
			// 				  (((float)rand()) / RAND_MAX - 0.5) * ASTEROID_LARGE_MAX_ROTATION_RATE,
			// 				  &displacement, &velocity, &acceleration, asteroid_masses[LARGE_ASTEROID], 1);
		}
		// always reset the timer so asteroids will spawn as normal when slots are available
		asteroid_spawn_counter = rand() % (ASTEROID_SPAWN_COUNTER_MAX - ASTEROID_SPAWN_COUNTER_MIN) + ASTEROID_SPAWN_COUNTER_MIN;
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

void display_lives()
{
	struct vector2d livesLocation = {
		.x = -MAX_X_COORD / 2 + 10,
		.y = -MAX_Y_COORD / 2 + 34};
	for (int i = 0; i < lives; i++)
	{
		draw_entity_to_buffer(ROCKET_INDEX, livesLocation, 0.6, 0);
		livesLocation.x += 20;
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
	add_array_to_display_buffer(star_array, star_counter);
	display_lives();
	// display score info
	sprintf(debug_text, "%d", score);
	buffer_text(5, 5, debug_text);
	buffer_to_LCD();
}

// updates the state of the director to reflect the current mode and any changes that have occurred
void game_state_handler()
{
	mode_countdown--;
	if (mode_countdown == 0 && lives > 0)
	{
		if (state == RESPAWNING)
		{
			state = ALIVE;
			gravity_object_array[0].visible = 1;
		}
		else if (state == EXPLODING)
		{
			// only respawn if lives remaining
			state = RESPAWNING;
			spawn_rocket();
			mode_countdown = RESPAWNING_GAME_CYCLES;
		}
	}
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

//this function will be called whenever you want to draw a new frame
void update_game_space(void)
{
	//TODO spawn asteroids
	//if debug is true, print debug info
	if (debug)
	{
		print_object_values();
	}
	//update the objects
	update_objects();
	//handles any necessary processes based on the game state
	game_state_handler();
	// spawn asteroids if appropriate (checks done in generate_fresh_asteroids)
	generate_fresh_asteroids();
	//TODO have a collision handling call here
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

void start_game(void)
{
	lives = MAX_LIVES;
	score = 0;
	gravity_object_counter = 0;
	black_hole_counter = 0;
	large_asteroid_counter = 0;
	bullet_counter = 0;
	particle_counter = 0;

	struct vector2d dship;
	dship.x = 0;
	dship.y = 0;
	struct vector2d zeroVector;
	zeroVector.x = 0;
	zeroVector.y = 0;

	spawn_rocket();
	gravity_object_counter++;

	spawn_starfield();

	state = RESPAWNING;

	asteroid_spawn_counter = ASTEROID_SPAWN_COUNTER_MIN; //rand() % (ASTEROID_SPAWN_COUNTER_MAX - ASTEROID_SPAWN_COUNTER_MIN) + ASTEROID_SPAWN_COUNTER_MIN;
}

int get_score()
{
	return score;
}

int get_lives()
{
	return lives;
}

int get_max_lives()
{
	return MAX_LIVES;
}

uint8_t is_game_over()
{
	return state == GAME_OVER;
}