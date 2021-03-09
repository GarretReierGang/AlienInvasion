
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wav.h"
\

// Struct for rapid/easy read of .wav header
typedef struct{
    char rID[4];      // 'RIFF'
    uint32_t rLen; // make sure to read only 4 bits
    char wID[4];      // 'WAVE'
    char fId[4];      // 'fmt'
    uint32_t pcmHeaderLength;
    int16_t wFormatTag;
    int16_t numChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    int16_t numBlockAlingn;
    int16_t numBitsPerSample;
} WAV_HDR;

// Chunk header
typedef struct
{
    char dId[4];  // 'data' or 'fact'
    uint32_t dLen;
} CHUNK_HDR;

#define HEADER_STD_SIZE 4
#define HEADER_OFFSET 20
#define MAX_ATTEMPTS 10
#define FSEEK_FAIL 0
#define FREAD_SUCCESS 1

#define SAMPLE_16BIT 16
#define SAMPLE_32BIT 32
#define SAMPLE_24BIT 24
#define SAMPLE_8BIT 8


#define CONVERT_16_BIT_TO_24 8
#define SAMPLE_SIZE_16_BIT 2
#define CONVERT_8_BIT_TO_24 16
#define SAMPLE_SIZE_24_BIT 3
#define SAMPLE_SIZE_32_BIT 4
#define MOST_SIGNIFICANT_BYTE 16
#define SECOND_SIGNIFICANT_BYTE 8

#ifdef DEBUG
#define DEBUG_PRINT_LEN 100
#endif


