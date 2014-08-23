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

#include <apps.h>
#include <lcd.h>
#include <touch.h>
#include <utils.h>

/*
 * This simple function prints a message inside a window, prints the name of
 * file related to that message and waits until users presses the "OK" button.
 * Messages are hardcoded inside the function and all other functions must be
 * aware about the numbers they should return to print a specific message.
 */
void system_message(uint8_t number) {
	char* message;
	uint16_t message_length;
	switch (number) {
	case 0:
		message = "Can't open the file:";
		message_length = 20;
		break;
	case 1:
		message = "Failed to open txt file:";
		message_length = 24;
		break;
	case 2:
		message = "Txt file too big to open:";
		message_length = 25;
		break;
	case 3:
		message = "Filesystem error in file:";
		message_length = 25;
		break;
	case 4:
		message = "Unknown error occurred:";
		message_length = 23;
		break;
	case 5:
		message = "Player failed in file:";
		message_length = 22;
		break;
	default:
		message_length = 0;
	}

	struct Box OK_button;
	OK_button.x_start = 286;
	OK_button.y_start = 147;
	OK_button.x_end = 345;
	OK_button.y_end = 182;

	//Paint borders of the window
	paint_areaLCD(86, 90, 87, 189, 0x0000);
	paint_areaLCD(88, 90, 385, 91, 0x0000);
	paint_areaLCD(384, 92, 385, 189, 0x0000);
	paint_areaLCD(88, 188, 383, 189, 0x0000);

	//Clear the content inside the window
	paint_areaLCD(88, 92, 383, 187, 0xFFFF);

	//Write the content inside the window
	write_phraseLCD(message, message_length, 94, 96, 0x0000, 0xFFFF);
	write_phraseLCD(target_file, 13, 94, 120, 0x0000, 0xFFFF);

	//Paint the "OK" button
	paint_areaLCD(OK_button.x_start, OK_button.y_start, OK_button.x_start + 1, OK_button.y_end, 0x0000);
	paint_areaLCD(OK_button.x_start + 2, OK_button.y_start, OK_button.x_end, OK_button.y_start + 1, 0x0000);
	paint_areaLCD(OK_button.x_end - 1, OK_button.y_start + 2, OK_button.x_end, OK_button.y_end, 0x0000);
	paint_areaLCD(OK_button.x_start + 2, OK_button.y_end - 1, OK_button.x_end - 2, OK_button.y_end, 0x0000);

	//paint_areaLCD(288, 149, 343, 180, 0xFFFF);

	write_phraseLCD("OK", 2, 298, 155, 0x0000, 0xFFFF);

	uint8_t done = 0;
	uint16_t x, y;

	reset_touch_fifo();

	while (SDCard_present() && !done) {
		if (detect_touch()) {
			uint8_t size = get_fifo_touch_size();
			if (size > 0) {
				get_touch_data(&x, &y);
				if (convert_touch_data(&x, &y)) {
					if ((x >= OK_button.x_start) && (x <= OK_button.x_end) &&
							(y >= OK_button.y_start) && (y <= OK_button.y_end)) {
						done = 1;
						paint_areaLCD(OK_button.x_start + 2, OK_button.y_start + 2, OK_button.x_end - 2, OK_button.y_end - 2, 0x0000);
						write_phraseLCD("OK", 2, 298, 155, 0xFFFF, 0x0000);
						Delay_ms(75);
					}
				}
			}
		}
		reset_touch_fifo();
	}

	//Clear the window
	paint_areaLCD(86, 90, 385, 189, 0xFFFF);
}

