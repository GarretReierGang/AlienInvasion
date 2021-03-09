#ifndef IMAGE_RENDER_H
#define IMAGE_RENDER_H

//HEADERS
#include <stdio.h>      //printf()
#include <unistd.h>     //read(), write()
#include <fcntl.h>      //O_RDWR
#include <stdint.h>     //uint32_t
#include <stdbool.h>
#include <stdlib.h>
#include "sprites.c"    //holds sprite arrays
#include "intc.h"
#include "button_handler.h"
#include "gpio_button.h"
#include "imageRender.h"
#include "saucer.h"
#include "alienControl.h"
#include "globals.h"

//STARTING PIXELS                  //640 x 480
#define ALIEN_START_ROW 75
#define ALIEN_START_COLUMN 75
#define START_COLUMN_TANK 80 - 2 //-2 alligns bullets before tank moves
#define START_COLUM_SAUCER 275
#define START_ROW_TANK 450
#define START_ROW_SAUCER IMAGE_RENDER_ONE_ROW_IN_BYTES*30
#define START_ROW_PLAYER_BULLET 435
#define START_COLUMN_BUNKERS 100
#define START_ROW_BUNKERS 390

//COLORS
#define IMAGE_RENDER_BACKGROUND_COLOR black
#define IMAGE_RENDER_STANDARD_LETTER_COLOR white
#define IMAGE_RENDER_SCORE_LETTER_COLOR green

//STARTING BYTES
#define START_BYTE_SCORE00000         IMAGE_RENDER_ONE_ROW_IN_BYTES*8   + PIXEL_SIZE_IN_BYTES*22
#define START_BYTE_LIVES              IMAGE_RENDER_ONE_ROW_IN_BYTES*8   + PIXEL_SIZE_IN_BYTES*350
#define START_BYTE_TANK_LIVES         IMAGE_RENDER_ONE_ROW_IN_BYTES*3   + PIXEL_SIZE_IN_BYTES*430
#define START_BYTE_SAUCER             IMAGE_RENDER_ONE_ROW_IN_BYTES*30  + PIXEL_SIZE_IN_BYTES*275
#define START_BYTE_ALIENS             IMAGE_RENDER_ONE_ROW_IN_BYTES*75  + PIXEL_SIZE_IN_BYTES*75
#define START_BYTE_BUNKERS            IMAGE_RENDER_ONE_ROW_IN_BYTES*390 + PIXEL_SIZE_IN_BYTES*100
#define START_BYTE_TANK               IMAGE_RENDER_ONE_ROW_IN_BYTES*450 + PIXEL_SIZE_IN_BYTES*80
#define START_BYTE_GAME_OVER          IMAGE_RENDER_ONE_ROW_IN_BYTES*15  + PIXEL_SIZE_IN_BYTES*100
#define START_BYTE_ENTER_NAME         IMAGE_RENDER_ONE_ROW_IN_BYTES*60  + PIXEL_SIZE_IN_BYTES*200
#define START_BYTE_NAME_ENTERING_SPOT IMAGE_RENDER_ONE_ROW_IN_BYTES*75  + PIXEL_SIZE_IN_BYTES*260
#define START_BYTE_HIGH_SCORES        IMAGE_RENDER_ONE_ROW_IN_BYTES*75  + PIXEL_SIZE_IN_BYTES*220

//SCALING
#define SPRITE_SCALING_FACTOR 2
#define BIG_A_SCALING_FACTOR  8

//PIXEL SIZING
#define PIXELS_BETWEEN_LETTERS       5
#define PIXELS_BETWEEN_WORDS         12
#define PIXELS_BETWEEN_BIG_A_LETTERS 9
#define PIXELS_BETWEEN_A_WORDS       20
#define PIXELS_BETWEEN_TANKS         5
#define PIXELS_BETWEEN_BUNKERS       80
#define PIXELS_ON_EACH_ALIENS_SIDE   5
#define PIXELS_BETWEEN_ALIENS        PIXELS_ON_EACH_ALIENS_SIDE*2
#define PIXELS_BETWEEN_ALIEN_ROW     10
#define PIXEL_SIZE_IN_BYTES          3
#define IMAGE_RENDER_LAST_COLUMN_BYTE PIXEL_SIZE_IN_BYTES * 640

