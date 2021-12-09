#include "LPC17xx.h"
#include "framebuffer.h"
#include "vector.h"

#ifndef __VECTOR_ENGINE_H__
#define __VECTOR_ENGINE_H__

#define NUM_SPRITES 9

// don't change this to index from zero
// it doesn't work, no idea why
// #define ROCKET_INDEX 1
// #define ASTEROID_INDEX 2
// #define ROCKET_FIRE_INDEX 3
// #define BULLET_INDEX 4
typedef enum
{
	ROCKET_INDEX,
	ASTEROID_INDEX,
	ROCKET_FIRE_INDEX,
	BULLET_INDEX,
	ROCKET_EXPLODE_1_INDEX,
	ROCKET_EXPLODE_2_INDEX,
	ROCKET_EXPLODE_3_INDEX,
	STAR_INDEX,
	BLACK_HOLE_INDEX
} sprite_index;

// struct for storing entity info
// each object rendered in the game will be an entity
// sprite is the sprite to be rendered for this entity
// orientation and size control how the sprite is rendered
// typedef struct
// {
//     int visible;  //0 = invisible, 1 = visible
//     int to_clear; //by default 0, set to 1 so that display engine knows to clear
//     int x;        //lower left-hand x-coord coordinate of sprite
//     int y;        //lower left-hand y-coord coordinate of sprite
//     float size;
//     float orientation; //orientation in radians
//     Sprite *sprite;
// } Entity;

void init_vector_render_engine();
void draw_entity_to_buffer(uint32_t sprite_index, struct vector2d displacement, float scale, float orientation);
float get_sprite_radius(int sprite_index);

#endif
