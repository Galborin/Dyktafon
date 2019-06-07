/**
 * 07.06.2019
 *
 */

/*includes*******************************************************************/
#include "audiorecorder.h"
#include "stm32f723e_discovery_ts.h"

/*imported*******************************************************************/
extern ApplicationState_t AppState;

/*defines********************************************************************/
#define STOP_XMIN 20
#define STOP_XMAX 125
#define STOP_YMIN 100
#define STOP_YMAX 239

/*function definitions*******************************************************/
void AudioProcess(void) {

	switch(AppState){

	case APPLICATION_IDLE:
		break;

	case APPLICATION_READY:

		BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
		BSP_LCD_FillRect(STOP_XMIN,  STOP_YMIN , /* Stop rectangle */
					     STOP_XMAX - STOP_XMIN,
						 STOP_YMAX - STOP_YMIN);

		AppState = APPLICATION_REC_INIT;
		break;

	case APPLICATION_REC_INIT:
		if(AudRecInit() < 0){
			LCD_ErrLog("ERROR : AudRecInit() fail! \n");
			AppState = APPLICATION_ERROR;
			break;
		}
		AppState = APPLICATION_RECORDING;
		break;

	case APPLICATION_RECORDING:

		if(AudRecProcess() < 0){
			LCD_ErrLog("ERROR : AudRecProcess() fail! \n");
			AppState = APPLICATION_ERROR;
		}
		break;

	case APPLICATION_REC_STOP:
		if(WavFile_Close() < 0){
			LCD_ErrLog("ERROR : WavFile_Close() fail! \n");
			AppState = APPLICATION_ERROR;
		}
		break;
	case APPLICATION_ERROR:
		AppState = APPLICATION_IDLE;
		break;

	default:
		AppState = APPLICATION_ERROR;
		LCD_ErrLog("ERROR : Application in ERROR state! \n");
		break;
	}

}
