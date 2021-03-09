#ifndef WAV_READ_H
#define WAV_READ_H
#include <stddef.h>
#include <stdint.h>


typedef struct wav_sound {
  size_t size;
  int32_t* buffer;
} wav_sound;


#define WAV_ERROR -1
#define WAV_NO_FILE -2
#define WAV_HEADER_MISSING -3
#define WAV_MALFORMED_HEADER -4
#define WAV_DATA_MISSING -5

// Purpose:
//  configure the codec chip to operate correctly
void wav_init();

// Purpose:
// Convert a RIFF formated wav file into a 24bit pcm array
// Takes:
//   Path to the wav file
// Returns converted wav file on a success
// Exits the program on a failure
wav_sound* wav_readFile(char* path);


#endif
