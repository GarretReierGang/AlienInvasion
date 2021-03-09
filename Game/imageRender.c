
#include "imageRender.h"
#include "globals.h"

//MACROS
#define hdmiDevPath "/dev/ecen427_hdmi"
#define HDMI_FAILED_TO_OPEN (hdmiFileDescriptor == -1)
#define BYTE_WIDTH (640 * 3)
#define BYTE_HEIGHT (480 * 3)
#define TOTAL_PIXELS (640 * 480) //307200
#define SCREEN_WIDTH_PIXELS 640
#define SCREEN_HEIGHT_PIXELS 480
#define TOTAL_BYTES (BYTE_WIDTH * BYTE_HEIGHT)
#define ALIEN_ROW_COUNT 11
#define SPACING_LETTER_LINES (IMAGE_RENDER_ONE_ROW_IN_BYTES * 12)
#define SPACING_SPACE PIXEL_SIZE_IN_BYTES*(SPACING_LETTER/2)
#define SPACING_BIG_A_WORDS   (PIXEL_SIZE_IN_BYTES * PIXELS_BETWEEN_A_WORDS)
#define SPACING_WORDS         (PIXEL_SIZE_IN_BYTES * PIXELS_BETWEEN_WORDS)
#define SPACING_BIG_A_LETTERS (PIXEL_SIZE_IN_BYTES *(LETTER_WIDTH * BIG_A_SCALING_FACTOR   + PIXELS_BETWEEN_BIG_A_LETTERS))
#define SPACING_TANKS         (PIXEL_SIZE_IN_BYTES *(TANK_WIDTH * SPRITE_SCALING_FACTOR    + PIXELS_BETWEEN_TANKS))
#define SPACING_BUNKERS       (PIXEL_SIZE_IN_BYTES *(BUNKER_WIDTH * SPRITE_SCALING_FACTOR  + PIXELS_BETWEEN_BUNKERS))
#define SPACING_ALIENS        (PIXEL_SIZE_IN_BYTES *(ALIEN_WIDTH * SPRITE_SCALING_FACTOR   + PIXELS_BETWEEN_ALIENS)) //84
#define START_HALL_OF_FAME_LIST (START_BYTE_HIGH_SCORES + SPACING_LETTER_LINES + SPACING_LETTER + IMAGE_RENDER_ONE_ROW_IN_BYTES)
#define ALIEN_NEW_LINE (IMAGE_RENDER_ONE_ROW_IN_BYTES * ALIEN_HEIGHT * SPRITE_SCALING_FACTOR) + (IMAGE_RENDER_ONE_ROW_IN_BYTES*PIXELS_BETWEEN_ALIEN_ROW) \
                       (- (11*SPACING_ALIENS))//(3*308)
#define SAUCER_Y_POSITION 30


#define BUTTON_DEBOUNCED 5
#define BLIPIXEL_SIZE_IN_BYTESTE 50
#define FIRST_LETTER 'a'
#define LAST_LETTER 'z'
#define GAME_SCREEN_INFO_SCORE "score"
#define HIGH_SCORES "high scores"
#define HIGH_SCORE_SAVE_LOCATION "high_score.txt"
#define PLAYER_NAME_LENGTH 3
#define MAX_SCORE_LENGTH 6
#define MAIN_GAME_SCREEN_NUMBER_OFFSET 5
#define MAIN_GAME_SCREEN_LIFE_TITLE "lives"
#define GAME_OVER_SCREEN_PROMPT "enter your name"
#define GAME_OVER_LAST_LETTER_POS 3
#define GAME_MAX_HIGHSCORE_PLAYER_COUNT 10
#define SCORE_INTEGER_COUNT 5
#define SCORE_ARRAY_SIZE 6
#define PLAYER_NAME_ARRAY_SIZE 4
#define DRAW 1
#define START_ALIENS_OUT 0
#define TOTAL_BYTES_IN_SCREEN TOTAL_PIXELS * PIXEL_SIZE_IN_BYTES
#define ROW_COUNT 640
#define RUN 97
#define DRAW_ALIEN true
#define ALIEN_STEP_LENGTH 10
#define MAX_WRITE_SIZE (128)
#define SAUCER_NEW_LINE (IMAGE_RENDER_ONE_ROW_IN_BYTES - (MOVING_SAUCER_WIDTH * SPRITE_SCALING_FACTOR) \
                        (- (2 * saucer_spixelsPerStep)))
#define SAUCER_CLEAR_NEWLINE (IMAGE_RENDER_ONE_ROW_IN_BYTES - LETTER_WIDTH * SPRITE_SCALING_FACTOR * PIXEL_SIZE_IN_BYTES)
#define NUM_PIXELS_ON_SIDE_CALC(x, beforeAlien) ( (x == 0 && beforeAlien) ? (ALIEN_CONTROL_STEP_LEN+1) : (x == ALIEN_ROW_COUNT-1 && !beforeAlien) ? (ALIEN_CONTROL_STEP_LEN+1): PIXELS_ON_EACH_ALIENS_SIDE)
#define TANK_TRAILING_PIXELS 3
#define BIT_BUFFER_OFFSET_0 0
#define BIT_BUFFER_OFFSET_1 1
#define BIT_BUFFER_OFFSET_2 2
#define NO_SCALING_FACTOR 1

