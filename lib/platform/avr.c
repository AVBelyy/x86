#include "../../libs.h"

struct X86_FILE linux_stdin, linux_stdout, linux_stderr;

int io_open(char *pathname, int flags, int mode)
{
    return 0;
}

void platform_init()
{
}
