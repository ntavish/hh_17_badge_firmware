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
//#include "serial.h"
#include "read_pins.h"
#include "usbmidi-target.h"

#define PORT_LED GPIOB
#define PIN_LED  GPIO5
#define RCC_LED	 RCC_GPIOB

//#define USBCDC
#define USBMIDI

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

#if defined(USBCDC)
/* for driver */
const usbd_backend *cdcacm_target_usb_driver(void)
{
       return USBD_STM32_FSDEV_V2;
}

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

#else
const usbd_backend *usbmidi_target_usb_driver(void)
{
       return USBD_STM32_FSDEV_V2;
}

void usbmidi_target_data_rx_cb(void)
{
}

static bool error_recoverable(usbd_transfer_status status)
{
	switch (status) {
	case USBD_ERR_TIMEOUT:
	case USBD_ERR_IO:
	case USBD_ERR_BABBLE:
	case USBD_ERR_DTOG:
	case USBD_ERR_SHORT_PACKET:
	case USBD_ERR_OVERFLOW:
	return true;

	case USBD_ERR_RES_UNAVAIL:
	case USBD_SUCCESS:
	case USBD_ERR_SIZE:
	case USBD_ERR_CONN:
	case USBD_ERR_INVALID:
	case USBD_ERR_CONFIG_CHANGE:
	case USBD_ERR_CANCEL:
	default:
	return false;
	}
}

static volatile usbd_urb_id button_event_urb_id = USBD_INVALID_URB_ID;

static void button_send_event_callback(usbd_device *usbd_dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		if (error_recoverable(status)) {
			button_event_urb_id = usbd_transfer_submit(usbd_dev, transfer);
			return;
		}
	}

	button_event_urb_id = USBD_INVALID_URB_ID;
}

static void button_send_event(usbd_device *usbd_dev, int pressed, int button)
{
	static char buf[4] = { 0x08, /* USB framing: virtual cable 0, note on */
			0x80, /* MIDI command: note on, channel 1 */
			60,   /* Note 60 (middle C) */
			64,   /* "Normal" velocity */
	};

	buf[0] |= pressed;
	buf[1] |= pressed << 4;
	
	buf[2] = 60+button;

	if (button_event_urb_id != USBD_INVALID_URB_ID) {
		/* already in progress */
		return;
	}

	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = 0x81,
		.ep_size = 64,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = buf,
		.length = sizeof(buf),
		.flags = USBD_FLAG_NONE,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = button_send_event_callback
	};

	button_event_urb_id = usbd_transfer_submit(usbd_dev, &transfer);
}

/* Called when systick fires */
void sys_tick_handler(void)
{
	int n = num_touch_pins();
	for(int i=0; i<n; i++)
	{
		if(read_pin(i) > 10)
		{
			button_send_event(_usbd_dev, 1, i);
		}
	}
	
	uint16_t potval = read_pot(0);
}

#endif

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
	//serial_init();
	adc_setup();
	
	init_usbmidi();
	
	systick_setup(500);
	
	while (1) 
	{
		//read_button();
		usbd_poll(_usbd_dev, 10);
	}
}

