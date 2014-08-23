/*
 * Copyright (c) 2013-2014, Daniel Flores Tafur
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * INFO:
 * This code requires STM32F4 Standard Framework freely available from ST
 * Electronics.
 *
 * I should note that since this example uses the framework functions, it is
 * not the fastest code possible. In fact, it's possible to do the same things
 * it does a lot faster if you directly access the registers instead of using
 * the functions from the framework. However, those functions make the code
 * much more readable, reusable, easy to create and delay they result in is
 * usually insignificant. However, if you need more performance, you would
 * definitely prefer to directly access the registers rather than use the
 * framework.
 *
 * The MCU is configured to run at maximum speed (168 Mhz) in
 * stm32f4xx_StdFrameworkConfig.h, which means that timers are configured
 * assuming the maximum speed. There is more information on that in "delay.c".
 * Also data and instruction caches are enabled along with prefetch buffer.
 *
 * The first thing that gets executed is the content of stm32f4xx_Startup.c. It
 * sets the configuration defined in stm32f4xx_StdFrameworkConfig.h, such as
 * clock frequencies and then it jumps to the main function of this program.
 *
 * This example is made specifically for Mikromedia+ for STM32 ARM development
 * board, but it should be easy to fit it for another board or project.
 */

#include <stm32f4xx.h>
#include <delay.h>
#include <rgb_led.h>
#include <images.h>
#include <lcd.h>
#include <touch.h>
#include <utils.h>
#include <apps.h>
#include <ff.h>
#include <stm324xg_eval_sdio_sd.h>
#include <vs10xx_uc.h>
#include <player.h>

#define NO_SDCARD 0
#define OPEN_FILE 1

char current_directory_path[20];
char visited_directories[50][13];
char target_file[13];
uint16_t cursors[50];
uint8_t depth;
uint8_t volume;
uint8_t volume_step;
uint8_t mute;

int main(void)
{
	GPIOLED_Init();
	GPIOLCD_Init();
	Timers_Init();
	LCD_Init();

	paint_areaLCD(0, 0, 479, 271, 0xFFFF);

	write_phraseLCD("Starting touch panel test.", 26, 0, 0, 0x0000, 0xFFFF);

	GPIOTouch_Init();
	Touch_Init();

	paint_areaLCD(0, 0, 479, 271, 0xFFFF);
	configure_touch_module();

	reset_touch_fifo();

	paint_areaLCD(0, 0, 479, 271, 0xFFFF);

	uint8_t init = 0;

	current_directory_path[0] = '/';
	current_directory_path[1] = 0;
	visited_directories[0][0] = '/';
	visited_directories[0][1] = 0;
	cursors[0] = 0;
	depth = 0;

	//Initialize audio codec VS1053
	GPIOVS1053_Init();
	VSTestInitHardware();
	while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_9) != 1);
	int i = VSTestInitSoftware();

	if (!i) {
		write_phraseLCD("VS1053 initialized correctly.", 29, 0, 24, 0x0000, 0xFFFF);
	}
	else {
		write_phraseLCD("VS1053 initialization failed.", 29, 0, 24, 0x0000, 0xFFFF);
		while(1);
	}

	paint_areaLCD(0, 0, 479, 271, 0xFFFF);

	/*
	 * If you want to try additional testing routines, uncomment one of them
	 * below. You can only test one at a time.
	 */
	//test_touch_values();
	//simple_drawing();
	//test_touch_boxes();

    while(1)
    {
    	if (SDCard_present()) {
    		FATFS file_system;

    		if (!init) {
    			write_phraseLCD("SDCard detected.", 16, 0, 0, 0x0000, 0xFFFF);
    			init = 1;
    			Delay_ms(2000);

    			paint_areaLCD(0, 0, 479, 271, 0xFFFF);
    			write_phraseLCD("Now will mount the volume...", 28, 0, 0, 0x0000, 0xFFFF);

    			FRESULT result;

    			result = f_mount(&file_system, "0:", 1);

    			if (result == FR_OK) {
    				paint_areaLCD(0, 0, 479, 271, 0xFFFF);
    				while (SDCard_present()) {
    					uint8_t command = file_manager();
    					if (command == OPEN_FILE) {
    						if (check_extension(target_file, ".TXT", 4)) {
    							command = txt_viewer();
    							if (command > 0) system_message(command);
    						}
    						else {
    							if (is_it_audio(target_file)) {
    								int success = VSTestHandleFile(target_file, 0);
    								if (success == -1) system_message(5);
    							}
    							else {
    							    system_message(0);
    							}
    						}
    					}
    				}
    			}
    			else {
    				write_phraseLCD("Couldn't mount file system.", 27, 0, 24, 0x0000, 0xFFFF);
    				char s[11];
    				itoa32bits(result, s);
    				write_phraseLCD(s, 11, 0, 48, 0x0000, 0xFFFF);
    			}
    		}
    	}
    	else {
    		if (init) {
    			init = 0;
    			depth = 0;
    			cursors[0] = 0;
    			paint_areaLCD(0, 0, 479, 271, 0xFFFF);
    			write_phraseLCD("SDCard absent.", 14, 0, 0, 0x0000, 0xFFFF);
    		}
    	}
    }
}
