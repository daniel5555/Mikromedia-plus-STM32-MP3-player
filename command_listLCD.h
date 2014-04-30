/*
 * This list contains all of the commands for SSD1963 LCD controller. All
 * commands have the exact same name as in SSD1963 datasheet.
 *
 * Copyright from datasheet: Copyright (c) Solomon Systech Limited
 * http://www.solomon-systech.com
 */

#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

#define NOP 						0x0000
#define SOFT_RESET 					0x0001
#define GET_POWER_MODE 				0x000A
#define GET_ADDRESS_MODE 			0x000B
#define RESERVED0 					0x000C
#define GET_DISPLAY_MODE 			0x000D
#define GET_TEAR_EFFECT_STATUS 		0x000E
#define RESERVED1 					0x000D
#define ENTER_SLEEP_MODE 			0x0010
#define EXIT_SLEEP_MODE 			0x0011
#define ENTER_PARTIAL_MODE 			0x0012
#define ENTER_NORMAL_MODE 			0x0013
#define EXIT_INVERT_MODE 			0x0020
#define ENTER_INVERT_MODE 			0x0021
#define SET_GAMMA_CURVE 			0x0026
#define SET_DISPLAY_OFF 			0x0028
#define SET_DISPLAY_ON 				0x0029
#define SET_COLUMN_ADDRESS 			0x002A
#define SET_PAGE_ADDRESS 			0x002B
#define WRITE_MEMORY_START 			0x002C
#define READ_MEMORY_START 			0x002E
#define SET_PARTIAL_AREA 			0x0030
#define SET_SCROLL_AREA 			0x0033
#define SET_TEAR_OFF 				0x0034
#define SET_TEAR_ON 				0x0035
#define SET_ADDRESS_MODE 			0x0036
#define SET_SCROLL_START 			0x0037
#define EXIT_IDLE_MODE 				0x0038
#define ENTER_IDLE_MODE 			0x0039
#define RESERVED2 					0x003A
#define WRITE_MEMORY_CONTINUE 		0x003C
#define READ_MEMORY_CONTINUE 		0x003E
#define SET_TEAR_SCANLINE 			0x0044
#define GET_SCANLINE 				0x0045
#define READ_DDB 					0x00A1
#define RESERVED3 					0x00A8
#define SET_LCD_MODE 				0x00B0
#define GET_LCD_MODE 				0x00B1
#define SET_HORI_PERIOD 			0x00B4
#define GET_HORI_PERIOD 			0x00B5
#define SET_VERT_PERIOD 			0x00B6
#define GET_VERT_PERIOD 			0x00B7
#define SET_GPIO_CONF 				0x00B8
#define GET_GPIO_CONF 				0x00B9
#define SET_GPIO_VALUE 				0x00BA
#define GET_GPIO_STATUS 			0x00BB
#define SET_POST_PROC 				0x00BC
#define GET_POST_PROC 				0x00BD
#define SET_PWM_CONF 				0x00BE
#define GET_PWM_CONF 				0x00BF
#define SET_LCD_GEN0 				0x00C0
#define GET_LCD_GEN0 				0x00C1
#define SET_LCD_GEN1 				0x00C2
#define GET_LCD_GEN1 				0x00C3
#define SET_LCD_GEN2 				0x00C4
#define GET_LCD_GEN2 				0x00C5
#define SET_LCD_GEN3 				0x00C6
#define GET_LCD_GEN3 				0x00C7
#define SET_GPIO0_ROP 				0x00C8
#define GET_GPIO0_ROP 				0x00C9
#define SET_GPIO1_ROP 				0x00CA
#define GET_GPIO1_ROP 				0x00CB
#define SET_GPIO2_ROP 				0x00CC
#define GET_GPIO2_ROP 				0x00CD
#define SET_GPIO3_ROP 				0x00CE
#define GET_GPIO3_ROP				0x00CF
#define SET_DBC_CONF 				0x00D0
#define GET_DBC_CONF 				0x00D1
#define SET_DBC_TH 					0x00D4
#define GET_DBC_TH 					0x00D5
#define SET_PLL 					0x00E0
#define SET_PLL_MN 					0x00E2
#define GET_PLL_MN 					0x00E3
#define GET_PLL_STATUS 				0x00E4
#define SET_DEEP_SLEEP 				0x00E5
#define SET_LSHIFT_FREQ 			0x00E6
#define GET_LSHIFT_FREQ 			0x00E7
#define RESERVED4 					0x00E8
#define RESERVED5 					0x00E9
#define SET_PIXEL_DATA_INTERFACE 	0x00F0
#define GET_PIXEL_DATA_INTERFACE 	0x00F1
#define RESERVED6 					0x00FF

#endif /* COMMAND_LIST_H */
