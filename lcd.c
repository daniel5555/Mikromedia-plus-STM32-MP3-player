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
 * This file contains all functions that display information on LCD screen.
 * They are tailored to screen used in Mikromedia+ for STM32 board, but it
 * should be easy to adapt them to other boards or projects. I tried to make
 * them generic enough, so the changes would be minimal.
 * The screen used in Mikromedia+ board is LB04302 produced by SunBond. Its
 * resolution is 480 x 272 and it is controlled by SSD1963 controller
 * manufactured by Solomon Systech. This code contains the initializing
 * sequence for that controller and screen.
 *
 * Some notes on SSD1963 usage
 * Since the documentation SSD1963 is pretty poor, it can be really unclear how
 * actually use it, so I decided to write a very small tutorial on it. It is
 * only about 8800 mode though, but things are similar for 6800 mode.
 *
 * Basically you have a bunch of control pins and data pins. Control pins serve
 * to indicate to SSD1963 what you actually want from it, like if you want to
 * read data from it, to send some data to it, to issue some command, etc. Data
 * pins, as the name suggests, serves to send data (parameters and pixel color)
 * and commands. Each command has an associated number to it, so this number
 * has to be sent to SSD1963 just like any other data to it, even though you
 * must indicate that it is actually a command via D/C control pin.
 *
 * What follows is a description of control pins...
 * CS pin serves to indicate to SSD1963 if we want some attention from it. If
 * it's up (1) then SSD1963 will ignore everything else. So, to perform any
 * action it is mandatory to set this pin low (0) before. You can simply set it
 * low once at the very beginning and forget about it. Its purpose is for the
 * case when you have more than one device connected to the same pins as
 * SSD1963, so you can select between devices using it.
 *
 * RST pin serves to perform hardware reset. When it is low (0) it means that
 * you are performing the hardware reset. When it is high (1) there is no reset
 * and SSD1963 is just functioning normally. This may seem counterintuitive.
 * When you perform a reset, you must wait at least 100 ms before setting the
 * pin high again. Hardware reset gets rid of all the settings, but the
 * contents of framebuffer are not erased.
 *
 * D/C pin serves to select whether the current data you're writing is data or
 * command. Setting it to low (0) means command and setting it to high (1)
 * means data. When you read the data its value doesn't matter.
 *
 * RD pin and WR pin serve to indicate, respectively, whether you want to read
 * data from SSD1963, or you want to write data into it. However their working
 * is a bit tricky. When any of those pins goes low (0) it means that SSD1963
 * will expect that you want to either read or write data depending on which
 * pin is low. When you finished reading or writing data, the pin should go
 * high (1) and it will mean that your MCU either finished obtaining data from
 * SSD1963, or that your MCU established some data or command and SSD1963
 * should immediately start reading it. This means, among other things, that RD
 * and WR should never be low together at the same time. The initial state of
 * those pins should be, assuming that you want to write data initially, the
 * following: RD pin high and WR pin doesn't matter, it can be both low or
 * high. Then you can start writing some data and when you finish writing it
 * and ensure that the output is stable (1 instruction delay is usually enough)
 * you should set WR pin high. If it was already high, then you should simply
 * put it to low, wait for 1 instruction to ensure that it's stable (better to
 * combine both WR and output port stabilization time) and then set it high.
 *
 * So, in essence, the sequence for writing any data looks like this:
 *
 * (RD pin high the whole time)
 * 1. write the data into output port
 * 2. select if it's command or data
 * (you can change the order of steps 1 and 2)
 * 3. WR = 0
 * 4. wait 1 inst
 * 5. WR = 1
 *
 * After that the data you wrote to the output port is fetched into SSD1963.
 *
 * Reading works the same way for RD pin instead of WR. WR pin must be high the
 * whole time while you read the data, of course.
 *
 * This is all for control pins for SSD1963. There are also some input pins
 * that transfer additional information to MCU, those are the tearing effect
 * pin and GPIO0 pin, but I find that the SSD1963 datasheet explains them well,
 * so I won't talk about them.
 */

#include <lcd.h>
#include <delay.h>
#include <ascii.h>

#define COMMAND 0
#define DATA 1

