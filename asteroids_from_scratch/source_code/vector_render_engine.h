// COEN 4720 final project
// author: Brendan Nenninger, Kassie Povinelli, and Carl Sustar
//
// rendering engine for drawing line-based sprites into a framebuffer

#include "LPC17xx.h"
#include "framebuffer.h"
#include "vector.h"

#ifndef __VECTOR_ENGINE_H__
#define __VECTOR_ENGINE_H__

#define NUM_SPRITES 9

// enum for storing the indexes of sprites for different objects within the sprite array
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

void init_vector_render_engine();
void draw_entity_to_buffer(uint32_t sprite_index, struct vector2d displacement, float scale, float orientation);
float get_sprite_radius(int sprite_index);

#endif
