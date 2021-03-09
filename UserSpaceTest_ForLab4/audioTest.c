// Created for Lab 4
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "audioTest.h"
#include <stdio.h>
#include "audio_adau1761.h"
#include "wav.h"
#include <stdlib.h>
#include <asm/ioctl.h>

#define AUDIO_TEST_MAP_SIZE 0x1000
#define OFFSET 0x00
#define FAILURE -1
#define SUCCESS 1
#define IIC_INDEX 0
#define NUM_WAVS 2
#define OPTION_2 3

#define SOUND_1_POS 1
#define SOUND_2_POS 2

#define IOCTL_MAGIC_NUMBER 'm'
#define IOCTL_START_LOOP _IO(IOCTL_MAGIC_NUMBER, 0)
#define IOCTL_STOP_LOOP _IO(IOCTL_MAGIC_NUMBER, 1)

static int fileDescriptor;
static char * virtualPointer;

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
  if (fileDescriptor == FAILURE)
  {
      return FAILURE;
  }
  config_audio_pll(IIC_INDEX);
  config_audio_codec(IIC_INDEX);
  return SUCCESS;
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
//    Throwaway Main File, used to prove the system is capable of printing
//  Takes:
//    At leaset 1 path to a .wav to play
//    Can use a second 1
int main(int argc, char* argv[])
{
    wav_sound* soundFile;
    wav_sound* sound2;
    char buf[30];
    if ( audio_init() == FAILURE)
    {
      printf( "ERROR: Failed to open audio file\n\r");
      return FAILURE;
    }
    wav_init();
    uint32_t result;
    printf("finished init\n\r");
    if (argc < NUM_WAVS)  // Not enough arguemnts included
    {
      printf("Usage: [wav1 path] <wav2 path>\n\r");
      return FAILURE;
    }
    soundFile = wav_readFile(argv[SOUND_1_POS]);
    if (argc >= OPTION_2)
    { //Option_2 play 2 different sounds right after eachother
      sound2 = wav_readFile(argv[SOUND_2_POS]);
    }
    else
    { //Option 1, therefore the second sound is the first sound.
      sound2 = soundFile;
    }
    printf("%x", (uint32_t) soundFile);
    printf("File Size: %d\n", soundFile->size);
    printf("\n");
    printf("Starting Write\n\r");

    printf("1 0x%x!\n\r", (uint32_t)virtualPointer);

    write(fileDescriptor, (void *) soundFile->buffer, soundFile->size);
    printf("2!\n\r");
    while (read(fileDescriptor, (void *)buf, sizeof(buf) )) {}
    ioctl(fileDescriptor, IOCTL_START_LOOP);
    write(fileDescriptor, (void *) sound2->buffer, sound2->size);

    for (uint32_t i = 0; i < 40000000; ++i)
    {
      //stub
      i--;
      i++;
    }
    ioctl(fileDescriptor, IOCTL_STOP_LOOP);

    free(soundFile->buffer);
    free(soundFile);
    if (argc >= OPTION_2)
    {
      free(sound2->buffer);
      free(sound2);
    }
    audio_exit();
    return 0;
}
