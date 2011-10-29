#include "../../libs.h"

int io_opened_files;
struct X86_FILE io_files[IO_MAX_FILES];

int io_free_file()
{
    int fd;
    for(fd = 8; fd < IO_MAX_FILES; fd++)
        if(io_files[fd].state == IO_STATE_CLOSED) return fd;
    return -1;
}

int io_close(int fd)
{
    struct X86_FILE *f = &io_files[fd];
    return f->handler(IO_HANDLER_CLOSE, f, NULL, 0);
}

int io_write(int fd, void *buf, int count)
{
    struct X86_FILE *f = &io_files[fd];
    return f->handler(IO_HANDLER_WRITE, f, buf, count);
}

int io_read(int fd, void *buf, int count)
{
    struct X86_FILE *f = &io_files[fd];
    return f->handler(IO_HANDLER_READ, f, buf, count);
}

void io_init()
{
    io_opened_files = 0;
}
