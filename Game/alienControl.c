#include <stdint.h> //uint32_t
#include <stdio.h>
#include <stdlib.h> //rand()
#include <stdbool.h>
#include "alienControl.h"
#include "alienBullets.h"
#include "imageRender.h"
#include "intc.h"
#include "gpio_button.h"
#include "globals.h"
#include "audioPlayer.h"

//MACROS
#define MOVE_RIGHT 1
#define MOVE_LEFT 0
#define ALIEN_MAX_POSITION 640
#define ALIEN_MIN_POSITION 0
#define DRAW 1
#define ERASE 0
#define DRAW_ALIENS_OUT 0
#define DRAW_ALIENS_IN 1
#define ALIEN_ROW_STEP ( 5 * IMAGE_RENDER_ONE_ROW_IN_BYTES)
#define ALIEN_WIDTH 12
#define ALIEN_HEIGHT 8
#define EXPLOSION_WIDTH 13
#define SPACING_ALIENS (PIXEL_SIZE_IN_BYTES *(ALIEN_WIDTH * SPRITE_SCALING_FACTOR + PIXELS_BETWEEN_ALIENS)) //84
#define ALIEN_NEW_LINE (IMAGE_RENDER_ONE_ROW_IN_BYTES * ALIEN_HEIGHT * SPRITE_SCALING_FACTOR) + (IMAGE_RENDER_ONE_ROW_IN_BYTES*PIXELS_BETWEEN_ALIEN_ROW) \
      - (11*SPACING_ALIENS)//(3*308)
#define ALIEN_END_COLUMN 450
#define ALIEN_INITAL_BOTTOM_ROW_START (75 + 120)
#define ALIENS_WIN 440
#define ALIEN_COLUMN_1  0
#define ALIEN_COLUMN_2  1
#define ALIEN_COLUMN_3  2
#define ALIEN_COLUMN_4  3
#define ALIEN_COLUMN_5  4
#define ALIEN_COLUMN_6  5
#define ALIEN_COLUMN_7  6
#define ALIEN_COLUMN_8  7
#define ALIEN_COLUMN_9  8
#define ALIEN_COLUMN_10 9
#define ALIEN_COLUMN_11 10
#define ALIEN_SPACING 5
#define ALIEN_COLUMN_SPACING_9 275
#define ALIEN_COLUMN_SPACING_8 242
#define ALIEN_COLUMN_SPACING_7 207
#define ALIEN_COLUMN_SPACING_5 139
#define ALIEN_ADJUST_RIGHT 1
#define ALIEN_ROW_1_DISTANCE_FROM_TOP (96)
#define ALIEN_ROW_2_DISTANCE_FROM_TOP (74)
#define ALIEN_ROW_3_DISTANCE_FROM_TOP (48)
#define ALIEN_ROW_4_DISTANCE_FROM_TOP (23)
#define ALIEN_DISTANCE_BETWEEN_COLUMNS 34
#define ALIEN_DISTANCE_BETWEEN_COLUMNS_RIGHT 34
#define ALIEN_DISTANCE_BETWEEN_ROWS 25
#define ALIEN_HORIZONTAL_SPACING 10
#define ALIEN_TOTAL_DISTANCE_BETWEEN PIXELS_ON_EACH_ALIENS_SIDE*2
#define ALIEN_HALFWAY_POINT (ALIEN_WIDTH * SPRITE_SCALING_FACTOR/2 + ALIEN_WIDTH/2)
#define DRAW_TANK true
#define MAX_SHOT_DELAY 220
#define MIN_SHOT_DELAY 100

typedef enum alienStateMachine {
  alienState_init = 0,
  alienState_waitToMarchRight,
  alienState_marchRight,
  alienState_waitToMarchLeft,
  alienState_marchLeft,
  alieanState_gameOver,
  alienState_dead
} alienStateMachine;

