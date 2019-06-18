/**
  * @file    main.h
  * @date    31.05.2019
*/
#ifndef _MAINH
#define _MAINH

/*includes*********************************************************************/
#include "stm32f7xx.h"
#include "stm32f723e_discovery.h"
#include "stdio.h"
#include "usbh_core.h"
#include "usbh_diskio_dma.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "lcd_log.h"
#include "stm32f723e_discovery_ts.h"

/*application state machine*/
typedef enum {
	APPLICATION_IDLE = 0,
	APPLICATION_READY,
	APPLICATION_REC_INIT,
	APPLICATION_RECORDING,
	APPLICATION_REC_PAUSE,
	APPLICATION_PAUSE_WAIT,
	APPLICATION_REC_RESUME,
	APPLICATION_REC_STOP,
	APPLICATION_ERROR,
}ApplicationState_t;

/*functions to manage touchscreen**********************************************/
uint8_t  Touchscreen_Calibration (void);
uint16_t TouchScreen_Get_Calibrated_X(uint16_t x);
uint16_t TouchScreen_Get_Calibrated_Y(uint16_t y);
uint8_t  TouchScreen_IsCalibrationDone(void);
void LCD_ClearTextZone(void);

#endif
