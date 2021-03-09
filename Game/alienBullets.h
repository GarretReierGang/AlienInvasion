#ifndef ALIEN_BULLETS_H
#define ALIEN_BULLETS_H
#include "imageRender.h"
#include <stdbool.h>

#define END_OF_SCREEN 456

//The maximum number of bullets the Aliens are allowed to have out at a time
#define ALIEN_BULLET_MAX_COUNT 5

// Distance each Alien Bullet travels every tick
#define ALIEN_BULLET_STEP_LEN 2

//number of Ticks per alien bullet step
#define ALIEN_BULLET_TICKS_STEP 3

// Color of Alien Bullets
#define ALIEN_BULLET_COLOR white


// Purpose:
//    Run the alienBullet Statemachine for one cycle
//  Takes:
void alienBullet_tick();

// Purpose:
//    Add a new bullet to the active bullet list
//  Takes:
//    x, horizontal pixel location to draw bullet
//    y, vertical pixel location to draw bullet
bool alienBullet_fire(int32_t x, int32_t y);



#endif
