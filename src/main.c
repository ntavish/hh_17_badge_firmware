#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/crs.h>
#include <unicore-mx/stm32/timer.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/cm3/systick.h>
#include <string.h>
#include "serial.h"
#include "read_pins.h"

#define PORT_LED GPIOB
#define PIN_LED  GPIO5
#define RCC_LED	 RCC_GPIOB


/* Called when systick fires */
void sys_tick_handler(void)
{
	int n = num_touch_pins();
	for(int i=0; i<n; i++)
	{
		if(read_pin(i) > 10)
		{
			serial_write(tb_table[i].name, strlen(tb_table[i].name));
			serial_write("\r\n", 2);
		}
	}
	
	uint16_t potval = read_pot(0);
	char buf[20];
	
	snprintf(buf, sizeof(buf), "pot: %u\r\n", potval);
	serial_write(buf, strnlen(buf, 20));
}

void usb_ready_cb(void)
{
	cdc_acm_ready_cb();
}

void usb_poll_cb()
{
	usbd_poll(_usbd_dev, 10);
}

/*
 * Set up timer to fire every x milliseconds
 * This is a unusual usage of systick, be very careful with the 24bit range
 * of the systick counter!  You can range from 1 to 2796ms with this.
 */
static void systick_setup(int xms)
{
	/* div8 per ST, stays compatible with M3/M4 parts, well done ST */
	systick_set_clocksource(STK_CSR_CLKSOURCE_EXT);
	/* clear counter so it starts right away */
	STK_CVR = 0;

	systick_set_reload(rcc_ahb_frequency / 8 / 1000 * xms);
	systick_counter_enable();
	
	// enable after config descriptors have been sent
	systick_interrupt_enable();
}


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
	
	initialize_pins();
}

int main(void)
{
	target_init();
	serial_init();
	adc_setup();
	
	systick_setup(500);
	
	while (1) 
	{
		//read_button();
		usb_poll_cb();
	}
}

