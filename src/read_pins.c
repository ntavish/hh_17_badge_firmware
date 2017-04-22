#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/cm3/cortex.h>
#include "read_pins.h"

const struct touch_button tb_table[] = {
	BUTTON(A, 0, "C"),
	BUTTON(A, 1, "C#"),
	BUTTON(A, 2, "D"),
	BUTTON(A, 3, "D#"),
	BUTTON(A, 6, "E"),
	BUTTON(A, 7, "F"),
	BUTTON(A, 9, "F#"),
	BUTTON(A, 10, "G"),
	BUTTON(B, 13, "G#"),
	BUTTON(B, 14, "A"),
	BUTTON(B, 2, "A#"),
	BUTTON(B, 3, "B"),
	BUTTON(B, 4, "C+"),
};

int num_buttons()
{
	return sizeof(tb_table)/sizeof(tb_table[0]);
}

void pin_init(int index)
{
	const struct touch_button* tb = &tb_table[index];
	rcc_periph_clock_enable(tb->rcc);
	gpio_mode_setup(tb->port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, tb->pin);
	gpio_set_output_options(tb->port, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, tb->pin);
}

int read_pin(int index)
{
	int cycles;

	const struct touch_button* tb = &tb_table[index];
	
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
