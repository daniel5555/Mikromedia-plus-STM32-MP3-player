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

#ifndef LCD_H
#define LCD_H

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx.h>

#include <delay.h>
#include <command_listLCD.h>

void GPIOLCD_Init();
void issue_commandLCD(uint16_t command);
void write_dataLCD(uint16_t data);
void write_LCD(uint16_t value, BitAction type);
void LCD_Init();
void define_paint_areaLCD(uint16_t x, uint16_t y, uint16_t x_end,
					uint16_t y_end);
void write_pixelLCD(uint16_t pixel);
void paint_areaLCD(uint16_t x, uint16_t y, uint16_t x_end, uint16_t y_end,
					uint16_t color);
uint16_t get_letter_length(char letter);
uint16_t write_letterLCD(char letter, uint16_t x, uint16_t y,
				uint16_t letter_color, uint16_t background_color);
uint16_t write_phraseLCD(char *phrase, uint16_t phrase_length, uint16_t x, uint16_t y,
					uint16_t letter_color, uint16_t background_color);
uint16_t write_numberLCD(char* number, uint16_t number_length, uint16_t x, uint16_t y,
					uint16_t number_color, uint16_t backgound_color);
void paint_imageLCD(uint16_t *image, uint16_t x, uint16_t y);

#endif /* LCD_H */