#define Set_InfoTypeToSend(TYPE) 	GPIO_WriteBit(GPIOF, GPIO_Pin_15, TYPE)
#define Set_WRbit_low() 			GPIO_WriteBit(GPIOF, GPIO_Pin_11, 0)
#define Set_WRbit_high() 			GPIO_WriteBit(GPIOF, GPIO_Pin_11, 1)
#define Set_RDbit_low() 			GPIO_WriteBit(GPIOF, GPIO_Pin_12, 0)
#define Set_RDbit_high() 			GPIO_WriteBit(GPIOF, GPIO_Pin_12, 1)
#define Set_CSbit_low() 			GPIO_WriteBit(GPIOF, GPIO_Pin_13, 0)
#define Set_CSbit_high() 			GPIO_WriteBit(GPIOF, GPIO_Pin_13, 1)
#define Send_hardware_reset() 		GPIO_WriteBit(GPIOF, GPIO_Pin_14, 0)
#define Stop_hardware_reset() 		GPIO_WriteBit(GPIOF, GPIO_Pin_14, 1)

/*
 * This function initializes the GPIO pins used to communicate with SSD1963
 * controller. Watch out for other initializations as they may affect this one.
 */
void GPIOLCD_Init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	/*
	 * We don't need the GPIOG clock initialization above if GPIOLED_Init() was
	 * executed before.
	 */

	/*
	 * Initializing commands pins, they all are output. Those pins are:
	 * WR 		-> GPIOF-Pin 11
	 * RD 		-> GPIOF-Pin 12
	 * CS 		-> GPIOF-Pin 13
	 * RESET 	-> GPIOF-Pin 14
	 * D/C 		-> GPIOF-Pin 15
	 *
	 * Note: D/C sometimes is called "RS" in some documents/examples.
	 */
	GPIO_InitTypeDef GPIOLCD_settings;
	GPIOLCD_settings.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13
											| GPIO_Pin_14 | GPIO_Pin_15;
	GPIOLCD_settings.GPIO_Mode = GPIO_Mode_OUT;
	GPIOLCD_settings.GPIO_Speed = GPIO_Speed_100MHz;
	GPIOLCD_settings.GPIO_OType = GPIO_OType_PP;
	GPIOLCD_settings.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(GPIOF, &GPIOLCD_settings);

	/*
	 * Initializing higher data pins, they can be input or output depending on
	 * command. By default we are configuring them as output, since initially
	 * what we want the most is writing information rather than obtaining it.
	 * Those pins are:
	 * D8 	-> GPIOE-Pin 8
	 * D9 	-> GPIOE-Pin 9
	 * D10 	-> GPIOE-Pin 10
	 * D11 	-> GPIOE-Pin 11
	 * D12 	-> GPIOE-Pin 12
	 * D13 	-> GPIOE-Pin 13
	 * D14 	-> GPIOE-Pin 14
	 * D15 	-> GPIOE-Pin 15
	 *
	 * There is also GPIO0 pin from LCD connected to GPIOE-Pin 7, we are not
	 * using it for now.
	 */

	GPIOLCD_settings.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
								| GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13
								| GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOE, &GPIOLCD_settings);

	/*
	 * Initializing lower data pins, everything said about higher data pins
	 * applies here as well. Those pins are:
	 * D0 	-> GPIOG-Pin 0
	 * D1 	-> GPIOG-Pin 1
	 * D2 	-> GPIOG-Pin 2
	 * D3 	-> GPIOG-Pin 3
	 * D4 	-> GPIOG-Pin 4
	 * D5 	-> GPIOG-Pin 5
	 * D6 	-> GPIOG-Pin 6
	 * D7 	-> GPIOG-Pin 7
	 */

	GPIOLCD_settings.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2
								| GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5
								| GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8
								| GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11
								| GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;

	GPIO_Init(GPIOG, &GPIOLCD_settings);

	/*
	 * Initializing input pin for Tearing Effect signal from SSD1963. This
	 * signal is always input. Currently it's not used.
	 */

	GPIOLCD_settings.GPIO_Pin = GPIO_Pin_4;
	GPIOLCD_settings.GPIO_Mode = GPIO_Mode_IN;

	GPIO_Init(GPIOD, &GPIOLCD_settings);
}

/*
 * This function sends a command to SSD1963. You can find the full list of
 * commands in command_listLCD.h. Check out the SSD1963 datasheet for more
 * information on each particular command.
 */
