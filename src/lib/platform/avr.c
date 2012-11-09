#include "../../libs.h"

struct X86_FILE avr_stdin, avr_stdout, avr_stderr;

int io_open(char *pathname, int flags, int mode)
{
    // NOT IMPLEMENTED YET!
    return 0;
}

int io_creat(char *pathname, int mode)
{
    // NOT IMPLEMENTED YET!
    return -1;
}

int io_link(char *oldpath, char *newpath)
{
    // NOT IMPLEMENTED YET!
    return -1;
}

void platform_init()
{
    digitalWrite(6, HIGH);
}
