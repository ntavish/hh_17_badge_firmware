#ifndef __SERIAL__H
#define __SERIAL__H

#include "cdcacm-target.h"

#define serial_init()	init_cdcacm()

int serial_read(char *buf, int maxlen);
int serial_write(const char *buf, int len);

// call this from 
void cdc_acm_ready_cb(void);

#endif
