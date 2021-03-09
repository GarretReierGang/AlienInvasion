#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>
#include <stdint.h>

//FUNCTIONS
void bullet_init();
void bullet_tick();
void bullet_firePlayerBullet();
void bullet_setCollision();
enum pixelColor bullet_getPixelColor(uint32_t x, uint32_t y);

//ENUMERATED TYPE
typedef enum pixelColor{
  redPixel,
  greenPixel,
  bluePixel,
  blackPixel,
  whitePixel,
  unknownPixel
} pixelColor;


#endif
