#include "alienBullets.h"
#include "bullet.h"
#include "imageRender.h"
#include "bunker.h"
#include "player.h"
#include "globals.h"


typedef enum alienBulletState {
  state_init,       //initialize the state machine
  state_wait,       //wait a random amount of time to fire a bullet
  state_advance,    //move all alien bullets down
  state_gameOver    //game is over, do not fire bullets
} alienBulletState;

typedef struct bullet {
  uint32_t x;         //x coordinate of the bullet
  uint32_t y;         //y coordinate of the bullet
  char currentState;  //current state of the bullet
  struct bullet* next;//next bullet in the linked list
  struct bullet* prev;//previous bullet in the linked list
} alienBullet;

//MACROS
#define ALIEN_BULLET_WIDTH 3
#define ALIEN_BULLET_HEIGHT 5
#define ALIEN_BULLET_ERASE_HEIGHT ALIEN_BULLET_STEP_LEN
#define ALIEN_BULLET_CHECK_OFFSET_Y (ALIEN_BULLET_HEIGHT * 2)
#define ALIEN_BULLET_COLISION_WIDTH (ALIEN_BULLET_WIDTH*SPRITE_SCALING_FACTOR)
#define ALIEN_BULLET_CHECK_OFFSET_X (0)
#define BULLET_TYPES 2
#define BULLET_1  0
#define BULLET_1B 1
#define BULLET_2  2
#define BULLET_2B 3
#define ALIEN_BULLET_HEIGHT_OFFSET 0

//VARIABLES
uint32_t numBulletsInUse; //number of alien bullets that are fired
alienBulletState currentState = state_init; //current state of the state machine
alienBulletState previousState = state_wait; //previous state used for printing current state
alienBullet* activeBullets; //points to an array of active bullets
alienBullet* sleepingBullets; //point to an array of inactive bullets
alienBullet stackOfBullets[ALIEN_BULLET_MAX_COUNT]; //data structure containing bullets
uint32_t* bulletSprites[] = {alienbullet2_up_3x5, alienbullet2_down_3x5, alienbullet1_down_3x5, alienbullet1_up_3x5};
uint32_t timer; //timer to count random time between bullet fires
uint32_t colisionPoint_x; //last x coordinate a pixel collision was detected


//FUNCTIONS
// Purpose: //This is not a general purpose stack_insert as it uses a specific type of node (alienBullet)
//    adds a node on top of a stack
// Takes:
//    stack, stack to insert a node onto
//    node,  node to add to the stack
//
void stack_insert(alienBullet** stack, alienBullet* node)
{
  if ( (*stack) == NULL) //if the stack is empty
  {
    (*stack) = node; //insert bullet
    node->next = NULL; //there is no next bullet
    node->prev = NULL; //there is no previous bullet
    return;
  }

  node->next = (*stack); //add bullet to the stack
  (*stack)->prev = node; //indicate the previous bullet
  node->prev = NULL;
  (*stack) = node;
}
// Purpose: //This is not a general purpose stack_insert as it uses a specific type of node (alienBullet)
//    returns the top item from a stack
// Takes:
//    stack, stack to pop a top node off of
// Returns:
//    the node at the top of the stack.
alienBullet* stack_pop(alienBullet** stack)
{
  alienBullet* temp = (*stack); //pop bullet off stack
  if (temp == NULL) //if there is no bullet
  {
    printf ("ERROR: Popped empty stack\n\r");
    return NULL; //error
  }
  (*stack) = temp->next; //otherwise pop bullet off stack
  return temp;
}

//Purpose:
//  remove a node from the middle of the double linked stack
//  Takes:
//    stack, stack to remove the node from
//    node, node to remove from the given stack
void stack_removeNode(alienBullet** stack, alienBullet* node)
{
  //printf("alienBullets.c: stack_removeNode() %x %x\n", (uint32_t)(*stack), (uint32_t)(*node));
  if ( (*stack) == node)
  { //the node is at the head of the stack, only reason why we need the stack

    //INPUT VALIDATION
    if (node == NULL){ //Node/ Stack are both Empty
      printf("ERROR: removed NULL node from empty stack\n\r");
      return;
    }
    else if (node->next == NULL){  //Node is at bottom of stack
      (*stack) = NULL; //assign to null
      return;
    }
    // standard case
    node->next->prev = NULL;//assign previous node to null
    (*stack) = node->next;
    return;
  }
  //removing the node by setting Previous Node's next node to Next Node
  // and Next Node's previous node, to the Previous Node
  node->prev->next = node->next;

  if (node->next != NULL) //Node is at bottom of stack
    node->next->prev = node->prev;
}

