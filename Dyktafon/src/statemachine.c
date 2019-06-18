/**
 * 07.06.2019
 *
 */

/*includes*******************************************************************/
#include "audiorecorder.h"

/*imported*******************************************************************/
extern ApplicationState_t AppState;
extern AudioInBuffer_t AudioBuffer;

/*private function prototypes************************************************/
static void buttonStopDraw();
static void buttonPauseDraw();
static void buttonRecDraw();
static void buttonPlayDraw();
static void detectButtonTouch();

/*defines********************************************************************/
#define STOP_XMIN 105
#define STOP_XMAX 135
#define STOP_YMIN 125
#define STOP_YMAX 155

#define REC_XMIN 80
#define REC_XMAX 160
#define REC_YMIN 100
#define REC_YMAX 180

#define	PAUSE_XMIN 20
#define PAUSE_DIST 10
#define PAUSE_WDTH 10

/*public variables***********************************************************/
TS_StateTypeDef TS_State = {0};

/*function definitions*******************************************************/
void AudioProcess(void) {
	uint32_t elapsed_time;
	uint32_t prev_elapsed_time = 0xffffffff;
	uint8_t str[16];

	switch (AppState) {

	case APPLICATION_IDLE:
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		break;

	case APPLICATION_READY:

		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		buttonRecDraw();

		detectButtonTouch();

		break;

	case APPLICATION_REC_INIT:
		if (AudRecInit() < 0) {
			LCD_ErrLog("AudRecInit() fail! \n");
			AppState = APPLICATION_ERROR;
			break;
		}
		BSP_LCD_ClearStringLine(18);

		LCD_ClearTextZone();
		BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
		buttonRecDraw();
		BSP_LCD_SetTextColor(LCD_COLOR_DARKCYAN);
		buttonStopDraw();

		BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
		buttonPauseDraw();

		AppState = APPLICATION_RECORDING;
		break;

	case APPLICATION_RECORDING:
		/* Display elapsed time */
		elapsed_time = AudioBuffer.fileptr / (DEFAULT_AUDIO_IN_FREQ * DEFAULT_AUDIO_IN_CHANNEL_NBR * 2);
		if (prev_elapsed_time != elapsed_time) {
			prev_elapsed_time = elapsed_time;
			sprintf((char *)str, "[%02d:%02d]", (int)(elapsed_time /60), (int)(elapsed_time%60));
			BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
			BSP_LCD_DisplayStringAtLine(18,str);
			sprintf((char *)str, "%4d KB", (int)((int32_t)AudioBuffer.fileptr/1024));
			BSP_LCD_DisplayStringAt(83, LINE(18), str, RIGHT_MODE);
		}

		detectButtonTouch();

		if (AudRecProcess() < 0) {
			LCD_ErrLog("AudRecProcess() fail! \n");
			AppState = APPLICATION_ERROR;
		}
		break;

	case APPLICATION_REC_PAUSE:
		LCD_ClearTextZone();
		BSP_LCD_SetTextColor(LCD_COLOR_CYAN);

		buttonRecDraw();

		buttonPlayDraw();

		BSP_LCD_SetTextColor(LCD_COLOR_DARKCYAN);

		buttonStopDraw();

		BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
		BSP_LCD_DisplayStringAtLine(5,(uint8_t *)"  [PAUSE] ");
		BSP_AUDIO_IN_Pause();

		AppState = APPLICATION_PAUSE_WAIT;

		break;

	case APPLICATION_PAUSE_WAIT:

		detectButtonTouch();
		break;

	case APPLICATION_REC_RESUME:
		LCD_ClearTextZone();
		BSP_LCD_SetTextColor(LCD_COLOR_CYAN);

		buttonRecDraw();

		buttonPauseDraw();

		BSP_LCD_SetTextColor(LCD_COLOR_DARKCYAN);

		buttonStopDraw();

		BSP_LCD_ClearStringLine(5);
		BSP_AUDIO_IN_Resume();
		AppState = APPLICATION_RECORDING;
		break;

	case APPLICATION_REC_STOP:
		if (WavFile_Close() < 0) {
			LCD_ErrLog("WavFile_Close() fail! \n");
			AppState = APPLICATION_ERROR;
		}
		LCD_ClearTextZone();
		AppState = APPLICATION_READY;
		break;

	case APPLICATION_ERROR:
		LCD_ClearTextZone();
		AppState = APPLICATION_IDLE;
		break;

	default:
		AppState = APPLICATION_ERROR;
		LCD_ErrLog("Application in ERROR state! \n");
		break;
	}//switch
}

/**
 * Display stop button.
 */
static void buttonStopDraw() {
	BSP_LCD_FillRect(STOP_XMIN,  STOP_YMIN , /* STOP rectangle */
					STOP_XMAX - STOP_XMIN,
					STOP_YMAX - STOP_YMIN);
}

/**
 * Display pause button.
 */
static void buttonPauseDraw() {
	BSP_LCD_FillRect(PAUSE_XMIN, STOP_YMIN , PAUSE_WDTH, STOP_YMAX - STOP_YMIN);    /* Pause rectangles */
	BSP_LCD_FillRect(PAUSE_XMIN + PAUSE_DIST + PAUSE_WDTH, STOP_YMIN, PAUSE_WDTH, STOP_YMAX - STOP_YMIN);
}

/**
 * Display record circle
 */
static void buttonRecDraw() {
	BSP_LCD_FillCircle((REC_XMAX+REC_XMIN)/2, /* Record circle */
				       (REC_YMAX+REC_YMIN)/2,
					   (REC_XMAX-REC_XMIN)/2);
}

/**
 * Display play button.
 */
static void buttonPlayDraw() {
	Point PlaybackLogoPoints[] = {{PAUSE_XMIN, STOP_YMIN},
	                              {PAUSE_XMIN + PAUSE_DIST + (2 * PAUSE_WDTH), (STOP_YMIN + STOP_YMAX)/2},
	                              {PAUSE_XMIN, STOP_YMAX}};
	BSP_LCD_FillPolygon(PlaybackLogoPoints, 3);
}

/**
 * Check if any button was touched.
 */
static void detectButtonTouch() {
	uint8_t x,y;
	if (TS_State.touchDetected == 1) {   /* If previous touch has not been released, we don't proceed any touch command */
		BSP_TS_GetState(&TS_State);
	}
	else {
		BSP_TS_GetState(&TS_State);
		x = TouchScreen_Get_Calibrated_X(TS_State.touchX[0]);
		y = TouchScreen_Get_Calibrated_Y(TS_State.touchY[0]);
		if (TS_State.touchDetected == 1) {
			if ((x > REC_XMIN) && (x < REC_XMAX) &&
				(y > REC_YMIN) && (y < REC_YMAX)) {

				if (AppState == APPLICATION_READY) {
					AppState = APPLICATION_REC_INIT;
				}
				else {
					AppState = APPLICATION_REC_STOP;
				}
			}
			else if ((x > PAUSE_XMIN) && (x < (PAUSE_XMIN + (2 * PAUSE_WDTH) + PAUSE_DIST)) &&
					(y > REC_YMIN) && (y < REC_YMAX)) {
				if (AppState == APPLICATION_RECORDING) {
					AppState = APPLICATION_REC_PAUSE;
				}
				else if (AppState == APPLICATION_PAUSE_WAIT) {
					AppState = APPLICATION_REC_RESUME;
				}
			}
		}
	}
}
