// This file was created for Lab4

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "audioPlayer.h"
#include <stdio.h>
#include "audio_adau1761.h"
#include "wav.h"
#include <stdlib.h>
#include <asm/ioctl.h>
#include <stdbool.h>
#include "globals.h"
#include "button_handler.h"
#include "gpio_switches.h"

#define AUDIO_TEST_MAP_SIZE 0x1000
#define OFFSET 0x00
#define LOAD_FAILURE -1
#define LOAD_SUCCESS 1
#define IIC_INDEX 0
#define NUM_WAVS 2
#define OPTION_2 3

#define SOUND_1_POS 1
#define SOUND_2_POS 2

#define IOCTL_MAGIC_NUMBER 'm'
#define IOCTL_START_LOOP _IO(IOCTL_MAGIC_NUMBER, 0)
#define IOCTL_STOP_LOOP _IO(IOCTL_MAGIC_NUMBER, 1)
#define SAUCER_OUT_STATE 1
#define SAUCER_NOT_OUT 0
#define NUM_ALIEN_WALK_SOUNDS 4

#define SW0 0x01

static int fileDescriptor;

static bool nonBackgroundNoise = false;



#define ALIEN_WALK_1 0
#define ALIEN_WALK_2 1
#define ALIEN_WALK_3 2
#define ALIEN_WALK_4 3

static wav_sound* alienWalk[NUM_ALIEN_WALK_SOUNDS];
static wav_sound* saucer = NULL;
static wav_sound* laserBlast = NULL;
static wav_sound* playerDeath = NULL;
static wav_sound* alienDeath = NULL;
static wav_sound* saucerDeath = NULL;

typedef enum AudioState {
  saucerGone,
  saucerOut,
  gameOver
} AudioState;
static AudioState currentState = saucerGone;


//  Purpose:
//    Setup software/hardware to enable sounds
//  Uses:
//    fileDescriptor and assigns it a value to communicate with AUDIO_ID
//  Returns:
//    Success or Failure of the task;
int audio_init()
{
  printf("Intialize!\n\r");
  fileDescriptor = open (AUDIO_ID, O_RDWR);
  if (fileDescriptor == LOAD_FAILURE)
  {
      return LOAD_FAILURE;
  }
  config_audio_pll(IIC_INDEX);
  config_audio_codec(IIC_INDEX);
  return LOAD_SUCCESS;
}

// Purpose:
//    Free every bit that audio_init claimed
//  Uses:
//    fileDescriptor, and frees it.
void audio_exit()
{
  close (fileDescriptor);
}

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
int audioPlayer_init()
{
  if (audio_init() != LOAD_SUCCESS)
  {
    return LOAD_FAILURE;
  }
  //Load the wavs
  alienWalk[ALIEN_WALK_1] = wav_readFile("Wav/walk1.wav");
  alienWalk[ALIEN_WALK_2] = wav_readFile("Wav/walk2.wav");
  alienWalk[ALIEN_WALK_3] = wav_readFile("Wav/walk3.wav");
  alienWalk[ALIEN_WALK_4] = wav_readFile("Wav/walk4.wav");
  saucer                  = wav_readFile("Wav/ufo.wav");
  laserBlast              = wav_readFile("Wav/laser.wav");
  playerDeath             = wav_readFile("Wav/player_die.wav");
  alienDeath              = wav_readFile("Wav/invader_die.wav");
  saucerDeath             = wav_readFile("Wav/ufo_die.wav");

}

//  Purpose:
//    Frees all memory that audioPlayer_init allocated
//  Uses:
//    wav_sound* and frees the memory associated with the pointer;
void audioPlayer_exit()
{
  ioctl(fileDescriptor, IOCTL_STOP_LOOP); //Stop looping the sound
  //free wavs

  free(alienWalk[ALIEN_WALK_1]->buffer);
  free(alienWalk[ALIEN_WALK_1]);
  free(alienWalk[ALIEN_WALK_2]->buffer);
  free(alienWalk[ALIEN_WALK_2]);
  free(alienWalk[ALIEN_WALK_3]->buffer);
  free(alienWalk[ALIEN_WALK_3]);
  free(alienWalk[ALIEN_WALK_4]->buffer);
  free(alienWalk[ALIEN_WALK_4]);

  free(saucer->buffer);
  free(saucer);

  free(laserBlast->buffer);
  free(laserBlast);

  free(playerDeath->buffer);
  free(playerDeath);

  free(alienDeath->buffer);
  free(alienDeath);

  free(saucerDeath->buffer);
  free(saucerDeath);

  audio_exit();
}

