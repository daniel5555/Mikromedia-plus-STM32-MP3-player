/*
 * Copyright (c) 2014, Daniel Flores Tafur
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
 * Code related to I2C communication with touch module is adapted from a code
 * example by Elia. Elia's code is licensed under Creative Commons Attribution
 * license v3.0. The code example is highly recommended to check out. It is
 * located at:
 * http://eliaselectronics.com/stm32f4-tutorials/stm32f4-i2c-master-tutorial/
 * Elia's Electronics: http://eliaselectronics.com/
 *
 * Another code which was consulted and which helped me a lot while creating
 * this code is code by Peter S. (psavr) from here:
 * http://www.mikrocontroller.net/topic/277415#2928742
 * That code was adapted from original Elia's code.
 */

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx.h>

#include <touch.h>
#include <utils.h>
#include <lcd.h>
#include <delay.h>
#include <registers_listTouch.h>

/*
 * Function to initialize GPIO pins for I2C protocol. Note that we don't
 * initialize the input pin from STMPE610, because we are not using it.
 */
void GPIOTouch_Init() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/*
	 * we don't need the commented statement above, if GPIOLED_Init() was
	 * executed previously.
	 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1); //SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); //SDA

	GPIO_InitTypeDef GPIOTouch_settings;
	GPIOTouch_settings.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIOTouch_settings.GPIO_Mode = GPIO_Mode_AF;
	GPIOTouch_settings.GPIO_Speed = GPIO_Speed_50MHz;
	GPIOTouch_settings.GPIO_OType = GPIO_OType_OD;
	GPIOTouch_settings.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOB, &GPIOTouch_settings);

	I2C_InitTypeDef I2C1_settings;
	I2C1_settings.I2C_ClockSpeed = 300000;
	I2C1_settings.I2C_Mode = I2C_Mode_I2C;
	I2C1_settings.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C1_settings.I2C_OwnAddress1 = 0x0000; //not relevant
	I2C1_settings.I2C_Ack = I2C_Ack_Disable;
	I2C1_settings.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	I2C_Init(I2C1, &I2C1_settings);
	I2C_Cmd(I2C1, ENABLE);
}

/*
 * Function to start any type of transaction regarding STMPE610 touch
 * controller. The transaction can be either Transmitter (from MCU to STMPE610)
 * or Receiver (from STMPE610 to MCU). The function will return 0 if case of
 * success and an error number indicating the stage where communication broke.
 */
uint8_t start_touch_module_transaction(uint8_t transaction_type) {
	int timeout = 1000;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {
		--timeout;
		if (timeout <= 0) return 1;
	}

	timeout = 1000;

	I2C_GenerateSTART(I2C1, ENABLE);

	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
		--timeout;
		if (timeout <= 0) return 2;
	}

	timeout = 1000;

	I2C_Send7bitAddress(I2C1, STMPE610_ADDRESS + transaction_type, transaction_type);

	if (transaction_type == I2C_Direction_Transmitter) {
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
			--timeout;
			if (timeout <= 0) return 3;
		}
	}
	else {
		if (transaction_type == I2C_Direction_Receiver) {
			while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
				--timeout;
				if (timeout <= 0) return 4;
			}
		}
	}
	return 0;
}

/*
 * Function to send data towards STMPE610. It can be either a register number
 * or a value you want to write into a previously indicated register. Returns 0
 * if success and 1 if there was a failure.
 */
uint8_t send_data_to_touch_module(uint8_t data) {
	int timeout = 1000;
	I2C_SendData(I2C1, data);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
		--timeout;
		if (timeout <= 0) return 1;
	}
	return 0;
}

/*
 * This function simply receives the data from a previously indicated register.
 * Returns 0 if success and 1 if there was a failure.
 * ATTENTION: this function is meant to be used if you want to read more than 1
 * byte and it shouldn't be used if the byte you are receiving is meant to be
 * the last one. If that's the case or you want to receive just 1 byte, use
 * "receive_1byte_from_touch_module()" function instead. This is because of
 * STMPE610 specifics.
 */
