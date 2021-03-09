
#include <stdint.h>     //uint32_t
#include <stdio.h>
#include "player.h"
#include "imageRender.h"
#include "intc.h"
#include "gpio_button.h"
#include "button_handler.h"
#include "globals.h"
#include "bullet.h"
#include "audioPlayer.h"

//MACROS
#define BYTES 3
#define TANK_MAX_POSITION 600
#define TANK_MIN_POSITION 40
#define TANK_STEP 4  //pixels the tank moves per function call
#define MOVE_RIGHT 1
#define MOVE_LEFT 0
#define TANK_WAIT_STEP 1
#define PLAYER_BULLET true
#define ALIEN_BULLET false
#define HALF_TANK ((TANK_WIDTH * SPRITE_SCALING_FACTOR) / 2)+1 //half the width of a tank
#define EXPLODE_TICK_2 60
#define TANK_GONE 120
#define STAY_DEAD 175
#define ERASE_TANK false
#define TANK_EXPLOSION_WIDTH 15
#define TANK_EXPLOSION_HEIGHT 8
#define TANK_EXPLOSION_OFFSET 2

//STATE MACHINES
typedef enum tankState{
  tankInit = 0,
  tankWaitForInput,
  tankMoveRight,
  tankMoveRightStepWait,
  tankMoveLeft,
  tankMoveLeftStepWait,
  tankGameOver, //create a tank maybe 2 and have it roam about shooting randomly appearing monsters on either side
  tankDead
} tankState;
typedef enum gunState{
  gunInit = 0,
  gunTriggerWait,
  gunFire,
  gunGameOver,
  gunDeadTank
} gunState;

//GLOBAL VARIABLES
static enum tankState currentTankState = tankInit;
static enum gunState currentGunState = gunInit;
static uint32_t tankPosition = START_COLUMN_TANK; //column in pixels
static uint32_t tankStepTimer;
static bool buttonHeld = false;

//HELPER FUNCTIONS
static void moveTank(bool moveRight){
  /* FUNCTION HEADER
    Purpose:      move the tank left or right
    Arguments:    bool moveRight - true if tanks should move right
    Return type:  void
  */

  //MOVE RIGHT
  if (moveRight && tankPosition < TANK_MAX_POSITION)
    tankPosition += TANK_STEP;
  //MOVE LEFT
  else if (!moveRight && tankPosition > TANK_MIN_POSITION)
    tankPosition -= TANK_STEP;

  ///DRAW NEW TANK
  imageRender_drawNewTank(tankPosition); //new column in pixels
}
static void tank_init(){
  /* FUNCTION HEADER
    Purpose:    Initialize the tank state machine
    Arguments:  None
    Returns:    void
  */

  //INITIALIZE
  tankStepTimer = 0;
  currentTankState = tankWaitForInput;
  imageRender_drawNewTank(tankPosition); //new column in pixels
}
static void gun_init(){
  /* FUNCTION HEADER
    Purpose:    Initialize gun
    Arguments:  None
    Returns:    void
  */
  currentGunState = gunTriggerWait;
}