//  Purpose:
//    Called every time a hardware click occurs
//    Plays the saucer sound if applicable, allows alien walking sounds to be made
//    if no other sound is currently playing
//    Also increments sound volume
//  Uses:
//    currentState: to determine which actions should
void audioPlayer_tick()
{
  //Determine which tasks to perform based on the given state
  switch(currentState)
  {
    case saucerOut:
      if (global_gameOver) //Game over, stop looping sound
      {
        currentState = gameOver;
        ioctl(fileDescriptor, IOCTL_STOP_LOOP);
        printf("AudioPlayer: Transitioning to Game Over\n");
      }
      else if (!global_saucerIsOut) //The saucer is no longer out, leave saucer out state, stop looping sound
      {
        currentState = saucerGone;
        printf("Saucer is No longer out\n\r");
        ioctl(fileDescriptor, IOCTL_STOP_LOOP);
        printf("Disabled looping\n\r");
      }
      else if (!read(fileDescriptor, NULL, 0)) //No other sound is pllaying, start saucer sounds again
      {
        printf("Playing Alien Saucer Sounds\n");
        nonBackgroundNoise = false;
        audioPlayer_play(PLAY_SAUCER);
      }

      if (button3_released) //button3 used to loop through volume settings
      {
        button3_released = false; //clearinng button3 flag
        bool increment = (gpio_switch_read() & SW0);
        audioCodec_incrementSound(IIC_INDEX, !increment);
      }
    break;
    case saucerGone:
      if (!read(fileDescriptor, NULL, 0)) //No sound is playing, allow Alien Walk sounds to play
      {
          nonBackgroundNoise = false;
      }
      if (global_gameOver) //Game is over, move to gameOver state
      {
        currentState = gameOver;
        printf("AudioPlayer: Transitioning to Game Over\n");
      }
      else if (global_saucerIsOut) //saucer just came out, move to saucerOut state
      {
        printf("Saucer is now out\n\r");
        currentState = saucerOut;
        audioPlayer_play(PLAY_SAUCER);
        printf("Started playing saucer sounds\n\r");
      }

      if (button3_released) //button3 used to loop through volume settings
      {
        button3_released = false; //clearinng button3 flag
        bool increment = (gpio_switch_read() & SW0);
        audioCodec_incrementSound(IIC_INDEX, !increment);
      }
    break;
    case gameOver:
      //Do nothing while the game is Over
    break;
  }
  //In saucer out state
}
//  Purpose:
//    plays the requested sound,
//    background noises ALIEN_WALK/SAUCER cannot play over currently playing sounds
//    non-background noises will play over other sounds
//  Takes:
//    whatSound, an integer identifying which sound to play, the integer should be between PLAY_ALIEN_WALK and PLAY_DEATH
//  Uses:
//    fileDescriptor, writes the sound array to play to the device file
//    Every wav_sound*, to get the sound buffer to write
void audioPlayer_play(uint32_t whatSound)
{
  static uint32_t whichAlienWalk = 0;
  switch( whatSound) //Play the sound chosen by whatSound
  {
    case PLAY_ALIEN_WALK:
      if (nonBackgroundNoise || currentState == saucerOut) //Do not play alien walking sounds while the saucer is out, or a nonBackground noise is playing
      {
        break;
      }
      write (fileDescriptor, alienWalk[whichAlienWalk]->buffer, alienWalk[whichAlienWalk]->size);
      whichAlienWalk++;
      whichAlienWalk %= NUM_ALIEN_WALK_SOUNDS;
    break;
    case PLAY_SAUCER:
      if (nonBackgroundNoise) //do not play saucer sound while a nonBackgroundNoise is playing
      {
        break;
      }
      write (fileDescriptor, saucer->buffer, saucer->size);
      ioctl(fileDescriptor, IOCTL_START_LOOP);
    break;
    case PLAY_ALIEN_DEATH:
      nonBackgroundNoise = true;
      ioctl(fileDescriptor, IOCTL_STOP_LOOP);
      write (fileDescriptor, alienDeath->buffer, alienDeath->size);
      printf("Alien Explosion Noise\n\r");
      break;
    case PLAY_SAUCER_DEATH:
      nonBackgroundNoise = true;
      ioctl(fileDescriptor, IOCTL_STOP_LOOP);
      write (fileDescriptor, saucerDeath->buffer, saucerDeath->size);
      printf("Saucer Explosion Noise\n\r");
      break;
    case PLAY_LASER:
      nonBackgroundNoise = true;
      ioctl(fileDescriptor, IOCTL_STOP_LOOP);
      write (fileDescriptor, laserBlast->buffer, laserBlast->size);
      printf("Laser Firing Noise\n\r");
      break;
    case PLAY_DEATH:
      nonBackgroundNoise = true;
      ioctl(fileDescriptor, IOCTL_STOP_LOOP);
      write (fileDescriptor, playerDeath->buffer, playerDeath->size);
      printf("Player Explosion Noise\n\r");
      break;
    break;
    default:
      printf("audioPlayer: ERROR unknown sound %d requested\n\r", whatSound);
    break;
  }
}

//Files edited to add sound:
// saucer.c       -- added play(Saucer die), togle global_saucerIsOut);
// bullet.c       -- added play(laser)
// player.c       -- added play(player die)
// alienControl.c -- added play(alien walk)/ added play(alien die)
// globals.h/globals.c --Added gloabl_saucerIsOut;
