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
        case 0x03: // read
        {
            /* ebx - int fd;
               ecx - void *buf;
               edx - size_t count; */
            uint8_t *buf;
            calc_mem(buf, ECX);
            caller->EAX = read(EBX, buf, EDX);
            break;
        }
        case 0x04: // write
        {
            /* ebx - int fd;
               ecx - void *buf;
               edx - size_t count; */
            uint8_t *buf;
            calc_mem(buf, ECX);
            caller->EAX = write(EBX, buf, EDX);
            break;
        }
        case 0x05: // open
        {
            /* ebx - char *pathname;
               ecx - int flags;
               edx - mode_t mode; */
            uint8_t *pathname;
            calc_mem(pathname, EBX);
            caller->EAX = open(pathname, ECX, EDX);
            break;
        }
        case 0x06: // close
        {
            /* ebx - int fd; */
            caller->EAX = close(EBX);
            break;
        }
        case 0x08: // creat
        {
            /* ebx - char *pathname;
               ecx - mode_t mode; */
            uint8_t *pathname;
            calc_mem(pathname, EBX);
            caller->EAX = creat(pathname, ECX);
            break;
        }
        case 0x09: // link, not implemented yet
        {
            /* ebx - const char *oldpath;
               ecx - const char *newpath; */
            uint8_t *oldpath,*newpath;
            calc_mem(oldpath, EBX);
            calc_mem(newpath, ECX);
            caller->EAX = link(oldpath, newpath);
            break;
        }
    }
}

void linux_init()
{
    // set interrupt handler
    set_intr(0x80, int80_handler);
}
