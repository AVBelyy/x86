#ifndef __MEMMGR_H
#define __MEMMGR_H

#define MEMMGR_HEAP     256*1024 // default is 256KB

struct MEMMGR_STACK
{
    uint64_t ptr;
    uint64_t len;
    struct MEMMGR_STACK *next;
};

uint32_t memmgr_alloc(uint64_t);
int memmgr_free(uint64_t);

#endif
