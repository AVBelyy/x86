#ifndef __X86_H
#define __X86_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TRUE            1
#define FALSE           0

#define EAX             regs[0]
#define EBX             regs[1]
#define ECX             regs[2]
#define EDX             regs[3]
#define ESI             regs[4]
#define EDI             regs[5]
#define EBP             regs[6]
#define ESP             regs[7]

#define CF              (1<<0)
#define PF              (1<<2)
#define AF              (1<<4)
#define ZF              (1<<6)
#define SF              (1<<7)
#define TF              (1<<8)
#define IF              (1<<9)
#define DF              (1<<10)
#define OF              (1<<11)

#define bset(x)         (flags |= (x))
#define bget(x)         (flags & (x))
#define bclear(x)       (flags &= ~(x))

#define INTTABLE_MAX    256

#define skip(sz)        pc+=sz
#define skip_reg()      skip(1)
#define skip_const()    skip(*pc+1)
#define skip_mem()      int mem = get(); if(mem&0x70) skip(1); if(mem&(1<<7)) skip_const();
#define get()           *pc++
#define get_reg_ptr(id) ((id)<20 ? &regs[(id)%8] : (uint32_t*)((uint8_t*)&regs[(id)-20]+1))
#define get_reg(id)     *get_reg_ptr(id)
#define set_reg(id,vl)  *get_reg_ptr(id) = vl
#define get_mem()       mem = get(); size = mem&0xF; if(mem&0x70) base = get(); \
                        if(mem&(1<<4)) rel_addr = 1<<(base>>6); \
                        if(mem&(1<<5)) { if(!rel_addr) rel_addr = 1; rel_addr *= regs[base>>3&7]; } \
                        if(mem&(1<<6)) rel_addr += regs[base&7]; \
                        if(mem&(1<<7)) { uint32_t buf = 0; int size = get(); memcpy(&buf, pc, size); rel_addr += buf; skip(size); }
#define calc_mem(a,ra)  if((int32_t)(ra) < 0) a = ((ra)&0xFFFFFF)+search((ra)>>24&0x7F); \
                        else a = (ra)+text;
#define get_mem_ptr()   get_mem(); \
                        calc_mem(addr, rel_addr);
#define try_PF(vl)      int i, res = 1; for(i = 0; i < 8; i++) \
                        { res^=(vl)&1; (vl)>>=1; } bset(res*PF)
#define intr(id,p)      if(id < INTTABLE_MAX) inttable[id](p)
#define set_intr(id,p)  if(id < INTTABLE_MAX) inttable[id] = p
#define caller          ((struct CODE*)p)
#define push(d,sz)      uint8_t *ptr; \
                        ESP -= sz; \
                        calc_mem(ptr, ESP); \
                        memcpy(ptr, d, sz);
#define pop(d,sz)       uint8_t *ptr; \
                        calc_mem(ptr, ESP); \
                        memcpy(d, ptr, sz); \
                        ESP += sz;

// debug stuff
#define regs_out()      printf("EAX=%d EBX=%d ECX=%d EDX=%d\n", EAX, EBX, ECX, EDX)
#define flags_out()     printf("CF=%d PF=%d ZF=%d SF=%d OF=%d\n", bget(CF), bget(PF), bget(ZF), bget(SF), bget(OF))

// signals
#define X86_EXIT        1

typedef void (*handler)(void*);
typedef void (*sighandler)(void*, void*);
typedef struct {
    int             pid;
    int             offset;
} code_sigh;
extern uint16_t flags;
extern int pid_counter;
extern int reg_size[];
extern int pusha_order[];
extern handler inttable[INTTABLE_MAX];

struct CODE
{
    uint32_t        *regs;
    uint8_t         *text;
    uint8_t         *pc;
    int             pid;
    int             cmp_flag;
    int             ret_count;
};

struct STACK
{
    struct CODE     *item;
    struct STACK    *next;
};

struct SIGNALS
{
    int             sig;
    sighandler      h;
    void            *data;
    struct SIGNALS  *next;
};

extern struct STACK *head;
extern struct SIGNALS *sig_head;

struct CODE *code_load(char *);
int code_exec(struct CODE *);
int code_free(struct CODE *);
uint8_t *search(int);
void sig_attach(int, sighandler, void *);
void sig_raise(int, void *);
#endif