uint8_t receive_data_from_touch_module(uint8_t* data) {
	int timeout = 1000;
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
		--timeout;
		if (timeout <= 0) return 1;
	}
	*data = I2C_ReceiveData(I2C1);
	return 0;
}

/*
 * This function should be used to receive just 1 byte of data or to receive
 * the last byte from a series of bytes. Return 0 if in case of success and 1
 * in case of failure.
 */
uint8_t receive_1byte_from_touch_module(uint8_t* data) {
	int timeout = 1000;
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_GenerateSTOP(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
		--timeout;
		if (timeout <= 0) return 1;
	}
	*data = I2C_ReceiveData(I2C1);
	return 0;
}

/*
 * This define should be used at the end of transaction to properly finish it.
 */
#define stop_touch_module_transaction() I2C_GenerateSTOP(I2C1, ENABLE)

/*
 * This function just checks that STMPE610 works. It's probably a good idea to
 * perform a check like this before actually attempting to configure something.
 * This check asks the device for its revision number and if it responds we
 * can assume that it listens to us, at least. The revision number for STMPE610
 * should be 0x03 if it's final silicon version.
 */
void Touch_Init() {
	write_phraseLCD("Touch_Init().", 13, 0, 24, 0x0000, 0xFFFF);

	char s[6];
	uint8_t error = 0;
	uint8_t data;
	uint8_t finish = 0;

	while (error || !finish) {
		error = start_touch_module_transaction(I2C_Direction_Transmitter);
		if (!error) error = send_data_to_touch_module(ID_VER);
		if (!error) stop_touch_module_transaction();
		if (!error) error = start_touch_module_transaction(I2C_Direction_Receiver);
		if (!error) error = receive_1byte_from_touch_module(&data);
		else {
			itoa16bits(error, s);
			write_phraseLCD("Error detected!", 15, 40, 72, 0x0000, 0xFFFF);
			write_phraseLCD(s, 6, 40, 96, 0x0000, 0xFFFF);
			stop_touch_module_transaction();
			Delay_us(1);
		}
		finish = 1;
	}

	write_phraseLCD("Touch_Init() finished.", 22, 0, 168, 0x0000, 0xFFFF);

	Delay_ms(1000);

	paint_areaLCD(0, 0, 479, 271, 0xFFFF);
}

/*
 * This function actually configures the STMPe610 controller to work as a touch
 * controller with specific parameters. It should be noted that accuracy
 * heavily depends on those parameters and you should always test exhaustively
 * your screen to determine the best ones in your case. Sometimes the accuracy
 * is very limited because of quality of the screen and there is not much we
 * can do about it once we have reached that limit.
 */