//  Purpose:
//    convert from an 8Bit pcm buffer to a 24bit buffer
//  Takes:
//    buffer, 8bit pcm to convert
//    length, length of the 8bit pcm buffer
// Returns struct containing converted value
wav_sound* convertFrom8Bit(char* buffer, size_t length);
//  Purpose:
//    convert from an 8Bit pcm buffer to a 24bit buffer
//  Takes:
//    buffer, 8bit pcm to convert
//    length, length of the 8bit pcm buffer
// Returns struct containing converted value
wav_sound* convertFrom16Bit(char* buffer, size_t length);
//  Purpose:
//    converts the 24bit pcm buffer from a byte array into a int32_t array
//  Takes:
//    buffer, 24bit pcm to change to an int32_t array
//    length, length of the 24bit pcm buffer
// Returns struct containing converted value
wav_sound* convertFrom24Bit(char* buffer, size_t length);
//  Purpose:
//    convert from a 32Bit pcm buffer to a 24bit buffer
//  Takes:
//    buffer, 32bit pcm to convert
//    length, length of the 32bit pcm buffer
// Returns struct containing converted value
wav_sound* convertFrom32Bit(char* buffer, size_t length);
// Purpose:
//  Initialize wav_reader
//  Currently does nothing
void wav_init()
{
  //stub
}
// Purpose:
// Convert a RIFF formated wav file into a pcm array
// Takes:
//   Path to the wav file
// Returns converted wav file on a success
// Exits the program on a failure
wav_sound* wav_readFile(char* path)
{
  FILE *fp;
  WAV_HDR header;
  CHUNK_HDR chunkHeader;

  char buffer[80];
  char * sampleBuffer;
  uint32_t status;
  uint32_t n;


  fp = fopen(path, "rb"); //open and read bytes
  if (fp == NULL)
  {
    printf("WAV_READER: could not find file\n\r");
    exit(WAV_NO_FILE);
  }
  status = fread( (void *)&header, sizeof(WAV_HDR), 1, fp ); //should only have 1 header
  if (status != FREAD_SUCCESS)
  {
    fprintf(stderr, "WAV_READER: Header missing\n\r");
    exit(WAV_HEADER_MISSING); //throw error
  }

  //Double check it is a wav formated file
  for (n = 0; n < HEADER_STD_SIZE; ++n)
  {
    buffer[n] = header.rID[n];
  }
  buffer[HEADER_STD_SIZE] = '\0';
  printf("Title: %s\n", buffer);
  if (strcmp(buffer, "RIFF"))
  {
    fprintf(stderr,"WAV_READER: bad RIFF format. unsuported file type\n\r");
    exit(WAV_MALFORMED_HEADER); //NOT RIFF wav, throw error message
  }
  printf("Overal Size: %d\n", header.rLen);
  printf("Samples Per Second: %u\n", header.nSamplesPerSec);
  printf("Bytes Per Second: %u\n", header.nAvgBytesPerSec);

  uint32_t skipLen = header.pcmHeaderLength - (sizeof(WAV_HDR) - HEADER_OFFSET);
  if (FSEEK_FAIL != fseek(fp, skipLen, SEEK_CUR))
  {
    fprintf(stderr,"WAV_READER: file is only a header\n");
    exit(WAV_DATA_MISSING);
  }
  uint32_t numAttempts = 0;

  //Travel through wav until a data chunk is found
  do
  {
      status = fread((void *)&chunkHeader, sizeof(CHUNK_HDR), 1, fp);
      fprintf(stderr,"Chunk Header Size %d\n\r", sizeof(CHUNK_HDR));
      if (status != FREAD_SUCCESS)
      {
        fprintf(stderr, "WAV_READER: can't read\n");
        exit(WAV_ERROR);
      }

      //Read until a data chunk is found
      for(n = 0; n < HEADER_STD_SIZE; n++)
      {
          buffer[n] = chunkHeader.dId[n];
      }
      buffer[HEADER_STD_SIZE] = '\0';
      if (strcmp(buffer, "data") == 0)
      {
        printf("Data Found!\n");
        break;
      }
      ++numAttempts;
      status = fseek(fp, chunkHeader.dLen, SEEK_CUR);
      if(status != FSEEK_FAIL)
      {
          fprintf(stderr,"Can't seek.");
          exit(WAV_ERROR);
      }
  } while (numAttempts < MAX_ATTEMPTS);
//data chunk found, read samples now

  uint32_t sampleBufferLen = chunkHeader.dLen;
  printf("Chunk Size: %x,%d\n\r", sampleBufferLen, sampleBufferLen);
  while (!(sampleBuffer = malloc(sampleBufferLen * sizeof(char)))) //Wait for malloc to find a chunk large enough to use
  {
    //posibly add error handling if it takes long for malloc to succeed
  }
  fread((void*)sampleBuffer, sampleBufferLen, 1, fp); // dump the samples into memory
  fclose(fp);
switch (header.numBitsPerSample) //convert it to 24bit pcm format
{
  case SAMPLE_8BIT:
    return convertFrom8Bit(sampleBuffer, sampleBufferLen);
  case SAMPLE_16BIT:
    return convertFrom16Bit(sampleBuffer, sampleBufferLen);
  case SAMPLE_24BIT:
    return convertFrom24Bit(sampleBuffer, sampleBufferLen);
  case SAMPLE_32BIT:
    return convertFrom32Bit(sampleBuffer, sampleBufferLen);
}

}

