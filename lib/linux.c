#include "../libs.h"

#define GETSTR_(string_, regist_) \
uint8_t *string_; \
calc_mem(string_, regist_); \
string_[strlen(string_)-1]=0;

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
	    caller->EAX = fork();
            break;
        case 0x03: // read
        {
            /* ebx - int fd;
               ecx - void *buf;
               edx - size_t count; */
	    GETSTR_(buf, ECX);
            caller->EAX = read(EBX, (uint32_t)buf, EDX);
            break;
        }
        case 0x04: // write
        {
            /* ebx - int fd;
               ecx - void *buf;
               edx - size_t count; */
	    GETSTR_(buf, ECX);
            caller->EAX = write(EBX, (uint32_t)buf, EDX);
            break;
        }
        case 0x05: // open
        {
            /* ebx - char *pathname;
               ecx - int flags;
               edx - mode_t mode; */
	    GETSTR_(pathname, EBX);
            caller->EAX = open((uint32_t)pathname, ECX, EDX);
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
	    GETSTR_(pathname, EBX);
            caller->EAX = creat((uint32_t)pathname, ECX);
            break;
        }
	case 0x09: // link, not work yet
        {
            /* ebx - const char *oldpath;
               ecx - const char *newpath; */
	    GETSTR_(oldpath, EBX);
	    GETSTR_(newpath, ECX);
            caller->EAX = link(oldpath, newpath);
            break;
        }
	case 0xBE: // vfork, not work yet
	    caller->EAX = vfork();
            break;
    }
}

void linux_init()
{
    // set interrupt handler
    set_intr(0x80, int80_handler);
}
