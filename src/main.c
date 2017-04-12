/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2014 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/crs.h>
#include <unicore-mx/stm32/syscfg.h>
#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/stm32/timer.h>
#include <unicore-mx/stm32/flash.h>
#include <string.h>
#include "serial.h"


#define PORT_LED GPIOB
#define PIN_LED  GPIO5
#define RCC_LED	 RCC_GPIOB


/* for driver */
const usbd_backend *cdcacm_target_usb_driver(void)
{
       return USBD_STM32_FSDEV_V2;
}

static void gpio_setup(void)
{
	gpio_mode_setup(PORT_LED, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, PIN_LED);
	gpio_set_output_options(PORT_LED, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, PIN_LED); // 100mhz means highspeed actually
	gpio_set_af(PORT_LED, GPIO_AF1, PIN_LED);
}

static void tim_setup(void)
{
	//timer_reset(TIM1);
	timer_set_prescaler(TIM3, 0xFF);
	//timer_set_clock_division(TIM1, 0xFF);
	timer_set_period(TIM3, 0xFFF);

	timer_continuous_mode(TIM3);
	timer_direction_up(TIM3);

	timer_disable_oc_output(TIM3, TIM_OC2);
	timer_set_oc_mode(TIM3, TIM_OC2, TIM_OCM_PWM1);
	timer_set_oc_value(TIM3, TIM_OC2, 0xFF);
	timer_enable_oc_output(TIM3, TIM_OC2);
	timer_enable_break_main_output(TIM3);
	timer_enable_preload(TIM3);
	timer_enable_counter(TIM3);
}



static void target_init(void)
{
	/* start HSI48 */
	rcc_clock_setup_in_hsi48_out_48mhz();

	/* use usb SOF as correction source for HSI48 */
	crs_autotrim_usb_enable();

	/* use HSI48 for USB */
	rcc_set_usbclk_source(RCC_HSI48);

	/* usb HSI48 for system clock */
	rcc_set_sysclk_source(RCC_HSI48);


	/* Enable TIM3 clock. */
	rcc_periph_clock_enable(RCC_TIM3);
	/* Enable PORT_LED, Alternate Function clocks. */
	rcc_periph_clock_enable(RCC_LED);
	
	gpio_setup();
	tim_setup();
}

int main(void)
{
	target_init();
	serial_init();
	
	for(uint8_t i='a';i<='z';i++)
	{
		serial_write(&i, 1);
	}
	
	serial_write((uint8_t*)"testing123", 10);
	
	while (1) 
	{
		//read_button();
		usbd_poll(_usbd_dev, 0);
	}
}

