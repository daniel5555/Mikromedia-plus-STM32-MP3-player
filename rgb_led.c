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
 * LEDs:
 * On Mikromedia+ the LEDs are connected to the following pins:
 *
 * Red LED		-> GPIOG-Pin 15
 * Green LED 	-> GPIOB-Pin 3
 * Blue LED 	-> GPIOB-Pin 4
 *
 * Together they form a single RGB LED unit capable of generating many colors,
 * but they can also be used as separate LEDs. This code takes advantage of
 * their multicolor capacity, but the means of controlling those LEDs are the
 * same as in case of regular separate LEDs.
 *
 * If your LEDs are connected to different pins, you should change that in the
 * code accordingly.
 *
 * Let's define as cycle the minimal period of time during which a LED is
 * illuminating with specific brightness. During a cycle a LED can be in two
 * different states - turned on and turned off. Continuous cycles create an
 * illusion of a LED constantly turned on with specific brightness. The
 * brightness is defined by the amount of time that a LED is turned on during
 * a cycle. For example, if a LED is turned on 50% of a cycle, then it will be
 * illuminating with 50% of its maximum brightness. So, by changing the amount
 * of time during which a LED is turned on within a cycle, you can adjust its
 * overall brightness and create effects such as fading. You can also create
 * all sorts of colors with an RGB LED unit by adjusting the brightness of each
 * of 3 LEDs that make the unit.
 *
 * Naturally, the length of cycle must be small enough, otherwise the LED will
 * appear blinking very quickly rather than giving impression of being turned
 * on constantly. By default we use 10000 microseconds cycle.
 *
 * Important note on brightness:
 * The LEDs used on Mikromedia+ board appear to be way brighter than is safe
 * for eyes. They are literally blinding even when used just at half of their
 * full brightness. As such, the sane values for their brightness seem to be up
 * to 10% of their full capacity, and even at that point they seem to be very
 * bright. So, for the safety reasons this code never turns brightness to more
 * than 2,5% of full capacity.
 *
 * If LEDs on your board appear to be too dim for you, you should increase
 * brightness in this code until it is enough.
 */

#include <delay.h>
#include <rgb_led.h>

/*
 * This function initializes GPIO pins registers. We start by enabling their
 * peripheral clock and then filling their settings structure. STM32 allows
 * a separate configuration for each pin, which we use, since other pins in the
 * same group may be used for different purposes. If we need to, we can use the
 * same configuration for all pins in the group.
 */

void GPIOLED_Init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_InitTypeDef GPIOLEDs_settings;
	GPIOLEDs_settings.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
	GPIOLEDs_settings.GPIO_Mode = GPIO_Mode_OUT;
	GPIOLEDs_settings.GPIO_Speed = GPIO_Speed_2MHz;
	GPIOLEDs_settings.GPIO_OType = GPIO_OType_PP;
	GPIOLEDs_settings.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOB, &GPIOLEDs_settings);

	GPIOLEDs_settings.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOG, &GPIOLEDs_settings);
}

/*
 * This function will simply blink all LEDs on the board. Each LED will be
 * turned on for a second, separately and in group, and there is a 1 second
 * period in-between during which every LED is turned off. The most important
 * part is usage of Delay_us() to adjust the brightness of LEDs.
 */

