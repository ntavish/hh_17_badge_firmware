#ifndef __READ_PINS_H
#define __READ_PINS_H

struct touch_button
{
	uint32_t port;
	uint32_t pin;      //GPIO14
	uint32_t pin_num;  //14
	uint32_t rcc;
	const char *name;
};

#define BUTTON(port, pin, name)  {GPIO##port, GPIO##pin, pin, RCC_GPIO##port, name}

void pin_init(int index);
int read_pin(int index);
int num_buttons();

extern const struct touch_button tb_table[];

#endif
