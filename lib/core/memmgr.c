#include "../../libs.h"

void *memmgr_heap;

uint32_t memmgr_alloc(int size)
{
    return 123;
}

void memmgr_init()
{
    // initialize heap
    memmgr_heap = malloc(MEMMGR_HEAP);
}