void blink_LEDs() {
	int cycle_counter = 0;
	int intensity = DEFAULT_INTENSITY;

	while (cycle_counter < 100) {
	    Delay_us(CYCLE_LENGHT - intensity);
	    TurnOn_GreenLED();
	    Delay_us(intensity);
	    TurnOff_GreenLED();
	    ++cycle_counter;
	}

	cycle_counter = 0;
	Delay_ms(1000);

	while (cycle_counter < 100) {
	    Delay_us(CYCLE_LENGHT - intensity);
	    TurnOn_GreenLED();
	    TurnOn_RedLED();
	    Delay_us(intensity);
	    TurnOff_GreenLED();
	    TurnOff_RedLED();
	    ++cycle_counter;
	}

	cycle_counter = 0;
	Delay_ms(1000);

	while (cycle_counter < 100) {
	    Delay_us(CYCLE_LENGHT - intensity);
	    TurnOn_RedLED();
	    Delay_us(intensity);
	    TurnOff_RedLED();
	    ++cycle_counter;
	}

	cycle_counter = 0;
	Delay_ms(1000);

	while (cycle_counter < 100) {
	    Delay_us(CYCLE_LENGHT - intensity);
	    TurnOn_BlueLED();
	    TurnOn_RedLED();
	    Delay_us(intensity);
	    TurnOff_BlueLED();
	    TurnOff_RedLED();
	    ++cycle_counter;
	}

	cycle_counter = 0;
	Delay_ms(1000);

	while (cycle_counter < 100) {
	    Delay_us(CYCLE_LENGHT - intensity);
	    TurnOn_BlueLED();
	    Delay_us(intensity);
	    TurnOff_BlueLED();
	    ++cycle_counter;
	}

	cycle_counter = 0;
	Delay_ms(1000);

	while (cycle_counter < 100) {
	    Delay_us(CYCLE_LENGHT - intensity);
	    TurnOn_GreenLED();
	    TurnOn_BlueLED();
	    Delay_us(intensity);
	    TurnOff_GreenLED();
	    TurnOff_BlueLED();
	    ++cycle_counter;
	}

	cycle_counter = 0;
	Delay_ms(1000);

	while (cycle_counter < 100) {
	    Delay_us(CYCLE_LENGHT - intensity);
	    TurnOn_GreenLED();
	    TurnOn_BlueLED();
	    TurnOn_RedLED();
	    Delay_us(intensity);
	    TurnOff_GreenLED();
	    TurnOff_BlueLED();
	    TurnOff_RedLED();
	    ++cycle_counter;
	}

	cycle_counter = 0;
	Delay_ms(1000);
}

/*
 * This function shows how to dim the LEDs linearly. That is, it will dim the
 * LEDs over a second period and the brightness will be decreasing linearly
 * the whole time. It is achieved by decreasing brightness each cycle by a
 * fixed value.
 */

void dim_LEDs() {
	int cycle_counter = 0;
	int intensity = DEFAULT_INTENSITY;

	while (cycle_counter < 100) {
		Delay_us(CYCLE_LENGHT - intensity);
		TurnOn_GreenLED();
		Delay_us(intensity);
		TurnOff_GreenLED();
		++cycle_counter;
		intensity -= DEFAULT_INTENSITY/100;
	}

	cycle_counter = 0;
	intensity = DEFAULT_INTENSITY;
	Delay_ms(1000);

	while (cycle_counter < 100) {
		Delay_us(CYCLE_LENGHT - intensity);
		TurnOn_BlueLED();
		Delay_us(intensity);
		TurnOff_BlueLED();
		++cycle_counter;
		intensity -= DEFAULT_INTENSITY/100;
	}

	cycle_counter = 0;
	intensity = DEFAULT_INTENSITY;
	Delay_ms(1000);

	while (cycle_counter < 100) {
		Delay_us(CYCLE_LENGHT - intensity);
		TurnOn_RedLED();
		Delay_us(intensity);
		TurnOff_RedLED();
		++cycle_counter;
		intensity -= DEFAULT_INTENSITY/100;
	}

	intensity = DEFAULT_INTENSITY;
	cycle_counter = 0;
	Delay_ms(1000);

	while (cycle_counter < 100) {
		Delay_us(CYCLE_LENGHT - intensity);
		TurnOn_GreenLED();
		TurnOn_BlueLED();
		TurnOn_RedLED();
		Delay_us(intensity);
		TurnOff_GreenLED();
		TurnOff_BlueLED();
		TurnOff_RedLED();
		++cycle_counter;
		intensity -= DEFAULT_INTENSITY/100;
	}

	Delay_ms(1000);
}

/*
 * This function shows how to light up LEDs increasing their brightness
 * non-linearly. This is achieved by increasing brightness value each cycle by
 * a value that is getting bigger over time. In this particular case, it's
 * getting bigger every 100 cycles. The cycle_counter%4 == 0 condition ensures
 * that the final brightness will not exceed 2,5% for eye safety reasons.
 */

void lightUp_LEDs() {
	int cycle_counter = 0;
	int intensity = 0;

	while (cycle_counter < 500) {
		Delay_us(CYCLE_LENGHT - intensity);
		TurnOn_GreenLED();
		Delay_us(intensity);
		TurnOff_GreenLED();
		++cycle_counter;
		if (cycle_counter%4 == 0) intensity += cycle_counter/100;
	}

	cycle_counter = 0;
	intensity = 0;
	Delay_ms(1000);

	while (cycle_counter < 500) {
		Delay_us(CYCLE_LENGHT - intensity);
		TurnOn_BlueLED();
		Delay_us(intensity);
		TurnOff_BlueLED();
		++cycle_counter;
		if (cycle_counter%4 == 0) intensity += cycle_counter/100;
	}

	cycle_counter = 0;
	intensity = 0;
	Delay_ms(1000);

	while (cycle_counter < 500) {
		Delay_us(CYCLE_LENGHT - intensity);

		TurnOn_RedLED();
		Delay_us(intensity);
		TurnOff_RedLED();
		++cycle_counter;
		if (cycle_counter%4 == 0) intensity += cycle_counter/100;
	}

	cycle_counter = 0;
	intensity = 0;
	Delay_ms(1000);

	while (cycle_counter < 500) {
		Delay_us(CYCLE_LENGHT - intensity);
		TurnOn_GreenLED();
		TurnOn_BlueLED();
		TurnOn_RedLED();
		Delay_us(intensity);
		TurnOff_GreenLED();
		TurnOff_BlueLED();
		TurnOff_RedLED();
		++cycle_counter;
		if (cycle_counter%4 == 0) intensity += cycle_counter/100;
	}

	Delay_ms(1000);
}