void configure_touch_module() {
	int error = 0;

	/*
	 * Enable clocks (disable clock gating) of TSC and ADCs
	 */
	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(SYS_CTRL2);
	if (!error) error = send_data_to_touch_module(0x00);
	stop_touch_module_transaction();

	/*
	 * Configure the ADC parameters. Here we set the maximum sample time to
	 * improve accuracy (124). We also choose the 10 bits mode, because we
	 * don't need at all 12 bits resolution for this screen.
	 */
	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(ADC_CTRL1);
	if (!error) error = send_data_to_touch_module(0xE4);
	stop_touch_module_transaction();

	/*
	 * Configure ADC frequency. We set the slowest value possible in hopes of
	 * improving accuracy (1.625 MHz).
	 */
	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(ADC_CTRL2);
	if (!error) error = send_data_to_touch_module(0x00);
	stop_touch_module_transaction();

	/*
	 * Basic touch screen controller configuration. The operating mode will be
	 * to get only X and Y and we also want to cover the whole screen area.
	 */
	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(TSC_CTRL);
	if (!error) error = send_data_to_touch_module(0x02);
	stop_touch_module_transaction();

	/*
	 * Interrupt configuration. We are not using those, so those lines aren't
	 * necessary. By default, the interrupts are not used.
	 */
	/*
	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(INT_EN);
	if (!error) error = send_data_to_touch_module(0x01);
	stop_touch_module_transaction();

	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(INT_CTRL);
	if (!error) error = send_data_to_touch_module(0x01);
	stop_touch_module_transaction();*/

	/*
	 * Touch screen configuration. We configure it thus: set 8 samples per
	 * actual value that we'll get in order to achieve maximum accuracy and set
	 * delay touch detection and settling times respectively to 5 ms. It should
	 * be noted that at times this configuration still gives us erroneous
	 * values. It's possible to change the parameter to 0xE4 for slower
	 * response and less errors.
	 */
	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(TSC_CFG);
	if (!error) error = send_data_to_touch_module(0xE3); /* or E4? */
	stop_touch_module_transaction();

	/*
	 * FIFO threshold configuration. It doesn't matter to us, but we'll set it
	 * to 1 just in case (it's 0 by default, but the STMPE610 manual says it
	 * shouldn't be 0).
	 */
	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(FIFO_TH);
	if (!error) error = send_data_to_touch_module(0x01);
	stop_touch_module_transaction();

	/*
	 * Finally, enable the touch screen controller.
	 */
	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(TSC_CTRL);
	if (!error) error = send_data_to_touch_module(0x03);
	stop_touch_module_transaction();

	if (error) {
		write_phraseLCD("An error has occurred.", 22, 0, 0, 0x0000, 0xFFFF);
		Delay_ms(3000);
	}
	else {
		write_phraseLCD("Configuration finished.", 23, 0, 0, 0x0000, 0xFFFF);
		Delay_ms(3000);
	}
}

/*
 * Clean up the FIFO. We only get 1 value from it and try to process it as fast
 * as possible, so generally we want to discard the values it may get while we
 * are processing the data. This function can be used to clean FIFO before
 * getting any new and more important data.
 */
void reset_touch_fifo() {
	int error = 0;

	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(FIFO_STA);
	if (!error) error = send_data_to_touch_module(0x01);
	stop_touch_module_transaction();

	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(FIFO_STA);
	if (!error) error = send_data_to_touch_module(0x00);
	stop_touch_module_transaction();

	if (error) {
		write_phraseLCD("Reset fifo failed.", 18, 0, 0, 0x0000, 0xFFFF);
	}
}

/*
 * If we had used interrupts, this function couldn't be used to clear a touch
 * interrupt. WARNING: it wasn't tested, so it may not work!
 */
/*
void clear_touch_interrupt() {
	int error = 0;

	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(INT_STA);
	if (!error) error = send_data_to_touch_module(0x01);
	stop_touch_module_transaction();
}*/

/*
 * Function to detect touch. There are several possibilities to do this. Here
 * we are using the simplest one - we are just polling the data periodically
 * from the register. This function is used to get the data on touch once and
 * should be called continuously. It the screen is being touched while the
 * function is called, it should detect the touch.
 */
uint8_t detect_touch() {
	int error = 0;
	uint8_t data;

	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(TSC_CTRL);
	stop_touch_module_transaction();
	if (!error) error = start_touch_module_transaction(I2C_Direction_Receiver);
	if (!error) error = receive_1byte_from_touch_module(&data);
	stop_touch_module_transaction();
	if (error) {
		write_phraseLCD("Couldn't perform touch detection.", 33, 0, 0, 0x0000,
				0xFFFF);
	}

	return (data & 0x80);
}

/*
 * Get the touch data, in our case it's X and Y. This function doesn't return
 * the values in pixels, so they must be processed according to screen's
 * resolution and other hardware-related details.
 */
void get_touch_data(uint16_t* x, uint16_t* y) {
	int error = 0;
	uint8_t data;

	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(TSC_DATA_XYZ);
	stop_touch_module_transaction();
	if (!error) error = start_touch_module_transaction(I2C_Direction_Receiver);
	if (!error) error = receive_data_from_touch_module(&data);
	*x = data << 4;
	if (!error) error = receive_data_from_touch_module(&data);
	*x += (data & 0xF0) >> 4;
	*y = (data & 0x0F) << 8;
	if (!error) error = receive_1byte_from_touch_module(&data);
	stop_touch_module_transaction();
	*y += data;

	if (error) {
		write_phraseLCD("Couldn't retrieve data.", 23, 0, 0, 0x0000, 0xFFFF);
		Delay_ms(3000);
	}
}

