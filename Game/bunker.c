#include "bunker.h"
#include "imageRender.h"

/*
  Bunkers are divided into 10 sections
   _______   _________  _________  _______
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

//MACROS
#define BUNKER_FULL_HEALTH 3
#define BUNKER_COUNT 4
#define SECTION_COUNT 10
#define SECTION_COLUMN_COUNT 4
#define SECTION_ROW_COUNT 3
#define SECTION_WIDTH 6
#define SECTION_HEIGHT 6
#define SECTION_Y_MAX (SECTION_HEIGHT * SECTION_ROW_COUNT)
#define SECTION_0 0
#define SECTION_1 1
#define SECTION_2 2
#define SECTION_3 3
#define DEAD_BUNKER -1
#define Y_SECTION_0_BOTTOM 5
#define Y_SECTION_1_BOTTOM 11
#define X_SECTION_0_RIGHT 5
#define X_SECTION_1_RIGHT 11
#define X_SECTION_2_RIGHT 17
#define X_SECTION_3_RIGHT 23
#define BUNKER_MIN_Y 390

//GLOBAL VARIABLES
static int32_t bunkerHealth[BUNKER_COUNT][SECTION_COUNT];

//HELPER FUNCTIONS
static uint32_t getXSection(uint32_t x, uint32_t bunkerNumber){
  //FUNCTION HEADER
  /*
  PURPOSE: get the x section of the given bunker
  ARGUMENTS:
    x - the x coordnate of the section
    bunkerNumber - identifies which bunker
  RETURN VALUE:
    the x section of the given bunker
  */

  //LOCAL VARIABLES
  uint32_t xNormalized = x - (START_COLUMN_BUNKERS + bunkerNumber*NEXT_BUNKER); //ranges from 0 to 48 to identify section
  uint32_t sectionLeftSide = 0;
  uint32_t sectionRightSide = SECTION_WIDTH;

  //DE-SCALE
  xNormalized /= SPRITE_SCALING_FACTOR; //ranges from 0 to 24

  //GET THE X SECTION
  if (xNormalized <= X_SECTION_0_RIGHT)//x section is determined by the xNormalized coordinate
    return SECTION_0;
  else if (xNormalized <= X_SECTION_1_RIGHT)//x section is determined by the xNormalized coordinate
    return SECTION_1;
  else if (xNormalized <= X_SECTION_2_RIGHT)//x section is determined by the xNormalized coordinate
    return SECTION_2;
  else if (xNormalized <= X_SECTION_3_RIGHT)//x section is determined by the xNormalized coordinate
    return SECTION_3;

  //ERROR CHECKING: OUTSIDE OF BOUNDARIES
  for (uint32_t i = 0; i < SECTION_COLUMN_COUNT; i++){ //for each column in the bunker
    if (xNormalized >= sectionLeftSide && xNormalized <= sectionRightSide)
      return i; //correct section
    sectionLeftSide  += SECTION_WIDTH;
    sectionRightSide += SECTION_WIDTH;
  }
}
static uint32_t getYSection(uint32_t y){
  //FUNCTION HEADER
  /*
  PURPOSE: get the y section of the given bunker
  ARGUMENTS:
    y - the y coordnate of the section
  RETURN VALUE:
    the y section of the given funker
  */
  //LOCAL VARIABLES
  int32_t yNormalized = (y - START_ROW_BUNKERS) / SPRITE_SCALING_FACTOR; //ranges 0 to 18
  uint32_t sectionTopSide = 0;
  uint32_t sectionBottomSide = SECTION_HEIGHT+1; //+1 makes the top section's hit box within bullet range
  uint32_t section;

  //GET Y SECTION
  if (yNormalized <= Y_SECTION_0_BOTTOM)
    return SECTION_0;
  else if (yNormalized <= Y_SECTION_1_BOTTOM)
    return SECTION_1;
  else
    return SECTION_2;

  //ERROR CHECKING: OUTSIDE OF BOUNDARIES
  for (uint32_t i = 0; i < SECTION_ROW_COUNT; i++){ //for each row in the bunker
    if (yNormalized >= sectionTopSide && yNormalized < sectionBottomSide)
      return i; //return the correct section
    sectionTopSide    += SECTION_HEIGHT;
    sectionBottomSide += SECTION_HEIGHT;
  }

  //BOUNARY CASES
  if (yNormalized < 0)
    return SECTION_0;
  if (yNormalized > SECTION_Y_MAX)
    return SECTION_2;

}
static uint32_t getBunkerNumber(uint32_t x){
  //FUNCTION HEADER
  /*
  PURPOSE: Indentifies one of the four bunkers
  ARGUMENTS:
    x - the x coordnate of the bunker
  RETURN VALUE:
    The integer value of the bunker identifier
  */

  //LOCAL VARIABLES
  uint32_t bunkerLeftSide = START_COLUMN_BUNKERS;
  uint32_t bunkerRightSide = bunkerLeftSide + (BUNKER_WIDTH * SPRITE_SCALING_FACTOR);

  //FOR EACH BUNKER
  for (uint32_t bunkerNumber = 0; bunkerNumber < BUNKER_COUNT; bunkerNumber++){
    if (x >= bunkerLeftSide && x <= bunkerRightSide) //if this is the correct bunker
      return bunkerNumber; //return the correct value
  bunkerLeftSide  += NEXT_BUNKER;
  bunkerRightSide += NEXT_BUNKER;
  }
}
static uint32_t getBunkerSection(uint32_t sectionX, uint32_t sectionY){
  //FUNCTION HEADER
  /*
  PURPOSE: Gets the section number of the bunker
  ARGUMENTS:
    sectionX - the x section of the bunker
    sectionY - the y section of the bunker
  RETURN VALUE:
    returns which of the 9 sections of the bunker
  */

  //LOCAL VARIABLES
  uint32_t bunkerSection = sectionY * (SECTION_COLUMN_COUNT) + sectionX;

  //EXCEPTION
  if (bunkerSection >= SECTION_COUNT) //section 9 is computed to be 11
    bunkerSection = (SECTION_COUNT-1);

  return bunkerSection;
}
static void applyDamage(uint32_t bunker, uint32_t section){
  //FUNCTION HEADER
  /*
  PURPOSE: Erode the section of the bunker that got hit
  ARGUMENTS:
    bunker - identifies which of the four bunkers got hit
    section - identifies which section in the bunker got hit
  RETURN VALUE:
    void
  */

  //HIT DEAD BUNKER
  if (bunkerHealth[bunker][section] == BUNKER_DAMAGE_DEAD){
    printf("ERROR! bunker.c: Registered hit on dead bunker: %d SECTION: %d\n", bunker, section);
    return; //do not apply damage to a dead bunker
  }
  bunkerHealth[bunker][section]--; //apply damage
}
static void printBunkers(){
  //FUNCTION HEADER
  /*
  PURPOSE: Print the condition of each bunker for debugging
  ARGUMENTS: None
  RETURN VALUE: None
  */

  for (uint32_t i = 0; i < BUNKER_COUNT; i++){ //for each bunker
    printf("Bunker %d\n", i);
    for (uint32_t j = 0; j < SECTION_COUNT; j++){ //for each section
      if (j == SECTION_COLUMN_COUNT || j == SECTION_COLUMN_COUNT * SECTION_2)
        printf("\n"); //end of row
      if (j == SECTION_COUNT-1)
        printf("    "); //missing middle space
      if (bunkerHealth[i][j] != DEAD_BUNKER) //if bunker is dead
        printf("%d ", bunkerHealth[i][j]);
      else
        printf("- ");
    }
    printf("\n\n");
  }
}

