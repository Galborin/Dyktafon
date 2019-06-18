/**
 * 31.05.2019
 *
 */
#ifndef _AUDIORECORDER
#define _AUDIORECORDER

/*includes*******************************************************************/
#include "ff.h"
#include "stm32f723e_discovery_audio.h"
#include "main.h"

/*defines********************************************************************/
#define REC_WAVE_NAME 	"Wave.wav"
#define BUFFER_SIZE 	1024

/*format typedef*/
typedef struct {
  uint32_t ChunkID;       /* 0 */
  uint32_t FileSize;      /* 4 */
  uint32_t FileFormat;    /* 8 */
  uint32_t SubChunk1ID;   /* 12 */
  uint32_t SubChunk1Size; /* 16*/
  uint16_t AudioFormat;   /* 20 */
  uint16_t NbrChannels;   /* 22 */
  uint32_t SampleRate;    /* 24 */

  uint32_t ByteRate;      /* 28 */
  uint16_t BlockAlign;    /* 32 */
  uint16_t BitPerSample;  /* 34 */
  uint32_t SubChunk2ID;   /* 36 */
  uint32_t SubChunk2Size; /* 40 */
}WaveFormat_t;

/*buffer state typedef*/
typedef enum {
  BUFFER_EMPTY = 0,
  BUFFER_FULL,
}BufferState_t;

/*buffer structure typedef*/
typedef struct {
  uint16_t buffer[BUFFER_SIZE];
  uint32_t pcm_ptr;
  uint32_t fileptr;
  BufferState_t state;
  uint32_t offset;
}AudioInBuffer_t;

/*function declarations****************************************************/

/**
 * @brief 	Create .WAV header and write it to USB.
 * @return 	Return number of bytes written if success, otherwise -1.
 */
int WavFile_Create(void);

/**
 * @brief 	Update header and close file.
 */
int WavFile_Close(void);

/*
 * @brief	Prepare WavFile and AudioBuffer to record
 */
int AudRecInit(void);

/*
 * @brief	Recording process
 */
int AudRecProcess(void);

void BSP_AUDIO_IN_TransferComplete_CallBack(void);

void BSP_AUDIO_IN_TransferComplete_CallBack(void);

#endif
