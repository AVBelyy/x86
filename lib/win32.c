#include "../libs.h"

HANDLE stdIn, stdOut, stdErr;

HANDLE win32_GetHandle(uint32_t fd)
{
    switch(fd)
    {
        case 0: // stdin
            return stdIn;
        case 1: // stdout
            return stdOut;
        case 2: // stderr
            return stdErr;
        default:
            return NULL;
    }
}

void int32_handler(void *p)
{
    uint32_t *regs = caller->regs;
    uint8_t  *text = caller->text;
    switch(EAX)
    {
        case 0x01:  // exit
            sig_raise(X86_EXIT, p);
            break;
        case 0x02:  // fork, not implemented yet
            break;
        case 0x03:  // read
            break;
        case 0x04:  // write
        {
            /* ebx - HANDLE hFile;
               ecx - LPCVOID lpBuffer;
               edx - DWORD nNumberOfBytesToWrite; */
            uint8_t* lpBuffer;
            uint32_t count;
            calc_mem(lpBuffer, ECX);
            WriteFile(win32_GetHandle(EBX), lpBuffer, EDX, (LPDWORD)&count, NULL);
            caller->EAX = count;
            break;
        }
    }
}

void win32_init()
{
    // get standart handles
    stdIn  = GetStdHandle(STD_INPUT_HANDLE);
    stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    stdErr = GetStdHandle(STD_ERROR_HANDLE);
    // set interrupt handler
    set_intr(0x32, int32_handler);
}
