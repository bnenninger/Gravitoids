#include "display_engine.h"
#include "GLCD.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
struct SPRITE sprites[NUMBER_OF_SPRITES];
int sprite_counter;
int lives;
int length;
//text used for displaying lives
char text[32];
//asteroid x, y, color data
static int asteroid_x_data[9] =
    {0};
static int asteroid_y_data[9] =
    {0};
static uint16_t asteroid_color_data[9] =
    {White};
static uint16_t asteroid_clear_data[9] =
    {Black};

//note: all sprite data should be read bottom-up -- the first line is the lowest one on the actual sprite


//Draw BLOCK_SIZE*BLOCK_SIZE block on LCD at coords y and x (switched to match horizontal game implementation for easy use, inputs should be x, y)
void LCD_draw_block(int x, int y, uint16_t color) //orientation changed in GLCD, don't worry about it
{
    int i;
    int j;
    for (i = x; i < x + BLOCK_SIZE; i++)
    {
        for (j = y; j < y + BLOCK_SIZE; j++)
        {
            LCD_SetPoint(i, j, color);
        }
    }
}
//draw the sprite by drawing blocks at the relative coordinates
void LCD_draw_sprite(struct SPRITE *sprite)
{
    int i;
    if (sprite->color_palet == 0) //if color palette 0, draw with color palette 0
    {
        for (i = 0; i < sprite->number_of_blocks; i++)
        {
            LCD_draw_block((sprite->x + sprite->block_x_data[i]) * BLOCK_SIZE, (sprite->y + sprite->block_y_data[i]) * BLOCK_SIZE, sprite->block_color_data[i]);
        }
    }
    else //if color palette 1, draw with color palette 1
    {
        for (i = 0; i < sprite->number_of_blocks; i++)
        {
            LCD_draw_block((sprite->x + sprite->block_x_data[i]) * BLOCK_SIZE, (sprite->y + sprite->block_y_data[i]) * BLOCK_SIZE, sprite->block_color_data_2[i]);
        }
    }
}
//clear sprite helper for accessing sprite outside of displayengine
void clear_sprite(int index)
{
    LCD_clear_sprite(&sprites[index]);
}
//clear the sprite by drawing the clear mask over it
void LCD_clear_sprite(struct SPRITE *sprite)
{
    int i;

    for (i = 0; i < sprite->number_of_blocks; i++)
    {
        LCD_draw_block((sprite->x + sprite->block_x_data[i]) * BLOCK_SIZE, (sprite->y + sprite->block_y_data[i]) * BLOCK_SIZE, sprite->blockout_color_data[i]);
    }
    sprite->to_clear = 0; //sprite has been cleared, negate the to_clear flag
}
//initialize the display engine by initializing the LCD and the sprites
void initialize_display_engine(void)
{
    int i;
    sprite_counter = 0;
    LCD_Clear(Black); // Clear graphical LCD display
    //initialize asteroids
    initialize_asteroid(30, 20);
    initialize_asteroid(60, 40);
}
//called by director to update the display whenever a new frame is ready to be drawn
void update_display(void)
{
    int i;
    //draw sprites
    for (i = sprite_counter; i >= 0; i--)
    {
        if (sprites[i].visible == 1) //draw sprite if visible
        {
            if (sprites[i].position_updated)
            {
                //updating the position, so reset the flag
                sprites[i].position_updated = 0;
                //clear the previous sprite
                LCD_clear_sprite(&(sprites[i]));
            }
            //draw the sprite
            LCD_draw_sprite(&(sprites[i]));
            
        }
        else if (sprites[i].to_clear != 0) //clear sprite if you should
        {
            LCD_clear_sprite(&(sprites[i]));
        }
    }
}
//access function so director can change a sprite's visibility value (0=invisible, 1 = visible)
void update_sprite_visibility(int sprite_index, int visibility)
{
    //if setting visible object to invisible, increment to_clear flag
    if (sprites[sprite_index].visible == 1 && visibility == 0)
    {
        sprites[sprite_index].to_clear++;
    }
    sprites[sprite_index].visible = visibility;
}
//access function so director can check a sprite's visibility
int check_sprite_visibility(int sprite_index)
{
    return sprites[sprite_index].visible;
}
//access function so director can update sprite at index's x and y coordinates according to inputs
void update_sprite_position(int sprite_index, int x, int y)
{
    sprites[sprite_index].x = x;
    sprites[sprite_index].y = y;
    sprites[sprite_index].position_updated = 1;
}
//access function so director can get sprite at index's x coord
int check_x_pos(int sprite_index)
{
    return sprites[sprite_index].x;
}
//access function so director can get sprite at index's y coord
int check_y_pos(int sprite_index)
{
    return sprites[sprite_index].y;
}
int check_width(int sprite_index)
{
    return sprites[sprite_index].width;
}
int check_height(int sprite_index)
{
    return sprites[sprite_index].height;
}
//access function so director can switch the color palette of the sprite at the index to the other one
void update_sprite_palet(int sprite_index)
{
    sprites[sprite_index].color_palet ^= 1; //xor with 1 means swap between 1 and 0
}
void initialize_asteroid(int x, int y)
{
    //set coordinates, colors, number of blocks, other palletes, visibility
    sprites[sprite_counter].x = x;
    sprites[sprite_counter].y = y;
    sprites[sprite_counter].visible = 0; //by default invisible
    sprites[sprite_counter].to_clear = 0;
    sprites[sprite_counter].width = 1;            
    sprites[sprite_counter].height = 1;           
    sprites[sprite_counter].number_of_blocks = 1;
    sprites[sprite_counter].color_palet = 0;
    sprites[sprite_counter].block_x_data = asteroid_x_data;
    sprites[sprite_counter].block_y_data = asteroid_y_data;
    sprites[sprite_counter].block_color_data = asteroid_color_data;
    sprites[sprite_counter].block_color_data_2 = asteroid_color_data;
    sprites[sprite_counter].blockout_color_data = asteroid_clear_data;
    sprites[sprite_counter].position_updated = 0;
    //update sprite_counter
    sprite_counter++;
}