//GLOBALS
static char blackRow[IMAGE_RENDER_ONE_ROW_IN_BYTES];
char black[SINGLE_PIXEL] = {0x00, 0x00, 0x00};
char white[SINGLE_PIXEL] = {0xFF, 0xFF, 0xFF};
char red[SINGLE_PIXEL] = {0xFF, 0x00, 0x00};
char green[SINGLE_PIXEL] = {0x00, 0xFF, 0x00};
char blue[SINGLE_PIXEL] = {0x00, 0x00, 0xFF};
char yellow[SINGLE_PIXEL] = {0xFF, 0xFF, 0x00};


static char playerName[PLAYER_NAME_ARRAY_SIZE] = {'a','a','a','\0'};
static char** hallOfFamePlayers = NULL;
static bool buttonHeld = false;
static bool buttonStateChange = false; //https://github.com/dukesook/Ec-En-427-Lab-3
static bool blank = false;
static bool game_not_over = true;
// static int32_t hdmiFileDescriptor; //used to perform r/w operations

static uint32_t buttonDebounceTimer = 0;
static uint32_t currentButton;
static uint32_t previousButton;
static uint32_t currentLetter = 0;
static uint32_t blinkTimer = 0;
static uint32_t letterPosition = START_BYTE_NAME_ENTERING_SPOT;
static uint32_t* highScores = NULL;
static uint32_t numPlayersInHallOfFame = 0;
static uint32_t bitBufferOffset = 0;
static char bitBuffer[IMAGE_RENDER_ONE_ROW_IN_BYTES];

//HELPER FUNCTIONS

//Purpose add a single horizontally scaled bit of an image to the bit Buffer
//  Takes:
//    color, 3 byte char containing the color it should draw
//    scaling factor, number of pixels this single bit is scaled to
//  Uses:
//    bitBuffer, stores the pixels to be written later
//    bitBufferOffset: Increments the offset as it adds pixels to the buffer
static void drawBit(char color[SINGLE_PIXEL], uint32_t scaling_factor) {
  for (uint32_t x = 0; x < scaling_factor; ++x)
  { //write scalining_factor number of bits
    bitBuffer[bitBufferOffset] = color[BIT_BUFFER_OFFSET_0];
    bitBuffer[BIT_BUFFER_OFFSET_1 + bitBufferOffset] = color[BIT_BUFFER_OFFSET_1];
    bitBuffer[BIT_BUFFER_OFFSET_2 + bitBufferOffset] = color[BIT_BUFFER_OFFSET_2];
    bitBufferOffset += SINGLE_PIXEL;
  } //Finished writing scaling factor
}

// Purpose
//     Draw the initial game screen
// Dependencies:
//  imageRender_drawWord
//  imageRender_drawPlayerScore
//  imageRender_drawSpriteB
static void drawStartScreen(){
  // DrawStart Screen
  // a long function that draws sprites on the SCREEN
  // so that the player can start the game.
  //LOCAL VARIABLES
  uint32_t cursor;

  //DRAW "SCORE 00000"
  cursor = START_BYTE_SCORE00000;
  imageRender_drawWord(GAME_SCREEN_INFO_SCORE, (uint32_t) sizeof(GAME_SCREEN_INFO_SCORE), cursor, IMAGE_RENDER_STANDARD_LETTER_COLOR);
  imageRender_drawPlayerScore();

  //DRAW "LIVES"
  cursor = START_BYTE_LIVES;
  imageRender_drawWord(MAIN_GAME_SCREEN_LIFE_TITLE, (uint32_t) sizeof(MAIN_GAME_SCREEN_LIFE_TITLE), cursor, IMAGE_RENDER_STANDARD_LETTER_COLOR);

  //DRAW PLAYER-LIVES
  cursor = START_BYTE_TANK_LIVES;
  for (uint32_t i = 0; i < PLAYER_STARTING_LIVES; i++){ // Draw the Players starting lives on the screen
    imageRender_drawSpriteB(cursor, tank_15x8, TANK_WIDTH, TANK_HEIGHT, green, SPRITE_SCALING_FACTOR);
    cursor += SPACING_TANKS;
  }

  //DRAW BUNKERS
  cursor = START_BYTE_BUNKERS;
  imageRender_drawSpriteB(cursor, bunker_24x18, BUNKER_WIDTH, BUNKER_HEIGHT, green, SPRITE_SCALING_FACTOR);
  cursor += SPACING_BUNKERS;
  imageRender_drawSpriteB(cursor, bunker_24x18, BUNKER_WIDTH, BUNKER_HEIGHT, green, SPRITE_SCALING_FACTOR);
  cursor += SPACING_BUNKERS;
  imageRender_drawSpriteB(cursor, bunker_24x18, BUNKER_WIDTH, BUNKER_HEIGHT, green, SPRITE_SCALING_FACTOR);
  cursor += SPACING_BUNKERS;
  imageRender_drawSpriteB(cursor, bunker_24x18, BUNKER_WIDTH, BUNKER_HEIGHT, green, SPRITE_SCALING_FACTOR);

  //DRAW TANK
  imageRender_drawSpriteB(START_BYTE_TANK, tank_15x8, TANK_WIDTH, TANK_HEIGHT, green, SPRITE_SCALING_FACTOR);
}

