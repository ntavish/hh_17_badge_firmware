#include <stdint.h>
#include "circular_buf.h"

/* lifted from http://embedjournal.com/implementing-circular-buffer-embedded-c/ */

int circBufPush(circBuf_t *c, uint8_t data)
{
    // next is where head will point to after this write.
    int next = c->head + 1;
    if (next >= c->maxLen)
        next = 0;

    if (next == c->tail) // check if circular buffer is full
        return -1;       // and return with an error.

    c->buffer[c->head] = data; // Load data and then move
    c->head = next;            // head to next data offset.
    return 0;  // return success to indicate successful push.
}

int circBufPop(circBuf_t *c, uint8_t *data)
{
    // if the head isn't ahead of the tail, we don't have any characters
    if (c->head == c->tail) // check if circular buffer is empty
        return -1;          // and return with an error

    // next is where tail will point to after this read.
    int next = c->tail + 1;
    if(next >= c->maxLen)
        next = 0;

    *data = c->buffer[c->tail]; // Read data and then move
    c->tail = next;             // tail to next data offset.
    return 0;  // return success to indicate successful push.
}

int circBufIsEmpty(circBuf_t *c)
{
	return (c->head == c->tail);
}
