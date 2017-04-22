#include "serial.h"
#include "circular_buf.h"
#include "cdcacm-target.h"

#define RECV_BUF_SIZE	128
uint8_t recv_buf_space[RECV_BUF_SIZE];
CIRCBUF_DEF(recv_buf, RECV_BUF_SIZE);

#define TX_BUF_SIZE		128
uint8_t tx_buf_space[TX_BUF_SIZE];
CIRCBUF_DEF(tx_buf, TX_BUF_SIZE);

static volatile uint8_t tx_in_progress = 0;

static uint8_t serial_ready = 0;
static void set_cb(void);

void cdc_acm_ready_cb()
{
	serial_ready = 1;
	set_cb();
}

static void tx_cb(usbd_device *usbd_dev,
	const usbd_transfer *transfer, usbd_transfer_status status,
	usbd_urb_id urb_id)
{
	(void) urb_id;
	(void) usbd_dev;

	if (status == USBD_SUCCESS) {
		if (transfer->transferred) {
			//tx_to_host(usbd_dev, transfer->buffer, transfer->transferred);
			set_cb();
		}
	}

	/* this was only found in f1/lisa-m-1/usb_cdcacm.c */
	cdcacm_target_data_rx_cb_before_return();
}

static void serial_transfer(uint8_t *data, int len)
{
	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = 0x82,
		.ep_size = 64,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = data,
		.length = len,
		.flags = USBD_FLAG_SHORT_PACKET,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = tx_cb
	};
	usbd_transfer_submit(_usbd_dev, &transfer);
}

static void set_cb()
{
	// set up RX / TX callbacks now
	uint8_t data;
	
	if(!serial_ready)
	{
		return;
	}
	
	if(circBufPop(&tx_buf, &data) == 0)
	{
		tx_in_progress = 1;
		serial_transfer(&data, 1);
	}
	else
	{
		tx_in_progress = 0;
	}
}

int serial_write(const char *buf, int len)
{
	int i;
	for(i=0; i<len; i++)
	{
		circBufPush(&tx_buf, (uint8_t)buf[i]);
	}
	
	if(tx_in_progress == 0)
	{
		set_cb();
	}
	// else the data should automatically be transferred
	
	while(!circBufIsEmpty(&tx_buf))
	{
		usb_poll_cb();
	}
	
	return len;
}
