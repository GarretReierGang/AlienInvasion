
#define AUDIO_ID "/dev/audio"
#include <stdint.h>


#define PLAY_ALIEN_WALK   0
#define PLAY_SAUCER       1
#define PLAY_ALIEN_DEATH  2
#define PLAY_SAUCER_DEATH 3
#define PLAY_LASER        4
#define PLAY_DEATH        5

//  Purpose:
//    Initializes fileDescriptor
//    Loads .wav files from disk to memory
// Uses:
//    alienWalk, assigns it a value
//    saucer, assigns it a value
//    laserBlast, assigns it a value
//    playerDeath, assigns it a value
//    alienDeath, assigns it a value
//    saucerDeath, assigns it a value
//  Returns:
//    Success/Failure of the attempt
int32_t audioPlayer_init();

//  Purpose:
//    Called every time a hardware click occurs
//    Plays the saucer sound if applicable, allows alien walking sounds to be made
//    if no other sound is currently playing
//    Also increments sound volume
//  Uses:
//    currentState: to determine which actions should
void audioPlayer_tick();

//  Purpose:
//    plays the requested sound,
//    background noises ALIEN_WALK/SAUCER cannot play over currently playing sounds
//    non-background noises will play over other sounds
//  Takes:
//    whatSound, an integer identifying which sound to play, the integer should be between PLAY_ALIEN_WALK and PLAY_DEATH
//  Uses:
//    fileDescriptor, writes the sound array to play to the device file
//    Every wav_sound*, to get the sound buffer to write
void audioPlayer_play(uint32_t whatSound);
//  Purpose:
//    Frees all memory that audioPlayer_init allocated
//  Uses:
//    wav_sound* and frees the memory associated with the pointer;
void audioPlayer_exit();