// Purpose
//    add a single row of a sprite to the byte buffer, for more efficient alienSwarm display
// takes:
//    currentColumn, current x pixel in the screen to write to.
//    bytesTilNextLine, Used to calcualte how far to jump to move the pointer to the next line
//    sprite, sprite we are pulling a row of pixels out of
//    width,  number of pixels wide the sprite is.
//    color, 3 byte array detalining what color to colorize the aliens with
//    scaling_factor, how many pixels each of hte bitmaps pixels are worth
//    currentRow, current Row to output to the bitBuffer
//    draw, determines if we just blackfill for this sprite.
//  Dependencies:
//    DrawBit- scales each of the pixels horizontally by scaling_factor
static void createRowBuffer(int32_t* currentColumn, int32_t* bytesTilNextline, uint32_t sprite[], uint32_t width, char color[], uint32_t scaling_factor, uint32_t currentRow, bool draw) {
  int32_t bytesPerBit = PIXEL_SIZE_IN_BYTES * scaling_factor;
  for (uint32_t x = 0; x < width; ++ x)
  { //Loop trough each bit of the bitmap
    if ( (*currentColumn) >= 0 && (*currentColumn) < (SCREEN_WIDTH_PIXELS-scaling_factor))
    { //draw something if the currentColumn is within the Screen Boundry
      if (draw && (sprite[currentRow] & ( 1 << (width - 1 - x))) ) { //Draw scaled colorized pixel
        drawBit(color, scaling_factor); //draw bit
      }
      else
      { //Draw scaled IMAGE_RENDER_BACKGROUND_COLOR pixel
        drawBit(IMAGE_RENDER_BACKGROUND_COLOR, scaling_factor); //draw bit
      }
      *bytesTilNextline -= bytesPerBit;
    }
    (*currentColumn) += scaling_factor;
  } //Finished looping thru each pixel in the bitmap
}

// Purpose
//  add IMAGE_RENDER_BACKGROUND_COLOR space to the buffer,
//  and increment the column
// Takes:
//    currentColumn, current x pixel in the screen to write to.
//    bytesTilNextLine, Used to calcualte how far to jump to move the pointer to the next line
//    width,  number of pixels wide of blank space to write
//    color, 3 byte array detalining what color to write the blank space in.
//  Dependencies:
//    DrawBit- scales each of the pixels horizontally by scaling_factor
static void createBlankSpaceBuffer(int32_t* column, int32_t* bytesTilNextline, uint32_t width, char color[]) {
  for (uint32_t x = 0; x < width; ++x)
  { //loop to write width number of blank pixels
    if ( ((*column) >= 0) && (*column) < SCREEN_WIDTH_PIXELS)
    { //the column is within screen boundry, therefore ad a pixel to the buffer
        drawBit(color, NO_SCALING_FACTOR); //draw bit
        *bytesTilNextline -= PIXEL_SIZE_IN_BYTES;
    }
    (*column) += 1;
  }
}

// Purpose:
//    draw the Bunker pieces/damage for the portions of the bunker where corner's are missing
// Takes:
//    Starting Location, the byte psition to start drawing the sprite
//    damageSprite, the sprite of the damage to inflic on the bunker
//    cornerSprite, sprite of the corner to inflict damage on
//    section, which section to deal damage to.
//  Dependencies:
//    DrawBit- scales each of the pixels horizontally by scaling_factor
static void drawBunkerMissingCorner(uint32_t startingLocation, uint32_t* damageSprite, uint32_t* cornerSprite, uint32_t section){
  //LOCAL VARIABLES
   uint32_t row, column;
   uint32_t newLine = BYTE_WIDTH - (BUNKER_SECTION_WIDTH * PIXEL_SIZE_IN_BYTES * SPRITE_SCALING_FACTOR);
   bool damageBit = true;
   bool cornerBit = true;

   //SET CURSOR
   lseek(hdmiFileDescriptor, startingLocation, SEEK_SET);

   //DRAW
   for (row = 0; row < BUNKER_SECTION_HEIGHT; row++){ //Iterate through each y pixel of the original image
      for (uint32_t scale = 0; scale < SPRITE_SCALING_FACTOR; scale++) { //to account for scaled up images, print pixels on extra lines scaling_factor times
         bitBufferOffset = 0;
         for (column = 0; column < BUNKER_SECTION_WIDTH; column++) { // iterate though each x pixel
            damageBit = damageSprite[row] & (1 << (BUNKER_SECTION_WIDTH - 1 - column));
            cornerBit = cornerSprite[row] & (1 << (BUNKER_SECTION_WIDTH - 1 - column));
            if (damageBit && cornerBit) { // draw a bit in color if a pixel should be drawn at the given location.
                drawBit(green, SPRITE_SCALING_FACTOR); //draw bit
            }
            else {
              drawBit(IMAGE_RENDER_BACKGROUND_COLOR, SPRITE_SCALING_FACTOR); //clear bit
            }
         }
         write(hdmiFileDescriptor, bitBuffer, bitBufferOffset);
         lseek(hdmiFileDescriptor, newLine, SEEK_CUR); //Next line
      }
   }
}


