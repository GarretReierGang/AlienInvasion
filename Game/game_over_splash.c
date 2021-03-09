#include "game_over_splash.h"
#include "imageRender.h"
#include "button_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"

//MACROS
#define HIGH_SCORES "high scores"
#define HIGH_SCORE_SAVE_LOCATION "high_score.txt"
#define CLEAR_INITIALS "   "
#define FIRST_LETTER 'a'
#define LAST_LETTER 'z'
#define INITIALS_COUNT 3
#define HIGH_SCORES_LETTER_COUNT 11
#define MAX_LETTER_POSITION_FOR_HIGHSCORE 3
#define GAME_MAX_HIGHSCORE_PLAYER_COUNT 10
#define MAX_SCORE_LENGTH 6
#define START_HALL_OF_FAME_LIST (START_BYTE_HIGH_SCORES + SPACING_LETTER_LINES + SPACING_LETTER + IMAGE_RENDER_ONE_ROW_IN_BYTES)
#define SPACING_LETTER_LINES (IMAGE_RENDER_ONE_ROW_IN_BYTES * 12)
#define SPACING_WORDS         (PIXEL_SIZE_IN_BYTES * PIXELS_BETWEEN_WORDS)
#define LOWEST_HIGH_SCORE 9
#define START_OF_ENTER_YOUR_NAME_POMPT
#define START_OF_GAME_OVER_LETTERING
#define START_OF_NAME_ENTERING_SPOT
#define PLAYER_NAME_ARRAY_SIZE 4
#define BEGINING_OF_SCREEN 0
#define ERASE_20_ROWS 20

//STATE MACHINE
typedef enum stateMachine
{
  splash_waitForGameToEnd = 0,
  splash_init,
  splash_enternameBlink,
  splash_enternameFade,
  splash_displayScore,
  splash_waitForGameRestart
} stateMachine;

//GLOBAL VARIALBES
static enum stateMachine currentState = splash_waitForGameToEnd;
static uint32_t blinkTimer;
static uint32_t splash_currentLetter = 0;
static bool prompt_forNameEntering;
static char splash_playerName[PLAYER_NAME_ARRAY_SIZE] = {'a','a','a','\0'};
static uint32_t splash_hallOfFamePlayerCount;
static uint32_t* splash_hallOfFameScores = NULL;
static char **  splash_hallOfFamePlayers = NULL;

// Purpose:
//    Load the hall of fame players from file.
//  Uses: local global splash_hallOfFameScores, && splash_hallOfFamePlayers
//          As these contain the hallOfFamePlayers
//  uses: global_playerScore, contains the current players score, and displays it
//  Depends on <stdio> for acces to non-volatile memory
static void getHallOfFame() {
  splash_hallOfFamePlayerCount = 0;
  if (splash_hallOfFameScores != NULL)
  { //Check to see if the hallOfFamePlayers variable is NULL, if it is not free it
    for (uint32_t player = 0; player < splash_hallOfFamePlayerCount; ++player)
    { //as hallOfFamePlayers is an array of arrays, need to free the memmory allocated for each name
        free((void*)splash_hallOfFamePlayers[player]);
    }
    splash_hallOfFamePlayers = NULL;
  }
  if (splash_hallOfFameScores = NULL)
  { //if the highScores variable is not NULL, free the memory there as well
    free(splash_hallOfFameScores);
    splash_hallOfFameScores = NULL;
  }

  //WRITE TO FILE
  FILE* file;
  char* buffer = NULL;
  uint32_t bufferLen;
  file = fopen(HIGH_SCORE_SAVE_LOCATION, "r");
  while(getline(&buffer, &bufferLen, file) > 0)
  { //Counting the number of lines--Players--in the file
    splash_hallOfFamePlayerCount++;
  }

  splash_hallOfFamePlayers = (char **) malloc(  sizeof(char*) * splash_hallOfFamePlayerCount);
  for (uint32_t t = 0; t < splash_hallOfFamePlayerCount; ++t)
  { //Malocing 4 byte char arrays to store the players names in.
    splash_hallOfFamePlayers[t] = malloc( sizeof(char[MAX_LETTER_POSITION_FOR_HIGHSCORE]));
  }

  splash_hallOfFameScores =  (uint32_t *) malloc(sizeof(uint32_t) * splash_hallOfFamePlayerCount);
  rewind(file);
  uint32_t count = 0;
  while(!feof (file))
  { //Iterate through the file a second time to fill the newly allocated hallOfFamePlayers, and highScore arrays
      if (count == splash_hallOfFamePlayerCount)
        break;
      fscanf(file, "%s %d", (splash_hallOfFamePlayers[count]), (splash_hallOfFameScores + count));
      count++;
  }
}

