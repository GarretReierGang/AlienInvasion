#ifndef ALIEN_CONTROL_H
#define ALIEN_CONTROL_H
#include <stdbool.h>

//MACROS
#define ALIEN_ALIVE    4
#define ALIEN_EXPLODE1 3
#define ALIEN_EXPLODE2 2
#define ALIEN_EXPLODE3 1
#define ALIEN_DEAD     0
#define ALIEN_ROW_1 0
#define ALIEN_ROW_2 1
#define ALIEN_ROW_3 2
#define ALIEN_ROW_4 3
#define ALIEN_ROW_5 4
#define ALIEN_COLOR red
#define ALIEN_VERTICAL_SPACING 10
#define ALIEN_CONTROL_STEP_LEN 2 //in pixels. If above 6 the aliens leave a trail (October 14th)
#define ALIEN_CONTROL_TICKS_PER_STEP 20

//FUNCTIONS
void alienControl_tick();
void alienControl_init();
void alienControl_registerHit(uint32_t x, uint32_t y);

#endif
