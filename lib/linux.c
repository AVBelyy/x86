#include "../libs.h"

void int80_handler(void *p)
{
    uint32_t *regs = caller->regs;
    uint8_t  *text = caller->text;
    switch(EAX)
    {
        case 0x01: // exit
            sig_raise(X86_EXIT, p);
            break;
        case 0x02: // fork, not implemented yet
            break;
        case 0x03: // read
        {
            /* ebx - int fd;
               ecx - void *buf;
               edx - size_t count; */
            uint8_t *buf;
            calc_mem(buf, ECX);
            caller->EAX = read(EBX, (uint32_t)buf, EDX);
            break;
        }
        case 0x04: // write
        {
            /* ebx - int fd;
               ecx - void *buf;
               edx - size_t count; */
            uint8_t *buf;
            calc_mem(buf, ECX);
            caller->EAX = write(EBX, (uint32_t)buf, EDX);
            break;
        }
        case 0x05: // open
        {
            /* ebx - char *pathname;
               ecx - int flags;
               edx - mode_t mode; */
            uint8_t *pathname;
            calc_mem(pathname, EBX);
            caller->EAX = open((uint32_t)pathname, ECX, EDX);
            break;
        }
        case 0x06: // close
        {
            /* ebx - int fd; */
            caller->EAX = close(EBX);
            break;
        }
    }
}

void linux_init()
{
    // set interrupt handler
    set_intr(0x80, int80_handler);
}