//HEADER FUNCITONS
void bunker_init(){
  //FUNCTION HEADER
  /*
  PURPOSE: Initialize the bunkers
  ARGUMENTS: None
  RETURN VALUE: None
  */

  //bunkerDamage3_6x6 = full health = 3
  //bunkerDamage0_6x6 = low health = 0
  //-1 = no bunker
  for (uint32_t i = 0; i < BUNKER_COUNT; i++) //for each bunker
    for (uint32_t j = 0; j < SECTION_COUNT; j++)//for each section
      bunkerHealth[i][j] = BUNKER_FULL_HEALTH; //start at full health
}
bool bunker_registerHit(uint32_t x, uint32_t y){
  //FUNCTION HEADER
  /*
  PURPOSE: A hit has occured on a bunker
  ARGUMENTS:
    x - the x coordinate of the bunker
    y - the y coordinate of the bunker
  RETURN VALUE:
    bool - true if the hit is valid, false if invalid
  */

  //OUT OF BOUNDS
  if (y < BUNKER_MIN_Y)
    return false;

  //LOCAL VARIABLES
  uint32_t bunkerNumber = getBunkerNumber(x);
  uint32_t sectionX = getXSection(x,bunkerNumber);
  uint32_t sectionY = getYSection(y);
  uint32_t bunkerSection = getBunkerSection(sectionX, sectionY);
  uint32_t sectionHealth;

  //APPLY DAMAGE
  applyDamage(bunkerNumber, bunkerSection);

  //DRAW DAMAGED SECTION
  sectionHealth = bunkerHealth[bunkerNumber][bunkerSection];
  imageRender_drawBunkerDamage(bunkerNumber, sectionX, sectionY, bunkerSection, sectionHealth);

  //PRINT BUNKERS
  //printBunkers();
}