/*
 * This function is a very simple text viewer, which can only open txt files.
 * Its design is similar to a file manager function. First, it tries to
 * calculate the total number of lines in the txt file. A line reaches its end
 * either when the last pixel of last written character trespasses the text
 * area border or when a newline symbol is reached. The screen can display 10
 * lines of text with the standard 24p font we use. So, each 10 lines of text
 * are considered a page. When we reach the start of new page we write its
 * location inside the file in an array called "pages" which should store all
 * pages of file. We fill this array when we scan the file for the first time.
 * So, once the file is scanned, the user can easily navigate by pages and the
 * viewer can easily display them by navigating to a location stored in "pages"
 * and then reading characters byte by byte from it until the screen is
 * completely filled. The most complex part of this function is that the line
 * counting and line displaying should count/display the same number of lines
 * and characters in each line.
 * The function can only display ASCII characters (visible ones and horizontal
 * tab which is interpreted as a single space). Any other character except
 * newline, which starts a new line immediately, is ignored and not displayed
 * at all. This means that some symbols may be ignored even when file was saved
 * as plain text since there are different encoding schemes.
 */
uint8_t txt_viewer() {
	struct Box arrow_up;
	arrow_up.x_start = 456;
	arrow_up.y_start = 32;
	arrow_up.x_end = 479;
	arrow_up.y_end = 56;

	struct Box arrow_down;
	arrow_down.x_start = 456;
	arrow_down.y_start = 248;
	arrow_down.x_end = 479;
	arrow_down.y_end = 271;

	struct Box exit_app;
	exit_app.x_start = 0;
	exit_app.y_start = 0;
	exit_app.x_end = 23;
	exit_app.y_end = 23;

	FRESULT result;
	FIL file;
	result = f_open(&file, target_file, FA_READ | FA_OPEN_EXISTING);
	if (result == FR_OK) {
		uint32_t size = f_size(&file);
		char size_to_display[11];
		itoa32bits(size, size_to_display);
		uint16_t length = write_phraseLCD(target_file, 13, 29, 0, 0x0000, 0xFFFF);
		paint_areaLCD(length + 1, 0, 199, 31, 0xFFFF);
		length = write_numberLCD(size_to_display, 11, 240, 0, 0x0000, 0xFFFF);
		write_phraseLCD(" bytes", 6, length + 1, 0, 0x0000, 0xFFFF);
		paint_areaLCD(0, 32, 455, 271, 0xFFFF);
		char buff[512];
		uint32_t pages[100];
		pages[0] = 0;
		uint16_t current_page = 1;
		uint32_t i = 0;
		//i is a current position in a file, used to move within file.
		uint32_t lines = 0;
		uint16_t last_pixel_of_line = 0;
		UINT number_bytes;
		result = f_read(&file, buff, 512, &number_bytes);
		//Calculate the number of lines.
		while (i < size && result == FR_OK) {
			uint32_t j = 0;
			while (j < number_bytes && current_page < 100) {
				if (((buff[j] >= 32) && (buff[j] <= 126)) || (buff[j] == 0x09)) {
					if (buff[j] == 0x09) buff[j] = ' ';
					if (last_pixel_of_line > 430) {
						++lines;
						last_pixel_of_line = 0;
						if (lines == 10) {
							pages[current_page] = i + j;
							++current_page;
							lines = 0;
						}
					}
					else {
						if (last_pixel_of_line)
							last_pixel_of_line += get_letter_length(buff[j]);
						else
							last_pixel_of_line += get_letter_length(buff[j]) - 1;
						++j;
					}
				}
				else {
					if (buff[j] == 0x0A) {
						++lines;
						last_pixel_of_line = 0;
						++j;
						if (lines == 10) {
							if (i + j < size) {
								pages[current_page] = i + j;
								++current_page;
								lines = 0;
							}
						}
					}
					else
						++j;
				}
			}
			if (current_page == 100) {
				paint_areaLCD(29, 0, 455, 32, 0xFFFF);
				return 2;
			}
			i += 512;
			result = f_read(&file, buff, 512, &number_bytes);
		}
		/*
		 * After we calculated the total number of pages in a file, we display
		 * the first page. The function goes into the typical loop which is the
		 * base for a state machine. The function will exit if SC card is
		 * removed or if user wants to leave the opened text file. Otherwise,
		 * just like with file manager, the user will issue a command, we will
		 * change the appropriate current parameters and indicate that we have
		 * a new order, and the next iteration the state machine will resolve
		 * the order itself.
		 */
		if (result == FR_OK) {
			uint16_t total_number_pages = current_page;
			if (f_lseek(&file, 0) == FR_OK) {
				uint8_t new_order = 1;
				uint32_t current_row = 0;
				i = 0;
				current_page = 0;

				uint8_t arrow_up_button_pressed = 0;
				uint8_t arrow_down_button_pressed = 0;

				reset_touch_fifo();

				while (SDCard_present()) {
					if (new_order) {
						paint_areaLCD(0, 32, 455, 271, 0xFFFF);
						uint8_t screen_filled = 0;
						uint16_t row = 0;
						uint32_t j = 0;
						uint16_t x = 0;
						result = f_read(&file, buff, 512, &number_bytes);
						while (!screen_filled) {
							if (((buff[j] >= 32) && (buff[j] <= 126)) ||
									(buff[j] == 0x09)) {
								if (buff[j] == 0x09) buff[j] = ' ';
								if (x <= 430) {
									if (x)
										x = write_letterLCD(buff[j], x + 1,
												32 + row*24, 0x0000, 0xFFFF);
									else
										x = write_letterLCD(buff[j], x,
												32 + row*24, 0x0000, 0xFFFF);
									++j;
								}
								else {
									++row;
									x = 0;
								}
							}
							else {
								if (buff[j] == 0x0A) {
									++row;
									x = 0;
									++j;
								}
								else
									++j;
							}
							if (row == 10) {
								screen_filled = 1;
								new_order = 0;
								x = 0;
							}
							if (j == number_bytes) {
								if (!screen_filled) {
									if (i + j < size) {
										result = f_read(&file, buff, 512,
												&number_bytes);
										if (result != FR_OK) return 3;
										j = 0;
										i += 512;
									}
									else
										screen_filled = 1;
								}
							}
						}

						/*
						 * Scroll bar calculation code, very similar to a code
						 * that does the same in file manager function.
						 */
						uint16_t empty_area1, filled_area, empty_area2;
						if (total_number_pages == 1) {
							empty_area1 = 0;
							filled_area = 192;
							empty_area2 = 0;
						}
						else {
							float percentage = (float)1/(float)total_number_pages;
							percentage = (float)192*percentage;
							filled_area = (uint16_t)percentage;
							if (current_page) {
								percentage = (float)current_page/(float)total_number_pages;
								percentage = (float)192*percentage;
								empty_area1 = (uint16_t)percentage;
							}
							else
								empty_area1 = 0;
						}
						if (total_number_pages > 1 &&
								current_page < total_number_pages - 1)
							empty_area2 = 192 - empty_area1 - filled_area;
						else empty_area2 = 0;
						if (empty_area1) {
							paint_areaLCD(456, 56, 479, 56 + empty_area1 - 1,
									0xFFFF);
						}
						if (!empty_area2) ++filled_area;
						paint_areaLCD(456, 56 + empty_area1, 479,
								56 + empty_area1 + filled_area - 1, 0x0000);
						if (empty_area2) paint_areaLCD(456,
								56 + empty_area1 + filled_area, 479, 247, 0xFFFF);

						new_order = 0;
					}
					else {
						if (arrow_up_button_pressed) {
							paint_imageLCD((uint16_t*)arrow_up_image, arrow_up.x_start, arrow_up.y_start);
							arrow_up_button_pressed = 0;
						}
						if (arrow_down_button_pressed) {
							paint_imageLCD((uint16_t*)arrow_down_image, arrow_down.x_start, arrow_down.y_start);
							arrow_down_button_pressed = 0;
						}
					}
					/*
					 * Checking for action from user, again it's very similar
					 * to the code with same functionality from file manager
					 * function.
					 */
					if (detect_touch()) {
						uint8_t size_fifo = get_fifo_touch_size();
						if (size_fifo > 0) {
							uint16_t x, y;
							get_touch_data(&x, &y);
							if (convert_touch_data(&x, &y)) {
								if ((x >= exit_app.x_start) &&
										(x <= exit_app.x_end) &&
										(y >= exit_app.y_start) &&
										(y <= exit_app.y_end)) {
									paint_imageLCD((uint16_t*)folder_up_pressed_image,
										exit_app.x_start, exit_app.y_start);
									paint_areaLCD(29, 0, 455, 32, 0xFFFF);
									paint_areaLCD(0, 32, 455, 271, 0xFFFF);
									return 0;
								}
								if ((x >= arrow_down.x_start) &&
										(x <= arrow_down.x_end) &&
										(y >= arrow_down.y_start) &&
										(y <= arrow_down.y_end)) {
									if (current_page < total_number_pages - 1) {
										++current_page;
										if (f_lseek(&file, pages[current_page]) == FR_OK) {
											new_order = 1;
											current_row += 10;
											i = pages[current_page];
											paint_imageLCD((uint16_t*)arrow_down_pressed_image,
												arrow_down.x_start, arrow_down.y_start);
											arrow_down_button_pressed = 1;
											paint_areaLCD(0, 32, 455, 271, 0xFFFF);
										}
										else return 3;
									}
								}
								if ((x >= arrow_up.x_start) &&
										(x <= arrow_up.x_end) &&
										(y >= arrow_up.y_start) &&
										(y <= arrow_up.y_end)) {
									if (current_page > 0) {
										--current_page;
										if (f_lseek(&file, pages[current_page]) == FR_OK) {
											new_order = 1;
											current_row -= 10;
											i = pages[current_page];
											paint_imageLCD((uint16_t*)arrow_up_pressed_image,
												arrow_up.x_start, arrow_up.y_start);
											arrow_up_button_pressed = 1;
											paint_areaLCD(0, 32, 455, 271, 0xFFFF);
										}
										else return 3;
									}
								}
							}
						}
					}
					reset_touch_fifo();
				}
			}
			else return 3;
		}
		else {
			f_close(&file);
			return 3;
		}
		paint_areaLCD(29, 0, 479, 31, 0xFFFF);
		f_close(&file);
		return 0;
	}
	else return 1;
}

