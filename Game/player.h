#include <stdbool.h>

#define PLAYER_SPACE_START 440

//HEADER FUNCTIONS
uint32_t player_getCannonColumn(); //in pixels
void player_init();
void player_tick();
void player_registerHit();