//  Purpose:
//    Initalize alienBullet statemachine
//    setup unused bullet stack
//  Takes:
//    Nothing
//  Dependencies:
//    stack_insert
//  Uses:
//    numBulletsInUse
//    sleepingBullets
//    activeBullets
//    stackOfBullets
void alienBullet_init()
{
  numBulletsInUse = 0;
  sleepingBullets = NULL;
  activeBullets = NULL;
  for (uint32_t n = 0; n < ALIEN_BULLET_MAX_COUNT; ++n) //for each alien bullet
  {
    printf("Creating Bullet Stack: bullet %d\n\r", n);
    stack_insert(&sleepingBullets, &stackOfBullets[n]); //add to the stack
  }
}
//  Purpose:
//    Check the pixels that the alienBullet might colide with
//  Takes
//    x, pixel column
//    y, pixel row
//    count, number of pixels left to check
//  Returns:
//    the color of the first non-black pixel encountered.
//    blackPixel if no non-black pixels were encountered.
//  Uses:
//    colisionPoint_x
pixelColor checkNextPixel(uint32_t x, uint32_t y, uint32_t count)
{
  switch( bullet_getPixelColor(x, y))
  {
      case greenPixel:
        colisionPoint_x = x; //this pixel is where the collision happened
        return greenPixel; // return to start of while loop
      case redPixel:
        return redPixel; //hit an alien
      case blackPixel:
        //Do not react to blackPixels
        if (count)
          return checkNextPixel(++x, y, --count);
        else
          return blackPixel;
      default:
        return unknownPixel;
  }
}

// Purpose:
//    move each of the bullets one step downards
//  Takes:
//  Dependencies:
//    checkNextPixel
//    imageRender_drawSprite
//    stack_removeNode
//    stack_insert
//    player_registerHit
//  Uses:
//    activeBullets
//    sleepingBullets
//    colisionPoint_x
//    bulletSprites
void alienBullet_advance()
{
  alienBullet* head = activeBullets;
  while (head != NULL) { //while not at the end
    uint32_t oldY = head->y;
    head->y += ALIEN_BULLET_STEP_LEN; //advance the bullet forward by one step

    switch( checkNextPixel(head->x + ALIEN_BULLET_CHECK_OFFSET_X, head->y + ALIEN_BULLET_CHECK_OFFSET_Y, ALIEN_BULLET_COLISION_WIDTH))
    { //check to see if there is a collision
        case greenPixel: //if you hit a tank or bunker
          imageRender_drawSprite(head->x,  oldY, bulletSprites[head->currentState], ALIEN_BULLET_WIDTH, ALIEN_BULLET_HEIGHT, IMAGE_RENDER_BACKGROUND_COLOR, SPRITE_SCALING_FACTOR );
          //erase Bullet
          if (head->y >= PLAYER_SPACE_START)
          { //The green pixel detected must belong to the player's tank
            player_registerHit();
            stack_removeNode (&activeBullets, head);
            alienBullet* temp = head;
            head = head->next;
            stack_insert(&sleepingBullets, temp);
            --numBulletsInUse;
            continue; // return to start of while loop
          }
          else if ( bunker_registerHit(colisionPoint_x, head->y+ ALIEN_BULLET_CHECK_OFFSET_Y))
          { //Alien Bullet hit a bunker
            stack_removeNode (&activeBullets, head);
            alienBullet* temp = head;
            head = head->next;
            stack_insert(&sleepingBullets, temp);
            --numBulletsInUse;
            continue; // return to start of while loop
          }
          else
          { //Alien Bullet hit regular bullet
            break;
          }
        case blackPixel:
          //Do not react to blackPixels
          break;
        case redPixel:
          stack_removeNode (&activeBullets, head);
          alienBullet* temp = head;
          head = head->next;
          stack_insert(&sleepingBullets, temp);
          --numBulletsInUse;
          continue; // return to start of while loop
        default:
          break;
    }

    //erase bullet
    imageRender_drawSprite(head->x, oldY, bulletSprites[head->currentState], ALIEN_BULLET_WIDTH, ALIEN_BULLET_ERASE_HEIGHT, IMAGE_RENDER_BACKGROUND_COLOR, SPRITE_SCALING_FACTOR );
    if (head->y >= END_OF_SCREEN)
    { //erase bullet end of screen
      imageRender_drawSprite(head->x,  head->y, bulletSprites[head->currentState], ALIEN_BULLET_WIDTH, ALIEN_BULLET_HEIGHT, IMAGE_RENDER_BACKGROUND_COLOR, SPRITE_SCALING_FACTOR );
      alienBullet* temp = head;
      stack_removeNode (&activeBullets, head);
      head = head->next;
      stack_insert(&sleepingBullets, temp);
      --numBulletsInUse;
    }
    else
    {//erase standard bullet
      imageRender_drawSprite(head->x, head->y, bulletSprites[head->currentState], ALIEN_BULLET_WIDTH, ALIEN_BULLET_HEIGHT, ALIEN_BULLET_COLOR, SPRITE_SCALING_FACTOR );
      head->currentState = (head->currentState == BULLET_1 ) ? BULLET_1B :
                           (head->currentState == BULLET_1B) ? BULLET_1  :
                           (head->currentState == BULLET_2 ) ? BULLET_2B : BULLET_2;
      head = head->next; //move to the next active bullet
    }
  }
}

