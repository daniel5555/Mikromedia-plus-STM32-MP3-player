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

#include <utils.h>

uint8_t SDCard_present() {
	return !GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3);
}

/*
 * Function to check whether or not the specified file has a specified
 * extension. Basically, we use it to determine which file we can open and
 * which we can not. The length must include the dot, the first symbol of any
 * extension.
 */
uint8_t check_extension(char *filename, char* extension, uint8_t extension_length) {
	uint8_t i;
	for(i = 0; i < 13 && filename[i] != 0; ++i);
	if (i != 13) {
		i -= extension_length;
		uint8_t j = 0;
		while (extension_length > 0) {
			if (filename[i] != extension[j]) return 0;
			else {
				++i;
				++j;
				--extension_length;
			}
		}
		return 1;
	}
	else {
		return 0;
	}
}

/*
 * Function to copy the contents of a memory buffer into another memory buffer.
 * It is taken from Chan's FS module.
 */
void mem_cpy (void* dst, void* src, uint32_t cnt) {
	char *d = (char*)dst;
	char *s = (char*)src;

	while (cnt--)
		*d++ = *s++;
}

int mem_cmp (void* dst, void* src, uint32_t cnt) {
	const char *d = (char *)dst, *s = (char *)src;
	int r = 0;

	while (cnt-- && (r = *d++ - *s++) == 0) {
		if (*d == 0) break;
	}
	return r;
}

void itoa_time_segment(uint8_t number, char* ascii) {
	uint8_t first_digit = number / 10;
	uint8_t second_digit = number % 10;
	ascii[0] = first_digit + 48;
	ascii[1] = second_digit + 48;
}

/*
 * An itoa function for unsigned 16 bits integers. The size of "ascii" should
 * be at least 5. The range of possible values is from 0 to 65,535 for unsigned
 * integer.
 */
void itoa16bits(uint16_t number, char* ascii) {
	int i, j, k;
	i = 0;
	for (k = 10000; k > 10; k /= 10) {
		j = number/k;
		ascii[i] = j + 48;
		++i;
		number %= k;
	}
	j = number/k;
	ascii[i] = j + 48;
	++i;
	j = number%k;
	ascii[i] = j + 48;
}

/*
 * An itoa function for unsigned 32 bits integers. The size of "ascii" should
 * be at least 10. The range of possible values is from 0 to 4,294,967,295 for
 * unsigned integer.
 */
void itoa32bits(uint32_t number, char* ascii) {
	int i, j, k;
	i = 0;
	for (k = 1000000000; k > 10; k /= 10) {
		j = number/k;
		ascii[i] = j + 48;
		++i;
		number %= k;
	}
	j = number/k;
	ascii[i] = j + 48;
	++i;
	j = number%k;
	ascii[i] = j + 48;
}

int is_it_audio(char* filename) {
	return (check_extension(filename, ".WAV", 4) ||
			check_extension(filename, ".MP3", 4) ||
			check_extension(filename, ".FLA", 4) ||
			check_extension(filename, ".WMA", 4) ||
			check_extension(filename, ".M4A", 4));
}
