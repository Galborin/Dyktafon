/**
  * @file    main.c
  * @date    31.05.2019
*/

/*
 * Be sure that symbol USE_USB_FS is present before build.
 */

/*includes*********************************************************************/
#include "main.h"


/*private functions prototypes*************************************************/
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
static void SystemClock_Config(void);
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);
static void Error_Handler(void);

/*global variables************************************************************/
char USBKey_Path[4] = "0:/";
USBH_HandleTypeDef hUSBHost;
FATFS USBH_FatFs;
ApplicationState_t AppState = APPLICATION_IDLE;

/*imported********************************************************************/
extern void AudioProcess(void);

int main(void) {
	/* Configure the MPU attributes for PSRAM external memory */
	MPU_Config();
	/* Enable the CPU Cache */
	CPU_CACHE_Enable();
	/* STM32F7xx HAL library initialization:
	   - Configure the Flash ART accelerator on ITCM interface
	   - Configure the Systick to generate an interrupt each 1 msec
	   - Set NVIC Group Priority to 4
	   - Global MSP (MCU Support Package) initialization
	*/
	HAL_Init();
	/*configure clock*/
	SystemClock_Config();
	/*Initialize USBH Process*/
	USBH_Init(&hUSBHost, USBH_UserProcess, 0);
	/* Add Supported Class */
	USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
	/* Start Host Process */
	USBH_Start(&hUSBHost);
	/* Initialize the LCD */
	BSP_LCD_Init();
	/* Enable the display */
	BSP_LCD_DisplayOn();
    /* Initialize the TS in IT mode if not already initialized */
	if (TouchScreen_IsCalibrationDone() == 0) {
		Touchscreen_Calibration();
	}
	BSP_TS_ITConfig();
	/* Init TS module */
  	BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  	/* Init the LCD Log module */
	LCD_LOG_Init();
	/*Display header*/
	LCD_LOG_SetHeader((uint8_t *)"D Y K T A F O N");
	/*main loop**************************************************************/
	while (1) {
		/* USB Host Background task */
		USBH_Process(&hUSBHost);
		/*Audio background task*/
		AudioProcess();
	}
}

/**
  * @brief  Configure the MPU attributes as Write Back for PSRAM mapped on FMC
  *         BANK2.
  * @note   The Base Address is 0x64000000.
  *         The Region Size is 512KB, it is related to PSRAM memory size.
  * @param  None
  * @retval None
  */
static void MPU_Config(void) {
  MPU_Region_InitTypeDef MPU_InitStruct;
  /* Disable the MPU */
  HAL_MPU_Disable();
  /* Configure the MPU attributes for SRAM1 as WT */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = SRAM1_BASE;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Configure the MPU attributes for PSRAM with recomended configurations:
     Normal memory, Shareable, write-back */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x64000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void) {
  /* Enable I-Cache */
  SCB_EnableICache();
  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if (ret != HAL_OK) {
    Error_Handler();
  }

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if (ret != HAL_OK) {
    Error_Handler();
  }

  /* Select PLLSAI output as USB clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 4;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
  ret = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  if (ret != HAL_OK) {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if (ret != HAL_OK) {
    Error_Handler();
  }
}

/**
 * callback function to USBHInit().
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id) {
	switch (id) {
	  case HOST_USER_SELECT_CONFIGURATION:
	    break;

	  case HOST_USER_DISCONNECTION:
		AppState = APPLICATION_ERROR;
		LCD_ErrLog("Host disconnection! \n");
	    LCD_ClearTextZone();
	    if (FATFS_UnLinkDriver(USBKey_Path) != 0) {
	      LCD_ErrLog("Cannot unlink FatFS driver! \n");
	    }
	    if (f_mount(NULL, "", 0) != FR_OK) {
	      LCD_ErrLog("Cannot DeInitialize FatFs! \n");
	    }
	    break;

	  case HOST_USER_CLASS_ACTIVE:
		AppState = APPLICATION_READY;
		LCD_ClearTextZone();
	    break;

	  case HOST_USER_CONNECTION:
	     /* Link the USB Mass Storage disk I/O driver */
	    if (FATFS_LinkDriver(&USBH_Driver, USBKey_Path) != 0) {
	      LCD_ErrLog("Cannot link FatFS driver! \n");
	      break;
	    }
	    if (f_mount(&USBH_FatFs, "", 0) != FR_OK) {
	      LCD_ErrLog("Cannot Initialize FatFs! \n");
	      break;
	    }
	    break;

	  default:
	    break;
	  }
}

/*
 * @brief Clears lines from 3 to 15
 */
void LCD_ClearTextZone(void) {
  uint8_t i = 0;

  for (i= 1; i < 13; i++) {
    BSP_LCD_ClearStringLine(i + 3);
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void) {
  while (1)
  {
	  //nop
  }
}