void issue_commandLCD(uint16_t command) {
	uint16_t temp = 0xFF00 & ((uint16_t)GPIOG->ODR);
	temp = temp | command;
	GPIOG->ODR = temp;
	Set_InfoTypeToSend(COMMAND);
	Set_WRbit_low();
	Delay_1inst();
	Set_WRbit_high();
}

/*
 * This function sends data to SSD1963 controller. The can be send only after a
 * specific command was issued. Some commands have a list of parameters that
 * must be sent to them after issuing them. The WRITE_MEMORY_START and
 * WRITE_MEMORY_CONTINUE are special commands, because an indefinite number of
 * parameters, each representing a pixel color, can be sent after them.
 */
void write_dataLCD(uint16_t data) {
	uint16_t temp = 0xFF00 & ((uint16_t)GPIOG->ODR);
	temp = temp | data;
	GPIOG->ODR = temp;
	Set_InfoTypeToSend(DATA);
	Set_WRbit_low();
	Delay_1inst();
	Set_WRbit_high();
}

/*
 * This function combines in one the two function from above. The two functions
 * from above are essentially the same in everything, but in type of data they
 * send. You may use this function instead of those two function if you want to
 * save program memory at expense of losing a tiny amount of performance.
 */
/*void write_LCD(uint16_t value, BitAction type) {
	uint16_t temp = 0xFF00 & ((uint16_t)GPIOG->ODR);
	temp = temp | value;
	GPIOG->ODR = temp;
	Set_InfoTypeToSend(type);
	Set_WRbit_low();
	//Delay_us(1);
	Delay_1inst();
	Set_WRbit_high();
}*/

/*
 * This is the generic sequence for initializing SSD1963 controller. The
 * parameters depend on screen and the ones used here are specifically for
 * LB04302 by SunBond. If you have different screen, you should change those
 * parameters accordingly. Usually you can obtain all those parameters from
 * screen datasheet.
 * Given that the documentation on SSD1963 is pretty poor, here is the
 * alternative description of everything necessary in order to initialize the
 * controller:
 *
 * 1.	Perform hardware reset. Most likely, this is optional, but I perform it
 *   	it anyway just to show how to do it.
 * 2.	Perform software reset. Most likely, it is optional as well. You only
 *    	need to issue the reset command and wait 5 ms after it.
 * 3.	Configure the PLL clock of SSD1963. This is a very important operation,
 *   	because it defines the performance, for example, how fast it will read
 *   	commands and data. Here I set to maximum operating speed (120 Mhz). It
 *   	should be noted that before it is set (and it doesn't happen at this
 *   	step) the controller operates slowly, so I ensure a 1 us delay after
 *   	sending it any data.
 * 4.	Enable the PLL clock. After this the configuration defined in previous
 *   	step will actually take effect. This step is tricky, because it requires
 *   	issuing the same command (SET_PLL) two times with slightly different
 *   	parameters. This is due to particularities of SSD1963. Please, check the
 *   	SSD1963 datasheet for more information.
 * 5.	Perform software reset. Once again, this is, probably, optional. Please
 *   	note that the software reset does not affect the PLL clock configuration,
 *   	but it deletes everything else. The hardware reset deletes even the PPL
 *   	clock configuration.
 * 6.	Set the LCD mode, which is basically tell the controller about LCD
 *   	characteristics such is internal data width and screen resolution.
 *   	Internal data width normally corresponds to how many colors the screen is
 *   	able to display.
 * 7.	Set the interface width used to communicate pixel data to SSD1963. It
 *   	supports several modes, including 24-bit mode. Here, for simplicity, we
 *   	just use 16-bit mode, because it allows us to send one pixel data in one
 *   	transfer.
 * 8.	Set pixel clock frequency. This parameter should definitely be in screen
 *   	datasheet. It is not a single value, but rather a range of acceptable
 *   	values. You should set it somewhere near the typical value.
 * 9.	Set horizontal clock period. You need to specify a bunch of parameters,
 * 		such as period time, back porch and sync pulse width. There are some
 * 		more, so you should check the SSD1963 datasheet. All of those
 * 		parameters should be in display datasheet, but they may have different
 * 		names. Also back porch is sometimes called "non-display period" in
 * 		SSD1963 datasheet. As in case of previous step, those parameters
 * 		normally aren't a single value, but	rather a range of acceptable
 * 		values.The other parameters (horizontal sync pulse and subpixel start
 * 		position) should be equal to 0 in most cases.
 * 10.	Set vertical clock period. It contains some more parameters that you
 * 		should get from your display datasheet. The names from SSD1963
 * 		datasheet are confusing, but basically they ask you, once again, for
 * 		period time, back porch, pulse width and sync start location. Those
 * 		parameters should be indicated in "vsync" section of your display
 * 		datasheet. The sync start location is usually equal to 0. Other
 * 		parameters, once again, usually can be in a range of acceptable values.
 * 11.	Set the landscape orientation. Basically it depends on initial screen
 * 		rotation in your project. The orientation itself is defined by
 * 		specifying the address mode which will define in which order the data
 * 		is displayed and lenght of row. On a normally designed board the
 * 		default screen rotation should correspond with 0x0000 mode.
 * 		After this step the SSD1963 is actually set, so the initialization
 * 		procedure is formally finished, however we also turn on the screen and
 * 		backlight before finishing.
 * 12.	Turn on the screen. After this step it will start to display the
 * 		contents of frame buffer.
 * 13.	Turn on the backlight. After this step the user will actually be able
 * 		to clearly see what is on screen.
 *
 * 		If the screen is initialized properly, after this function the
 * 		backlight should turn on and user will see random content similar to TV
 * 		screen being tuned to no channel (white noise). If you want to get rid
 * 		of this random content being displayed at all, you should fill the
 * 		buffer with something you want to show and only then turn on the screen
 * 		and backlight (perform the steps 12 and 13).
 */