/*
 * This function acts like a file manager program. It manages both the file
 * managing mechanics and interface, so there is no formal separation between
 * internal proceedings and external interface. The algorithm is
 * straightforward: we a have a "current" position that indicates the first
 * file to be displayed on screen. In total, we'll display 10 files or
 * directories on screen. The user can go up and down, and each time he/she
 * issues a command, the file manager will have to read the whole directory,
 * because we don't save anywhere contents of directory. This is by far not the
 * best approach, but it is efficient enough given that we want to save as much
 * RAM as we can.
 */
uint8_t file_manager() {
	struct Box arrow_up;
	arrow_up.x_start = 456;
	arrow_up.y_start = 32;
	arrow_up.x_end = 479;
	arrow_up.y_end = 56;

	struct Box arrow_down;
	arrow_down.x_start = 456;
	arrow_down.y_start = 248;
	arrow_down.x_end = 479;
	arrow_down.y_end = 271;

	struct Box folder_up;
	folder_up.x_start = 0;
	folder_up.y_start = 0;
	folder_up.x_end = 23;
	folder_up.y_end = 23;

	struct Menu_area files_menu;
	files_menu.x_start = 0;
	files_menu.y_start = 32;
	files_menu.x_end = 239;
	files_menu.y_end = 271;
	files_menu.step = 24;

	/*
	 * The Menu_object stores some attributes of files that are displayed on
	 * screen, we'll use them to open the file or directory selected by user.
	 */
	struct Menu_object file_list[10];

	uint16_t filename_lenght[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t current_file;
	uint8_t new_order = 1;
	uint16_t total_files;
	uint16_t x, y;
	uint8_t size;
	uint8_t folder_up_button_pressed = 0;
	uint8_t arrow_up_button_pressed = 0;
	uint8_t arrow_down_button_pressed = 0;
	char* s_file = "file";
	char* s_dir = "dir ";

	write_phraseLCD(&visited_directories[depth][0], 13, 29, 0, 0x0000, 0xFFFF);
	paint_imageLCD((uint16_t*)folder_up_image, folder_up.x_start, folder_up.y_start);
	paint_imageLCD((uint16_t*)arrow_up_image, arrow_up.x_start, arrow_up.y_start);
	paint_imageLCD((uint16_t*)arrow_down_image, arrow_down.x_start, arrow_down.y_start);

	reset_touch_fifo();

	/*
	 * This is the general working loop. We don't want our file manager to
	 * crash if SD card is suddenly extracted. The loop will perform some
	 * action if a new order was issued by user during the previous cycle of
	 * this loop. There are few possible actions for user: go to parent
	 * directory, go up or down in menu, open a directory and open a file. The
	 * latter case is the only one which will close the file manager. It will
	 * write the name of the file to be opened in "target_file" global variable
	 * and let the main() of the firmware do the rest, like determine if the
	 * file can be opened or not.
	 */
	while (SDCard_present()) {
		/*
		 * If a need order is issued, it should be processed. The order is
		 * defined implicitly by setting the the variables that define the
		 * details of current state.
		 */
		if (new_order == 1) {
			DIR directory;
			FRESULT result = f_opendir(&directory, current_directory_path);
			if (result == FR_OK) {
				FILINFO file;
				uint16_t screen_position = 32;
				total_files = 0;
				uint8_t proceed = 1;
				/*
				 * What follows is the code to process the first 2 entries in
				 * any directory except the root directory - "." and "..". In
				 * that current opened directory is the root one, we don't have
				 * to do anything, otherwise we'll simply read and drop first 2
				 * entries. If file name is just empty or an error occurred
				 * while trying to read the entries, something went very wrong
				 * and we should immediately stop.
				 */
				if (depth != 0) {
					result = f_readdir(&directory, &file);
					if (file.fname[0] == 0 || result != FR_OK) {
						proceed = 0;
					}
					result = f_readdir(&directory, &file);
					if (file.fname[0] == 0 || result != FR_OK) {
						proceed = 0;
					}
				}
				/*
				 * After processing first 2 entries, if that was necessary, we
				 * should reach the position of the first entry that should be
				 * displayed on screen, depending on where the user had arrived
				 * previously. This position is stored in current entry in
				 * "cursors" array and the current entry is always indicated by
				 * global "depth" variable.
				 */
				while (total_files < cursors[depth] && proceed) {
					result = f_readdir(&directory, &file);
					if (file.fname[0] == 0 || result != FR_OK) {
						proceed = 0;
					}
					else ++total_files;
				}
				/*
				 * Once we arrived at the current position, we need to fill the
				 * menu with files that are displayed on screen. We also will
				 * display file names and their attributes (file or directory)
				 * on screen.
				 */
				current_file = 0;
				while (total_files < cursors[depth] + 10 && proceed) {
					result = f_readdir(&directory, &file);
					if (file.fname[0] == 0 || result != FR_OK) {
						proceed = 0;
					}
					else {
						mem_cpy(file_list[current_file].fname, file.fname, 13);
						file_list[current_file].fattrib = file.fattrib;
						file_list[current_file].exists = 1;
						uint16_t lenght = write_phraseLCD(file.fname, 13, 0,
								screen_position, 0x0000, 0xFFFF);
						if (lenght < filename_lenght[current_file]) {
							paint_areaLCD(lenght + 1, screen_position,
									filename_lenght[current_file],
									screen_position + 23, 0xFFFF);
						}
						filename_lenght[current_file] = lenght;
						if (file.fattrib == AM_DIR) {
							write_phraseLCD(s_dir, 4, 415, screen_position,
									0x0000, 0xFFFF);
						}
						else {
							write_phraseLCD(s_file, 4, 415, screen_position,
									0x0000, 0xFFFF);
						}
						++current_file;
						screen_position += 24;
						++total_files;
					}
				}
				/*
				 * What follows is proceeding for the case when there are less
				 * than 10 files in the directory. In that case we will simply
				 * indicate that the corresponding files don't exist in menu
				 * and clean the screen just in case something was written
				 * there before (for example, if we arrived there from parent
				 * directory which had 10 or more files). If there are 10 or
				 * more files in this directory, then this loop will not be
				 * executed.
				 */
				while (current_file < 10) {
					file_list[current_file].exists = 0;
					paint_areaLCD(0, screen_position, 239,
							screen_position + 23, 0xFFFF);
					paint_areaLCD(415, screen_position, 455,
							screen_position + 23, 0xFFFF);
					screen_position += 24;
					++current_file;
				}
				/*
				 * At this point we just have to read the remaining files. This
				 * is the only way to get the total number of files using
				 * Chan's generic FatFS module. We don't have to do this every
				 * time while we are in the same directory, but for now we'll
				 * do it this way because it is fast enough and simple too.
				 */
				while (proceed) {
					result = f_readdir(&directory, &file);
					if (file.fname[0] == 0 || result != FR_OK) {
						proceed = 0;
					}
					else ++total_files;
				}
				/*
				 * What follows are proceedings to draw a scroll bar, which
				 * will show, proportionally, the amount of files above, below
				 * and the 10 (or less) files displayed on screen. If there 10
				 * or less files, then 100% of files are shown and there is
				 * almost nothing to determine. Else we'll have to calculate
				 * the percentages.
				 */
				uint16_t empty_area1, filled_area, empty_area2;
				if (total_files <= 10) {
					empty_area1 = 0;
					filled_area = 192;
					empty_area2 = 0;
				}
				else {
					float percentage = (float)10/(float)total_files;
					percentage = (float)192*percentage;
					filled_area = (uint16_t)percentage;
					if (cursors[depth]) {
						percentage = (float)cursors[depth]/(float)total_files;
						percentage = (float)192*percentage;
						empty_area1 = (uint16_t)percentage;
					}
					else {
						empty_area1 = 0;
					}
				}
				if (total_files > 10 && cursors[depth] < total_files - 10)
					empty_area2 = 192 - empty_area1 - filled_area;
				else empty_area2 = 0;
				if (empty_area1) {
					paint_areaLCD(456, 56, 479, 56 + empty_area1 - 1, 0xFFFF);
					++empty_area1;
				}
				if (!empty_area2) ++filled_area;
				paint_areaLCD(456, 56 + empty_area1, 479,
						56 + empty_area1 + filled_area - 1, 0x0000);
				if (empty_area2) paint_areaLCD(456,
						56 + empty_area1 + filled_area, 479, 247, 0xFFFF);

				f_closedir(&directory);
			}
			new_order = 0;

		}
		/*
		 * The proceedings below change visual state of buttons. If they were
		 * pressed we should return them to initial state after the order was
		 * fully processed. Note that if user holds a button and, thus, issues
		 * orders repeatedly, this animation will not happen until he/she will
		 * stop holding the button.
		 */
		else {
			if (folder_up_button_pressed) {
				paint_imageLCD((uint16_t*)folder_up_image, folder_up.x_start, folder_up.y_start);
				folder_up_button_pressed = 0;
			}
			if (arrow_up_button_pressed) {
				paint_imageLCD((uint16_t*)arrow_up_image, arrow_up.x_start, arrow_up.y_start);
				arrow_up_button_pressed = 0;
			}
			if (arrow_down_button_pressed) {
				paint_imageLCD((uint16_t*)arrow_down_image, arrow_down.x_start, arrow_down.y_start);
				arrow_down_button_pressed = 0;
			}
		}
		/*
		 * Now we'll check if there is a new order from user. This is the only
		 * thing that this loop does repeatedly.
		 */
		if (detect_touch()) {
			/*
			 * If a touch is detected, we should check first if the data are
			 * ready to be fetched from STMPE610 touch-controller. If they are
			 * not, we'll try to read them again next time. We rely on the
			 * touch-controller quite a lot.
			 */
			size = get_fifo_touch_size();
			if (size > 0) {
				/*
				 * If the data re ready we fetch and convert them into actual
				 * pixel-measured data. If data can't be converted, because
				 * they are invalid, then do nothing and just clean the buffer.
				 */
				get_touch_data(&x, &y);
				if (convert_touch_data(&x, &y)) {
					/*
					 * We should determine what button, if any, was pressed and
					 * immediately act accordingly. We change the current
					 * parameters depending on user's action and we indicated
					 * that a new order is to be processed. Nothing else should
					 * be done about the order, the state machine will simply
					 * proceed according to now-current parameters and arrive
					 * by itself at a desired state. We also change the
					 * not-pressed button for pressed one and it will remain
					 * that way until the order will be completely processed.
					 */
					if ((x >= arrow_up.x_start) && (x <= arrow_up.x_end) &&
							(y >= arrow_up.y_start) && (y <= arrow_up.y_end)) {
						if (cursors[depth] > 0) {
							--cursors[depth];
							paint_imageLCD((uint16_t*)arrow_up_pressed_image,
									arrow_up.x_start, arrow_up.y_start);
							arrow_up_button_pressed = 1;
							new_order = 1;
						}
					}
					if ((x >= arrow_down.x_start) && (x <= arrow_down.x_end) &&
							(y >= arrow_down.y_start) &&
							(y <= arrow_down.y_end)) {
						if (cursors[depth] < total_files - 10) {
							++cursors[depth];
							paint_imageLCD((uint16_t*)arrow_down_pressed_image,
									arrow_down.x_start, arrow_down.y_start);
							arrow_down_button_pressed = 1;
							new_order = 1;
						}
					}
					if ((x >= files_menu.x_start) && (x <= files_menu.x_end) &&
							(y >= files_menu.y_start) &&
							(y <= files_menu.y_end)) {
						uint8_t selected_file = (y - files_menu.y_start)/files_menu.step;
						if (file_list[selected_file].exists) {
							if (file_list[selected_file].fattrib == AM_DIR) {
								if (depth == 49) {
									write_phraseLCD("Not enough memory to go so deep!",
											32, 0, 0, 0x0000, 0xFFFF);
									current_directory_path[0] = '.';
									current_directory_path[1] = 0;
								}
								else {
									++depth;
									mem_cpy(&visited_directories[depth][0],
											file_list[selected_file].fname,
											13);
									cursors[depth] = 0;
									current_directory_path[0] = '.';
									current_directory_path[1] = '/';
									mem_cpy(&current_directory_path[2],
											file_list[selected_file].fname,
											13);
								}
								if (f_chdir(current_directory_path) == FR_OK) {
									paint_areaLCD(24, 0, 200, 31, 0xFFFF);
									write_phraseLCD(&visited_directories[depth][0],
											13, 29, 0, 0x0000, 0xFFFF);
									new_order = 1;
									current_directory_path[0] = '.';
									current_directory_path[1] = 0;
								}
								else {
									write_phraseLCD("Error!", 6, 0, 0, 0x0000,
											0xFFFF);
									write_phraseLCD(current_directory_path, 13,
											100, 0, 0x0000, 0xFFFF);
								}
							}
							else {
								/*
								 * When user wants to open a file this is the
								 * only instance when we close the file
								 * manager.
								 */
								mem_cpy(target_file,
										file_list[selected_file].fname, 13);
								return OPEN_FILE;
							}
						}
					}
					if ((x >= folder_up.x_start) && (x <= folder_up.x_end) &&
							(y >= folder_up.y_start) &&
							(y <= folder_up.y_end)) {
						if (depth > 0) {
							--depth;
							current_directory_path[0] = '.';
							current_directory_path[1] = '/';
							current_directory_path[2] = '.';
							current_directory_path[3] = '.';
							current_directory_path[4] = 0;
							if (f_chdir(current_directory_path) == FR_OK) {
								paint_imageLCD((uint16_t*)folder_up_pressed_image,
										0, 0);
								folder_up_button_pressed = 1;
								paint_areaLCD(24, 0, 200, 31, 0xFFFF);
								write_phraseLCD(&visited_directories[depth][0],
										13, 29, 0, 0x0000, 0xFFFF);
								new_order = 1;
								current_directory_path[0] = '.';
								current_directory_path[1] = 0;
							}
							else {
								write_phraseLCD("Error!", 6, 0, 0, 0x0000,
										0xFFFF);
								write_phraseLCD(current_directory_path, 13,
										100, 0, 0x0000, 0xFFFF);
							}
						}
					}
				}
			}
		}
		reset_touch_fifo();
	}
	return NO_SDCARD;
}