// Purpose:
//    Print currently used state
//  Takes:
//  Dependencies:
//    printf
//  Uses:
//    currentState
//    previousState
void printState()
{
  if (currentState != previousState){ // only print state changes, to avoid cluttering the terminal with too much noise
    printf("alienBullet.c STATE: ");
    previousState = currentState;
    switch(currentState){
      case state_init:
        printf("state_init\n");
        break;
      case state_wait:
        printf("state_wait\n");
        break;
      case state_advance:
        printf("state_advance\n");
        break;
      case state_gameOver:
        printf("state_gameOver\n");
        break;
      default:
        printf("UNKNOWN STATE\n");

    }
  }
}

// Purpose:
//    Run the alienBullet Statemachine for one cycle
//  Takes:
//  Dependencies:
//    alienBullet_advance
//    alienBullet_init
//  Uses:
//    timer
//    currentState
void alienBullet_tick()
{ //Actions
  switch(currentState)
  { //These are Mealy actions
      case state_init: //initialize state machine
        alienBullet_init();
        break;
      case state_wait: //wait to fire
        ++timer;
        break;
      case state_advance: //move each bullet
        alienBullet_advance();
        break;
      case state_gameOver: //game has ended
        break;
  }

  //State Change
  switch(currentState)
  { //This is a moore state machine
      case state_init: //initialize state machine
        currentState = state_wait;
        break;
      case state_wait: //wait to fire
        if (global_gameOver)
        { //if game is over
          currentState = state_gameOver;
        }
        else if (timer >= ALIEN_BULLET_TICKS_STEP)
        { //if timer has exired
          currentState = state_advance;
        }
        break;
      case state_advance:
        if (global_gameOver)
        { //if game is over
          currentState = state_gameOver;
        }
        else
        { //return to wait state
          currentState = state_wait;
          timer = 0;
        }
        break;
      case state_gameOver:
        break;
  }
}


// Purpose:
//    Add a new bullet to the active bullet list
//  Takes:
//    x, horizontal pixel location to draw bullet
//    y, vertical pixel location to draw bullet
//  Dependencies:
//    stack_insert
//    stack_pop
//  Uses:
//    activeBullets
//    sleepingBullets
bool alienBullet_fire(int32_t x, int32_t y)
{
  if (numBulletsInUse >= ALIEN_BULLET_MAX_COUNT)
  { //if maximum number of bullets has been reached
    return false; //Cannot fire a bullet at this time, as too many are in use
  }
  numBulletsInUse++;
  alienBullet* newBullet = stack_pop(&sleepingBullets);
  // setting bullets initial position
  newBullet->x  = x;
  newBullet->y  = y + ALIEN_BULLET_HEIGHT_OFFSET;
  //Determining what type of bullet to use
  newBullet->currentState = (rand() % BULLET_TYPES) ? BULLET_1 : BULLET_2 ;
  //add newly created bullet to the stack
  stack_insert(&activeBullets, newBullet);
  return true;
}
