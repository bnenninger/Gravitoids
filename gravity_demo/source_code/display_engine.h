#include "LPC17xx.h"
#include "GLCD.h"

#define BLOCK_SIZE 4
#define MAX_X_COORD 79
#define MAX_Y_COORD 59
#define NUMBER_OF_SPRITES 10


struct SPRITE
{
    int visible;  //0 = invisible, 1 = visible
    int to_clear; //by default 0, set to 1 so that display engine knows to clear
    int x;        //lower left-hand x-coord coordinate of sprite
    int y;        //lower left-hand y-coord coordinate of sprite
    int width;
    int height;
    int number_of_blocks;          //number of blocks that make up the sprite
    int *block_x_data;             //x coordinate for each block compared to x
    int *block_y_data;             //y coordinate for each block compared to y
    int color_palet;               //color palet to use: 0 or 1 for choice of 2 color palets
    uint16_t *block_color_data;    //color data for each block
    uint16_t *block_color_data_2;  //color data for each block (optional extra palet)
    uint16_t *blockout_color_data; //color data for each block (optional extra palet)
    uint8_t position_updated; //whether the position of the sprite has been updated
		//uint8_t leave_trail;
		//uint16_t *trail_color_data;
};
void LCD_draw_block(int x, int y, uint16_t color);
void LCD_draw_sprite(struct SPRITE *sprite); //use pointer for faster implementation
void clear_sprite(int index);
void LCD_clear_sprite(struct SPRITE *sprite);
void initialize_display_engine(void);
void update_display(void);
void update_sprite_visibility(int sprite_index, int visibility);
int check_sprite_visibility(int sprite_index);
int check_x_pos(int sprite_index);
int check_y_pos(int sprite_index);
int check_width(int sprite_index);
int check_height(int sprite_index);
void update_sprite_position(int sprite_index, int x, int y);
void update_sprite_palet(int sprite_index);
void initialize_asteroid(int x, int y);