/**
 * 31.05.2019
 *
 */
#ifndef _APPFILES
#define _APPFILES

/*includes*******************************************************************/
#include "ff.h"
#include "stm32f723e_discovery_audio.h"

/*defines********************************************************************/
#define REC_WAVE_NAME "Wave.wav"

/*format structure*/
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
}WAVE_FormatTypeDef;

/*function declarations****************************************************/
/**
 * @brief Create .WAV header and write it to USB.
 * @return Return 0 if success, otherwise -1.
 */
int file_create(void);

/**
 * @brief Update header and close file.
 */
int file_close(void);


#endif