typedef enum alienShootStateMachine {
    alien_Shoot_init = 0,
    alien_Shoot_waitRandomTime,
    alien_Shoot_fire,
    alien_Shoot_gameOver,
    alien_Shoot_allDead
}alienShootStateMachine;

//GLOBAL VARIABLES
static enum alienStateMachine currentState = alienState_init;
static enum alienStateMachine previousState = alieanState_gameOver;
static bool buttonHeld = false;
static bool aliensIn = false; //aliens toggle between being In/Out
static bool aliens[NUM_ALIENS_X][NUM_ALIENS_Y];
static int32_t alienRow = ALIEN_START_ROW;
static int32_t alienColumn = ALIEN_START_COLUMN;
static int32_t positionOfRightMostAlien;
static uint32_t rightMostColumn;
static int32_t positionOfLeftMostAlien;
static uint32_t leftMostColumn;
static int32_t positionOfBottomMostAlien;
static uint32_t bottomRow;
static int32_t aliens_linesLeft = NUM_ALIENS_Y;
static uint32_t alienCounter;
static uint32_t numAliensLeft;
static const uint32_t alienScoreWorth[] = {40, 20 , 20, 10 , 10};
static enum alienShootStateMachine waitToShootState = alien_Shoot_init;
static int32_t shootTimer;