void LCD_Init() {
	Set_RDbit_high(); //RD high, can't be low at the same time with WR
	Set_CSbit_low(); //CS low

	// 1. Hardware reset
	Send_hardware_reset();
	Delay_ms(100);
	Stop_hardware_reset();
	Delay_ms(100);

	// 2. Software reset.
	issue_commandLCD(SOFT_RESET);
	Delay_ms(5);

	// 3. Configure the PLL clock.
	issue_commandLCD(SET_PLL_MN);
	Delay_us(1);
	write_dataLCD(0x0023);
	Delay_us(1);
	write_dataLCD(0x0002);
	Delay_us(1);
	write_dataLCD(0x0004);
	Delay_us(1);

	// 4. Enable the PLL clock.
	issue_commandLCD(SET_PLL);
	Delay_us(1);
	write_dataLCD(0x0001);
	Delay_us(100);
	issue_commandLCD(SET_PLL);
	Delay_us(1);
	write_dataLCD(0x0003);
	Delay_us(1);

	//5. Software reset.
	issue_commandLCD(SOFT_RESET);
	Delay_ms(5);

	// 6. Set the LCD mode.
	issue_commandLCD(SET_LCD_MODE);
	write_dataLCD(0x0028);
	write_dataLCD(0x0020);
	write_dataLCD(0x0001);
	write_dataLCD(0x00DF);
	write_dataLCD(0x0001);
	write_dataLCD(0x000F);
	write_dataLCD(0x0000);

	// 7. Set the interface width.
	issue_commandLCD(SET_PIXEL_DATA_INTERFACE);
	write_dataLCD(0x0003);

	/*issue_commandLCD(0x003A);
	Delay_ms(1);
	write_dataLCD(0x0060);
	Delay_ms(1);*/

	// 8. Set the pixel clock frequency.
	issue_commandLCD(SET_LSHIFT_FREQ);
	write_dataLCD(0x0001);
	write_dataLCD(0x0045);
	write_dataLCD(0x0047);

	// 9. Set horizontal clock period.
	issue_commandLCD(SET_HORI_PERIOD);
	write_dataLCD(0x0002);
	write_dataLCD(0x000D);
	write_dataLCD(0x0000);
	write_dataLCD(0x002B);
	write_dataLCD(0x0028);
	write_dataLCD(0x0000);
	write_dataLCD(0x0000);
	write_dataLCD(0x0000);

	// 10. Set vertical clock period.
	issue_commandLCD(SET_VERT_PERIOD);
	write_dataLCD(0x0001);
	write_dataLCD(0x001D);
	write_dataLCD(0x0000);
	write_dataLCD(0x000C);
	write_dataLCD(0x0009);
	write_dataLCD(0x0000);
	write_dataLCD(0x0000);

	// 11. Set the landscape orientation.
	issue_commandLCD(SET_ADDRESS_MODE);
	write_dataLCD(0x0000);

	// 12. Turn on the LCD.
	issue_commandLCD(SET_DISPLAY_ON);

	// 13. Turn on the backlight.
	issue_commandLCD(SET_PWM_CONF);
	write_dataLCD(0x0006);
	write_dataLCD(0x00FF);
	write_dataLCD(0x0001);
	write_dataLCD(0x00FF);
	write_dataLCD(0x0000);
	write_dataLCD(0x0001);
}

