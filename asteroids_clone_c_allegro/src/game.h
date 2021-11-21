#ifndef _GAME_H
#define _GAME_H

#include <stdbool.h>

#include <allegro5/allegro.h>

#include "./vector.h"
#include "./asteroid.h"
#include "./particles.h"
#include "./ship.h"
#include "./bullets.h"

#define LIVESN 3
#define SHIP_COLOR al_map_rgb(255, 255, 255)
#define BACKGROUND_COLOR al_map_rgb(0, 0, 0)
#define HUD_COLOR al_map_rgb(255, 255, 255)
#define ASTEROID_SCORE 100
#define SHIP_FRICTION 0.5
#define ASTEROID_PARTICLEN 20
#define ASTEROIDN 8

extern ALLEGRO_FONT *ttf_font;

// Initialises and loads fonts
bool init_font();

typedef enum {
    Won, Lost, Paused, Quit, Playing
} GameStatus;

typedef struct Game {
    // The currently displayed asteroids
    AsteroidNode* asteroids;

    // The particle manager for the scene
    ParticleManager* particlemanager;

    // The player's ship
    Ship ship;

    // The currently displayed bullets
    BulletManager* bulletmanager;

    // The players score
    int score;

    // The players lives
    int lives;

    // The game size
    Vector size;

    // The status of the game
    GameStatus status;
} Game;

// Creates a new empty game with the given screen size
Game* new_game(Vector size);
// Deletes the game and all component parts
void delete_game(Game*);
// Restarts the game
void restart_game(Game*);
// Spawns a new asteroid. The ship will _not_ be inside the asteroid.
void spawn_asteroid(Game*);

// Does the right thing (tm) with regards to key presses
void handle_key_status(Game*, ALLEGRO_KEYBOARD_STATE*);
// Handles key events (i.e. one off presses)
void handle_key_event(Game*, int);

// Updates the gamestate for a single frame
void update_game(Game*, float);
// Updates the ship
void update_ship(Game*, float);
// Updates the particle managers
void update_particles(Game*, float);
// Updates the game when it is paused
void update_paused(Game*);

// Draws the game. If the second argument is true, it will be drawn with
// a transparent layer over it (for pause and scores etc)
void draw_game(Game*, float);
// Draws the spaceship
void draw_ship(Game*);
// Draws the hud
void draw_hud(Game*);
// Draws the pause menu
void draw_paused(Game*);
// Draws the won message
void draw_won(Game*);
// Draws the lost message
void draw_lost(Game*);

#endif
