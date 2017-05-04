#ifndef __READ_PINS_H
#define __READ_PINS_H

typedef enum
{
	NOT_PRESSED,
	PRESSED
}touch_state_t;

// can be digital/touch/pot.
struct input_pin
{
	uint32_t port;
	uint32_t pin;      //GPIO14
	uint32_t pin_num;  //14
	uint32_t rcc;
	const char *name;
};

#define INPUT(port, pin, name)  {GPIO##port, GPIO##pin, pin, RCC_GPIO##port, name}

void initialize_pins(void);
int read_pin(int index);
int num_touch_pins(void);
uint8_t * scan_buttons(void);

void adc_setup(void);
uint16_t read_pot(uint8_t pot);

extern const struct input_pin tb_table[];

#endif