// Purpose:
//    Save the hallOfFamePlayer list to non-volatile memory
//  Uses: local global splash_hallOfFameScores, && splash_hallOfFamePlayers
//          As these contain the hallOfFamePlayers
//  Depends on:
//    getHallOfFame(). getHallOfFame must be ran some point before
//      this code runs.
static void storeHallOfFame() {
  FILE* file = fopen(HIGH_SCORE_SAVE_LOCATION,"w");
  bool playerScoredHigher = false;
  uint32_t hallOfFamePlayer = 0;
  for (uint32_t player = 0; player < splash_hallOfFamePlayerCount; ++player)
  { // Loop through each of the players in the hall of Fame

    if (player == GAME_MAX_HIGHSCORE_PLAYER_COUNT)
    { //if the Maximum number of highscore players has been reached, stop writing to the file
      fclose(file);
      return;
    }
    if (!playerScoredHigher && splash_hallOfFameScores[player] < global_playerScore)
    { // If the player has not yet scored higher, but is higher than this highscore, add them to file
      playerScoredHigher = true;
      fprintf(file, "%s %d\n", splash_playerName, global_playerScore);
    }
    else
    {
      fprintf(file, "%s %d\n", splash_hallOfFamePlayers[hallOfFamePlayer], splash_hallOfFameScores[hallOfFamePlayer]);
      hallOfFamePlayer++;
    }
  }
  fclose(file);
}
// Purpose To draw the hall of Fame scores.
//  Takes: Nothing
//  Uses local global splash_hallOfFameScores, && splash_hallOfFamePlayers
// to draw score
static void drawScores() {
  //Draw scores
  // draws the highscores of previous players
  //
  getHallOfFame();
  for (uint32_t player = 0; player < splash_hallOfFamePlayerCount; ++player)
  { // loop through each player in the hallOfFame and print them out onto the screen
    uint32_t cursor = START_HALL_OF_FAME_LIST + ( player * SPACING_LETTER_LINES);
    cursor = imageRender_drawWord(splash_hallOfFamePlayers[player], MAX_LETTER_POSITION_FOR_HIGHSCORE, cursor, white);
    cursor += SPACING_WORDS;
    char buffer[GAME_MAX_HIGHSCORE_PLAYER_COUNT];
    sprintf(buffer, "%06d", splash_hallOfFameScores[player]);
    imageRender_drawWord(buffer, MAX_SCORE_LENGTH, cursor, green);
  }
}
// Purpose:
//   statemachine action to write players Name
//  Sets: playerName;
//  depends on: button_handler for manipulating the player's name
void spellHighScoreName() {
  if (button0_released)
  { //button 0 is no longer pressed
    button_handler_clear_flags();
    imageRender_drawLetter(splash_playerName[splash_currentLetter], SPRITE_SCALING_FACTOR, START_BYTE_NAME_ENTERING_SPOT + (splash_currentLetter * SPACING_LETTER), white);\
    currentState = splash_enternameFade;
    blinkTimer = 0;
    splash_currentLetter++; //go to next letter
  }
  else if (button1_released)
  { //button 1 is no longer pressed
    button_handler_clear_flags();

    if (splash_playerName[splash_currentLetter] == LAST_LETTER) //if letter is Z
        splash_playerName[splash_currentLetter] = FIRST_LETTER; //roll over to A
    else //go to next initial
        splash_playerName[splash_currentLetter]++;
    imageRender_drawLetter(splash_playerName[splash_currentLetter], SPRITE_SCALING_FACTOR, START_BYTE_NAME_ENTERING_SPOT + (splash_currentLetter * SPACING_LETTER), white);

  }
  else if (button2_released)
  { //button 2 is no longer pressed
    button_handler_clear_flags();
    if (splash_playerName[splash_currentLetter] == FIRST_LETTER) //if letter is A
        splash_playerName[splash_currentLetter] = LAST_LETTER; //roll over to Z
    else
        splash_playerName[splash_currentLetter]--;
    imageRender_drawLetter(splash_playerName[splash_currentLetter], SPRITE_SCALING_FACTOR, START_BYTE_NAME_ENTERING_SPOT + (splash_currentLetter * SPACING_LETTER), white);
  }
  else if (button3_released)
  { //button 3 is no longer pressed
    button_handler_clear_flags();

    imageRender_drawLetter(splash_playerName[splash_currentLetter], SPRITE_SCALING_FACTOR, START_BYTE_NAME_ENTERING_SPOT + (splash_currentLetter * SPACING_LETTER), white);
    currentState = splash_enternameFade;
    blinkTimer = 0;
    splash_currentLetter--;
    if (splash_currentLetter < 0) //roll over
        splash_currentLetter = 0;
  }
}

