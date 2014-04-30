/*
 * STM32F4xx_Framework V1.0
 * @file	stm32f4xx_FrameworkConfig.h
 * @brief	Configure the Framework through this defines.
 */

/*----------Define to prevent recursive inclusion-----------------------------*/
#ifndef __STM32F4XX_STDFRAMEWORKCONFIG_H
#define __STM32F4XX_STDFRAMEWORKCONFIG_H

/*
 * You can configure the Framework through following defines:
 *
 * 		HSE_VALUE				The frequency of the external oscillator.
 * 								Standard Value: 8000000
 *
 * 		HSE_STARTUP_TIMEOUT		Timeout for HSE.
 * 								Standard Value: 0x500
 *
 * 		HSI_VALUE				The frequency of the internal oscillator.
 * 								Standard Value: 16000000
 *
 * 		SET_SYS_MAX_SPEED		If set to 1 the Framework sets up the clocks to maximum speeds. The PLL is clocked by HSI.
 * 								Standard Value: 0
 * 								SYSCLK:				168MHz
 * 								HCLK / AHB: 		168MHz
 * 								PCLK1 / APB1:		42MHz
 * 								PCLK2 / APB2:		84MHz
 * 								USB, SDIO, RNG:		48MHz
 *
 * 		D_I_CACHE				If set to 1 the Framework will enable the Data- and the Instruction-Cache
 * 								Standard Value: 0
 *
 * 		PREFETCH_BUFFER			If set to 1 the Framework will enable the Prefetch-Buffer.
 * 								Note: By some Revision A devices the Prefetch-Buffer is not supported!
 * 								Standard Value: 0
 *
 * 		USE_FPU					If set to 1 the Framework will enable the FPU. Set FPU in your IDE configurations to FPU hard,
 * 								only then the compiler will generate FPU instructions.
 * 								Standard Value: 0
 *
 * 		STACK_SIZE				Configures the Stack Size (In Words).
 * 								Standard Value: 0x200 (2kB)
 * 								Changed to 0x800 (8kB)
 * 								0x1000 (16kB)
 *
 * 		VECT_TAB_OFFSET			Offset of the Vector Table.
 * 								Standard Value: 0x00
 *
 * 		VECT_TAB_SRAM			If set to 1 the Vector Table will be located in SRAM
 * 								Standard Value: 0
 *
 * 		USE_FULL_ASSERT			If set to 1 full assert is enabled (Used in ST Standard Peripheral Library).
 * 								You have to add the function void assert_failed(uint8_t* file, uint8_t line)
 * 								to handle wrong parameters in ST Standard Peripheral Library
 * 								Standard Value: 0
 */


/*----------Defines-----------------------------------------------------------*/
/* Clocks and Flash */
#define HSE_VALUE				8000000
#define HSE_STARTUP_TIMEOUT		0x500
#define HSI_VALUE				16000000
#define SET_SYS_MAX_SPEED		1
#define D_I_CACHE				1
#define PREFETCH_BUFFER			1

/* Floating Pointing Unit */
#define USE_FPU 				1

/* Stack and Vector Table */
#define STACK_SIZE				0x1000
#define VECT_TAB_OFFSET			0x00
#define VECT_TAB_SRAM			0

/* Assert for ST Standard Driver */
#define USE_FULL_ASSERT			0


#endif
