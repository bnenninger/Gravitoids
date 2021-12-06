#include "LPC17xx.h"
#include "framebuffer.h"
#include "vector.h"

#ifndef __VECTOR_ENGINE_H__
#define __VECTOR_ENGINE_H__

#define NUM_SPRITES 4

// don't change this to index from zero
// it doesn't work, no idea why
#define ROCKET_INDEX 1
#define ASTEROID_INDEX 2
#define ROCKET_FIRE_INDEX 3

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

#endif
