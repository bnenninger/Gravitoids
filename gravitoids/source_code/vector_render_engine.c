// COEN 4720
// Project
// Gravitoids: Asteroids with Extra Physics and Multiplayer
// Brendan Nenninger, Kassie Povinelli, Carl Sustar
//
// vector_render_engine.c
// rendering engine for drawing line-based sprites into a framebuffer

#include "vector_render_engine.h"
#include <stdio.h>
#include <math.h>
#include "framebuffer.h"

#define MAX_SPRITE_LINES 10

// struct for storing standard sprite info
// sprites are intended to be used for multiple different entities
typedef struct
{
    int number_of_lines; // number of lines that make up the sprite NOTE: there are twice as many endpoints as there are lines
    // lines are drawn such that the first two coordinates correspond to the two endpoints of the first line,
    // second two coordinates with two endpoints of second line, etc
    int *endpoint_x_data; //x coordinate for each line endpoint
    int *endpoint_y_data; //y coordinate for each line endpoint
    // radius of the sprite, used for collision handling
    float radius
} Sprite;

float x_data_buffer[MAX_SPRITE_LINES * 2];
float y_data_buffer[MAX_SPRITE_LINES * 2];

Sprite sprites[NUM_SPRITES];

// endpoint data for the sprites
#define ASTEROID_LINES 10
int asteroid_x[] = {0, 2, 2, 2, 2, 4, 4, 3, 3, 1, 1, 0, 0, -2, -2, -4, -4, -3, -3, 0}; //20 points/10 lines long
int asteroid_y[] = {4, 3, 3, 1, 1, 0, 0, -2, -2, -2, -2, -3, -3, -2, -2, 0, 0, 3, 3, 4};
Sprite asteroid;

#define ROCKET_LINES 3
#define ROCKET_FIRE_LINES 5
int rocket_fire_x[] = {0, -12, -10, 10, 12, 0, -5, 0, 5, 0};
int rocket_fire_y[] = {-15, 15, 10, 10, 15, -15, 10, 25, 10, 25};

#define BULLET_LINES 4
int bullet_x[] = {-1, 1, 1, 1, 1, -1, -1, -1};
int bullet_y[] = {-1, -1, -1, 1, 1, 1, 1, -1};

#define BLACK_HOLE_LINES 8
int black_hole_x[] = {10, 7, 7, 0, 0, -7, -7, -10, -10, -7, -7, 0, 0, 7, 7, 10};
int black_hole_y[] = {0, 7, 7, 10, 10, 7, 7, 0, 0, -7, -7, -10, -10, -7, -7, 0};

// making the stars a sprite is a little unnecessary,
// but it allows the standard game object system to be used for them
#define STAR_LINES 1
int star[] = {0, 0};

// calculates the largest radius of a sprite
// the distance from the center to the furthest line endpoint
float calculate_sprite_radius(int sprite_index)
{
    int *x = sprites[sprite_index].endpoint_x_data;
    int *y = sprites[sprite_index].endpoint_y_data;
    // finds the maximum radius of a point from the center
    float max = 0.0;
    // as sprites are not required to be continguous, every endpoint must be checked
    for (int i = 0; i < sprites[sprite_index].number_of_lines * 2; i++)
    {
        // calculate the distance of the point from zero
        float value = sqrtf((float)(x[i] * x[i] + y[i] * y[i]));
        if (value > max)
        {
            max = value;
        }
    }
    return max;
}

