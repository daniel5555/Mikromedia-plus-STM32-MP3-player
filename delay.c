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
 * TIMERS:
 * In order to use timers we need to understand a bit how the whole clocking
 * system works on STM. It's a bit complicated. In our case we use the internal
 * oscillator that generates so called HSI clock signal. The frequency of that
 * signal is always 16 Mhz, however using multipliers it can be "accelerated"
 * up to 168 Mhz, so the CPU will actually be working at that frequency. Now,
 * not all of the system can actually work at that frequency, instead each
 * sub-system has its own maximum frequency. We will be using general-purpose
 * timers and those are clocked from APB1 Peripheral clock which has maximum
 * frequency of just 42 Mhz. So, it has to use a prescaler, which is set in
 * stm32f4xx_Startup.c with RCC_PCLK1Config(RCC_HCLK_Div4) function. It will
 * prescale 168 Mhz down to 42 Mhz (168/4 = 42). You can not set it higher than
 * this, but you can set it lower if you prefer (using RCC_HCLK_Div8 or
 * RCC_HCLK_Div16 instead of RCC_HCLK_Div4 which will prescale it to 21 and
 * 10.5 Mhz respectively). Now, when the value of APB1 prescaler is not 1
 * (RCC_HCLK_Div1) the frequency of timer's clock will be multiplied by 2. Yes,
 * it's a little bit confusing. So, in our case since we prescale the APB1
 * to 42 Mhz with RCC_HCLK_Div4, the timer's clock frequency will be 84 Mhz. If
 * we had used RCC_HCLK_Div8 it would be 42 Mhz instead and, similarly, 21 Mhz if
 * we had used RCC_HCLK_Div16. This only applies to general-purpose timers.
 * Those timers are TIM2 to TIM5. For the frequencies of other timers and the
 * rest of the system, consult the manual.
 *
 * Each timer has an associated counter which we can freely access to read and
 * write into it. Every time a timer makes a tick this counter gets increased
 * by one. So if we want to measure 10 "ticks" we can simply make a loop that
 * consults the counter and once it arrives to 10 we can exit the loop and
 * perform some action. How often a timer makes a tick depends on its
 * frequency. If, say, it has a frequency of 42 Mhz, it will make 42000000
 * ticks per second.
 *
 * Measuring with ticks, however, may be uncomfortable and often impractical,
 * since counters are not infinite. In fact, they are either 16 or 32 bits,
 * and the maximum number a 16 bit counter can hold is 65535. Fortunately, each
 * timer also has its personal prescaler. It acts as an additional counter in
 * the following way - you set there a value and the main counter only gets
 * increased when this secondary counter is filled from regular frequency. The
 * secondary counter then restarts automatically. This allows you, for example,
 * to make the timer tick each microsecond by setting the prescaler to 42 if
 * the timer's frequency is 42 Mhz.
 *
 * Delay functions:
 * We use timers to implement delay functions, Delay_ms(), which will allow us
 * to turn off the LEDs or maintain them turned on for a fixed period of time
 * and Delay_us(), which will be used to adjust LED's brightness.
 *
 * There are different ways to implement delay functions. For example, instead
 * of timers you can use SysTick functionality. Or you could use interrupts.
 * Finally, for delaying for a short period of time (microseconds) you could
 * simply use a loop, that is, you would actually delay by executing some fixed
 * amount of instructions, performing some useless action. Here, for the sake
 * of simplicity, we will use timers, but I'm not claiming that this is the
 * best way to actually implement a delay function.
 *
 * Delaying for a precise amount of time:
 * One of the challenges of creating a delay function is to make it precise
 * enough to perform a necessary task. Knowing the frequency of a timer clock
 * we can measure precisely the amount of time we need and we can also use a
 * specific prescaler for a timer to do that comfortable. For example, if we
 * want to measure a certain amount of milliseconds and we know that the
 * timer's frequency is 42 Mhz, by prescaling it by 42000, we will effectively
 * make it measure milliseconds with each tick. We can define it with
 * TIM_Prescaler parameter.
 *
 * However, there are certain complications that make those measurements less
 * precise. First of all, the frequency of timer's clock and every other clock
 * in the system is imprecise. It depends on manufacturing process, so it
 * varies slightly on different MCUs (normally, within +/-1% range). It also
 * varies with the ambient temperature and there is only so much you can do
 * about that. It is very difficult to obtain an exact frequency number and
 * it's not feasible with internal oscillators like the one in this MCU.
 *
 * However, there is also another major factor which is instructions overhead.
 * The instructions, even the modest amount of them, may create a significant
 * overhead, even if you are just consulting the timer's counter. So if you use
 * a timer clocked at 42 Mhz and prescaled with 42000 to measure milliseconds,
 * in practice the MCU won't be able to leave from the waiting loop exactly
 * after a millisecond, instead it'll leave slightly delayed. If you want to
 * wait a whole second by waiting for 1000 milliseconds that delay will be
 * pretty big and definitely perceivable on user's side. So instead of 42000
 * value for prescaler, you will have to use another value which will give you
 * just the necessary amount of time to make a delay you really need, taking
 * into account the particular properties of your code and MCU. You can
 * discover this amount only by experimentation.
 *
 * In this example, I'm using a 84 Mhz timers and a prescaler values that
 * worked well on my board. This values may be slightly or not slightly
 * different on your MCU, if you'll use those functions in your software.
 */

#include <delay.h>

/*
 * In this example I use specifically TIM3 and TIM4 timers. The reason I use
 * them is that they both are 16 bit timers while the rest of general purpose
 * timers are 32 bits. Let's save them for something more important.
 *
 * If we use 16 bit timers, we'll have to use 2 of them, each for a different
 * delay function, because they should have a different prescaler to measure
 * microseconds and milliseconds.
 *
 * Now, instead of 2 16 bit timers it's actually possible to use 1 32 bit timer
 * and translate milliseconds into microseconds inside Delay_ms() function.
 * However, I prefer not to mess too much with instructions in this example.
 * Still, it could be wiser to use 1 32 bit timer in some cases.
 *
 * Note: CNT holds a counter value. We should always set it to 0 when we enter
 * the function, because the timers are enabled the whole time and they are
 * counting outside the functions. Then we simply wait until the counter
 * reaches a desired value.
 */

void Delay_ms(uint16_t value)
{
  TIM3->CNT = 0;
  while((uint16_t)(TIM3->CNT) <= value);
}

void Delay_us(uint16_t value)
{
  TIM4->CNT = 0;
  while((uint16_t)(TIM4->CNT) <= value);
}

/*
 * This function initializes the timers in a very similar way that is used to
 * initialize the GPIO pins. The smaller TIM_Prescaler value is used to measure
 * microseconds and the bigger to measure milliseconds.
 */

void Timers_Init() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseInitTypeDef TimerSettings;
	TimerSettings.TIM_Prescaler = 59;
	TimerSettings.TIM_Period = UINT16_MAX;
	TimerSettings.TIM_ClockDivision = TIM_CKD_DIV1;
	TimerSettings.TIM_CounterMode = TIM_CounterMode_Up;
	TimerSettings.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM4, &TimerSettings);

	TimerSettings.TIM_Prescaler = 59250;

	TIM_TimeBaseInit(TIM3, &TimerSettings);

	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}
