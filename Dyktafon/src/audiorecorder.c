/**
 * 31.05.2019
 *
 */

/*includes*******************************************************************/
#include "audiorecorder.h"



/*public variables***********************************************************/
FIL WavFile;
WaveFormat_t WaveFormat;
AudioInBuffer_t AudioBuffer;

/*private functions prototypes**********************************************/
static uint32_t WavProcess_EncInit(uint32_t Freq, uint8_t* pHeader);
static uint32_t WavProcess_HeaderInit(uint8_t* pHeader, WaveFormat_t* pWaveFormatStruct);
static uint32_t WavProcess_HeaderUpdate(uint8_t* pHeader, WaveFormat_t* pWaveFormatStruct);

/*private variables*********************************************************/
static uint8_t pHeaderBuff[44];

/*function definitions******************************************************/

int WavFile_Create(void) {
	uint32_t byteswritten = 0;

	if (f_open(&WavFile, REC_WAVE_NAME, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
		/* Initialize header file */
		WavProcess_EncInit(DEFAULT_AUDIO_IN_FREQ, pHeaderBuff);
		/* Write header file */
		if (f_write(&WavFile, pHeaderBuff, 44, (void*)&byteswritten) == FR_OK) {
			return byteswritten;
		}
	}
	return -1;
}

int WavFile_Close(void) {
	uint32_t byteswritten = 0;

	if (f_lseek(&WavFile, 0) == FR_OK) {
		/* Update the wav file header save it into wav file */
		WavProcess_HeaderUpdate(pHeaderBuff, &WaveFormat);
		if (f_write(&WavFile, pHeaderBuff, sizeof(WaveFormat_t), (void*)&byteswritten) == FR_OK) {
			/* Close file */
			f_close(&WavFile);
			return byteswritten;
		}
	}
	return -1;
}

int AudRecInit(void) {
	uint32_t byteswritten = 0;

	if ((byteswritten = WavFile_Create()) > 0) {

		BSP_AUDIO_IN_InitEx(INPUT_DEVICE_DIGITAL_MICROPHONE_2, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
		BSP_AUDIO_IN_Record((uint16_t*)&AudioBuffer.buffer, BUFFER_SIZE);
		AudioBuffer.pcm_ptr = 0;
		AudioBuffer.offset = 0;
		AudioBuffer.fileptr = byteswritten;
		AudioBuffer.state = BUFFER_EMPTY;
		return byteswritten;
	}
	return -1;
}

int AudRecProcess(void) {

	uint32_t byteswritten = 0;

	if (AudioBuffer.state == BUFFER_FULL) {

		if (f_write(&WavFile, (uint8_t*)(AudioBuffer.buffer + AudioBuffer.offset),
					BUFFER_SIZE, (void*)&byteswritten) != FR_OK) {

			return -1;
		}

		AudioBuffer.fileptr += byteswritten;
		AudioBuffer.state = BUFFER_EMPTY;
	}
	return byteswritten;
}

/*
 * @brief	Implementation of callback. Handles DMA Transfer Complete event.
 */
void BSP_AUDIO_IN_TransferComplete_CallBack(void) {
	AudioBuffer.pcm_ptr += BUFFER_SIZE/2;
	if (AudioBuffer.pcm_ptr == BUFFER_SIZE/2) {
		AudioBuffer.state   =  BUFFER_FULL;
		AudioBuffer.offset  = 0;
	}
	if (AudioBuffer.pcm_ptr >= BUFFER_SIZE) {
		AudioBuffer.state   =  BUFFER_FULL;
		AudioBuffer.offset  = BUFFER_SIZE/2;
		AudioBuffer.pcm_ptr = 0;
	}
}

/*
 * @brief	Implementation of callback. Handles DMA Half Transfer Complete event.
 */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void) {
	AudioBuffer.pcm_ptr += BUFFER_SIZE/2;
	if (AudioBuffer.pcm_ptr == BUFFER_SIZE/2) {
		AudioBuffer.state   =  BUFFER_FULL;
		AudioBuffer.offset  = 0;
	}

	if (AudioBuffer.pcm_ptr >= BUFFER_SIZE) {
		AudioBuffer.state   =  BUFFER_FULL;
		AudioBuffer.offset  = BUFFER_SIZE/2;
		AudioBuffer.pcm_ptr = 0;
	}
}

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


/**
  * @brief  Encoder initialization.
  * @param  Freq: Sampling frequency.
  * @param  pHeader: Pointer to the WAV file header to be written.
  * @retval 0 if success, !0 else.
  */
static uint32_t WavProcess_EncInit(uint32_t Freq, uint8_t* pHeader) {
	/* Initialize the encoder structure */
	WaveFormat.SampleRate = Freq;        /* Audio sampling frequency */
	WaveFormat.NbrChannels = DEFAULT_AUDIO_IN_CHANNEL_NBR;          /* Number of channels: 1:Mono or 2:Stereo */
	WaveFormat.BitPerSample = DEFAULT_AUDIO_IN_BIT_RESOLUTION;        /* Number of bits per sample (16, 24 or 32) */
	WaveFormat.FileSize = 0x001D4C00;    /* Total length of useful audio data (payload) */
	WaveFormat.SubChunk1Size = 44;       /* The file header chunk size */
	WaveFormat.ByteRate = (WaveFormat.SampleRate * \
						  (WaveFormat.BitPerSample/8) * \
						  WaveFormat.NbrChannels);     /* Number of bytes per second  (sample rate * block align)  */
	WaveFormat.BlockAlign = WaveFormat.NbrChannels * \
						    (WaveFormat.BitPerSample/8); /* channels * bits/sample / 8 */

	/* Parse the wav file header and extract required information */
	if (WavProcess_HeaderInit(pHeader, &WaveFormat)) {
		return 1;
	}
	return 0;
}

/**
  * @brief  Initialize the wave header file
  * @param  pHeader: Header Buffer to be filled
  * @param  pWaveFormatStruct: Pointer to the wave structure to be filled.
  * @retval 0 if passed, !0 if failed.
  */
static uint32_t WavProcess_HeaderInit(uint8_t* pHeader, WaveFormat_t* pWaveFormatStruct) {
	/* Write chunkID, must be 'RIFF'  ------------------------------------------*/
	pHeader[0] = 'R';
	pHeader[1] = 'I';
	pHeader[2] = 'F';
	pHeader[3] = 'F';

	/* Write the file length ---------------------------------------------------*/
	/* The sampling time: this value will be written back at the end of the
	 recording operation.  Example: 661500 Btyes = 0x000A17FC, byte[7]=0x00, byte[4]=0xFC */
	pHeader[4] = 0x00;
	pHeader[5] = 0x4C;
	pHeader[6] = 0x1D;
	pHeader[7] = 0x00;
	/* Write the file format, must be 'WAVE' -----------------------------------*/
	pHeader[8]  = 'W';
	pHeader[9]  = 'A';
	pHeader[10] = 'V';
	pHeader[11] = 'E';

	/* Write the format chunk, must be'fmt ' -----------------------------------*/
	pHeader[12]  = 'f';
	pHeader[13]  = 'm';
	pHeader[14]  = 't';
	pHeader[15]  = ' ';

	/* Write the length of the 'fmt' data, must be 0x10 ------------------------*/
	pHeader[16]  = 0x10;
	pHeader[17]  = 0x00;
	pHeader[18]  = 0x00;
	pHeader[19]  = 0x00;

	/* Write the audio format, must be 0x01 (PCM) ------------------------------*/
	pHeader[20]  = 0x01;
	pHeader[21]  = 0x00;

	/* Write the number of channels, ie. 0x01 (Mono) ---------------------------*/
	pHeader[22]  = pWaveFormatStruct->NbrChannels;
	pHeader[23]  = 0x00;

	/* Write the Sample Rate in Hz ---------------------------------------------*/
	/* Write Little Endian ie. 8000 = 0x00001F40 => byte[24]=0x40, byte[27]=0x00*/
	pHeader[24]  = (uint8_t)((pWaveFormatStruct->SampleRate & 0xFF));
	pHeader[25]  = (uint8_t)((pWaveFormatStruct->SampleRate >> 8) & 0xFF);
	pHeader[26]  = (uint8_t)((pWaveFormatStruct->SampleRate >> 16) & 0xFF);
	pHeader[27]  = (uint8_t)((pWaveFormatStruct->SampleRate >> 24) & 0xFF);

	/* Write the Byte Rate -----------------------------------------------------*/
	pHeader[28]  = (uint8_t)((pWaveFormatStruct->ByteRate & 0xFF));
	pHeader[29]  = (uint8_t)((pWaveFormatStruct->ByteRate >> 8) & 0xFF);
	pHeader[30]  = (uint8_t)((pWaveFormatStruct->ByteRate >> 16) & 0xFF);
	pHeader[31]  = (uint8_t)((pWaveFormatStruct->ByteRate >> 24) & 0xFF);

	/* Write the block alignment -----------------------------------------------*/
	pHeader[32]  = pWaveFormatStruct->BlockAlign;
	pHeader[33]  = 0x00;

	/* Write the number of bits per sample -------------------------------------*/
	pHeader[34]  = pWaveFormatStruct->BitPerSample;
	pHeader[35]  = 0x00;

	/* Write the Data chunk, must be 'data' ------------------------------------*/
	pHeader[36]  = 'd';
	pHeader[37]  = 'a';
	pHeader[38]  = 't';
	pHeader[39]  = 'a';

	/* Write the number of sample data -----------------------------------------*/
	/* This variable will be written back at the end of the recording operation */
	pHeader[40]  = 0x00;
	pHeader[41]  = 0x4C;
	pHeader[42]  = 0x1D;
	pHeader[43]  = 0x00;

	/* Return 0 if all operations are OK */
	return 0;
}

/**
  * @brief  Initialize the wave header file
  * @param  pHeader: Header Buffer to be filled
  * @param  pWaveFormatStruct: Pointer to the wave structure to be filled.
  * @retval 0 if passed, !0 if failed.
  */
static uint32_t WavProcess_HeaderUpdate(uint8_t* pHeader, WaveFormat_t* pWaveFormatStruct) {
	/* Write the file length ---------------------------------------------------*/
	/* The sampling time: this value will be written back at the end of the
	 recording operation.  Example: 661500 Btyes = 0x000A17FC, byte[7]=0x00, byte[4]=0xFC */
	pHeader[4] = (uint8_t)(AudioBuffer.fileptr);
	pHeader[5] = (uint8_t)(AudioBuffer.fileptr >> 8);
	pHeader[6] = (uint8_t)(AudioBuffer.fileptr >> 16);
	pHeader[7] = (uint8_t)(AudioBuffer.fileptr >> 24);
	/* Write the number of sample data -----------------------------------------*/
	/* This variable will be written back at the end of the recording operation */
	AudioBuffer.fileptr -= 44;
	pHeader[40] = (uint8_t)(AudioBuffer.fileptr);
	pHeader[41] = (uint8_t)(AudioBuffer.fileptr >> 8);
	pHeader[42] = (uint8_t)(AudioBuffer.fileptr >> 16);
	pHeader[43] = (uint8_t)(AudioBuffer.fileptr >> 24);

	/* Return 0 if all operations are OK */
	return 0;
}