//HEADER FUNCTIONS
void tankTick(){
  /* FUNCTION HEADER
    Purpose:    Tick function that controls the tank state machine
    Arguments:  None
    Returns:    void
  */
  switch(currentTankState)
  {
    case tankInit:
      tank_init();
    break;
    case tankWaitForInput:
      if (global_gameOver)
      { //if game is over, move to game over state
        currentTankState = tankGameOver;
      } else
      if (button0_pressed)
      { //move right
        currentTankState = tankMoveRight;
      }
      else if (button2_pressed)
      { //move left
        currentTankState = tankMoveLeft;
      }
    break;
    case tankMoveRight:
      moveTank(MOVE_RIGHT); //235 - cleared left block but hit right
      currentTankState = tankMoveRightStepWait;
      tankStepTimer = 0;
    break;
    case tankMoveRightStepWait:
      if (global_gameOver)
      {//if game is over, move to game over state
        currentTankState = tankGameOver;
      } else
      if (button0_released)
      {//move right
        button0_released = false;
        currentTankState = tankWaitForInput;
      }
      else if (tankStepTimer >= TANK_WAIT_STEP)
      { //if wait step has expires
        currentTankState = tankMoveRight;
      }
      else
      { //otherwise increment counter
        ++tankStepTimer;
      }
    break;
    case tankMoveLeft:
      moveTank(MOVE_LEFT); //move tank left
      currentTankState = tankMoveLeftStepWait;
      tankStepTimer = 0;
    break;
    case tankMoveLeftStepWait:
      if (global_gameOver)
      {//if game is over, move to game over state
        currentTankState = tankGameOver;
      } else
      if (button2_released)
      { //no longer moving left
        button2_released = false;
        currentTankState = tankWaitForInput;
      }
      else if (tankStepTimer >= TANK_WAIT_STEP)
      { //if tank counter has expires
        currentTankState = tankMoveLeft;
      }
      else
      { //increment the tank counter
        ++tankStepTimer;
      }
    break;
    case tankGameOver:
    //stub for now
    break;
    case tankDead:
      if (tankStepTimer == EXPLODE_TICK_2)
      { //if tank explode counter has expires
        imageRender_drawSprite(tankPosition+TANK_EXPLOSION_OFFSET, START_ROW_TANK, tank_explosion2_15x8, TANK_EXPLOSION_WIDTH, TANK_EXPLOSION_HEIGHT, yellow, SPRITE_SCALING_FACTOR);
      }
      if (tankStepTimer == TANK_GONE)
      { //if tank need to be erased
        imageRender_drawSprite(tankPosition+TANK_EXPLOSION_OFFSET, START_ROW_TANK, tank_gone_15x8, TANK_EXPLOSION_WIDTH, TANK_EXPLOSION_HEIGHT, green, SPRITE_SCALING_FACTOR);
      }
      if (tankStepTimer >= STAY_DEAD)
      {//if game is over, move to game over state
        global_gameOver = (0 == global_playerLives);
        currentTankState = (global_gameOver) ? tankGameOver : tankInit;
        currentGunState = gunInit;
      }
      else
      { //increment the tank step timer
        ++tankStepTimer;
      }
    break;
  }
}
void gunTick(){
  /* FUNCTION HEADER
    Purpose:    The tick function that handles the gun on the tank
    Arguments:  None
    Returns:    void
  */
  switch(currentGunState)
  { //compare against current gun state
    case gunInit:
      gun_init();
      break;
    case gunTriggerWait:
      if (global_gameOver)
      { //if game is over, move to game over state
        currentGunState = gunGameOver;
      }
      else if (button1_pressed && !global_gameOver)
      { //if game is over, move to game over state
        currentGunState = gunFire;
      }
      break;
    case gunFire:
      if (global_gameOver)
      { //if game is over, move to game over state
        currentGunState = gunGameOver;
        break;
      }
      bullet_firePlayerBullet();
      currentGunState = gunTriggerWait;
      break;
    case gunGameOver:
      break;
    case gunDeadTank:
      //wait for tank to get tank out of this position
      break;
    default:
      printf("ERROR: Non-existent gun state\n");
  }
}
void player_tick(){
  /* FUNCTION HEADER
    Purpose:    The tick function that handles the player input state machine
    Arguments:  None
    Returns:    void
  */
  tankTick();
  gunTick();
}
void player_init(){
  /* FUNCTION HEADER
    Purpose:    Initialized the player state machine
    Arguments:  None
    Returns:    void
  */
  tank_init();
  gun_init();
  global_playerLives = PLAYER_STARTING_LIVES;
}
uint32_t player_getCannonColumn(){
  /* FUNCTION HEADER
    Purpose:    get the x coordinate of where the cannon should fire
    Arguments:  None
    Returns:    The x coordinate of the cannon for firing a bullet
  */

  //LOCAL VARIABLES
  uint32_t cannonColumn = tankPosition + HALF_TANK;

  return cannonColumn;
}
void player_registerHit(){
  /* FUNCTION HEADER
    Purpose:    Handles the case when the player is hit by an alien bullet
    Arguments:  None
    Returns:    void
  */

  if (currentTankState == tankDead)
    return; //Tanks is still dead

  //ERASE A TANK-LIFE
  imageRender_drawTankLife(global_playerLives, ERASE_TANK);

  global_playerLives--;
  currentTankState = tankDead;
  currentGunState = gunDeadTank;
  tankStepTimer = 0;
  //---------------New Code inserted for lab4---------------
  printf("Playing Player is Hit Sound\n");
  audioPlayer_play(PLAY_DEATH);
  //---------------End new Code inserted for lab4-----------

  //void imageRender_drawSprite(int32_t col, int32_t row, uint32_t sprite[], uint32_t width, uint32_t height, char color[], uint32_t scaling_factor );
  imageRender_drawSprite(tankPosition+TANK_EXPLOSION_OFFSET, START_ROW_TANK, tank_explosion1_15x8, TANK_EXPLOSION_WIDTH, TANK_EXPLOSION_HEIGHT, green, SPRITE_SCALING_FACTOR);


}