// Purpose:
//    Draws an alien Row
//  Takes:
//    Column, pixel coordinate of the left most column of the alien Swarm
//    alienRow,    identity of the alien Row to drawn
//    alienSprite:  which alien sprite to draw.
//    aliens[][]:   2d array containing information regarding which aliens are alive/dead
void drawAlienRow(int32_t column, int32_t alienRow, uint32_t* alienSprite, bool aliens[NUM_ALIENS_X][NUM_ALIENS_Y]){
  // IMAGE_RENDER_ONE_ROW_IN_BYTES 1

  for (uint32_t y = 0; y < ALIEN_HEIGHT; ++y)
  {
    for (uint32_t scaleY = 0; scaleY < SPRITE_SCALING_FACTOR; ++scaleY)
    {
      bitBufferOffset = 0;
      int32_t tempColumn = column;
      uint32_t newLine = IMAGE_RENDER_ONE_ROW_IN_BYTES;
      for (uint32_t x = 0; x < ALIEN_ROW_COUNT; x++){ //iterativly draw each of the aliens
        bool beforeAlien = true;
        createBlankSpaceBuffer(&tempColumn, &newLine, NUM_PIXELS_ON_SIDE_CALC(x, beforeAlien), IMAGE_RENDER_BACKGROUND_COLOR);
        createRowBuffer(&tempColumn, &newLine, alienSprite, ALIEN_WIDTH, ALIEN_COLOR, SPRITE_SCALING_FACTOR, y, aliens[x][alienRow]);
        beforeAlien = false;
        createBlankSpaceBuffer(&tempColumn, &newLine, NUM_PIXELS_ON_SIDE_CALC(x, beforeAlien), IMAGE_RENDER_BACKGROUND_COLOR);
      }
      write(hdmiFileDescriptor, bitBuffer, bitBufferOffset );
      lseek(hdmiFileDescriptor, newLine, SEEK_CUR); //Next line
    }
  }
  for (uint32_t i = 0; i < ALIEN_VERTICAL_SPACING; i++){
    write(hdmiFileDescriptor, blackRow, IMAGE_RENDER_ONE_ROW_IN_BYTES);
  };
}




//HEADER FUNCITONS
bool imageRender_init() {
  // imageRender_init()
  // performs the necesiarry actions to initialize the HDMI port
  // returns the success of the operation
	//OBTAIN FILE DESCRIPTOR
  hdmiFileDescriptor = open(hdmiDevPath, O_RDWR);
  if (HDMI_FAILED_TO_OPEN) { //failed to open the HDMI port, print an error message and report a failure
    printf("ERROR imageRender_init(): Failed to open hdmi File Descriptor\n");
    printf("Make sure you are using sudo\n");
    return false; //failed to open
  }

  //CLEAR SCREEN
  imageRender_clearScreen();

  //DRAW START SCREEN
  drawStartScreen();

  //!!!THESE DO NOT BELONG IN THIS FILE!!!//
  intc_init(INTC_ID);
  gpio_button_init();// GPIO_BUTTON_ID);
  intc_irq_enable(GPIO_BUTTON_INTC_MASK | GPIO_FIT_INTC_MASK);
  intc_irq_disable(GPIO_SWITCH_INTC_MASK);
  intc_enable_uio_interrupts();

  //SUCCESSFULL INITIAIZLIATION
  return true;
}

//MULTI-PURPOSE

// Purpose:
//    draw a sprite at a given location, does not handle writing beyond screen boundries
//  Takes:
//    Starting Location, the byte psition to start drawing the sprite
//    sprite:             bitmap of the sprite to draw
//    width:              Width of the sprite
//    height:             The height of the sprite
//    color:              3Byte array specifiying what color to write at each of the bitmap's pixels
//    scaling_factor:     How much to scale the image by
//  Dependencies:
//    DrawBit- scales each of the pixels horizontally by scaling_factor
void imageRender_drawSpriteB (uint32_t startingLocation, uint32_t sprite[], uint32_t width, uint32_t height, char color[], uint32_t scaling_factor) {
  //imageRender_drawSpriteB ()
  // takes a position to place the hdmi cursor at, a sprite to print at that location, and the width/height of the sprite
  //
 //LOCAL VARIABLES
  uint32_t row, column;
  uint32_t newLine = BYTE_WIDTH - (width * PIXEL_SIZE_IN_BYTES * scaling_factor);

  //SET CURSOR
  lseek(hdmiFileDescriptor, startingLocation, SEEK_SET);

  //DRAW
  for (row = 0; row < height; row++){ //Iterate through each y pixel of the original image
     for (uint32_t scale = 0; scale < scaling_factor; scale++) { //to account for scaled up images, print pixels on extra lines scaling_factor times
        bitBufferOffset = 0;
        for (column = 0; column < width; column++) { // iterate though each x pixel
           if (sprite[row] & (1 << (width - 1 - column))) { // draw a bit in color if a pixel should be drawn at the given location.
             drawBit(color, scaling_factor); //draw bit
           }
           else {
             drawBit(IMAGE_RENDER_BACKGROUND_COLOR, scaling_factor); //clear bit
           }
        }
        write(hdmiFileDescriptor, bitBuffer, bitBufferOffset );
        lseek(hdmiFileDescriptor, newLine, SEEK_CUR); //Next line
     }
  }
}