/*
 * This function shows how to make different colors using varying amount of
 * brightness of each LED. The trick is that we should never exceed the cycle
 * length time and that there can be time period over which all 3 LEDs are
 * turned on, a period when only 2 LEDs are turned on and a period when only 1
 * LED (that has maximum brightness) is turned on. So you only need to
 * determine the brightness of each of LEDs involved and turn them off at the
 * appropriate moment.
 *
 * Note: It only works on RGB LED unit, like the one that Mikromedia+ board
 * has.
 */

void Rainbow() {
	int intensityRed = 0;
	int intensityGreen = 0;
	int intensityBlue = 0;
	int cycle_counter = 0;
	int rainbows = 0;

	while (cycle_counter < 100) {
		Delay_us(CYCLE_LENGHT - intensityRed);
		TurnOn_RedLED();
		Delay_us(intensityRed);
		TurnOff_RedLED();
		++cycle_counter;
		intensityRed += INTESITY_STEP;
	}

	while(rainbows < 10) {
		cycle_counter = 0;

		while (cycle_counter < 100) {
			Delay_us(CYCLE_LENGHT - intensityRed);
			TurnOn_GreenLED();
			TurnOn_RedLED();
			Delay_us(intensityGreen);
			TurnOff_GreenLED();
			Delay_us(intensityRed - intensityGreen);
			TurnOff_RedLED();
			++cycle_counter;
			intensityGreen += INTESITY_STEP;
		}

		cycle_counter = 0;

		while (cycle_counter < 100) {
			Delay_us(CYCLE_LENGHT - intensityGreen);
			TurnOn_GreenLED();
			TurnOn_RedLED();
			Delay_us(intensityRed);
			TurnOff_RedLED();
			Delay_us(intensityGreen - intensityRed);
			TurnOff_GreenLED();
			++cycle_counter;
			intensityRed -= INTESITY_STEP;
		}

		cycle_counter = 0;

		while (cycle_counter < 100) {
			Delay_us(CYCLE_LENGHT - intensityGreen);
			TurnOn_BlueLED();
			TurnOn_GreenLED();
			Delay_us(intensityBlue);
			TurnOff_BlueLED();
			Delay_us(intensityGreen - intensityBlue);
			TurnOff_GreenLED();
			++cycle_counter;
			intensityBlue += INTESITY_STEP;
		}

		cycle_counter = 0;

		while (cycle_counter < 100) {
			Delay_us(CYCLE_LENGHT - intensityBlue);
			TurnOn_BlueLED();
			TurnOn_GreenLED();
			Delay_us(intensityGreen);
			TurnOff_GreenLED();
			Delay_us(intensityBlue - intensityGreen);
			TurnOff_BlueLED();
			++cycle_counter;
			intensityGreen -= INTESITY_STEP;
		}

		cycle_counter = 0;

		while (cycle_counter < 100) {
			Delay_us(CYCLE_LENGHT - intensityBlue);
			TurnOn_RedLED();
			TurnOn_BlueLED();
			Delay_us(intensityRed);
			TurnOff_RedLED();
			Delay_us(intensityBlue - intensityRed);
			TurnOff_BlueLED();
			++cycle_counter;
			intensityRed += INTESITY_STEP;
		}

		cycle_counter = 0;

		while (cycle_counter < 100) {
			Delay_us(CYCLE_LENGHT - intensityRed);
			TurnOn_RedLED();
			TurnOn_BlueLED();
			Delay_us(intensityBlue);
			TurnOff_BlueLED();
			Delay_us(intensityRed - intensityBlue);
			TurnOff_RedLED();
			++cycle_counter;
			intensityBlue -= INTESITY_STEP;
		}
		++rainbows;
	}
}
