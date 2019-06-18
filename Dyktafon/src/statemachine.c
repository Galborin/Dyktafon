/**
 * 07.06.2019
 *
 */

/*includes*******************************************************************/
#include "audiorecorder.h"

/*imported*******************************************************************/
extern ApplicationState_t AppState;

/*defines********************************************************************/
#define STOP_XMIN 80
#define STOP_XMAX 160
#define STOP_YMIN 100
#define STOP_YMAX 239

/*public variables***********************************************************/
TS_StateTypeDef TS_State = {0};

/*function definitions*******************************************************/
void AudioProcess(void) {
	uint8_t x,y;

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
			LCD_ErrLog("AudRecInit() fail! \n");
			AppState = APPLICATION_ERROR;
			break;
		}
		AppState = APPLICATION_RECORDING;
		break;

	case APPLICATION_RECORDING:

		if(TS_State.touchDetected == 1){   /* If previous touch has not been released, we don't proceed any touch command */
			BSP_TS_GetState(&TS_State);
		}
		else{
			BSP_TS_GetState(&TS_State);
			x = TouchScreen_Get_Calibrated_X(TS_State.touchX[0]);
			y = TouchScreen_Get_Calibrated_Y(TS_State.touchY[0]);
			if(TS_State.touchDetected == 1)
			{
				if ((x > STOP_XMIN) && (x < STOP_XMAX) &&
					(y > STOP_YMIN) && (y < STOP_YMAX)){

					AppState = APPLICATION_REC_STOP;
				}

			}
		}

		if(AudRecProcess() < 0){
			LCD_ErrLog("AudRecProcess() fail! \n");
			AppState = APPLICATION_ERROR;
		}
		break;

	case APPLICATION_REC_STOP:
		if(WavFile_Close() < 0){
			LCD_ErrLog("WavFile_Close() fail! \n");
			AppState = APPLICATION_ERROR;
		}
		break;
	case APPLICATION_ERROR:
		AppState = APPLICATION_IDLE;
		break;

	default:
		AppState = APPLICATION_ERROR;
		LCD_ErrLog("Application in ERROR state! \n");
		break;
	}

}
