#include "LPC17xx.h"
#include <math.h>
#include "vector.h"
#include "vector_render_engine.h"

#define GAME_OBJECT_NUM 100
#define BULLET_NUM 20
#define GRAVITATIONAL_CONSTANT 1

struct GAME_OBJECT
{
    //the sprite index in the display engine that this object is tied to
    uint32_t sprite_index;
    //size multiplier of the sprite
    float size;
    //orientation of the sprite in radians
    float orientation;
    //the rotation rate of the object in degrees/frame
    float rotation_rate;
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
    uint8_t visible;
    uint16_t lifespan;
};

struct vector2d calculate_gravity(uint32_t affected_object_index, uint32_t cause_object_index);
void update_acceleration(uint32_t object_index);
void update_velocity(uint32_t object_index);
// void update_displacement(uint32_t object_index);
void update_displacement(struct GAME_OBJECT *obj);
void update_sprite(uint32_t object_index);
void update_objects(void);
void initialize_object(uint32_t sprite_index, float scale, float orientation, float rotation_rate, struct vector2d *displacement, struct vector2d *velocity, struct vector2d *acceleration, float mass, uint8_t movable);

void render_gamestate_to_LCD(void);
void update_game_space(void);
void print_object_values(void);
void control_input(int x, int y, uint8_t thrust, uint8_t fire);
void start_game(void);