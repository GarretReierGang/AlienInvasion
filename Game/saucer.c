#include <stdint.h>     //uint32_t
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "saucer.h"
#include "imageRender.h"
#include "intc.h"
#include "gpio_button.h"
#include "globals.h"
#include "audioPlayer.h"

//Saucer Explosion Sound call = True

//MACROS
#define BYTES 3
#define SAUCER_MAX_POSTION 640
#define SAUCER_MIN_POSITION 480
#define MOVE_RIGHT 1
#define MOVE_LEFT 0
#define SAUCER_STEP 4 //pixel
#define SAUCER_TICKS_PER_STEP 4 //TICKS PER STEP
#define SAUCER_STARTING_POINT -40
#define SAUCER_POINT_INCREMENT 50
#define SAUCER_POINT_STEPS 6
#define SAUCER_EXPLOSION_TIME_OUT 27
#define SAUCER_MIN_DELAY 500
#define SAUCER_DELAY_RANGE 50

//STATE MACHINE
typedef enum saucerState {
  saucerInit = 0,               //inilize state machine
  saucerWaitToSpawn,            //wait to spawn
  saucerWaitToMoveRight,        //wait to move the saucer
  saucerMoveRightAcrossScreen,  //move the saucer across the screen
  saucerExplode,                //saucer has been hit and explodes
  saucerGameOver                //game is over, diable saucer
} saucerState;

//GLOBAL VARIABLES
static enum saucerState currentState = saucerInit;
static uint32_t saucerTimer;
static int32_t saucerPosition = 0;
static bool buttonHeld = false;

// HELPER FUNCTIONS
static void moveSaucer(bool moveRight){
  //FUNCTION HEADER
  /*
  PURPOSE: Moves the flying saucer
  ARGUMENTS:
    moveRight - true if the saucer moves right, false if it moves left
  RETURN VALUE: None
  */

  //MOVE SAUCER
  if (moveRight)
    saucerPosition += SAUCER_STEP;
  else
    saucerPosition -= SAUCER_STEP;
  imageRender_drawNewSaucer(saucerPosition);
}
static void generateDelay(){
  //FUNCTION HEADER
  /*
  PURPOSE: Randomly decides how long for the saucer to wait
  ARGUMENTS: None
  RETURN VALUE: None
  */

  //currently a testing value, not actual in game value
  saucerTimer = rand() % SAUCER_DELAY_RANGE + SAUCER_MIN_DELAY;
}


