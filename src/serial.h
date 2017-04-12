#ifndef __SERIAL__H
#define __SERIAL__H

#include "cdcacm-target.h"

#define serial_init()	init_cdcacm()

int serial_read(uint8_t *buf, int maxlen);
int serial_write(uint8_t *buf, int len);

#endif
