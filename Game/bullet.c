
#include "bullet.h"
#include "player.h"
#include "bunker.h"
#include "imageRender.h"
#include <stdio.h>
#include <string.h>
#include "audioPlayer.h"

//MACROS 640x480
#define BULLET_STEP 10
#define TOP_BOUNDRY 30
#define NO_DIFFERENCE 0
#define FIRST_PIXEL true
#define SECOND_PIXEL false

//STATES
typedef enum bulletState {
  bullet_initState,
  bullet_waitToFire,
  bullet_fire,
  bullet_collisionDetection, //check before moving
  bullet_moveBullet,
  bullet_collisionHandler
} bulletState;

//GLOBAL VARIABLES
static uint32_t playerBullet_x = 0; //left most pixel
static uint32_t playerBullet_y = 0;
static uint32_t collisionPoint_y = 0; //which pixel in the bullet got hit
static uint32_t collisionPoint_x = 0;
static bool playerBullet_fire = false;
static bool playerBullet_moving = false;
static bool playerBullet_collision = false;
static enum bulletState currentState = bullet_initState;
static enum bulletState previousState = bullet_fire;

//HELPER FUNCTIONS
static void printPixel(enum pixelColor pixel){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Prints the given pixel to the terminal
  ARGUMENTS:
    pixel - an enumerated type that represents a color
  RETURN TYPE
    None
  */

  switch(pixel){ //Compare the given pixel
    case redPixel:
      printf("Red Pixel\n");
      break;
    case greenPixel:
      printf("Green Pixel\n");
      break;
    case blackPixel:
      printf("Black Pixel\n");
  }
}
static void fireBullet(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Create a bullet fired by the tank
  ARGUMENTS:
    None
  RETURN TYPE
    None
  */
  //UPDATE VARIABLES
  playerBullet_fire = false;
  playerBullet_x = player_getCannonColumn();
  playerBullet_y = START_ROW_PLAYER_BULLET;

  //DRAW BULLET
  imageRender_drawPlayerBullet(START_ROW_PLAYER_BULLET, playerBullet_x, white);
  audioPlayer_play(PLAY_LASER);
}
static void printState(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    print the current state of the state machine
  ARGUMENTS:
    None
  RETURN TYPE
    None
  */

  //if state changed
  if (currentState != previousState){
    previousState = currentState;
    printf("Bullet State: ");
    switch(currentState){
      case bullet_initState:
        printf("bullet_initState\n");
        break;
      case bullet_waitToFire:
        printf("bullet_waitToFire\n");
        break;
      case bullet_fire:
        printf("bullet_fire\n");
        break;
      case bullet_moveBullet:
        printf("bullet_moveBullet\n");
        break;
      case bullet_collisionDetection:
        printf("bullet_collisionDetection\n");
        break;
      case bullet_collisionHandler:
        printf("bullet_collisionHandler\n");
        break;
    }
  }
}
static void moveBullet(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Move the player bullet up by bullet step pixels
  ARGUMENTS:
    None
  RETURN TYPE
    None
  */

  //ERASE CURRENT BULLET
  imageRender_drawPlayerBullet(playerBullet_y, playerBullet_x, IMAGE_RENDER_BACKGROUND_COLOR);

  //DRAW NEW BULLET;
  playerBullet_y -= BULLET_STEP;
  imageRender_drawPlayerBullet(playerBullet_y, playerBullet_x, white);
}
static enum pixelColor checkBulletFront(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Check the pixels in front of the moving bullet for a collisoin
  ARGUMENTS:
    None
  RETURN TYPE
    Returns the color of any non-black pixel found in front of the bullet
    Returns black is no colored pixels are found
  */

  //LOCAL VARIABLES
  enum pixelColor nextPixel = blackPixel;

  // start at the top left of the bullet
  // go up y axis (i--)
  for (uint32_t x = playerBullet_x; x < playerBullet_x + SPRITE_SCALING_FACTOR; x++) //for the width of the bullet
    for (uint32_t y = playerBullet_y-1; y > playerBullet_y - (BULLET_STEP+1); y--){ //search the length of a bullet step
      nextPixel = bullet_getPixelColor(x, y);
      //printPixel(nextPixel);
      if (nextPixel != blackPixel){
        collisionPoint_y = y;
        collisionPoint_x = x;
        return nextPixel;
      }
    }

  //NO COLLISION
  if (nextPixel == blackPixel){
    return blackPixel;
  }

  //ERROR
  return unknownPixel;
}
static bool collisionDetected(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Identify whether or not a collision has occured
  ARGUMENTS:
    None
  RETURN TYPE
    returns a bool that is true is a collision has occured
  */

  //LOCAL VARIABLES
  enum pixelColor nextPixel;

  //TOP BOUND
  if (playerBullet_y < TOP_BOUNDRY)
    playerBullet_collision = true; //bullet reached the to top of the screen
  else
  { //bullet is within the game boundaries
      nextPixel = checkBulletFront();
      switch(nextPixel)
      { //compare the nextPixel to see if it's non-black
        case blackPixel: //no collision
          break;
        case greenPixel: //Bunker Collision
          playerBullet_collision = true;
          bunker_registerHit(collisionPoint_x, collisionPoint_y);
          break;
        case bluePixel: //Hit saucer
          playerBullet_collision = true;
          saucer_registerHit();
          break;
        case redPixel: //Hit an alien
          playerBullet_collision = true;
          alienControl_registerHit(collisionPoint_x, collisionPoint_y);
          break;
        case whitePixel: //Hit alien bullet
          playerBullet_collision = true;
          break;
        default: //hit unkown pixel color
          playerBullet_collision = true;
          break;
      }
  }

  //return whether or not there was a collision
  return playerBullet_collision;
}
static void collisionHandler(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Called when a collision has occured to clean up the mess
  ARGUMENTS:
    None
  RETURN TYPE
    None
  */

  //UPDATE VARUABLES
  playerBullet_collision = false;
  playerBullet_moving = false;
  playerBullet_fire = false;

  //ERASE CURRENT BULLET
  imageRender_drawPlayerBullet(playerBullet_y, playerBullet_x, IMAGE_RENDER_BACKGROUND_COLOR);
}

