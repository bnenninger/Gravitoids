#include "LPC17xx.h"
#include <math.h>
#include "vector.h"
#define GAME_OBJECT_NUM 10
#define GRAVITATIONAL_CONSTANT 1

struct GAME_OBJECT
{
    //the sprite index in the display engine that this object is tied to
    uint32_t sprite_index; 
    //the displacement of the center of the object to the screen reference
    struct vector2d displacement;
    //the velocity of the object
    struct vector2d velocity;
    //the acceleration of the object
    struct vector2d acceleration;
    //the mass of the object
    float mass;
    //whether this object should move
    uint8_t movable;
};
struct vector2d calculate_gravity(uint32_t affected_object_index, uint32_t cause_object_index);
void update_acceleration(uint32_t object_index);
void update_velocity(uint32_t object_index);
void update_displacement(uint32_t object_index);
void update_sprite(uint32_t object_index);
void update_objects(void);
void initialize_object(uint32_t sprite_index, struct vector2d* displacement, struct vector2d* velocity, struct vector2d* acceleration, float mass, uint8_t movable);


void update_place_space(void);
void print_object_values(void);
void start_game(void);