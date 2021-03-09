#ifndef SAUCER_H
#define SAUCE_H
#include <stdint.h>


//HEADER FUNCTIONS
void saucer_init();
void saucer_tick();
void saucer_runTest();
void saucer_registerHit();

uint32_t saucer_spixelsPerStep;

#endif
