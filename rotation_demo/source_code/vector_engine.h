#include "LPC17xx.h"
#include "framebuffer.h"

#ifndef __VECTOR_ENGINE_H__
#define __VECTOR_ENGINE_H__

#define MAX_SPRITE_LINES 100

// struct for storing standard sprite info
// sprites are intended to be used for multiple different entities
typedef struct
{
    int number_of_lines; // number of lines that make up the sprite NOTE: there are twice as many endpoints as there are lines
    // lines are drawn such that the first two coordinates correspond to the two endpoints of the first line,
    // second two coordinates with two endpoints of second line, etc
    int *endpoint_x_data; //x coordinate for each line endpoint
    int *endpoint_y_data; //y coordinate for each line endpoint
} Sprite;

// struct for storing entity info
// each object rendered in the game will be an entity
// sprite is the sprite to be rendered for this entity
// orientation and size control how the sprite is rendered
typedef struct
{
    int visible;  //0 = invisible, 1 = visible
    int to_clear; //by default 0, set to 1 so that display engine knows to clear
    int x;        //lower left-hand x-coord coordinate of sprite
    int y;        //lower left-hand y-coord coordinate of sprite
    float size;
    float orientation; //orientation in radians
    Sprite *sprite;
} Entity;

void init_vector_engine();
void draw_entity_to_buffer(Entity *entity);
Sprite *get_asteroid_sprite();
Sprite *get_rocket_sprite();

#endif
