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

/*application state machine*/
typedef enum {
	APPLICATION_IDLE = 0,
	APPLICATION_READY,
	APPLICATION_REC_INIT,
	APPLICATION_RECORDING,
	APPLICATION_REC_STOP,
	APPLICATION_ERROR,
}ApplicationState_t;

#endif