/*
 * This function defines an area where the following pixels will be written.
 * The values of x and y represent the first pixel to be written, while x_end
 * and y_end represent the last one to be written. Therefore x column and y row
 * will be the first to be written to and x_end column and y_end row will be
 * the last ones. This behavior may be changed by SET_ADDRESS_MODE command. For
 * example, it can be reversed. Also notice that in SSD1963 datasheet they use
 * the term "page" instead of "row".
 * After defining the area you need to issue WRITE_MEMORY_START command before
 * sending any pixel, otherwise they either will be ignored and lost, or sent
 * to the wrong area.
 */
void define_paint_areaLCD(uint16_t x, uint16_t y, uint16_t x_end,
					uint16_t y_end) {
	issue_commandLCD(SET_COLUMN_ADDRESS);
	write_dataLCD((x >> 8) & 0x00FF);
	write_dataLCD(x & 0x00FF);
	write_dataLCD((x_end >> 8) & 0x00FF);
	write_dataLCD(x_end & 0x00FF);

	issue_commandLCD(SET_PAGE_ADDRESS);
	write_dataLCD((y >> 8) & 0x00FF);
	write_dataLCD(y & 0x00FF);
	write_dataLCD((y_end >> 8) & 0x00FF);
	write_dataLCD(y_end & 0x00FF);
}

/*
 * This function serves for specifying the color of particular (next to write)
 * pixel. It writes the specified color and SSD1963 automatically will increase
 * its internal pixel counter to next pixel in a row. If it was the last pixel,
 * it will pass to the first pixel in the next row. If it was the last pixel in
 * the area, the following pixels will be ignored and lost until the pixel
 * counter will be reset by WRITE_MEMORY_START command.
 */
void write_pixelLCD(uint16_t pixel) {
	uint16_t temp = 0xFF00 & ((uint16_t)GPIOG->ODR);
	temp = temp | (pixel & 0x00FF);
	GPIOG->ODR = temp;
	temp = 0x00FF & ((uint16_t)GPIOE->ODR);
	temp = temp | (pixel & 0xFF00);
	GPIOE->ODR = temp;
	Set_InfoTypeToSend(DATA);
	Set_WRbit_low();
	Delay_1inst();
	Set_WRbit_high();
}

/*
 * Paint the desired rectangular area with a specified color. It's pretty
 * self-explanatory. It should be used to paint the whole screen with a single
 * color, to paint rectangular areas and also to clear the written text with a
 * background color. The parameters x_end and y_end define the last pixel that
 * is painted.
 */
void paint_areaLCD(uint16_t x, uint16_t y, uint16_t x_end, uint16_t y_end,
					uint16_t color) {
	int i;

	define_paint_areaLCD(x, y, x_end, y_end);
	issue_commandLCD(WRITE_MEMORY_START);

	for(i = 0; i < ((x_end - x + 1)*(y_end - y + 1)); ++i) {
		write_pixelLCD(color);
	}
}

/*
 * Get letter's length. It is necessary to use it to calculate something, for
 * example, whether a given output can be displayed on screen prior to actually
 * trying to display it on screen.
 */
uint16_t get_letter_length(char letter) {
	uint32_t address = (uint32_t)letter - 32;
	address = (uint32_t)ascii_table[address];
	++address;
	return (uint16_t)bitmap[address];
}

