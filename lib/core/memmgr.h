#ifndef __MEMMGR_H
#define __MEMMGR_H

#define MEMMGR_HEAP     256*1024 // default is 256KB

struct MEMMGR_STACK
{
    uint32_t ptr;
    unsigned int len;
    struct MEMMGR_STACK *next;
};

uint32_t memmgr_alloc(unsigned int);
int memmgr_free(uint32_t);

#endif
