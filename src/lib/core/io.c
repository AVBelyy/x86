#include "../../libs.h"

int io_opened_files;
struct X86_FILE io_files[IO_MAX_FILES];

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

int io_size(int fd)
{
    struct X86_FILE *f = &io_files[fd];
    return f->handler(IO_HANDLER_SIZE, f, NULL, 0);
}

void io_init()
{
    io_opened_files = 0;
}
