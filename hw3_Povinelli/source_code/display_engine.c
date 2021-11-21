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

//snake body, head, and trail part sprite data
//note: all sprite data should be read bottom-up -- the first line is the lowest one on the actual sprite

static int body_part_x_data[1] =
    {0};
static int body_part_y_data[1] =
    {0};
static uint16_t body_part_color_data[1] =
    {Green};
//the trailing part deletes the last piece when the snake moves, should be displayed FIRST
static uint16_t trailing_part_color_data[1] =
    {Black};
//head should be displayed LAST
static uint16_t head_color_data[1] =
    {Red};

//mouse sprite data
//note: all sprite data should be read bottom-up -- the first line is the lowest one on the actual sprite

static int mouse_x_data[1] =
    {0};
static int mouse_y_data[1] =
    {0};
static uint16_t mouse_color_data[1] =
    {Magenta};

//display the number of lives left on the LCD display
void display_win()
{
    sprintf(text, "YOU WIN!");
    LCD_PutText(WIN_MESSAGE_X_POS, WIN_MESSAGE_Y_POS, text, White, Black);
}
void display_lose()
{
    sprintf(text, "YOU LOSE!");
    LCD_PutText(WIN_MESSAGE_X_POS, WIN_MESSAGE_Y_POS, text, White, Black);
}
void display_lives()
{
    sprintf(text, "Lives: %d", lives);
    LCD_PutText(LIVES_X_POS, LIVES_Y_POS, text, White, Black);
}
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
    //initialize all the snake's parts invisibly
    //initialize head
    initialize_head(SNAKE_STARTING_X, SNAKE_STARTING_Y);
    //initialize body parts so they're folowing each other
    for (i = 0; i < MAX_NUM_BODY_PARTS; i++)
        initialize_body_part(check_x_pos(sprite_counter - 1) - 1, check_y_pos(sprite_counter - 1));
    //initialize the mouse
    initialize_mouse(9, 0);
    //initialize the snake trail at position to left of farthest one
    length = 3;
    initialize_trail(check_x_pos(length) - 1, check_y_pos(length));
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

            LCD_draw_sprite(&(sprites[i]));
        }
        else if (sprites[i].to_clear != 0) //clear sprite if you should
        {
            LCD_clear_sprite(&(sprites[i]));
        }
    }
    //display lives counter in corner
    display_lives();
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
//access function so director can switch the color palette of the sprite at the index to the other one
void update_sprite_palet(int sprite_index)
{
    sprites[sprite_index].color_palet ^= 1; //xor with 1 means swap between 1 and 0
}
void initialize_head(int x, int y)
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
    sprites[sprite_counter].block_x_data = body_part_x_data;
    sprites[sprite_counter].block_y_data = body_part_y_data;
    sprites[sprite_counter].block_color_data = head_color_data;
    sprites[sprite_counter].block_color_data_2 = head_color_data;
    sprites[sprite_counter].blockout_color_data = trailing_part_color_data;
    //update sprite_counter
    sprite_counter++;
}
void initialize_body_part(int x, int y)
{
    //set coordinates, colors, number of blocks, other palletes, visibility
    sprites[sprite_counter].x = x;
    sprites[sprite_counter].y = y;
    sprites[sprite_counter].visible = 0;
    sprites[sprite_counter].to_clear = 0;
    sprites[sprite_counter].width = 1;
    sprites[sprite_counter].height = 1;
    sprites[sprite_counter].number_of_blocks = 1;
    sprites[sprite_counter].color_palet = 0;
    sprites[sprite_counter].block_x_data = body_part_x_data;
    sprites[sprite_counter].block_y_data = body_part_y_data;
    sprites[sprite_counter].block_color_data = body_part_color_data;
    sprites[sprite_counter].block_color_data_2 = body_part_color_data;
    sprites[sprite_counter].blockout_color_data = trailing_part_color_data;
    //update sprite_counter
    sprite_counter++;
}
void initialize_trail(int x, int y)
{
    //set coordinates, colors, number of blocks, other palletes, visibility
    sprites[sprite_counter].x = x;
    sprites[sprite_counter].y = y;
    sprites[sprite_counter].visible = 0;
    sprites[sprite_counter].to_clear = 0;
    sprites[sprite_counter].width = 1;
    sprites[sprite_counter].height = 1;
    sprites[sprite_counter].number_of_blocks = 1;
    sprites[sprite_counter].color_palet = 0;
    sprites[sprite_counter].block_x_data = body_part_x_data;
    sprites[sprite_counter].block_y_data = body_part_y_data;
    sprites[sprite_counter].block_color_data = trailing_part_color_data;
    sprites[sprite_counter].block_color_data_2 = trailing_part_color_data;
    sprites[sprite_counter].blockout_color_data = trailing_part_color_data;
    //update sprite_counter
    sprite_counter++;
}
void initialize_mouse(int x, int y)
{
    //set coordinates, colors, number of blocks, other palletes, visibility
    sprites[sprite_counter].x = x;
    sprites[sprite_counter].y = y;
    sprites[sprite_counter].visible = 0;
    sprites[sprite_counter].to_clear = 0;
    sprites[sprite_counter].width = 1;
    sprites[sprite_counter].height = 1;
    sprites[sprite_counter].number_of_blocks = 1;
    sprites[sprite_counter].color_palet = 0;
    sprites[sprite_counter].block_x_data = body_part_x_data;
    sprites[sprite_counter].block_y_data = body_part_y_data;
    sprites[sprite_counter].block_color_data = mouse_color_data;
    sprites[sprite_counter].block_color_data_2 = mouse_color_data;
    sprites[sprite_counter].blockout_color_data = trailing_part_color_data;
    //update sprite_counter
    sprite_counter++;
}
