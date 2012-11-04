#include "../../libs.h"

int linux_handler(int action, struct X86_FILE *f, void *buf, int count)
{
    int fd = (int)f->internal;
    switch(action)
    {
        case IO_HANDLER_CLOSE:
            if(fd >= 8)
            {
                f->state = IO_STATE_CLOSED;
                return close(fd);
            }
            break;
        case IO_HANDLER_WRITE:
            return write(fd, buf, count);
            break;
        case IO_HANDLER_READ:
            return read(fd, buf, count);
            break;
        case IO_HANDLER_SIZE:
        {
            struct stat statbuf;
            fstat(fd, &statbuf);
            return statbuf.st_size;
            break;
        }
        default:
            return -1;
    }
}

int io_open(char *pathname, int flags, mode_t mode)
{
    int fd = io_opened_files+8, l_fd;
    if((l_fd = open(pathname, flags, mode)) != -1)
    {
        struct X86_FILE f;
        f.state = IO_STATE_OPENED;
        f.internal = (void*)l_fd;
        f.handler = linux_handler;
        set_fd(fd, f);
        io_opened_files++;
    } else {
        fd = -1;
    }
    return fd;
}

struct X86_FILE linux_stdin, linux_stdout, linux_stderr;

void platform_init()
{
    /* firstly, let's initialize and open first-three-canonical files
       (stdin, stdout, stderr -- shame on you if you didn't know!) */
    linux_stdin.state = IO_STATE_OPENED;
    linux_stdin.internal = (void*)0;
    linux_stdin.handler = linux_handler;
    set_fd(0, linux_stdin);
    linux_stdout.state = IO_STATE_OPENED;
    linux_stdout.internal = (void*)1;
    linux_stdout.handler = linux_handler;
    set_fd(1, linux_stdout);
    linux_stderr.state = IO_STATE_OPENED;
    linux_stderr.internal = (void*)2;
    linux_stderr.handler = linux_handler;
    set_fd(2, linux_stderr);
    // randomize
    srand(time(NULL));
}