// Purpose:
//    draw a sprite at a location, specified by x,y coordinates. Will not print any portion of the sprite that leaves the edge of the screen
//  Takes:
//    Starting Location, the byte psition to start drawing the sprite
//    sprite:             bitmap of the sprite to draw
//    width:              Width of the sprite
//    height:             The height of the sprite
//    color:              3Byte array specifiying what color to write at each of the bitmap's pixels
//    scaling_factor:     How much to scale the image by
//  Dependencies:
//    DrawBit- scales each of the pixels horizontally by scaling_factor
void imageRender_drawSprite(int32_t col, int32_t row, uint32_t sprite[], uint32_t width, uint32_t height, char color[], uint32_t scaling_factor ){
  //LOCAL VARIABLES
  uint32_t newLine = IMAGE_RENDER_ONE_ROW_IN_BYTES;
  uint32_t initialColumn = (col < 0) ? -col : 0; //Every pixel to the left of 0 the initial column is, move column to the left by the amount
  uint32_t initalRow = (row < 0) ? -row : 0;
  uint32_t startingLocation = ( (col < 0) ? 0 : col * PIXEL_SIZE_IN_BYTES) + ( (row < 0) ? 0 : row * IMAGE_RENDER_ONE_ROW_IN_BYTES);

  lseek(hdmiFileDescriptor, startingLocation, SEEK_SET);

  for (uint32_t y = initalRow; y < height; ++ y)
  { //Loop through each and every row of the image, does not write above pixels for rows that are above the edge of the screen
    for (uint32_t i = 0; i < scaling_factor && (y + row + i < SCREEN_HEIGHT_PIXELS); ++i)
    { //vertical scaling, check to ensure that the row has not reached the bottom of the screen
      bitBufferOffset = 0;
      newLine = IMAGE_RENDER_ONE_ROW_IN_BYTES;
      for (uint32_t x = initialColumn; (x < width) && (x*scaling_factor + col < SCREEN_WIDTH_PIXELS); ++ x)
      {
        if (sprite[y] & (1 << (width - 1 - x)))
        { //Bit map has a colored pixel here
          drawBit(color, scaling_factor); //draw bit
        }
        else { //bitmap does not have a colored pixel
          drawBit(IMAGE_RENDER_BACKGROUND_COLOR, scaling_factor); //clear bit
        }
        newLine -= scaling_factor * PIXEL_SIZE_IN_BYTES; //decrimenting the newLine so that it can be used to move the cursor the appropriate number of bytes to start on the next line
      }
      write(hdmiFileDescriptor, bitBuffer, bitBufferOffset );
      lseek(hdmiFileDescriptor, newLine, SEEK_CUR); //Next line
    }
  }
}

