
#include "vector_engine.h"
#include "GLCD.h"
#include <stdio.h>
#include <math.h>
#include "framebuffer.h"

float x_data_buffer[MAX_SPRITE_LINES * 2];
float y_data_buffer[MAX_SPRITE_LINES * 2];

#define ASTEROID_LINES 10
int asteroid_x[] = {0, 2, 2, 2, 2, 4, 4, 3, 3, 1, 1, 0, 0, -2, -2, -4, -4, -3, -3, 0}; //20 points/10 lines long
int asteroid_y[] = {4, 3, 3, 1, 1, 0, 0, -2, -2, -2, -2, -3, -3, -2, -2, 0, 0, 3, 3, 4};
Sprite asteroid;

#define ROCKET_LINES 4
int rocket_x[] = {0, -10, -10, 10, 10, 0};
int rocket_y[] = {15, -10, -10, -10, -10, 15};
Sprite rocket;

void init_vector_engine()
{
    asteroid.number_of_lines = ASTEROID_LINES;
    asteroid.endpoint_x_data = asteroid_x;
    asteroid.endpoint_y_data = asteroid_y;
    rocket.number_of_lines = ROCKET_LINES;
    rocket.endpoint_x_data = rocket_x;
    rocket.endpoint_y_data = rocket_y;
}

// Applies rotational and scale transforms to the passed sprite
// Places the resulting array of endpoints into x_data and y_data
void apply_transforms(float *x_data, float *y_data, int numEndpoints, float angle, float scale, Sprite *sprite)
{
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    float x;
    float y;
    // loop through each point and apply the rotational and scale transform
    for (int i = 0; i < numEndpoints; i++)
    {
        x = sprite->endpoint_x_data[i];
        y = sprite->endpoint_y_data[i];
        x_data[i] = (x * cosTheta - y * sinTheta) * scale;
        y_data[i] = (y * cosTheta + x * sinTheta) * scale;
    }
}

// draws the passed entity into the framebuffer
void draw_entity_to_buffer(Entity *entity)
{
    Sprite *sprite = entity->sprite;
    int numEndpoints = 2 * sprite->number_of_lines;
    float *x_data = &x_data_buffer[0];
    float *y_data = &y_data_buffer[0];
    // apply the rotational and scale transforms
    apply_transforms(x_data, y_data, numEndpoints, entity->orientation, entity->size, sprite);
    // iterate through the lines and add them to the frame buffer
    for (int i = 0; i < numEndpoints; i += 2)
    {
        buffer_line((int)x_data[i] + entity->x, (int)y_data[i] + entity->y, (int)x_data[i + 1] + entity->x, (int)y_data[i + 1] + entity->y);
    }
}

// gets the sprite for an asteroid
Sprite *get_asteroid_sprite()
{
    return &asteroid;
}

// gets the sprite for a rocket
Sprite *get_rocket_sprite()
{
    return &rocket;
}