//HEADER FUNCTIONS
void saucer_tick(){
  //FUNCTION HEADER
  /*
  PURPOSE: Handles the flying saucer
  ARGUMENTS: None
  RETURN VALUE: None
  */

  switch(currentState)
  { //Tick has occured
    case saucerInit: //initialize state
      generateDelay();
      saucer_spixelsPerStep = SAUCER_STEP;
      saucerPosition = SAUCER_STARTING_POINT;
      currentState = saucerWaitToSpawn;
    break;
    case saucerWaitToSpawn: //wait to spawn
      if (global_gameOver)
      { //game is over, do not fly saucer
        currentState = saucerGameOver;
      } else if (saucerTimer == 0)
      { //game is not over, move saucer
        currentState = saucerWaitToMoveRight;
        global_saucerIsOut = true;
        //generate saucer
      }
      else
      { //decrement saucer Timer
        --saucerTimer;
      }
    break;
    case saucerWaitToMoveRight: //waiting to move to the right
      if (global_gameOver)
      { //game is over, do not move saucer
        currentState = saucerGameOver;
      }
      else if (saucerTimer >= SAUCER_TICKS_PER_STEP)
      { //Timer has expires
        currentState = saucerMoveRightAcrossScreen;
      }
      else
      { //increment saucer timer
        ++saucerTimer;
      }
    break;
    case saucerMoveRightAcrossScreen: //Moving saucer
      if (global_gameOver)
      { //game is over, do not move saucer
        currentState = saucerGameOver;
      }
      else if (saucerPosition >= SAUCER_MAX_POSTION)
      {
        //Saucer is off of the field
        //---------------------Added in Lab 4--------------------------------
        global_saucerIsOut = false;
        //-------------------------------------------------------------------
        generateDelay();
        currentState = saucerWaitToSpawn;
        saucerPosition = SAUCER_STARTING_POINT;
      }
      else
      { //move the saucer
        currentState = saucerWaitToMoveRight;
        saucerTimer = 0;
        moveSaucer(MOVE_RIGHT);
      }
    //Move the saucer one saucerGameOverstep right
    break;
    case saucerExplode: //Saucer has been hit
      if (global_gameOver)
      {//game is over, do not move saucer
        currentState = saucerGameOver;
      }
      else if (saucerTimer == SAUCER_EXPLOSION_TIME_OUT)
      { //explision has finished
        currentState = saucerInit;
        uint32_t cursor = saucerPosition * PIXEL_SIZE_IN_BYTES + START_ROW_SAUCER;
        imageRender_drawSpriteB ( cursor, saucer_explosion_19x7, MOVING_SAUCER_WIDTH, SAUCER_HEIGHT, IMAGE_RENDER_BACKGROUND_COLOR, SPRITE_SCALING_FACTOR);
      }
      else
      { //increment saucer timer
        ++saucerTimer;
      }
    break;
    case saucerGameOver: //game over!
    //do nothing
    break;
  }
}
void saucer_init(){
  //FUNCTION HEADER
  /*
  PURPOSE: Initializes the saucer state machine
  ARGUMENTS: None
  RETURN VALUE: None
  */

  //There is nothing to initalize
}
void saucer_runTest(){
  //FUNCTION HEADER
  /*
  PURPOSE: Testing basic saucer functionality
  ARGUMENTS: None
  RETURN VALUE: None
  */

  while(1)
  { //Main Test Loop
    uint32_t activatedInterupts = intc_wait_for_interrupt();
    uint32_t pressedButtons = gpio_button_read();
    if (activatedInterupts & GPIO_BUTTON_INTC_MASK)
    { //Check if a button was pressed, ignore FITs for now
      if (buttonHeld == false && pressedButtons)
      { //ensure that a button cannot be acidently pressed 2 in a row
          gpio_button_acknowledge_interupt();
          intc_ack_interrupt(GPIO_BUTTON_INTC_MASK);

          switch (gpio_button_read())
          { //read the button value and perform the approiate action
            case GPIO_BUTTON0: //MOVE RIGHT
              printf("btn0\n");
              moveSaucer(MOVE_RIGHT);
            break;
            case GPIO_BUTTON1: //FIRE BULLET
              printf("btn1\n");
              printf("FIRE!\n");
            break;
            case GPIO_BUTTON2: //MOVE LEFT
              printf("btn2\n");
              moveSaucer(MOVE_LEFT);
            break;
        }

        buttonHeld = true;
      }
      else if (buttonHeld == true && ! pressedButtons)
      { // report a button release has occured
        buttonHeld = false;
      }
    }
    else
    { //clear FIT interupts
      intc_ack_interrupt(GPIO_FIT_INTC_MASK);
    }
    intc_enable_uio_interrupts();
  }
}

void saucer_registerHit(){
  //FUNCTION HEADER
  /*
  PURPOSE: Handles a hit on the saucer
  ARGUMENTS: None
  RETURN VALUE: None
  */

  //RESET VARIABLES
  currentState = saucerExplode;
  saucerTimer = 0;

  //INCREMENT PLAYER SCORE
  global_playerScore += ( (rand() % SAUCER_POINT_STEPS + 1) * SAUCER_POINT_INCREMENT );

  //DRAW EXPLODING SAUCER
  uint32_t cursor = saucerPosition * PIXEL_SIZE_IN_BYTES + START_ROW_SAUCER;
  imageRender_drawSpriteB ( cursor, saucer_explosion_19x7, MOVING_SAUCER_WIDTH, SAUCER_HEIGHT, blue, SPRITE_SCALING_FACTOR);

  //Play Saucer Explosion
  //------------------Added in Lab 4------------------------------
  audioPlayer_play(PLAY_SAUCER_DEATH);
    global_saucerIsOut = false;
  //------------------End of added in Lab 4-----------------------

  //DRAW PLAYER SCORE
  imageRender_drawPlayerScore();

}
