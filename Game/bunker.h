#ifndef BUNKER_H
#define BUNKER_H

//HEADERS
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

//Purpose initialize the bunkers for gameplay
void bunker_init();

//Purpose register a hit against a bunker, cause the bunker to crack and shatter
// arg x horizontal pixel location of the collision
// arg y vertical pixel location of the collision
bool bunker_registerHit(uint32_t x, uint32_t y);

#endif
