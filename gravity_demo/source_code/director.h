#include "LPC17xx.h"
//defines for snake game
#define INITIAL_LIVES 3
#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3
int update_mouse(void);
void move_snake(void);
void update_place_space(void);
void start_game(void);