/*
 * STM32F4xx_Framework V1.0
 * @file	stm32f4xx_StdPeriphLib.h
 * @brief	Include ST Standard Peripheral Driver
 */


/*----------Define to prevent recursive inclusion-----------------------------*/
#ifndef __STM32F4XX_STDPERIPHLIB_H
#define __STM32F4XX_STDPERIPHLIB_H


/*----------Includes----------------------------------------------------------*/
#include "STM32F4xx_StdPeriph_Driver\inc\misc.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_adc.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_can.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_crc.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_cryp.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_dac.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_dbgmcu.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_dcmi.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_dma.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_exti.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_flash.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_fsmc.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_gpio.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_hash.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_i2c.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_iwdg.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_pwr.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_rcc.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_rng.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_rtc.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_sdio.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_spi.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_syscfg.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_tim.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_usart.h"
#include "STM32F4xx_StdPeriph_Driver\inc\stm32f4xx_wwdg.h"


#if(USE_FULL_ASSERT == 1)
	#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
	void assert_failed(uint8_t* file, uint32_t line);
#else
  	  #define assert_param(expr) ((void)0)
#endif


#endif
