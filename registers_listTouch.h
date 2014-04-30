/*
 * This list contains all of the registers used in STMPE610 Touch controller.
 * All registers have the exact same name as in STMPE610 datasheet register
 * summary map table.
 * Note: those names sometimes differ throughout the datasheet.
 *
 * Copyright from datasheet:
 * Copyright (c) 2011 STMicroelectronics - All right reserved
 *
 * http://www.st.com
 */

#ifndef REGISTERS_LISTTOUCH_H
#define REGISTERS_LISTTOUCH_H

#define CHIP_ID 		0x00
#define ID_VER 			0x02
#define SYS_CTRL1 		0x03
#define SYS_CTRL2 		0x04
#define SPI_CFG 		0x08
#define INT_CTRL 		0x09
#define INT_EN 			0x0A
#define INT_STA 		0x0B
#define GPIO_EN 		0x0C
#define GPIO_INT_STA 	0x0D
#define ADC_INT_EN 		0x0E
#define ADC_INT_STA 	0x0F
#define GPIO_SET_PIN 	0x10
#define GPIO_CLR_PIN 	0x11
#define GPIO_MP_STA 	0x12
#define GPIO_DIR 		0x13
#define GPIO_ED 		0x14
#define GPIO_RE 		0x15
#define GPIO_FE 		0x16
#define GPIO_AF 		0x17
#define ADC_CTRL1 		0x20
#define ADC_CTRL2 		0x21
#define ADC_CAPT 		0x22
#define ADC_DATA_CH0 	0x30
#define ADC_DATA_CH1 	0x32
#define ADC_DATA_CH4 	0x38
#define ADC_DATA_CH5 	0x3A
#define ADC_DATA_CH6 	0x3C
#define ADC_DATA_CH7 	0x3E
#define TSC_CTRL 		0x40
#define TSC_CFG 		0x41
#define WDW_TR_X 		0x42
#define WDW_TR_Y 		0x44
#define WDW_BL_X 		0x46
#define WDW_BL_Y 		0x48
#define FIFO_TH 		0x4A
#define FIFO_STA 		0x4B
#define FIFO_SIZE 		0x4C
#define TSC_DATA_X 		0x4D
#define TSC_DATA_Y 		0x4F
#define TSC_DATA_Z 		0x51
#define TSC_DATA_XYZ 	0x52
#define TSC_FRACT_XYZ 	0x56
#define TSC_DATA 		0x57
#define TSC_L_DRIVE 	0x58
#define TSC_SHIELD 		0x59

#endif /* REGISTERS_LISTTOUCH_H */
