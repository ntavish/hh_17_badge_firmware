#ifndef __CIRC_BUF_
#define __CIRC_BUF_

#include <stdint.h>

typedef struct {
    uint8_t * const buffer;
    int head;
    int tail;
    const int maxLen;
} circBuf_t;

#define CIRCBUF_DEF(x,y)          \
    uint8_t x##_dataSpace[y];     \
    circBuf_t x = {               \
        .buffer = x##_space,      \
        .head = 0,                \
        .tail = 0,                \
        .maxLen = y               \
    }
    
int circBufPush(circBuf_t *c, uint8_t data);
int circBufPop(circBuf_t *c, uint8_t *data);
int circBufIsEmpty(circBuf_t *c);

#endif