// Purpose:
//     display game over screen
//     handles highscore player name entering
// Sets: nothing
// Uses: global_score
// depends on: button_handler
void game_over_splash_tick()
{ //File tick funciton
  switch (currentState) { //moore actions
    case splash_enternameFade:
      ++blinkTimer;
    break;
    case splash_enternameBlink:
      ++blinkTimer;
    break;
  }

  switch(currentState)
  { //File tick funciton
    case splash_waitForGameToEnd:
      // wait for the game to end, do nothing until then
      if (global_gameOver)
      { //if game is over
        printf("Game Over Splash: GameOver\n\r");
        imageRender_clearScreen();
        getHallOfFame();
        if (global_playerScore > splash_hallOfFameScores[LOWEST_HIGH_SCORE])
          currentState = splash_init;
        else
          currentState = splash_displayScore;
        imageRender_drawBlankLines(BEGINING_OF_SCREEN, ERASE_20_ROWS);
        imageRender_drawScoreScreen();
      }
      break;

    case splash_init:
      // displays game over splash
      currentState = splash_enternameFade;
      blinkTimer = 0;
      button_handler_clear_flags();
    break;

    case splash_enternameBlink:
      // turns the players chosen char back on
      if (blinkTimer == TIME_TO_BLINK)
      { //if blinker timer has expired
        currentState = splash_enternameFade;
        blinkTimer = 0;
        imageRender_drawLetter(splash_playerName[splash_currentLetter], SPRITE_SCALING_FACTOR, START_BYTE_NAME_ENTERING_SPOT + (splash_currentLetter * SPACING_LETTER), white);
      }
      spellHighScoreName();
    break;

    case splash_enternameFade:
      // causes the currrently selected char to blink off
      if(splash_currentLetter >= MAX_LETTER_POSITION_FOR_HIGHSCORE)
      { //if finished entering initials
        currentState = splash_displayScore;
        //Erase Enter Your name Prompt
      }
      else if (blinkTimer == TIME_TO_BLINK)
      { //if blink timer has expired
        currentState = splash_enternameBlink;
        blinkTimer = 0;
        imageRender_drawLetter(splash_playerName[splash_currentLetter], SPRITE_SCALING_FACTOR, START_BYTE_NAME_ENTERING_SPOT + (splash_currentLetter * SPACING_LETTER), IMAGE_RENDER_BACKGROUND_COLOR);
      }
      spellHighScoreName();
    break;

    case splash_displayScore:
      //ERASE "Enter YOUR NAME"
      imageRender_drawBlankLines(START_BYTE_ENTER_NAME, PIXELS_BETWEEN_LETTERS * SPRITE_SCALING_FACTOR );

      //DISPLAY HIGH SCORES
      storeHallOfFame();

      //CLEAR 3 INITIALS
      imageRender_drawWord(CLEAR_INITIALS, INITIALS_COUNT, START_BYTE_NAME_ENTERING_SPOT, IMAGE_RENDER_BACKGROUND_COLOR);

      //DRAW "high scores"
      imageRender_drawWord(HIGH_SCORES, HIGH_SCORES_LETTER_COUNT, START_BYTE_HIGH_SCORES, white);

      //DRAW SCORES
      drawScores();
      currentState = splash_waitForGameRestart;
      break;
    case splash_waitForGameRestart:
      break;
  }
}
