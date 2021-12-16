// COEN 4720
// Project
// Gravitoids: Asteroids with Extra Physics and Multiplayer
// Brendan Nenninger, Kassie Povinelli, Carl Sustar
//
// director.h
// handles all gameplay
// gravitational physics, collisions, bullet firing, thrust,
// game score, applying player input, etc

#include "LPC17xx.h"
#include "vector.h"

#ifndef __DIRECTOR_H__
#define __DIRECTOR_H__

#define DEBUG 0

#define GAME_OBJECT_NUM 40
#define BULLET_NUM 20
#define PARTICLE_NUM 10
#define STAR_NUM 100

#define GRAVITATIONAL_CONSTANT 1

typedef enum
{
    LARGE_ASTEROID = 0,
    MEDIUM_ASTEROID = 1,
    SMALL_ASTEROID = 2,
    ROCKET,
    STAR
} object_type;

typedef enum
{
    ALIVE,
    EXPLODING,
    RESPAWNING,
    GAME_OVER
} game_state;

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
    object_type type;
    uint8_t indestructible;
};

// functions
void start_game(void);
int get_score();
int get_lives();
int get_max_lives();
uint8_t is_game_over();
void spawn_rocket();
uint8_t spawn_black_holes();
void spawn_asteroid(object_type asteroid_type, struct vector2d displacement, struct vector2d velocity);
void update_game_space(void);
void control_input(int x, int y, uint8_t thrust, uint8_t fire);

#endif