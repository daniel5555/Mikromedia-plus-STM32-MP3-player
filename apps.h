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

#ifndef APPS_H
#define APPS_H

#include <ff.h>
#include <stm324xg_eval_sdio_sd.h>

#define NO_SDCARD 0
#define OPEN_FILE 1

extern char current_directory_path[20];
extern char visited_directories[50][13];
extern char target_file[13];
extern uint16_t cursors[50];
extern uint8_t depth;

extern const uint16_t arrow_up_image[];
extern const uint16_t arrow_down_image[];
extern const uint16_t folder_up_image[];
extern const uint16_t arrow_up_pressed_image[];
extern const uint16_t arrow_down_pressed_image[];
extern const uint16_t folder_up_pressed_image[];

/*
 * Struct that serves to display and identify the files presented in file
 * manager's menu that lists the file in a directory.
 */
struct Menu_object {
	BYTE	fattrib;
	TCHAR	fname[13];
	uint8_t exists;
};

/*
 * Struct for menu area, a special type of object on touch screen that is
 * composed of several elements that have the same dimensions and that form
 * one rectangular area on the screen. It's much more efficient to use it
 * instead of composing the menu with button-like objects and try to identify
 * an object by checking all the objects one by one.
 */
struct Menu_area {
	uint16_t x_start;
	uint16_t y_start;
	uint16_t x_end;
	uint16_t y_end;
	uint8_t step;
};

void system_message(uint8_t number);
uint8_t txt_viewer();
uint8_t file_manager();

#endif /* APPS_H */
