#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/f0/adc.h>
#include <unicore-mx/cm3/cortex.h>
#include "read_pins.h"

// Touch buttons
const struct input_pin tb_table[] = {
	INPUT(A, 0, "C"),
	INPUT(A, 1, "C#"),
	INPUT(A, 2, "D"),
	INPUT(A, 3, "D#"),
	INPUT(A, 6, "E"),
	INPUT(A, 7, "F"),
	INPUT(A, 9, "F#"),
	INPUT(A, 10, "G"),
	INPUT(B, 13, "G#"),
	INPUT(B, 14, "A"),
	INPUT(B, 2, "A#"),
	INPUT(B, 3, "B"),
	INPUT(B, 4, "C+"),
	INPUT(B, 11, "B1"),
	INPUT(B, 12, "B2"),
};

// States of touch buttons
volatile touch_state_t tb_states[] = {
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
	NOT_PRESSED,
};

int num_touch_pins(void)
{
	return sizeof(tb_table)/sizeof(tb_table[0]);
}

static void touch_pin_init(int index)
{
	const struct input_pin* tb = &tb_table[index];
	rcc_periph_clock_enable(tb->rcc);
	gpio_mode_setup(tb->port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, tb->pin);
	gpio_set_output_options(tb->port, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, tb->pin);
}

void initialize_pins(void)
{
	// For touch pins
	int n = num_touch_pins();
	for(int i=0; i<n; i++)
	{
		touch_pin_init(i);
	}
	
	// for pots, init adc
	rcc_periph_clock_enable(RCC_ADC);
}

int read_pin(int index)
{
	int cycles;

	const struct input_pin* tb = &tb_table[index];
	
	gpio_mode_setup(tb->port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, tb->pin);
	gpio_set_output_options(tb->port, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, tb->pin);
	
	gpio_clear(tb->port, tb->pin);
	
	__asm__("nop");
	__asm__("nop");
	__asm__("nop");
	__asm__("nop");
	__asm__("nop");
	
	// following code is equivalent to below 2 commented lines, faster
	//gpio_set_output_options(tb->port, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, tb->port);
	//gpio_mode_setup(tb->port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, tb->pin);
	
	uint32_t moder = GPIO_MODER(tb->port);
	moder &= ~GPIO_MODE_MASK(tb->pin_num);
	moder |= GPIO_MODE(tb->pin_num, GPIO_MODE_INPUT);
	
	uint32_t pupd = GPIO_PUPDR(tb->port);
	pupd &= ~GPIO_PUPD_MASK(tb->pin_num);
	pupd |= GPIO_PUPD(tb->pin_num, GPIO_PUPD_PULLUP);
	
	//disable irqs
	cycles = 25;
	// measure how long it takes to rise to high
	uint32_t pin = tb->pin;
	uint32_t port = tb->port;
	CM_ATOMIC_BLOCK() 
	{
		// actual mode change
		GPIO_MODER(tb->port) = moder;
		GPIO_PUPDR(tb->port) = pupd;
		
		if(GPIO_IDR(port) & pin) { cycles = 0; }
		else if(GPIO_IDR(port) & pin) { cycles = 1; }
		else if(GPIO_IDR(port) & pin) { cycles = 2; }
		else if(GPIO_IDR(port) & pin) { cycles = 3; }
		else if(GPIO_IDR(port) & pin) { cycles = 4; }
		else if(GPIO_IDR(port) & pin) { cycles = 5; }
		else if(GPIO_IDR(port) & pin) { cycles = 6; }
		else if(GPIO_IDR(port) & pin) { cycles = 7; }
		else if(GPIO_IDR(port) & pin) { cycles = 8; }
		else if(GPIO_IDR(port) & pin) { cycles = 9; }
		else if(GPIO_IDR(port) & pin) { cycles = 10; }
		else if(GPIO_IDR(port) & pin) { cycles = 11; }
		else if(GPIO_IDR(port) & pin) { cycles = 12; }
		else if(GPIO_IDR(port) & pin) { cycles = 13; }
		else if(GPIO_IDR(port) & pin) { cycles = 14; }
		else if(GPIO_IDR(port) & pin) { cycles = 15; }
		else if(GPIO_IDR(port) & pin) { cycles = 16; }
		else if(GPIO_IDR(port) & pin) { cycles = 17; }
		else if(GPIO_IDR(port) & pin) { cycles = 18; }
		else if(GPIO_IDR(port) & pin) { cycles = 19; }
		else if(GPIO_IDR(port) & pin) { cycles = 20; }
		else if(GPIO_IDR(port) & pin) { cycles = 21; }
		else if(GPIO_IDR(port) & pin) { cycles = 22; }
		else if(GPIO_IDR(port) & pin) { cycles = 23; }
		else if(GPIO_IDR(port) & pin) { cycles = 24; }
		else if(GPIO_IDR(port) & pin) { cycles = 25; }
	}
	
	// NOTE: may need to set this back to 0, to not affect others
	
	return cycles;
}


// POTS

// pots
const struct input_pin pb_table[] = {
	INPUT(B, 0, "PT1"),
	INPUT(B, 1, "PT2"),
};

volatile int pot_states[] = {
	0,
	0,
};

void adc_setup(void)
{
	rcc_periph_clock_enable(RCC_ADC);
	rcc_periph_clock_enable(RCC_GPIOB);

	gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);
	
	adc_power_off(ADC1);
	adc_set_clk_source(ADC1, ADC_CLKSOURCE_ADC);
	rcc_periph_reset_pulse(RST_ADC1);
	adc_calibrate_start(ADC1);
	adc_calibrate_wait_finish(ADC1);
	
	adc_set_operation_mode(ADC1, ADC_MODE_SCAN);
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPTIME_071DOT5);
	uint8_t channel_array[] = {9};
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_set_resolution(ADC1, ADC_RESOLUTION_12BIT);
	adc_disable_analog_watchdog(ADC1);
	adc_power_on(ADC1);

	/* Wait for ADC starting up. */
	int i;
	for (i = 0; i < 800000; i++)    /* Wait a bit. */
		__asm__("nop");
}

uint16_t read_pot(uint8_t pot)
{
	adc_start_conversion_regular(ADC1);
	while (! adc_eoc(ADC1));
	return adc_read_regular(ADC1);
}