//SPRITE SIZES
#define TANK_WIDTH 15
#define TANK_HEIGHT 8
#define SAUCER_WIDTH 16
#define SAUCER_HEIGHT 7
#define BUNKER_WIDTH 24
#define BUNKER_HEIGHT 18
#define ALIEN_WIDTH 12
#define ALIEN_HEIGHT 8
#define TANK_BULLET_WIDTH 1
#define TANK_BULLET_HEIGHT 5
#define MOVING_TANK_WIDTH TANK_WIDTH + 3
#define MOVING_SAUCER_WIDTH 19
#define LETTER_WIDTH 5
#define LETTER_HEIGHT 5
#define SPACING_LETTER (PIXEL_SIZE_IN_BYTES * \
        (LETTER_WIDTH * SPRITE_SCALING_FACTOR  +\
         PIXELS_BETWEEN_LETTERS))

//OTHER
#define RUN_TEST_SCORE 69
#define IMAGE_RENDER_ONE_ROW_IN_BYTES (640 * 3)
#define GPIO_BUTTON_INTC_MASK 0x2
#define GPIO_FIT_INTC_MASK 0x1
#define SINGLE_PIXEL 3
#define PLAYER_STARTING_LIVES 3

//BUNKER
#define NEXT_BUNKER (PIXELS_BETWEEN_BUNKERS + (BUNKER_WIDTH * SPRITE_SCALING_FACTOR))
#define BUNKER_SECTION_WIDTH 6
#define BUNKER_SECTION_HEIGHT 6
#define BUNKER_DAMAGE_DEAD -1
#define BUNKER_DAMAGE_0 0
#define BUNKER_DAMAGE_1 1
#define BUNKER_DAMAGE_2 2
#define MISSING_TOP_LEFT 0
#define MISSING_TOP_RIGHT 3
#define MISSING_BOTTOM_LEFT 6
#define MISSING_BOTTOM_RIGHT 5

// Pixel Coloration
char black[PIXEL_SIZE_IN_BYTES];  // = {0x00, 0x00, 0x00};
char white[PIXEL_SIZE_IN_BYTES];  // = {0xFF, 0xFF, 0xFF};
char red[PIXEL_SIZE_IN_BYTES];    // = {0xFF, 0x00, 0x00};
char green[PIXEL_SIZE_IN_BYTES];  // = {0x00, 0xFF, 0x00};
char blue[PIXEL_SIZE_IN_BYTES];   // = {0x00, 0x00, 0xFF};
char yellow[PIXEL_SIZE_IN_BYTES]; // = {0xFF, 0xFF, 0x00};
int32_t hdmiFileDescriptor; //used to perform r/w operations

//FUNCTIONS
bool imageRender_init();

//MULTI-PURPOSE

// Purpose: Draws a letter a given position
// Takes:
//  letter, the char you want to draw (a-z0-9 )
//  scalinng_factor how much larger to draw it (scaling factor of at least 2 is recomended)
//  position Where it should be drawn. calculated by (IMAGE_RENDER_ONE_ROW_IN_BYTES * Yloc + PIXEL_SIZE_IN_BYTES * COLUMN)
//  color: 3 byte array containing the RGB values to draw.
// Draws the image and returns a horizontal offset pointing to where the Letter ends
uint32_t imageRender_drawLetter(char letter, uint32_t scaling_factor, uint32_t position, char color[]);

// Purpose: Draws an entire word at a given position.
// Takes:
//  word char[] (a-z0-9 )+ to draw
//  length num chars to draw
//  position Where it should be drawn. calculated by (IMAGE_RENDER_ONE_ROW_IN_BYTES * Yloc + PIXEL_SIZE_IN_BYTES * COLUMN)
//  color 3 byte array contiang RGB values, or color of the letters to draw
// Return: the location of where a following word//letter could start
uint32_t imageRender_drawWord(char word[], uint32_t length, uint32_t position, char color[]);

