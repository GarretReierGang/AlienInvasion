#include <stdint.h> //uint32_t

//Macro Magic, allows sprites to be written in a more human understandable method
#define packWord24(b23,b22,b21,b20,b19,b18,b17,b16,b15,b14,b13,b12, \
                   b11,b10,b9 ,b8 , b7 ,b6 ,b5 ,b4 ,b3 ,b2 ,b1, b0) \
                 ((b23 << 23) | (b22 << 22) | (b21 << 21) | (b20 << 20) | (b19 << 19) | (b18 << 18) | (b17 << 17) | (b16 << 16) | \
                   (b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) | \
                   (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

#define packWord19(b18,b17,b16,b15,b14,b13,b12,b11,b10,b9 ,b8 ,b7 ,b6 ,b5 ,b4 ,b3 ,b2 ,b1, b0) \
             ( (b18 << 18) | (b17 << 17) | (b16 << 16) | (b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | \
               (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) | \
               (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | \
               (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )
#define packWord18(b17,b16,b15,b14,b13,b12,b11,b10,b9 ,b8 ,b7 ,b6 ,b5 ,b4 ,b3 ,b2 ,b1, b0) \
              ( (b17 << 17) | (b16 << 16) | (b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | \
                (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) | \
                (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | \
                (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )
#define packWord17(b16,b15,b14,b13,b12,b11,b10,b9 ,b8 ,b7 ,b6 ,b5 ,b4 ,b3 ,b2 ,b1, b0) \
               ( (b16 << 16) | (b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | \
                 (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) | \
                 (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | \
                 (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )
#define packWord16(b15,b14,b13,b12,b11,b10,b9 ,b8 ,b7 ,b6 ,b5 ,b4 ,b3 ,b2 ,b1, b0) \
                ( (b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | \
                  (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) | \
                  (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | \
                  (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )
#define packWord15(b14,b13,b12,b11,b10,b9 ,b8 ,b7 ,b6 ,b5 ,b4 ,b3 ,b2 ,b1, b0) \
                ( (b14 << 14) | (b13 << 13) | (b12 << 12) | \
                  (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) | \
                  (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | \
                  (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

#define packWord12(b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0) \
                ( (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) | \
                  (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | \
                  (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

#define packWord13(b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0) \
                ( (b12 << 12) | packWord12(b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0) )
#define packWord6(b5,b4,b3,b2,b1,b0) \
                ( (b5  << 5 ) | (b4  << 4 ) | (b3  << 3 ) | \
                  (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )
#define packWord5(b4,b3,b2,b1,b0) \
                ( (b4  << 4 ) | (b3  << 3 ) | \
                  (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )
#define packWord3(b2,b1,b0) \
                ((b2 << 2 ) | (b1 << 1 ) | (b0 << 0))
#define packWord1(b0) \
                ( (b0 << 0))

//SAUCER
static uint32_t saucer_16x7[] = {
	packWord16(0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0),
	packWord16(0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord16(0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0),
	packWord16(0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0),
	packWord16(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord16(0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0),
	packWord16(0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0)
};
static uint32_t saucer_19x7[] = {
	packWord19(0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0),
	packWord19(0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0),
	packWord19(0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord19(0,0,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,0),
	packWord19(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord19(0,0,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,0),
	packWord19(0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0)
};
static uint32_t saucer_explosion_19x7[] = {
	packWord19(0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0),
	packWord19(0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0),
	packWord19(0,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0,0,1,0),
	packWord19(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0),
	packWord19(0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0),
	packWord19(0,0,0,0,1,0,1,0,0,1,0,0,0,1,0,0,0,0,0),
	packWord19(0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0)
};
//ALIENS
static uint32_t alien_explosion_13x10[] = {
	packWord13(0,0,0,0,0,0,1,0,0,0,0,0,0),
	packWord13(0,0,0,1,0,0,1,0,0,0,1,0,0),
	packWord13(1,0,0,1,0,0,0,0,0,1,0,0,0),
	packWord13(0,1,0,0,1,0,0,0,1,0,0,0,0),
	packWord13(0,0,0,0,0,0,0,0,0,0,1,1,0),
	packWord13(1,1,0,0,0,0,0,0,0,0,0,0,0),
	packWord13(0,0,0,1,0,0,0,1,0,0,1,0,0),
	packWord13(0,0,1,0,0,0,0,0,1,0,0,1,0),
	packWord13(0,1,0,0,0,1,0,0,1,0,0,0,0),
	packWord13(0,0,0,0,0,1,0,0,0,0,0,0,0)
};
static uint32_t alien_top_in_12x8[] = {
	packWord12(0,0,0,0,0,1,1,0,0,0,0,0),
	packWord12(0,0,0,0,1,1,1,1,0,0,0,0),
	packWord12(0,0,0,1,1,1,1,1,1,0,0,0),
	packWord12(0,0,1,1,0,1,1,0,1,1,0,0),
	packWord12(0,0,1,1,1,1,1,1,1,1,0,0),
	packWord12(0,0,0,1,0,1,1,0,1,0,0,0),
	packWord12(0,0,1,0,0,0,0,0,0,1,0,0),
	packWord12(0,0,0,1,0,0,0,0,1,0,0,0)
};
static uint32_t alien_top_out_12x8[] = {
	packWord12(0,0,0,0,0,1,1,0,0,0,0,0),
	packWord12(0,0,0,0,1,1,1,1,0,0,0,0),
	packWord12(0,0,0,1,1,1,1,1,1,0,0,0),
	packWord12(0,0,1,1,0,1,1,0,1,1,0,0),
	packWord12(0,0,1,1,1,1,1,1,1,1,0,0),
	packWord12(0,0,0,0,1,0,0,1,0,0,0,0),
	packWord12(0,0,0,1,0,1,1,0,1,0,0,0),
	packWord12(0,0,1,0,1,0,0,1,0,1,0,0)
};
static uint32_t alien_middle_in_12x8[] = {
	packWord12(0,0,0,1,0,0,0,0,0,1,0,0),
	packWord12(0,0,0,0,1,0,0,0,1,0,0,0),
	packWord12(0,0,0,1,1,1,1,1,1,1,0,0),
	packWord12(0,0,1,1,0,1,1,1,0,1,1,0),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,1,0,1,0,0,0,0,0,1,0,1),
	packWord12(0,0,0,0,1,1,0,1,1,0,0,0)
};
static uint32_t alien_middle_out_12x8[] = {
	packWord12(0,0,0,1,0,0,0,0,0,1,0,0),
	packWord12(0,1,0,0,1,0,0,0,1,0,0,1),
	packWord12(0,1,0,1,1,1,1,1,1,1,0,1),
	packWord12(0,1,1,1,0,1,1,1,0,1,1,1),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,0,1,1,1,1,1,1,1,1,1,0),
	packWord12(0,0,0,1,0,0,0,0,0,1,0,0),
	packWord12(0,0,1,0,0,0,0,0,0,0,1,0)
};
static uint32_t alien_bottom_in_12x8[] = {
	packWord12(0,0,0,0,1,1,1,1,0,0,0,0),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,0),
	packWord12(1,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(1,1,1,0,0,1,1,0,0,1,1,1),
	packWord12(1,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,0,1,1,1,0,0,1,1,1,0,0),
	packWord12(0,1,1,0,0,1,1,0,0,1,1,0),
	packWord12(0,0,1,1,0,0,0,0,1,1,0,0)
};
static uint32_t alien_bottom_out_12x8[] = {
	packWord12(0,0,0,0,1,1,1,1,0,0,0,0),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,0),
	packWord12(1,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(1,1,1,0,0,1,1,0,0,1,1,1),
	packWord12(1,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,0,0,1,1,0,0,1,1,0,0,0),
	packWord12(0,0,1,1,0,1,1,0,1,1,0,0),
	packWord12(1,1,0,0,0,0,0,0,0,0,1,1)
};

//TANKS
static uint32_t tank_15x8[] = {
	packWord15(0,0,0,0,0,0,0,1,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,1,1,1,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,1,1,1,0,0,0,0,0,0),
	packWord15(0,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord15(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord15(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord15(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord15(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1)
};
static uint32_t tank_19x8[] = {
  packWord19(0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0),
  packWord19(0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0),
	packWord19(0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0),
	packWord19(0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord19(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0),
	packWord19(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0),
	packWord19(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0),
	packWord19(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0)
};
static uint32_t tankRight_18x8[] = {
	packWord18(0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0),
	packWord18(0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0),
	packWord18(0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0),
	packWord18(0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord18(0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
  packWord18(0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord18(0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord18(0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1)
};
static uint32_t tankLeft_18x8[] = {
	packWord18(0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0),
	packWord18(0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0),
	packWord18(0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0),
	packWord18(0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0),
	packWord18(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0),
  packWord18(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord18(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord18(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0)
};
static uint32_t tank_explosion1_15x8[] = {
	packWord15(0,0,0,0,1,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,1,0,0,0,0,1,0,0,1,0,0,0,0),
	packWord15(0,0,0,0,1,0,1,0,0,1,0,1,0,0,0),
	packWord15(0,0,1,0,0,0,0,0,0,0,0,0,1,0,0),
	packWord15(0,0,0,0,0,1,0,1,1,0,0,0,0,0,0),
	packWord15(1,0,0,1,1,1,1,1,1,1,1,0,0,0,0),
	packWord15(0,0,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord15(1,1,1,1,1,1,1,1,1,1,1,1,1,0,0)
};
static uint32_t tank_explosion2_15x8[] = {
	packWord15(1,0,0,0,0,0,1,0,0,0,0,0,1,0,0),
	packWord15(0,0,0,1,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,1,0,0,0,0,1,0,0,0,0,0,1,0,0),
	packWord15(0,0,0,0,0,1,0,0,0,0,1,0,0,0,1),
	packWord15(1,0,0,0,1,1,0,0,1,0,0,0,0,0,0),
	packWord15(0,0,0,1,1,1,1,1,1,1,0,0,1,0,0),
	packWord15(0,0,1,1,1,1,1,1,1,1,1,0,0,0,0),
	packWord15(1,1,1,1,1,1,1,1,1,1,1,1,1,0,0)
};
static uint32_t tank_gone_15x8[] = {
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
};

//BUNKERS
static uint32_t bunker_24x18[] = {
	packWord24(0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord24(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0),
	packWord24(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1)
};
static uint32_t bunkerGone_6x6[] = {
	packWord6(0,0,0,0,0,0),
	packWord6(0,0,0,0,0,0),
	packWord6(0,0,0,0,0,0),
	packWord6(0,0,0,0,0,0),
	packWord6(0,0,0,0,0,0),
	packWord6(0,0,0,0,0,0)
};
static uint32_t bunkerDamage0_6x6[] = {
	packWord6(0,1,1,0,0,0),
	packWord6(0,0,0,0,0,1),
	packWord6(1,1,0,1,0,0),
	packWord6(1,0,0,0,0,0),
	packWord6(0,0,1,1,0,0),
	packWord6(0,0,0,0,1,0)
};
static uint32_t bunkerDamage1_6x6[] = {
	packWord6(1,1,1,0,1,0),
	packWord6(1,0,1,0,0,1),
	packWord6(1,1,0,1,1,1),
	packWord6(1,0,0,0,0,0),
	packWord6(0,1,1,1,0,1),
	packWord6(0,1,1,0,1,0)
};
static uint32_t bunkerDamage2_6x6[] = {
	packWord6(1,1,1,1,1,1),
	packWord6(1,0,1,1,0,1),
	packWord6(1,1,0,1,1,1),
	packWord6(1,1,0,1,1,0),
	packWord6(0,1,1,1,0,1),
	packWord6(1,1,1,1,1,1)
};
static uint32_t bunkerDamage3_6x6[] = {
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1)
};
static uint32_t bunker_upper_left_gone_6x6[] = {
	packWord6(0,0,0,1,1,1),
	packWord6(0,0,1,1,1,1),
	packWord6(0,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1)
};
static uint32_t bunker_upper_right_gone_6x6[] = {
	packWord6(1,1,1,0,0,0),
	packWord6(1,1,1,1,0,0),
	packWord6(1,1,1,1,1,0),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1)
};
static uint32_t bunker_lower_left_gone_6x6[] = {
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(0,0,1,1,1,1),
	packWord6(0,0,0,1,1,1),
	packWord6(0,0,0,0,1,1),
	packWord6(0,0,0,0,0,1)
};
static uint32_t bunker_lower_right_gone_6x6[] = {
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,0,0),
	packWord6(1,1,1,0,0,0),
	packWord6(1,1,0,0,0,0),
	packWord6(1,0,0,0,0,0)
};

//BULLETS
static uint32_t tankbullet_1x5[] = {
	packWord1(1),
	packWord1(1),
	packWord1(1),
	packWord1(1),
	packWord1(1)
};
static uint32_t tankbullet_gone_1x5[] = {
	packWord1(0),
	packWord1(0),
	packWord1(0),
	packWord1(0),
	packWord1(0)
};
static uint32_t alienbullet1_down_3x5[] = {
	packWord3(0,1,0),
	packWord3(0,1,0),
	packWord3(0,1,0),
	packWord3(1,1,1),
	packWord3(0,1,0)
};
static uint32_t alienbullet1_up_3x5[] = {
	packWord3(0,1,0),
	packWord3(1,1,1),
	packWord3(0,1,0),
	packWord3(0,1,0),
	packWord3(0,1,0)
};
static uint32_t alienbullet2_up_3x5[] = {
	packWord3(0,1,0),
	packWord3(1,0,0),
	packWord3(0,1,0),
	packWord3(0,0,1),
	packWord3(0,1,0)
};
static uint32_t alienbullet2_down_3x5[] = {
	packWord3(0,1,0),
	packWord3(0,0,1),
	packWord3(0,1,0),
	packWord3(1,0,0),
	packWord3(0,1,0)
};
static uint32_t alienbullet2_gone_3x5[] = {
	packWord3(0,0,0),
	packWord3(0,0,0),
	packWord3(0,0,0),
	packWord3(0,0,0),
	packWord3(0,0,0),
};

//LETTERS
static uint32_t letterA_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1)
};
static uint32_t letterB_5x5[] = {
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,0)
};
static uint32_t letterC_5x5[] = {
	packWord5(0,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(0,1,1,1,1)
};
static uint32_t letterD_5x5[] = {
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,0)
};
static uint32_t letterE_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1)
};
static uint32_t letterF_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0)
};
static uint32_t letterG_5x5[] = {
	packWord5(0,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,1,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,1,1,0)
};
static uint32_t letterH_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1)
};
static uint32_t letterI_5x5[] = {
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0)
};
static uint32_t letterJ_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(1,1,1,0,0)
};
static uint32_t letterK_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,1,0),
	packWord5(1,1,1,0,0),
	packWord5(1,0,0,1,0),
	packWord5(1,0,0,0,1)
};
static uint32_t letterL_5x5[] = {
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1)
};
static uint32_t letterM_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,1,0,1,1),
	packWord5(1,0,1,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1)
};
static uint32_t letterN_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,1,0,0,1),
	packWord5(1,0,1,0,1),
	packWord5(1,0,0,1,1),
	packWord5(1,0,0,0,1)
};
static uint32_t letterO_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,1,1,0)
};
static uint32_t letterP_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0)
};
static uint32_t letterQ_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,1,0),
	packWord5(0,1,1,0,1)
};
static uint32_t letterR_5x5[] = {
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1)
};
static uint32_t letterS_5x5[] = {
	packWord5(0,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(0,1,1,1,0),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,0)
};
static uint32_t letterT_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0)
};
static uint32_t letterU_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,1,1,0)
};
static uint32_t letterV_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,0,1,0),
	packWord5(0,0,1,0,0)
};
static uint32_t letterW_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,1,0,1),
	packWord5(1,0,1,0,1),
	packWord5(0,1,0,1,0)
};
static uint32_t letterX_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(0,1,0,1,0),
	packWord5(0,0,1,0,0),
	packWord5(0,1,0,1,0),
	packWord5(1,0,0,0,1)
};
static uint32_t letterY_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(0,1,0,1,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0)
};
static uint32_t letterZ_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,1,0),
	packWord5(0,0,1,0,0),
	packWord5(0,1,0,0,0),
	packWord5(1,1,1,1,1)
};

//NUMBERS
static uint32_t number0_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,1,1,0)
};
static uint32_t number1_5x5[] = {
	packWord5(1,1,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(1,1,1,1,1)
};
static uint32_t number2_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1)
};
static uint32_t number3_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1)
};
static uint32_t number4_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(0,0,0,0,1)
};
static uint32_t number5_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1)
};
static uint32_t number6_5x5[] = {
	packWord5(0,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1)
};
static uint32_t number7_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(0,0,0,0,1),
	packWord5(0,0,0,0,1),
	packWord5(0,0,0,0,1)
};
static uint32_t number8_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1)
};
static uint32_t number9_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1)
};
static uint32_t letterBLANK_5x5[] = {
	packWord5(0,0,0,0,0),
	packWord5(0,0,0,0,0),
	packWord5(0,0,0,0,0),
	packWord5(0,0,0,0,0),
	packWord5(0,0,0,0,0)
};
