/*
 * Copyright (c) 2013, Daniel Flores Tafur
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
 * We use the cycle length of 10000 microseconds. The default intensity value,
 * as you can see in rgb_led.c, means that the LED will be turned on during 100
 * microseconds within a cycle. This means that its brightness will be 1% of
 * its full capacity. If that's too dim on your board, feel free to augment it.
 *
 * Note: if you set it below 100, some functions will not work properly and
 * you'll also have to change their code.
 */

#ifndef RGB_LED_H
#define RGB_LED_H

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx.h>

#define CYCLE_LENGHT 10000
#define DEFAULT_INTENSITY 100
#define INTESITY_STEP 1

/*
 * Let's use some macros to make the code more readable.
 */

#define TurnOn_RedLED() GPIO_WriteBit(GPIOG, GPIO_Pin_15, 1)
#define TurnOn_GreenLED() GPIO_WriteBit(GPIOB, GPIO_Pin_3, 1)
#define TurnOn_BlueLED() GPIO_WriteBit(GPIOB, GPIO_Pin_4, 1)

#define TurnOff_RedLED() GPIO_WriteBit(GPIOG, GPIO_Pin_15, 0)
#define TurnOff_GreenLED() GPIO_WriteBit(GPIOB, GPIO_Pin_3, 0)
#define TurnOff_BlueLED() GPIO_WriteBit(GPIOB, GPIO_Pin_4, 0)

void GPIOLED_Init();
void blink_LEDs();
void dim_LEDs();
void lightUp_LEDs();
void Rainbow();

#endif /* RGB_LED_H */