//HELPER FUNCTIONS
static void moveAliens(bool moveRight, bool moveDown) {
  //FUNCTION HEADER
  /*
    PURPOSE:
      Move the aliens left/right
    ARGUMENTS:
      moveRight - bool that determines whether the aliens should move left/right
      moveDown - bool that determines if the aliens should down or not
    RETURN VALUE:
      None
  */
  //-------------------New Code inserted for Lab4---------------------------
  audioPlayer_play(PLAY_ALIEN_WALK);
  //-------------------End new Code ----------------------------------------
  if (moveDown)
  { //Aliens have reached the end of the screen
    //CLEAN TOP
    //Increment and redraw below
    alienRow += ALIEN_CONTROL_STEP_LEN;
    positionOfBottomMostAlien += ALIEN_CONTROL_STEP_LEN;
    if (positionOfBottomMostAlien >= ALIENS_WIN)
    { //Aliens have reached the bottom of the screen
      global_gameOver = true;
      return;
    }
    imageRender_drawAliens(alienRow, alienColumn, aliensIn, aliens);
    aliensIn = !aliensIn;
    return;
  }

  //COMPUTE NEW POSITION
  if (moveRight)
  { //aliens move right
    alienColumn += ALIEN_CONTROL_STEP_LEN;
    positionOfRightMostAlien += ALIEN_CONTROL_STEP_LEN;
    positionOfLeftMostAlien += ALIEN_CONTROL_STEP_LEN;
  }
  else
  { //aliens move left
    alienColumn -= ALIEN_CONTROL_STEP_LEN;
    positionOfRightMostAlien -= ALIEN_CONTROL_STEP_LEN;;
    positionOfLeftMostAlien -= ALIEN_CONTROL_STEP_LEN;;
  }
  //DRAW NEW ALIENS
  imageRender_drawAliens(alienRow, alienColumn, aliensIn, aliens);
  aliensIn = !aliensIn;
}
static void printfAliens(){
  //FUNCTION HEADER
  /*
    PURPOSE:
      Display on the terminal the live aliens for debugging
    ARGUMENTS:
      None
    RETURN VALUE:
      None
  */

  //PRINT ALIENS
  for (uint32_t y = 0; y < NUM_ALIENS_Y; y++){ //for each row
    for (uint32_t x = 0; x < NUM_ALIENS_X; x++)//for each column
      printf("%d ", aliens[x][y]);//print out alien
    printf("\n");
  }
}
static bool withinAlienHitBox(uint32_t x, uint32_t y){
  //FUNCTION HEADER
  /*
    PURPOSE:
      Identify whether or not the (x,y) are withing alien hitbox
    ARGUMENTS:
      x - x coordinate inside hitbox
      y - y coordinate inside hitbox
    RETURN VALUE:
      true is (x,y) is within hitbox, false otherwise
  */

  //printf("%d %d", x, y);
  //printf(" vs %d %d", rightMostColumn + alienColumn + ALIEN_WIDTH  * SPRITE_SCALING_FACTOR, positionOfBottomMostAlien + ALIEN_HEIGHT * SPRITE_SCALING_FACTOR + ALIEN_VERTICAL_SPACING);

  //IF OUTSIDE HITBOX
  if (y > positionOfBottomMostAlien + ALIEN_HEIGHT * SPRITE_SCALING_FACTOR + ALIEN_VERTICAL_SPACING * SPRITE_SCALING_FACTOR)
    return false;

  //INSIDE HIT BOX
  return true;
}
static bool alienFireBullet() {
  //FUNCTION HEADER
  /*
    PURPOSE:
      Fire an alien Bullet
    ARGUMENTS:
      None
    RETURN VALUE:
      bool that's true if a bullet has fire correctly
  */

    uint32_t whichColumn = rand() % (rightMostColumn -  leftMostColumn + 1) + leftMostColumn;
    for (int32_t y = ALIEN_ROW_5; y >= 0; --y)
    { //look for the bottom most alien, if the randomly selected row does not contain an alien, randomly select a different row.
      if (aliens[whichColumn][ y]) { //Found an alien, request bullet from the alien's position,

        uint32_t xPixelFiringSpot = (( ALIEN_WIDTH * SPRITE_SCALING_FACTOR + ALIEN_TOTAL_DISTANCE_BETWEEN)*whichColumn) + alienColumn + ALIEN_HALFWAY_POINT + ALIEN_CONTROL_STEP_LEN;
        uint32_t yPixelFiringSpot = ((ALIEN_HEIGHT * SPRITE_SCALING_FACTOR+ ALIEN_VERTICAL_SPACING)* y) + alienRow + ALIEN_HEIGHT * SPRITE_SCALING_FACTOR;
        if ( alienBullet_fire(xPixelFiringSpot, yPixelFiringSpot) )
        { //Status message informing the system that a bullet was, infact fired.
          printf("Bullet Fired, Calculated Pixel: %d, %d/%d\n\r", (( ALIEN_WIDTH * SPRITE_SCALING_FACTOR + ALIEN_TOTAL_DISTANCE_BETWEEN)*whichColumn) + alienColumn, whichColumn, y);
        }

          return true; //weapon has fired, no longer need this loop
      } //End of found an alien at this position loop.
    } //End of Looping through alien Bullet Firing.
    return false;
}
static void alienShoot_tick() {
  //FUNCTION HEADER
  /*
    PURPOSE:
      The tick function that handles all alien bullets
    ARGUMENTS:
      None
    RETURN VALUE:
      None
  */

  switch (waitToShootState)
  { //Mealy state machine
    case alien_Shoot_init:
      shootTimer = rand() % (MAX_SHOT_DELAY - MIN_SHOT_DELAY) + MIN_SHOT_DELAY;
      waitToShootState = alien_Shoot_waitRandomTime;
      break;
    case alien_Shoot_waitRandomTime:
      if (global_gameOver)
      { //Game Over transition
        printf("AlienControl: GameOver\n\r");
        waitToShootState = alien_Shoot_gameOver;
      }
      else if (numAliensLeft == 0)
      { //Dead aliens cannot shoot bullets.
        waitToShootState = alien_Shoot_allDead;
      }
      else if (shootTimer <= 0)
      { //It is time to shoot again.
        waitToShootState = alien_Shoot_fire; //change state to wait for a successful shot
        shootTimer = rand() % (MAX_SHOT_DELAY - MIN_SHOT_DELAY) + MIN_SHOT_DELAY; //Random delay
      }
      else
      { //CurrentState = CurrentState. Signal --shootTimer
        --shootTimer;
      }
      break;
    case alien_Shoot_fire:
      if (global_gameOver)
      { //Game Over transition
        printf("AlienControl: GameOver\n\r");
        waitToShootState = alien_Shoot_gameOver;
      }
      else if (numAliensLeft == 0)
      { //If alien is dead
        waitToShootState = alien_Shoot_allDead;
      }
      else if (alienFireBullet())
      { //Alien is valid for firing a bullet
        waitToShootState = alien_Shoot_waitRandomTime;
      }
      break;
    case alien_Shoot_gameOver:
      break;
    case alien_Shoot_allDead:
      if (numAliensLeft > 0)
      { //Wait for the aliens to be resurected then start shooting again
        waitToShootState = alien_Shoot_init;
      }
      break;
  }
}
static void printState_alienControl(){
  //FUNCTION HEADER
  /*
    PURPOSE:
      prints the current state to the terminal for debugging
    ARGUMENTS:
      None
    RETURN VALUE:
      None
  */
  if (currentState != previousState){
    previousState = currentState;
    printf("alienControl State: ");
    switch (currentState){
      case alienState_init:
        printf("alienState_init\n");
        break;
      case alienState_waitToMarchRight:
        printf("alienState_waitToMarchRight\n");
        break;
      case alienState_marchRight:
        printf("alienState_marchRight\n");
        break;
      case alienState_waitToMarchLeft:
        printf("alienState_waitToMarchLeft\n");
        break;
      case alienState_marchLeft:
        printf("alienState_marchLeft\n");
        break;
      case alieanState_gameOver:
        printf("alieanState_gameOver\n");
        break;
    }
  }
}