//  Purpose: Draw a sprite at a given column and Row,
//    NOTE: any pixels that would exceed the screen length are not drawn
// Takes:
//    col, x position (in pixels) of location to draw, can be negative
//    row, y position (in pixels) of location to draw, can be negative
//    sprite:  bitmap of the sprite to draw
//    width,  sprites pixel width
//    height, sprites pixel height
//    color,  3 byte array describing the RGB values to display for the given sprite
//    scaling_factor, how much to up-scale the bitmap by (recomended 2x);
void imageRender_drawSprite(int32_t col, int32_t row, uint32_t sprite[], uint32_t width, uint32_t height, char color[], uint32_t scaling_factor );

//  Purpose: Draw a sprite at a given byte location
//    NOTE: Pixels that would exceed the screen length are drawn.
//  Takes:
//    position Where it should be drawn. calculated by (IMAGE_RENDER_ONE_ROW_IN_BYTES * Yloc + PIXEL_SIZE_IN_BYTES * COLUMN)
//    sprite:  bitmap of the sprite to draw
//    width,  sprites pixel width
//    height, sprites pixel height
//    color,  3 byte array describing the RGB values to display for the given sprite
//    scaling_factor, how much to up-scale the bitmap by (recomended 2x);
//
void imageRender_drawSpriteB (uint32_t position, uint32_t sprite[], uint32_t width, uint32_t height, char color[], uint32_t scaling_factor);

// Purpose: Draw a number of blank lines starting at position
//  Takes:
//    position Where it should be drawn. calculated by (IMAGE_RENDER_ONE_ROW_IN_BYTES * Yloc + PIXEL_SIZE_IN_BYTES * COLUMN)
//    numLines, number of lines to draw
//    returns nothing
void imageRender_drawBlankLines(uint32_t position, uint32_t numLines);

// Purpose: Clears the screen of everything
//    NOTE: this operation is rather slow
//  Takes:
//     Nothing
void imageRender_clearScreen();

//SINGLE-PURPOSE Functions

//  Purpose:
//    Update where the player's tank appears on screen
//    Clears old position of the tank
//  Takes:
//    xPosition, x coordinate of upper left corner of the tank
void imageRender_drawNewTank(uint32_t newPosition);

//  Purpose:
//    Draw alienSwarm
//  Takes:
//    row:  upper left corner y coord
//    column: upper left corner x coord
//    aliensIn: If the aliens are in their "in" animiation or "out" animiation
//    liveAliens[NUM_ALIENS_X][NUM_ALIENS_Y]: 2d Matrix contiang the life/death state of all aliens
void imageRender_drawAliens(int32_t row, int32_t column, bool aliensIn, bool liveAliens[NUM_ALIENS_X][NUM_ALIENS_Y]);

//  Purpose:
//    Draw Alien flying saucer
//  Takes:
//    xPosition of upper left corner of the saucer
//  Uses:
void imageRender_drawNewSaucer(int32_t newPosition);

// Purpose:
//    Draw Player's Bullet
//  Takes:
//    row:  y coord to draw bullet at
//    column: x coord to draw bullet at
//    color:  what color to make the bullet
void imageRender_drawPlayerBullet(uint32_t row, uint32_t column, char color[]);

// Purpose:
//    Draw damage on regular parts of the bunker
//  Takes:
//    bunker:
//    sectionX
//    sectionY
//    section
//    health
void imageRender_drawBunkerDamage(uint32_t bunker, uint32_t sectionX, uint32_t sectionY, uint32_t section, uint32_t health);

//  Purpose: Draw the Player's current score
//  Takes:
//    Nothing
//  Uses:
//    global_playerScore
void imageRender_drawPlayerScore();

// Purpose
//    Draw Game Over Scroe Screen
//  Takes:
void imageRender_drawScoreScreen();

// Purpose:
//    Updates display shwoing howmany player lives remain
//  Takes:
//    lifeNumber: which life to draw/erase
//    draw:       add/remove a life
void imageRender_drawTankLife(uint32_t lifeNumber, bool draw);
#endif
