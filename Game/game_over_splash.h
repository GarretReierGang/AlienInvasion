#ifndef GAME_OVER_SPLASH_H
#define GAME_OVER_SPLASH_H
#include "button_handler.h"

#define TIME_TO_BLINK 50

// Purpose:
//     display game over screen
//     handles highscore player name entering
// Sets: nothing
// Uses: global_score
// depends on: button_handler
void game_over_splash_tick();
#endif