//HEADER FUNCTIONS
void alienControl_init(){
  //FUNCTION HEADER
  /*
    PURPOSE:
      Ensure that alienControl is initialized
    ARGUMENTS:
      None
    RETURN VALUE:
      None
  */

  //There are no variables to initialize
  printfAliens();
}
void alienControl_tick(){
  //FUNCTION HEADER
  /*
    PURPOSE:
      The tick function that handles the alien momement and lives
    ARGUMENTS:
      None
    RETURN VALUE:
      None
  */

  //STATE MACHINE
  switch(currentState)
  {//Mealy Actions
    case alienState_init:
      numAliensLeft = 0;
      aliensIn = false;
      for (uint32_t y = 0; y < NUM_ALIENS_Y; y++) { //For each row of Aliens
        for (uint32_t x = 0; x < NUM_ALIENS_X; x++) { //For each column of Aliens
          aliens[x][y] = true; //Aliens are alive
          ++numAliensLeft;//increment the total number of aliens
        }
      }
      alienRow = ALIEN_START_ROW;
      alienColumn = ALIEN_START_COLUMN;
      currentState = alienState_waitToMarchRight;
      positionOfRightMostAlien = ALIEN_END_COLUMN;
      positionOfLeftMostAlien = ALIEN_START_COLUMN;
      positionOfBottomMostAlien = ALIEN_INITAL_BOTTOM_ROW_START;
      alienCounter = 0;
      rightMostColumn = ALIEN_COLUMN_11;
      leftMostColumn = ALIEN_COLUMN_1;
      bottomRow = ALIEN_ROW_5;
    break;

    case alienState_waitToMarchRight:
      if (global_gameOver)
      {// if game is over, erase aliens and go to game over state
        printf("AlienControl: GameOver\n\r");
        currentState = alieanState_gameOver;
      } else
      if (alienCounter == ALIEN_CONTROL_TICKS_PER_STEP)
      { //if alien counter has expired
        currentState = alienState_marchRight;
        alienCounter = 0;
      }
      else { //increment the alien counter
        ++alienCounter;
      }
    break;
    case alienState_marchRight:
      if (global_gameOver)
      {// if game is over, erase aliens and go to game over state
        printf("AlienControl: GameOver\n\r");
        currentState = alieanState_gameOver;
      } else
      if (positionOfRightMostAlien >= ALIEN_MAX_POSITION)
      { //if outside of alien bounds
        moveAliens(true, true);
        currentState = alienState_waitToMarchLeft;
      }
      else
      { //move the aliens
        moveAliens(true, false);
        currentState = alienState_waitToMarchRight;
      }

      if (numAliensLeft ==0)
      {// if there are no aliens left
        alienCounter = 0;
        currentState = alienState_dead;
      }
    break;
    case alienState_waitToMarchLeft:
      if (global_gameOver)
      {// if game is over, erase aliens and go to game over state
        printf("AlienControl: GameOver\n\r");
        currentState = alieanState_gameOver;
      } else //game is not over
      if (alienCounter == ALIEN_CONTROL_TICKS_PER_STEP)
      { //if counter has reached it's max value
        currentState = alienState_marchLeft;
        alienCounter = 0;
      }
      else { //increment the alien counter
        ++alienCounter;
      }
      break;

    case alienState_marchLeft:
      if (global_gameOver)
      {// if game is over, erase aliens and go to game over state
        currentState = alieanState_gameOver;
      } else //if aliens have reached the left most side
      if (positionOfLeftMostAlien <= ALIEN_MIN_POSITION)
      { //move aliens to the left and down
        moveAliens(false, true);
        currentState = alienState_waitToMarchRight;
      }
      else
      { //move aliens to the left
        moveAliens(false, false);
        currentState = alienState_waitToMarchLeft;
      }
      if (numAliensLeft ==0)
      { //if all aliens are dead
        alienCounter = 0;
        currentState = alienState_dead;
      }

    break;
    case alieanState_gameOver:
      // if game is over, erase aliens and go to game over state
      break;
    case alienState_dead:
      if (alienCounter == ALIEN_CONTROL_TICKS_PER_STEP)
      { //erase alien;
        currentState = alienState_init;
      }
      else { //increment the alien counter
        ++alienCounter;
      }
    break;
  }
  alienShoot_tick();
}
bool alienControl_isAlive(uint32_t x, uint32_t y){
  //FUNCTION HEADER
  /*
    PURPOSE:
      Identifies whether or not the alien at coordinates (x,y) is alive or not
    ARGUMENTS:
      x - the x coordinate of the alien
      y - the y coordinate of the alien
    RETURN VALUE:
      bool the is true if the alien is alive
  */
  return aliens[x][y];
}
void alienControl_registerHit(uint32_t x, uint32_t y){
  //FUNCTION HEADER
  /*
    PURPOSE:
      Handles the case when Aliens have been hit
    ARGUMENTS:
      x - The x coordinate of the alien
      y - The y coordnate of the alien
    RETURN VALUE:
      None
  */

  //Left Most Alien
  if (!withinAlienHitBox(x, y))
    return; //ERROR! Not within hitbox

  //IDENTIFY THE X POSITOIN OF THE ALIEN
  uint32_t xAlien = (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_11 + ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_11 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_10+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_10 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_9+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_9 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_8+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_8 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_7+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_7 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_6+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_6 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_5+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_5 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_4+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_4 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_3+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_3 :
                    (x >= alienColumn + (ALIEN_DISTANCE_BETWEEN_COLUMNS * ALIEN_COLUMN_2+ ALIEN_CONTROL_STEP_LEN)) ? ALIEN_COLUMN_2 :
                                                                                                                     ALIEN_COLUMN_1;
  //IDENTIFY THE Y POSITION OF THE ALIEN
 uint32_t yAlien = (y >= alienRow + ALIEN_ROW_1_DISTANCE_FROM_TOP) ? ALIEN_ROW_5 :
                   (y >= alienRow + ALIEN_ROW_2_DISTANCE_FROM_TOP) ? ALIEN_ROW_4 :
                   (y >= alienRow + ALIEN_ROW_3_DISTANCE_FROM_TOP) ? ALIEN_ROW_3 :
                   (y >= alienRow + ALIEN_ROW_4_DISTANCE_FROM_TOP) ? ALIEN_ROW_2 :
                                                                     ALIEN_ROW_1;


 //IF ALIEN IS ALIVE
  if (aliens[xAlien][yAlien]) {
    aliens[xAlien][yAlien] = false; //kill alien
    numAliensLeft--; //reduce the total number of aliens
    global_playerScore += alienScoreWorth[yAlien];

    //DRAW EXPLOSION
   imageRender_drawSprite( ( ALIEN_DISTANCE_BETWEEN_COLUMNS )*xAlien+ alienColumn + ALIEN_CONTROL_STEP_LEN,
                             alienRow + (ALIEN_HEIGHT * SPRITE_SCALING_FACTOR+ ALIEN_VERTICAL_SPACING)* yAlien,
                             alien_explosion_13x10, EXPLOSION_WIDTH, ALIEN_HEIGHT, red, SPRITE_SCALING_FACTOR);
   //PLAY SOUND
   //------------NEW CODE inserted for Lab4------------------------------------
   audioPlayer_play(PLAY_ALIEN_DEATH);
   //-----------END NEW CODE inserted for Lab4---------------------------------

   //DRAW NEW PLAYER SCORE
    imageRender_drawPlayerScore();

    if (numAliensLeft == 0 && global_playerLives < PLAYER_LIVES_MAX)
    { //Player had cleared the level, add a life
      global_playerLives++;
      imageRender_drawTankLife(global_playerLives, DRAW_TANK);
    }
  }
 // check to see if a new Right most column should be selected
 if (xAlien == rightMostColumn)
 { //Check to see if how far left the aliens travel needs to be adjusted
   for (uint32_t x = NUM_ALIENS_X-1; x >= 0; --x)
   { //for each column of aliens
     bool aliensInColumn = false;
     for (uint32_t y =0; y < NUM_ALIENS_Y; ++y)
     { //for each row of aliens
       aliensInColumn |=  aliens[x][y];
     }
     if (aliensInColumn)
     { //if the column is not empty
       rightMostColumn = x;
       positionOfRightMostAlien = ALIEN_DISTANCE_BETWEEN_COLUMNS_RIGHT * (rightMostColumn + ALIEN_ADJUST_RIGHT) + alienColumn + ALIEN_CONTROL_STEP_LEN;
       break;
     }
   }
 }
 if (xAlien == leftMostColumn)
 { //if at the left most end of the screen
   for (uint32_t x = 0; x < NUM_ALIENS_X; ++x)
   { //for each of column of aliens
     bool aliensInColumn = false;
     for (uint32_t y =0; y < NUM_ALIENS_Y; ++y)
     { //for each row of aliens
       aliensInColumn |=  aliens[x][y];
     }
     if (aliensInColumn)
     { //if the column is empty
       leftMostColumn = x;
       positionOfLeftMostAlien = ALIEN_DISTANCE_BETWEEN_COLUMNS * leftMostColumn + alienColumn;
       break;
     }
   }
 }
 if (yAlien == bottomRow)
 { //check if the lowest alien row has been hit
   for (uint32_t y = ALIEN_ROW_5; y >= 0; ++y)
   { //for each row of aliens
     bool aliensInRow = false;
     for (uint32_t x = 0; x < NUM_ALIENS_X; ++x)
     { //for each column of aliens
       aliensInRow |=  aliens[x][y];
     }
     if (aliensInRow && y != bottomRow)
     { //if the bottom row has changed
       bottomRow = y;
       positionOfBottomMostAlien = ALIEN_DISTANCE_BETWEEN_ROWS * bottomRow + alienRow;
       break;
     }
   }
 }
}