/*
 * This function can be used to get the current FIFO size. We only use it for
 * testing purposes.
 */
uint8_t get_fifo_touch_size() {
	int error = 0;
	uint8_t data;

	if (!error) error = start_touch_module_transaction(I2C_Direction_Transmitter);
	if (!error) error = send_data_to_touch_module(FIFO_SIZE);
	stop_touch_module_transaction();
	if (!error) error = start_touch_module_transaction(I2C_Direction_Receiver);
	if (!error) error = receive_1byte_from_touch_module(&data);
	stop_touch_module_transaction();

	if (error) {
		write_phraseLCD("Couldn't retrieve data.", 23, 0, 0, 0x0000, 0xFFFF);
		Delay_ms(3000);
	}

	return data;
}

/*
 * Function used to easily convert the raw touch data into the actual location
 * measured in pixels on screen. It returns 0 in case that raw values are
 * invalid (for example, they point to an area outside the screen).
 */
uint8_t convert_touch_data(uint16_t* x, uint16_t* y) {
	float final_x;
	float final_y;
	if ((*x >= LEFT_BORDER_X) && (*x < RIGHT_BORDER_X)
		&& (*y >= UPPER_BORDER_Y) && (*y < BOTTOM_BORDER_Y)) {
		*x /= 10;
		*y /= 10;
		*x -= LEFT_BORDER_X/10;
		*y -= UPPER_BORDER_Y/10;

		final_x = (float)(*x)*FLOAT_PER_PIXEL_X;
		final_y = (float)(*y)*FLOAT_PER_PIXEL_Y;
		*x = (uint16_t)final_x;
		*y = (uint16_t)final_y;
		return 1;
	}
	else return 0;
}

/*
 * This is the test function that allows you to study the characteristics of
 * your touch panel. It will display on screen the raw X and Y values, so you
 * can locate the touch screen borders and perform other adjustments.  It also
 * shows the current FIFO size and displays a dot in the center of the screen
 * when touch is detected. You should use this function and change the code
 * accordingly if other test functions don't work properly.
 */
void test_program() {
	uint16_t x = 0;
	uint16_t y = 0;
	uint16_t x_previous = 1;
	uint16_t y_previous = 1;

	int temp;

	temp = write_phraseLCD("X:", 2, 0, 224, 0x0000, 0xFFFF);
	write_phraseLCD("Y:", 2, 0, 248, 0x0000, 0xFFFF);
	write_phraseLCD("S:", 2, 0, 200, 0x0000, 0xFFFF);

	char s[6];

	itoa16bits(x, s);
	write_phraseLCD(s, 6, temp + 1, 224, 0x0000, 0xFFFF);
	itoa16bits(y, s);
	write_phraseLCD(s, 6, temp + 1, 248, 0x0000, 0xFFFF);

	uint8_t size = 0;

	while(1)
	{
	    if (detect_touch()) {
	    	size = get_fifo_touch_size();
	    	if (size > 0) {
	    		get_touch_data(&x, &y);
	    		itoa16bits(size, s);
	    		write_phraseLCD(s, 6, temp + 1, 200, 0x0000, 0xFFFF);
	    		if ((x != x_previous) || (y != y_previous)) {
	    			itoa16bits(x, s);
	    			write_letterLCD('.', 240, 136, 0x0000, 0xFFFF);
	    			write_phraseLCD(s, 6, temp + 1, 224, 0x0000, 0xFFFF);
	    			itoa16bits(y, s);
	    			write_phraseLCD(s, 6, temp + 1, 248, 0x0000, 0xFFFF);
	    			size = 0;
	    			x_previous = x;
	    			y_previous = y;
	    		}
	    		reset_touch_fifo();
	    	}
	    }
	    else {
	    	size = 0;
	    	x = 0;
	    	y = 0;
	    	itoa16bits(size, s);
	    	write_phraseLCD(s, 6, temp + 1, 200, 0x0000, 0xFFFF);
	    	itoa16bits(x, s);
	    	write_letterLCD(' ', 240, 136, 0x0000, 0xFFFF);
	    	write_phraseLCD(s, 6, temp + 1, 224, 0x0000, 0xFFFF);
	    	itoa16bits(y, s);
	    	write_phraseLCD(s, 6, temp + 1, 248, 0x0000, 0xFFFF);
	    }
	}
}