//  Purpose:
//    convert from an 8Bit pcm buffer to a 24bit buffer
//  Takes:
//    buffer, 8bit pcm to convert
//    length, length of the 8bit pcm buffer
// Returns struct containing converted value
wav_sound* convertFrom8Bit(char* buffer, size_t length)
{
  wav_sound* compatibleSound;
  compatibleSound = malloc(sizeof(wav_sound));
  compatibleSound->size =  length;
  compatibleSound->buffer = malloc( compatibleSound->size * sizeof(uint32_t));
  uint32_t n;
  for (n = 0; n < compatibleSound->size; ++n)
  {
    uint32_t sample = 0 | buffer[n] << MOST_SIGNIFICANT_BYTE;
    compatibleSound->buffer[n] = sample;
  }
  free (buffer);
  return compatibleSound;
}
//  Purpose:
//    convert from a 16Bit pcm buffer to a 24bit buffer
//  Takes:
//    buffer, 16bit pcm to convert
//    length, length of the 16bit pcm buffer
// Returns struct containing converted value
wav_sound* convertFrom16Bit(char* buffer, size_t length)
{
  wav_sound* compatibleSound;
  compatibleSound = malloc(sizeof(wav_sound));
  compatibleSound->size =  length / SAMPLE_SIZE_16_BIT;
  compatibleSound->buffer = malloc( compatibleSound->size * sizeof(uint32_t));
  uint32_t n;

  #ifdef DEBUG
  uint32_t i = 0;
  #endif
  for (n = 0; n < compatibleSound->size; ++n)
  {
    uint32_t sample = 0;
    sample |= buffer [n * SAMPLE_SIZE_16_BIT ] << SECOND_SIGNIFICANT_BYTE;
    sample |= (buffer[n * SAMPLE_SIZE_16_BIT + 1] << MOST_SIGNIFICANT_BYTE);
    compatibleSound->buffer[n] = sample;
  #ifdef DEBUG
    if (i < DEBUG_PRINT_LEN || i > (compatibleSound->size - DEBUG_PRINT_LEN))
    {
      printf("%x\n\r", sample);
    }
    if (i == DEBUG_PRINT_LEN || i == (compatibleSound->size - DEBUG_PRINT_LEN))
      printf("------End of Debug Print------\n\r");
    ++i;
  #endif
  }
  free (buffer);
  return compatibleSound;
}
//  Purpose:
//    converts the 24bit pcm buffer from a byte array into a int32_t array
//  Takes:
//    buffer, 24bit pcm to change to an int32_t array
//    length, length of the 24bit pcm buffer
// Returns struct containing converted value
wav_sound* convertFrom24Bit(char* buffer, size_t length)
{
    wav_sound* compatibleSound;
    compatibleSound = malloc(sizeof(wav_sound));
    compatibleSound->size =  length / SAMPLE_SIZE_24_BIT;
    compatibleSound->buffer = malloc( compatibleSound->size * sizeof(uint32_t));
    printf("Num Samples: %d\n\r", compatibleSound->size);
    uint32_t n;
    #ifdef DEBUG
    uint32_t i = 0;
    #endif
    for (n = 0; n < compatibleSound->size; ++n)
    {
      uint32_t sample = 0;
      sample |=  buffer[n*SAMPLE_SIZE_24_BIT];
      sample |=  buffer[n*SAMPLE_SIZE_24_BIT + 1] << SECOND_SIGNIFICANT_BYTE;
      sample |= (buffer[n*SAMPLE_SIZE_24_BIT + 2] << MOST_SIGNIFICANT_BYTE);
      compatibleSound->buffer[n] = sample;
    #ifdef DEBUG
      if (i < DEBUG_PRINT_LEN || i > (compatibleSound->size - DEBUG_PRINT_LEN))
      {
        printf("%x\n\r", sample);
      }
      if (i == DEBUG_PRINT_LEN || i == (compatibleSound->size-1))
        printf("------End of Debug Print------\n\r");
      ++i;
    #endif
    }
    printf ("\n\r");
    free (buffer);
    return compatibleSound;
}
//  Purpose:
//    convert from a 32Bit pcm buffer to a 24bit buffer
//  Takes:
//    buffer, 32bit pcm to convert
//    length, length of the 32bit pcm buffer
// Returns struct containing converted value
wav_sound* convertFrom32Bit(char* buffer, size_t length)
{
  wav_sound* compatibleSound;
  compatibleSound = malloc(sizeof(wav_sound));
  compatibleSound->size =  length / SAMPLE_SIZE_32_BIT;
  compatibleSound->buffer = malloc( compatibleSound->size * sizeof(uint32_t));
  uint32_t n;
  for (n = 0; n < compatibleSound->size; ++n)
  {
    uint32_t sample = 0;
    // buffer[n*SAMPLE_SIZE_32_BIT] contains high fidelity bits, ignored to convert to 24bit pcm
    sample |=  buffer[n*SAMPLE_SIZE_32_BIT + 1];
    sample |= (buffer[n*SAMPLE_SIZE_32_BIT + 2] << SECOND_SIGNIFICANT_BYTE);
    sample |= (buffer[n*SAMPLE_SIZE_32_BIT + 3] << MOST_SIGNIFICANT_BYTE);
    compatibleSound->buffer[n] = sample;
  }
  free (buffer);
  return compatibleSound;
}
