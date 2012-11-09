#include "../../libs.h"

struct STACK *heap;
struct CODE  *p;
struct MEMMGR_STACK *mmgr_heap = NULL;

uint32_t memmgr_alloc(uint64_t length)
{
    int flag = FALSE;
    uint64_t alloc_ptr = 0;
    struct MEMMGR_STACK *prev = mmgr_heap, *cur = mmgr_heap;
    while(cur != NULL)
    {
        if(cur->ptr - alloc_ptr >= length) { flag = TRUE; break; }
        else alloc_ptr = cur->ptr + cur->len; 
        prev = cur;
        cur = cur->next;
    }
    if(alloc_ptr >= MEMMGR_HEAP) return NULL;
    struct MEMMGR_STACK *new = malloc(sizeof(struct MEMMGR_STACK));
    new->ptr = alloc_ptr; new->len = length; new->next = flag ? cur : NULL;
    if(alloc_ptr) prev->next = new;
    else mmgr_heap = new;
    return (0x81LL<<56) | alloc_ptr;
}

int memmgr_free(uint64_t ptr)
{
    struct MEMMGR_STACK *prev = NULL, *cur = mmgr_heap;
    ptr &= 0xFFFFFFFFFFFFFFLL;
    while(cur != NULL)
    {
        if(cur->ptr == ptr)
        {
            if(prev != NULL) prev->next = cur->next;
            else mmgr_heap = cur->next;
            break;
        }
        cur = cur->next;
    }
}

void memmgr_init()
{
    /* the main idea is to create the fake process with pid=1
       and text section with MEMMGR_HEAP size. This is clear and simple
       way to provide data allocation and sharing
    */
    heap = malloc(sizeof(struct STACK));
    p = malloc(sizeof(struct CODE));
    p->text = malloc(MEMMGR_HEAP);
    p->pid = 1;
    heap->item = p;
    heap->next = head;
    head = heap;
}