/*
 * This function allows you to test touch screen coordinates error rate and
 * response time. It simply shows a blanck screen on which you can draw.
 */
void simple_drawing() {
	uint16_t x = 0;
	uint16_t y = 0;
	uint16_t x_previous = 10000;
	uint16_t y_previous = 10000;

	float final_x;
	float final_y;
	uint8_t reset = 0;

	reset_touch_fifo();
	uint8_t size;

	while(1) {
		if (detect_touch()) {
			reset = 1;
			size = get_fifo_touch_size();
			if (size > 0) {
				get_touch_data(&x, &y);
				if ((x >= LEFT_BORDER_X) && (x < RIGHT_BORDER_X)
						&& (y >= UPPER_BORDER_Y) && (y < BOTTOM_BORDER_Y)) {
					x /= 10;
					y /= 10;
					x -= LEFT_BORDER_X/10;
					y -= UPPER_BORDER_Y/10;
					if ((x != x_previous) || (y != y_previous)) {
						x_previous = x;
						y_previous = y;
						final_x = (float)x*FLOAT_PER_PIXEL_X;
						final_y = (float)y*FLOAT_PER_PIXEL_Y;
						x = (uint16_t)final_x;
						y = (uint16_t)final_y;
						paint_areaLCD(x, y, x + 1, y + 1, 0x0000);
					}
				}
			}
		}
		else {
			if (reset) {
				reset_touch_fifo();
				reset = 0;
			}
		}
	}
}

/*
 * This function allows you to test the touch screen readiness for simple touch
 * interfaces and also shows a possible way to build such interface. It will
 * display some boxes that define touch areas. Upon touching the area, the box
 * will disappear. So you can test the whole touching experience. This function
 * will also display a dot where the touch was detected in order to help you
 * detect possible errors in detection. When all the boxes are cleared, the
 * function will start the whole process again.
 */