// initializes the vector rendering engine
void init_vector_render_engine()
{
    // initialize all the sprites
    // initialize the rocket sprite
    sprites[ROCKET_INDEX].number_of_lines = ROCKET_LINES;
    sprites[ROCKET_INDEX].endpoint_x_data = rocket_fire_x;
    sprites[ROCKET_INDEX].endpoint_y_data = rocket_fire_y;
    sprites[ROCKET_INDEX].radius = calculate_sprite_radius(ROCKET_INDEX);
    //initialize the asteroid sprite
    sprites[ASTEROID_INDEX]
        .number_of_lines = ASTEROID_LINES;
    sprites[ASTEROID_INDEX].endpoint_x_data = asteroid_x;
    sprites[ASTEROID_INDEX].endpoint_y_data = asteroid_y;
    sprites[ASTEROID_INDEX].radius = calculate_sprite_radius(ASTEROID_INDEX);
    //initializes the rocket sprite with engine firing
    sprites[ROCKET_FIRE_INDEX].number_of_lines = ROCKET_FIRE_LINES;
    sprites[ROCKET_FIRE_INDEX].endpoint_x_data = rocket_fire_x;
    sprites[ROCKET_FIRE_INDEX].endpoint_y_data = rocket_fire_y;
    sprites[ROCKET_FIRE_INDEX].radius = calculate_sprite_radius(ROCKET_FIRE_INDEX);
    //initializes the bullet sprite
    sprites[BULLET_INDEX].number_of_lines = BULLET_LINES;
    sprites[BULLET_INDEX].endpoint_x_data = bullet_x;
    sprites[BULLET_INDEX].endpoint_y_data = bullet_y;
    sprites[BULLET_INDEX].radius = calculate_sprite_radius(BULLET_INDEX);
    //initialize the rocket exploding sprites
    sprites[ROCKET_EXPLODE_1_INDEX].number_of_lines = 1;
    sprites[ROCKET_EXPLODE_1_INDEX].endpoint_x_data = rocket_fire_x;
    sprites[ROCKET_EXPLODE_1_INDEX].endpoint_y_data = rocket_fire_y;
    sprites[ROCKET_EXPLODE_2_INDEX].number_of_lines = 1;
    sprites[ROCKET_EXPLODE_2_INDEX].endpoint_x_data = rocket_fire_x + 2;
    sprites[ROCKET_EXPLODE_2_INDEX].endpoint_y_data = rocket_fire_y + 2;
    sprites[ROCKET_EXPLODE_3_INDEX].number_of_lines = 1;
    sprites[ROCKET_EXPLODE_3_INDEX].endpoint_x_data = rocket_fire_x + 4;
    sprites[ROCKET_EXPLODE_3_INDEX].endpoint_y_data = rocket_fire_y + 4;
    //initialize the star sprites
    sprites[STAR_INDEX].number_of_lines = STAR_LINES;
    sprites[STAR_INDEX].endpoint_x_data = star;
    sprites[STAR_INDEX].endpoint_y_data = star;
    //initialize the black hole sprites
    sprites[BLACK_HOLE_INDEX].number_of_lines = BLACK_HOLE_LINES;
    sprites[BLACK_HOLE_INDEX].endpoint_x_data = black_hole_x;
    sprites[BLACK_HOLE_INDEX].endpoint_y_data = black_hole_y;
    sprites[BLACK_HOLE_INDEX].radius = calculate_sprite_radius(BLACK_HOLE_INDEX);
}

// Applies rotational and scale transforms to the passed sprite
// Places the resulting array of endpoints into x_data and y_data
void apply_transforms(float *x_data, float *y_data, int numEndpoints, float angle, float scale, uint32_t sprite_index)
{
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    float x;
    float y;
    // loop through each point and apply the rotational and scale transform
    for (int i = 0; i < numEndpoints; i++)
    {
        x = sprites[sprite_index].endpoint_x_data[i];
        y = sprites[sprite_index].endpoint_y_data[i];
        x_data[i] = (x * cosTheta - y * sinTheta) * scale;
        y_data[i] = (y * cosTheta + x * sinTheta) * scale;
    }
}

// draws the passed entity into the framebuffer
void draw_entity_to_buffer(uint32_t sprite_index, struct vector2d displacement, float scale, float orientation)
{
    int numEndpoints = 2 * sprites[sprite_index].number_of_lines;
    float *x_data = &x_data_buffer[0];
    float *y_data = &y_data_buffer[0];
    // apply the rotational and scale transforms
    apply_transforms(x_data, y_data, numEndpoints, orientation, scale, sprite_index);
    // iterate through the lines and add them to the frame buffer
    for (int i = 0; i < numEndpoints; i += 2)
    {
        buffer_line((int)(x_data[i] + displacement.x) + MAX_X_COORD / 2,
                    (int)(y_data[i] + displacement.y) + MAX_Y_COORD / 2,
                    (int)(x_data[i + 1] + displacement.x) + MAX_X_COORD / 2,
                    (int)(y_data[i + 1] + displacement.y) + MAX_Y_COORD / 2);
    }
}

// returns the distance from the center of the sprite to the most distant line endpoint
float get_sprite_radius(int sprite_index)
{
    return sprites[sprite_index].radius;
}