//HEADER FUNCTIONS
enum pixelColor bullet_getPixelColor(uint32_t x, uint32_t y){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Read the color of the pixel at coordinates (x,y)
  ARGUMENTS:
    x - The x coordinate of the pixel
    y - The y coordinate of the pixel
  RETURN TYPE
    enum pixelColor - the color of the pixel at the given coordnates
  */

  //LOCAL VARIABLES
  char buffer[SINGLE_PIXEL]; //3 above bytes are stored in this buffer
  uint32_t bulletByte = (IMAGE_RENDER_ONE_ROW_IN_BYTES*y + PIXEL_SIZE_IN_BYTES*x)-1; //current bullet position
  //uint32_t aboveByte = bulletByte - (IMAGE_RENDER_ONE_ROW_IN_BYTES * BULLET_STEP)-1; //-1 makes aboveByte a multiple of 3

  //GET ABOVE PIXEL
  lseek(hdmiFileDescriptor, bulletByte, SEEK_SET);
  read(hdmiFileDescriptor, buffer, SINGLE_PIXEL); //store above pixel in buffer[]

  bool check;
  //is it IMAGE_RENDER_BACKGROUND_COLOR?
  check = memcmp(buffer, red, SINGLE_PIXEL);
  if (check == NO_DIFFERENCE)
  return redPixel; //hit alien
  check = memcmp(buffer, blue, SINGLE_PIXEL);
  if (check == NO_DIFFERENCE)
    return bluePixel; //hit saucer
  check = memcmp(buffer, green, SINGLE_PIXEL);
  if (check == NO_DIFFERENCE)
    return greenPixel; //hit bunker or tank
  check = memcmp(buffer, IMAGE_RENDER_BACKGROUND_COLOR, SINGLE_PIXEL);
  if (check == NO_DIFFERENCE)
      return blackPixel; //no collision
  check = memcmp(buffer, white, SINGLE_PIXEL);
  if (check == NO_DIFFERENCE)
      return whitePixel; //hit bullet

  //ERROR DETECTION
  printf("ERROR bullet.c: Collision with unknown pixel\n");
  return unknownPixel;
}
void bullet_init(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Initlize the bullet state machine
  ARGUMENTS:
    None
  RETURN TYPE
    None
  */

  //Nothing to initialize
}
void bullet_tick(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Tick function that handles the bullets
  ARGUMENTS:
    None
  RETURN TYPE
    None
  */

  switch (currentState) {//Compare the currentState variable
    case bullet_initState: //init state
      bullet_init();
      currentState = bullet_waitToFire;
      break;
    case bullet_waitToFire: //waiting to fire state
      if (playerBullet_fire)
        currentState = bullet_fire;
      break;
    case bullet_fire: //fire bullet state
      fireBullet();
      currentState = bullet_collisionDetection;
      break;
    case bullet_collisionDetection: //check if there is a collision state
      if (collisionDetected())
      currentState = bullet_collisionHandler;
      else
      currentState = bullet_moveBullet; //all clear to move
      break;
    case bullet_moveBullet: //move bullet state
      moveBullet();
      currentState = bullet_collisionDetection;
      break;
    case bullet_collisionHandler: //handle a detected collision state
        collisionHandler();
        currentState = bullet_waitToFire;
        break;
  }
}
void bullet_firePlayerBullet(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Draw a bullet at the correct location
  ARGUMENTS:
    None
  RETURN TYPE
    None
  */

  playerBullet_fire = true;
}
void bullet_setCollision(){
  //FUNCTION HEADER
  /*
  PURPOSE:
    Indicates that a collision has occured
  ARGUMENTS:
    None
  RETURN TYPE
    None
  */

  playerBullet_collision = true;
}
