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

#ifndef TOUCH_H
#define TOUCH_H

#define STMPE610_ADDRESS 0x88

/*
 * The value given by STMPE610 touch screen controller doesn't correspond
 * directly to an actual pixel on the screen. So in order to find out the touch
 * location, we have to multiply the original values by the numbers given
 * below.
 */
#define FLOAT_PER_PIXEL_X (float)479/(float)380
#define FLOAT_PER_PIXEL_Y (float)271/(float)361

/*
 * Touch screen border in raw values. Smaller ones equal to first pixels
 * and bigger ones to last.
 */
#define LEFT_BORDER_X 	150
#define RIGHT_BORDER_X 	3960
#define UPPER_BORDER_Y 	240
#define BOTTOM_BORDER_Y 3860

/*
 * The touch box structure, it should only contain the borders of touch
 * element. It's better to store the actual graphics elsewhere.
 */
struct Box {
	uint16_t x_start;
	uint16_t y_start;
	uint16_t x_end;
	uint16_t y_end;
};

void GPIOTouch_Init();
uint8_t start_touch_module_transaction(uint8_t transaction_type);
uint8_t send_data_to_touch_module(uint8_t data);
uint8_t receive_data_from_touch_module(uint8_t* data);
uint8_t receive_1byte_from_touch_module(uint8_t* data);
void image_test();
/*
 * This define should be used at the end of transaction to properly finish it.
 */
#define stop_touch_module_transaction() I2C_GenerateSTOP(I2C1, ENABLE)
void Touch_Init();
void configure_touch_module();
void reset_touch_fifo();
//void clear_touch_interrupt();
uint8_t detect_touch();
void get_touch_data(uint16_t* x, uint16_t* y);
uint8_t get_fifo_touch_size();
void test_program();
void simple_drawing();
void test_touch_boxes();
uint8_t convert_touch_data(uint16_t* x, uint16_t* y);

#endif /* TOUCH_H */