/*
 * This function will write a character from_ascii table on specified position
 * on display. It redefines the writing area, so after using this function it
 * must be redefined again before sending any new pixel, or MEMORY_WRITE_START
 * command must be issued if you want to overwrite the letter. The values of x
 * and y define the first pixel that constitutes a character. The returned
 * parameter x_end allows you easily know how many columns were advanced in
 * process of writing a letter, so you can easily continue to write starting
 * that position (remember to increase it by one though, if you don't want to
 * affect the written character).
 * Your character can go beyond the borders of the screen without causing any
 * problem. This is not very efficient because it wastes time unnecessarily,
 * but it can be done if you intend to display incomplete character.
 */
uint16_t write_letterLCD(char letter, uint16_t x, uint16_t y,
				uint16_t letter_color, uint16_t background_color) {
	uint32_t address = (uint32_t)letter - 32;
	address = (uint32_t)ascii_table[address];
	uint16_t offset = (uint16_t)bitmap[address];
	++address;
	uint16_t length = (uint16_t)bitmap[address];
	++address;
	uint16_t height = (uint16_t)bitmap[address];
	++address;
	uint16_t x_end = x + length - 1;
	uint16_t y_end = y + 24 - 1;

	define_paint_areaLCD(x, y, x_end, y_end);

	uint16_t i;
	uint16_t j;

	issue_commandLCD(WRITE_MEMORY_START);

	for(i = 0; i < offset; ++i) {
		for(j = 0; j < length; ++j) {
			write_pixelLCD(background_color);
		}
	}

	uint8_t shift = 31;
	uint32_t bit;

	for(i = 0; i < height; ++i) {
		for(j = 0; j < length; ++j) {
			bit = (bitmap[address] >> shift) & 0x00000001;
			if (bit) {
				write_pixelLCD(letter_color);
			}
			else {
				write_pixelLCD(background_color);
			}
			--shift;
		}
		++address;
		shift = 31;
	}

	for (i = 0; i < 24 - height - offset; ++i) {
		for(j = 0; j < length; ++j) {
			write_pixelLCD(background_color);
		}
	}

	return x_end;
}

/*
 * This function will write a whole phrase on the specified position on
 * display. The parameters x and y define the first pixel of the first
 * character of phrase to be written. Your phrase can go beyond the borders of
 * screen without causing any problem, but that is inefficient, because it
 * wastes time unnecessarily. You can do it though for reasons of simplicity.
 */
uint16_t write_phraseLCD(char *phrase, uint16_t phrase_length, uint16_t x, uint16_t y,
					uint16_t letter_color, uint16_t background_color) {
	uint16_t i;

	for (i = 0; i < phrase_length && phrase[i] >= 32; ++i) {
		x = write_letterLCD(phrase[i], x, y, letter_color, background_color);
		++x;
	}

	if (!i) return x;
	return x - 1;
}

/*
 * A function to write a number stored as an array of ASCII characters. It will
 * not write zero digits, which will be stored as first digits in any numbers
 * that are produced by our itoa functions.
 */
uint16_t write_numberLCD(char* number, uint16_t number_length, uint16_t x, uint16_t y,
					uint16_t number_color, uint16_t background_color) {
	uint16_t i;
	uint8_t number_started = 0;

	for (i = 0; i < number_length && number[i] >= 32; ++i) {
		if ((number[i] >= '1') && (number[i] <= '9')) {
			x = write_letterLCD(number[i], x, y, number_color, background_color);
			if (!number_started) number_started = 1;
		}
		else {
			if ((number[i] == '0') && number_started)
				x = write_letterLCD(number[i], x, y, number_color, background_color);
			/*else
				x = write_letterLCD(' ', x, y, number_color, background_color);*/
		}
		++x;
		if (i == (number_length - 2))
			number_started = 1;
	}

	if (i) return x - 1;
	return x;
}

/*
 * This will paint an image starting from the specified position on display.
 * The image is stored in an inefficient RAW format, you can read its
 * specification in images.h.
 */
void paint_imageLCD(uint16_t *image, uint16_t x, uint16_t y) {
	uint16_t length = image[0];
	uint16_t height = image[1];
	uint16_t x_end = x + length - 1;
	uint16_t y_end = y + height - 1;

	define_paint_areaLCD(x, y, x_end, y_end);

	uint16_t i;
	uint16_t j;
	uint32_t address = 2;

	issue_commandLCD(WRITE_MEMORY_START);

	for (i = 0; i < height; ++i) {
		for (j = 0; j < length; ++j) {
			write_pixelLCD(image[address]);
			++address;
		}
	}
}