// Purpose: Draws an entire word at a given position.
// Takes:
//  word char[] (a-z0-9 )+ to draw
//  length num chars to draw
//  position Where it should be drawn. calculated by (IMAGE_RENDER_ONE_ROW_IN_BYTES * Yloc + PIXEL_SIZE_IN_BYTES * COLUMN)
//  color 3 byte array contiang RGB values, or color of the letters to draw
//  Returns the location of where a following word//letter could start
// Dependencies:
//  imageRender_drawSpriteB
uint32_t imageRender_drawLetter(char letter, uint32_t scaling_factor, uint32_t position, char color[]) {
  // imageRender_drawLetter( what Letter to draw, how big of a letter to draw, where to draw it, what color to draw it as)
  // takes an ascii letter and draws it on the screen
  switch(letter) //call imageRender_drawSpriteB with the correct sprite for the given letter/number, and draw it at position, in color, scaled up by scaling_factor
  {
    case 'a':
      imageRender_drawSpriteB(position, letterA_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'b':
      imageRender_drawSpriteB(position, letterB_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'c':
      imageRender_drawSpriteB(position, letterC_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'd':
      imageRender_drawSpriteB(position, letterD_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'e':
      imageRender_drawSpriteB(position, letterE_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'f':
      imageRender_drawSpriteB(position, letterF_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'g':
      imageRender_drawSpriteB(position, letterG_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'h':
      imageRender_drawSpriteB(position, letterH_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'i':
      imageRender_drawSpriteB(position, letterI_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'j':
      imageRender_drawSpriteB(position, letterJ_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'k':
      imageRender_drawSpriteB(position, letterK_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'l':
      imageRender_drawSpriteB(position, letterL_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'm':
      imageRender_drawSpriteB(position, letterM_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'n':
      imageRender_drawSpriteB(position, letterN_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'o':
      imageRender_drawSpriteB(position, letterO_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'p':
      imageRender_drawSpriteB(position, letterP_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'q':
      imageRender_drawSpriteB(position, letterQ_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'r':
      imageRender_drawSpriteB(position, letterR_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 's':
      imageRender_drawSpriteB(position, letterS_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 't':
      imageRender_drawSpriteB(position, letterT_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'u':
      imageRender_drawSpriteB(position, letterU_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'v':
      imageRender_drawSpriteB(position, letterV_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'w':
      imageRender_drawSpriteB(position, letterW_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'x':
      imageRender_drawSpriteB(position, letterX_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'y':
      imageRender_drawSpriteB(position, letterY_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case 'z':
      imageRender_drawSpriteB(position, letterZ_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case ' ':
      imageRender_drawSpriteB(position, letterBLANK_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '0':
      imageRender_drawSpriteB(position, number0_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '1':
      imageRender_drawSpriteB(position, number1_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '2':
      imageRender_drawSpriteB(position, number2_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '3':
      imageRender_drawSpriteB(position, number3_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '4':
      imageRender_drawSpriteB(position, number4_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '5':
      imageRender_drawSpriteB(position, number5_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '6':
      imageRender_drawSpriteB(position, number6_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '7':
      imageRender_drawSpriteB(position, number7_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '8':
      imageRender_drawSpriteB(position, number8_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
    case '9':
      imageRender_drawSpriteB(position, number9_5x5, LETTER_WIDTH, LETTER_HEIGHT, color, scaling_factor);
      break;
  }
  return position;
}

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
//  Dependencies:
//    imageRender_drawLetter
uint32_t imageRender_drawWord(char word[], uint32_t length, uint32_t cursor, char color[]){
  // imageRender_drawWord (what set of chars to output), the length of the word, where to start drawing it, and what color to draw them in
    //LOCAL VARIABLES
    for (uint32_t i = 0; i < length; i++){ //iterate through each char and draw it
      imageRender_drawLetter(word[i], SPRITE_SCALING_FACTOR, cursor, color);
      if (word[i] == ' ')
      {//if the char was a space, use the SPACING for a space, makes the kern look better
        cursor += SPACING_WORDS;
      }
      else { //if the char is not a space use regular letter spacing, does not handle the kern for I's very well
        cursor += SPACING_LETTER;
      }
    }
    return cursor;
}

// Purpose: Clears the screen of everything
//    NOTE: this operation is rather slow
//  Takes:
//     Nothing
void imageRender_clearScreen() {
  // imageRender_clearScreen()
  // This function fills the screen with black pixels
  lseek(hdmiFileDescriptor, 0, SEEK_SET); //Next line
  // for (uint32_t i = 0; i < TOTAL_PIXELS; i++)
  //   write(hdmiFileDescriptor, black, SINGLE_PIXEL);

  for (uint32_t i = 0; i < ROW_COUNT; i++){ //Write a row of IMAGE_RENDER_BACKGROUND_COLOR pxiels until every row has been written to
    write(hdmiFileDescriptor, blackRow, IMAGE_RENDER_ONE_ROW_IN_BYTES);
  }
}

// Purpose: Draw a number of blank lines starting at position
//  Takes:
//    position Where it should be drawn. calculated by (IMAGE_RENDER_ONE_ROW_IN_BYTES * Yloc + PIXEL_SIZE_IN_BYTES * COLUMN)
//    numLines, number of lines to draw
//    returns nothing
void imageRender_drawBlankLines(uint32_t cursor, uint32_t numLines) {
  lseek(hdmiFileDescriptor, cursor, SEEK_SET); //Next line
  for (uint32_t i = 0; i < numLines; i++){ //Write a row of IMAGE_RENDER_BACKGROUND_COLOR pixels, until the numLines has been reached
    write(hdmiFileDescriptor, blackRow, IMAGE_RENDER_ONE_ROW_IN_BYTES);
  }
}

//  Purpose: Draw the Player's current score
//  Takes:
//    Nothing
//  Uses:
//    global_playerScore
//  Dependencies:
//    imageRender_drawWord;
void imageRender_drawPlayerScore(){

  // drawPlayerScore()
  // uses the global variable "playerScore" and converts it from a uint into a 5 digit number stream, and draws it on the screen
    uint32_t cursor = START_BYTE_SCORE00000 + SPACING_LETTER * MAIN_GAME_SCREEN_NUMBER_OFFSET;
    char score [SCORE_ARRAY_SIZE];
    sprintf(score, "%05d", global_playerScore); //convert playerScore from a uint32_t to a char array.
    imageRender_drawWord(score, SCORE_INTEGER_COUNT, cursor, IMAGE_RENDER_SCORE_LETTER_COLOR);
}

//  Purpose:
//    Update where the player's tank appears on screen
//    Clears old position of the tank
//  Takes:
//    xPosition, x coordinate of upper left corner of the tank
//  Dependencies:
//    imageRender_drawSpriteB
//
void imageRender_drawNewTank(uint32_t xPosition){
  //LOCAL VARIABLES
  //Magic NUMBER HERE
  uint32_t newTankPosition = (IMAGE_RENDER_ONE_ROW_IN_BYTES*START_ROW_TANK) + ( (xPosition -TANK_TRAILING_PIXELS*SPRITE_SCALING_FACTOR) * PIXEL_SIZE_IN_BYTES);

  lseek(hdmiFileDescriptor, newTankPosition, SEEK_SET);
  for (uint32_t y = 0; y < TANK_HEIGHT*SPRITE_SCALING_FACTOR;++y)
  { //draw IMAGE_RENDER_BACKGROUND_COLOR space to errase the remminates of the old tank
    bitBufferOffset = 0;
    drawBit(IMAGE_RENDER_BACKGROUND_COLOR, 3*SPRITE_SCALING_FACTOR);
    write(hdmiFileDescriptor, bitBuffer, bitBufferOffset);
    lseek(hdmiFileDescriptor, (IMAGE_RENDER_ONE_ROW_IN_BYTES -  (TANK_TRAILING_PIXELS*SPRITE_SCALING_FACTOR*PIXEL_SIZE_IN_BYTES) ) , SEEK_CUR);
  }


  newTankPosition = (IMAGE_RENDER_ONE_ROW_IN_BYTES*START_ROW_TANK) + ( (xPosition) * PIXEL_SIZE_IN_BYTES);
  //DRAW TANK
  imageRender_drawSpriteB(newTankPosition, tank_19x8, MOVING_TANK_WIDTH,
            TANK_HEIGHT, green, SPRITE_SCALING_FACTOR);
}

//  Purpose:
//    Draw alienSwarm
//  Takes:
//    row:  upper left corner y coord
//    column: upper left corner x coord
//    aliensIn: If the aliens are in their "in" animiation or "out" animiation
//    liveAliens[NUM_ALIENS_X][NUM_ALIENS_Y]: 2d Matrix contiang the life/death state of all aliens
//  Uses:
//  Dependencies:
//    createBlankSpaceBuffer
//    createRowBuffer
void imageRender_drawAliens(int32_t row, int32_t column, bool aliensIn, bool liveAliens[NUM_ALIENS_X][NUM_ALIENS_Y]){
  //LOCAL VARIABLES
  uint32_t cursor = (row-ALIEN_VERTICAL_SPACING)*IMAGE_RENDER_ONE_ROW_IN_BYTES + ( (column < 0) ? 0 : (column)*PIXEL_SIZE_IN_BYTES);
  uint32_t* alienTop    = aliensIn? alien_top_in_12x8: alien_top_out_12x8;
  uint32_t* alienMiddle = aliensIn? alien_middle_in_12x8: alien_middle_out_12x8;
  uint32_t* alienBottom = aliensIn? alien_bottom_in_12x8: alien_bottom_out_12x8;

  //writing IMAGE_RENDER_BACKGROUND_COLOR lines for ALIEN step distance to make walking down easy for the aliens
  lseek(hdmiFileDescriptor, cursor, SEEK_SET);
  for (uint32_t i = 0; i < ALIEN_VERTICAL_SPACING; i++){
    write(hdmiFileDescriptor, blackRow, IMAGE_RENDER_ONE_ROW_IN_BYTES);

  };
  //TOP row
  drawAlienRow (column, ALIEN_ROW_1, alienTop, liveAliens);
  drawAlienRow (column, ALIEN_ROW_2, alienMiddle, liveAliens);
  drawAlienRow (column, ALIEN_ROW_3, alienMiddle, liveAliens);
  drawAlienRow (column, ALIEN_ROW_4, alienBottom, liveAliens);
  //BOTTOM row
  drawAlienRow (column, ALIEN_ROW_5, alienBottom, liveAliens);
}

//  Purpose:
//    Draw Alien flying saucer
//  Takes:
//    xPosition of upper left corner of the saucer
//  Uses:
//  Dependencies:
//    imageRender_drawSprite
void imageRender_drawNewSaucer(int32_t xPosition){
  //Overwrite last location saucer was at with IMAGE_RENDER_BACKGROUND_COLOR pixels
  imageRender_drawSprite(xPosition -  (LETTER_WIDTH)*SPRITE_SCALING_FACTOR, SAUCER_Y_POSITION, saucer_19x7, LETTER_WIDTH,
             SAUCER_HEIGHT, IMAGE_RENDER_BACKGROUND_COLOR, SPRITE_SCALING_FACTOR);

  //DRAW SAUCER
  imageRender_drawSprite(xPosition, SAUCER_Y_POSITION, saucer_19x7, MOVING_SAUCER_WIDTH,
             SAUCER_HEIGHT, blue, SPRITE_SCALING_FACTOR);
}
// Purpose
//    Draw Game Over Scroe Screen
//  Takes:
//  Uses:
//    imageRender_drawSpriteB
//    imageRender_drawWord
void imageRender_drawScoreScreen() {
  // End of GAME screen, draw this when the game is over!
  //LOCAL VARIABLES
  uint32_t cursor;

  //writes "Game Over", as this is written using large letters, imageRender_drawWord does not work
  // as it needs a different kerning distance for spaces to look good.
  cursor = START_BYTE_GAME_OVER;
  imageRender_drawSpriteB(cursor, letterG_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, BIG_A_SCALING_FACTOR);
  cursor += SPACING_BIG_A_LETTERS;
  imageRender_drawSpriteB(cursor, letterA_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, BIG_A_SCALING_FACTOR);
  cursor += SPACING_BIG_A_LETTERS;
  imageRender_drawSpriteB(cursor, letterM_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, BIG_A_SCALING_FACTOR);
  cursor += SPACING_BIG_A_LETTERS;
  imageRender_drawSpriteB(cursor, letterE_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, BIG_A_SCALING_FACTOR);
  cursor += SPACING_BIG_A_LETTERS;
  cursor += SPACING_BIG_A_WORDS;
  imageRender_drawSpriteB(cursor, letterO_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, BIG_A_SCALING_FACTOR);
  cursor += SPACING_BIG_A_LETTERS;
  imageRender_drawSpriteB(cursor, letterV_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, BIG_A_SCALING_FACTOR);
  cursor += SPACING_BIG_A_LETTERS;
  imageRender_drawSpriteB(cursor, letterE_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, BIG_A_SCALING_FACTOR);
  cursor += SPACING_BIG_A_LETTERS;
  imageRender_drawSpriteB(cursor, letterR_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, BIG_A_SCALING_FACTOR);

  //Write "Enter Your Name"
  cursor = START_BYTE_ENTER_NAME;
  imageRender_drawWord(GAME_OVER_SCREEN_PROMPT, (uint32_t) sizeof(GAME_OVER_SCREEN_PROMPT), cursor, IMAGE_RENDER_STANDARD_LETTER_COLOR );

  //Display the Current name you are entering
  cursor = START_BYTE_NAME_ENTERING_SPOT;
  imageRender_drawSpriteB(cursor, letterA_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, SPRITE_SCALING_FACTOR);
  cursor += SPACING_LETTER;
  imageRender_drawSpriteB(cursor, letterA_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, SPRITE_SCALING_FACTOR);
  cursor += SPACING_LETTER;
  imageRender_drawSpriteB(cursor, letterA_5x5, LETTER_WIDTH, LETTER_HEIGHT, IMAGE_RENDER_STANDARD_LETTER_COLOR, SPRITE_SCALING_FACTOR);

}

// Purpose:
//    Draw Player's Bullet
//  Takes:
//    row:  y coord to draw bullet at
//    column: x coord to draw bullet at
//    color:  what color to make the bullet
void imageRender_drawPlayerBullet(uint32_t row, uint32_t column, char color[SINGLE_PIXEL]){
  //LOCAL VARIABLES
  uint32_t cursor;

  //SET CURSOR
  //row -= (TANK_BULLET_HEIGHT * SPRITE_SCALING_FACTOR);
  cursor = IMAGE_RENDER_ONE_ROW_IN_BYTES*row + PIXEL_SIZE_IN_BYTES*column;

  //DRAW BULLET
  //imageRender_drawSpriteB (uint32_t startingLocation, uint32_t sprite[], uint32_t width, uint32_t height, char color[], uint32_t scaling_factor)
  imageRender_drawSpriteB(cursor, tankbullet_1x5, TANK_BULLET_WIDTH, TANK_BULLET_HEIGHT, color, SPRITE_SCALING_FACTOR);


}

//BUNKER SECTIONS
/*     _______   _________  _________  _______
      |      |  |        |  |       |  |     \
     |   0   |  |   1    |  |   2   |  |   3  \
    |________|  |________|  |_______|  |_______\
    _________   _________  _________  _________
    |        |  |        /  \       |  |       |
    |   4    |  |   5   /    \  6   |  |   7   |
    |________|  |______/      \_____|  |_______|
    _________                          _________
    |        |                         |       |
    |   8    |                         |   9   |
    |________|                         |_______|

*/

// Purpose:
//    Draw damage on regular parts of the bunker
//  Takes:
//    bunker: idetifies which of the 4 bunkers
//    sectionX: identifies the x coordinate of a bunker
//    sectionY: indentifies the y coordinate of a bunker
//    section: Identifies which of the 10 bunker sections is it
//    health: determins  how much to erode
void imageRender_drawBunkerDamage(uint32_t bunker, uint32_t sectionX, uint32_t sectionY, uint32_t section, uint32_t health){

  //LOCAL VARIABLES
  uint32_t cursor;
  uint32_t* damageSprite;
  uint32_t* cornerSprite;

  //SET CURSOR
   cursor = START_BYTE_BUNKERS + bunker*NEXT_BUNKER*PIXEL_SIZE_IN_BYTES; //find the correct bunker
   cursor += sectionX * BUNKER_SECTION_WIDTH  * PIXEL_SIZE_IN_BYTES * SPRITE_SCALING_FACTOR; //x section
   cursor += sectionY * BUNKER_SECTION_HEIGHT * IMAGE_RENDER_ONE_ROW_IN_BYTES * SPRITE_SCALING_FACTOR; //y section

   //GET DAMAGED SPRITE
  switch (health){
    case BUNKER_DAMAGE_DEAD:
      damageSprite = bunkerGone_6x6; //dead
      break;
    case BUNKER_DAMAGE_0:
      damageSprite = bunkerDamage0_6x6;//significant damage
      break;
    case BUNKER_DAMAGE_1:
      damageSprite = bunkerDamage1_6x6; //little damage
      break;
    case BUNKER_DAMAGE_2:
      damageSprite = bunkerDamage2_6x6; //no damage
      break;
    }

    //HANDLE CORNER SECTIONS
    switch (section){
      case MISSING_TOP_LEFT:
        cornerSprite = bunker_upper_left_gone_6x6;
        break;
      case MISSING_TOP_RIGHT:
        cornerSprite = bunker_upper_right_gone_6x6;
        break;
      case MISSING_BOTTOM_LEFT:
        cornerSprite = bunker_lower_left_gone_6x6;
        break;
      case MISSING_BOTTOM_RIGHT:
        cornerSprite = bunker_lower_right_gone_6x6;
        break;
      default: //Full Square
        cornerSprite = bunkerDamage3_6x6;
        break;
    }

    //DRAW DAMAGED BUNKER
    drawBunkerMissingCorner(cursor, damageSprite, cornerSprite, section);
}

// Purpose:
//    Updates display shwoing howmany player lives remain
//  Takes:
//    lifeNumber: which life to draw/erase
//    draw:       add/remove a life
void imageRender_drawTankLife(uint32_t lifeNumber, bool draw){
  //LIVES 1 2 3 4 5
  char* color;
  color = (draw) ? IMAGE_RENDER_SCORE_LETTER_COLOR: IMAGE_RENDER_BACKGROUND_COLOR;

  lifeNumber--; //start as if tank 1 is in positin 0
  uint32_t cursor = START_BYTE_TANK_LIVES + (lifeNumber * SPACING_TANKS);
  imageRender_drawSpriteB(cursor, tank_15x8, TANK_WIDTH, TANK_HEIGHT, color, SPRITE_SCALING_FACTOR);
  printf("ERASE LIFE %d\n", lifeNumber);


}