void test_touch_boxes() {
	paint_areaLCD(0, 0, 479, 271, 0xFFFF);

	struct Box box1;
	box1.x_start = 20;
	box1.y_start = 20;
	box1.x_end = 40;
	box1.y_end = 40;

	struct Box box2;
	box2.x_start = 50;
	box2.y_start = 60;
	box2.x_end = 150;
	box2.y_end = 120;

	struct Box box3;
	box3.x_start = 400;
	box3.y_start = 0;
	box3.x_end = 450;
	box3.y_end = 70;

	struct Box box4;
	box4.x_start = 400;
	box4.y_start = 260;
	box4.x_end = 479;
	box4.y_end = 271;

	struct Box box5;
	box5.x_start = 200;
	box5.y_start = 100;
	box5.x_end = 300;
	box5.y_end = 150;

	struct Box box6;
	box6.x_start = 20;
	box6.y_start = 170;
	box6.x_end = 24;
	box6.y_end = 270;

	struct Box box7;
	box7.x_start = 440;
	box7.y_start = 120;
	box7.x_end = 460;
	box7.y_end = 120;

	paint_areaLCD(box1.x_start, box1.y_start, box1.x_end, box1.y_end, 0xF800);
	paint_areaLCD(box2.x_start, box2.y_start, box2.x_end, box2.y_end, 0x07E0);
	paint_areaLCD(box3.x_start, box3.y_start, box3.x_end, box3.y_end, 0x001F);
	paint_areaLCD(box4.x_start, box4.y_start, box4.x_end, box4.y_end, 0xF81F);
	paint_areaLCD(box5.x_start, box5.y_start, box5.x_end, box5.y_end, 0xFFE0);
	paint_areaLCD(box6.x_start, box6.y_start, box6.x_end, box6.y_end, 0x07FF);
	paint_areaLCD(box7.x_start, box7.y_start, box7.x_end, box7.y_end, 0x0000);

	uint16_t x = 0;
	uint16_t y = 0;

	float final_x;
	float final_y;
	uint8_t reset = 0;

	int boxes = 7;
	uint8_t box1_on_screen = 1;
	uint8_t box2_on_screen = 1;
	uint8_t box3_on_screen = 1;
	uint8_t box4_on_screen = 1;
	uint8_t box5_on_screen = 1;
	uint8_t box6_on_screen = 1;
	uint8_t box7_on_screen = 1;

	uint8_t size;

	reset_touch_fifo();

	while (boxes > 0) {
		if (detect_touch()) {
			reset = 1;
			size = get_fifo_touch_size();
			if (size > 0) {
				get_touch_data(&x, &y);
				if ((x >= LEFT_BORDER_X) && (x < RIGHT_BORDER_X)
						&& (y >= UPPER_BORDER_Y) && (y < BOTTOM_BORDER_Y)) {
					x /= 10;
					y /= 10;
					x -= LEFT_BORDER_X/10;
					y -= UPPER_BORDER_Y/10;

					final_x = (float)x*FLOAT_PER_PIXEL_X;
					final_y = (float)y*FLOAT_PER_PIXEL_Y;
					x = (uint16_t)final_x;
					y = (uint16_t)final_y;
					if (box1_on_screen && (x >= box1.x_start) && (x <= box1.x_end)
							&& (y >= box1.y_start) && (y <= box1.y_end)) {
						paint_areaLCD(box1.x_start, box1.y_start, box1.x_end,
								box1.y_end, 0xFFFF);
						box1_on_screen = 0;
						--boxes;
					}
					else {
					if (box2_on_screen && (x >= box2.x_start) && (x <= box2.x_end)
							&& (y >= box2.y_start) && (y <= box2.y_end)) {
						paint_areaLCD(box2.x_start, box2.y_start, box2.x_end,
							box2.y_end, 0xFFFF);
						box2_on_screen = 0;
						--boxes;
					}
					else {
					if (box3_on_screen && (x >= box3.x_start) && (x <= box3.x_end)
							&& (y >= box3.y_start) && (y <= box3.y_end)) {
						paint_areaLCD(box3.x_start, box3.y_start, box3.x_end,
							box3.y_end, 0xFFFF);
						box3_on_screen = 0;
						--boxes;
					}
					else {
					if (box4_on_screen && (x >= box4.x_start) && (x <= box4.x_end)
							&& (y >= box4.y_start) && (y <= box4.y_end)) {
						paint_areaLCD(box4.x_start, box4.y_start, box4.x_end,
						box4.y_end, 0xFFFF);
						box4_on_screen = 0;
						--boxes;
					}
					else {
					if (box5_on_screen && (x >= box5.x_start) && (x <= box5.x_end)
							&& (y >= box5.y_start) && (y <= box5.y_end)) {
						paint_areaLCD(box5.x_start, box5.y_start, box5.x_end,
							box5.y_end, 0xFFFF);
						box5_on_screen = 0;
						--boxes;
					}
					else {
					if (box6_on_screen && (x >= box6.x_start) && (x <= box6.x_end)
							&& (y >= box6.y_start) && (y <= box6.y_end)) {
						paint_areaLCD(box6.x_start, box6.y_start, box6.x_end,
					    	box6.y_end, 0xFFFF);
						box6_on_screen = 0;
						--boxes;
					}
					else {
					if (box7_on_screen && (x >= box7.x_start) && (x <= box7.x_end)
							&& (y >= box7.y_start) && (y <= box7.y_end)) {
						paint_areaLCD(box7.x_start, box7.y_start, box7.x_end,
							box7.y_end, 0xFFFF);
						box7_on_screen = 0;
						--boxes;
					} } } } } }
					paint_areaLCD(x, y, x + 1, y + 1, 0x0000);
					}
				}
			}
		}
		else {
			if (reset) {
				reset_touch_fifo();
				reset = 0;
			}
		}
	}
}
