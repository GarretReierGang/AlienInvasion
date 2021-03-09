#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include <stdint.h>

//MACROS
#define NUM_ALIENS_X 11
#define NUM_ALIENS_Y 5
#define PLAYER_LIVES_MAX 5

//VARIABLES
//current players score is stored here
extern uint32_t global_playerScore;

// Signaling flag used to inform the various state machines that the game has ended
extern bool global_gameOver;

//  the number of lives the player currently has, used to determine if the game is over
extern int32_t global_playerLives;

extern bool global_saucerIsOut;

#endif
