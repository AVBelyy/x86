#include "../../libs.h"

int win32_handler(int action, struct X86_FILE *f, void *buf, int count)
{
    HANDLE hFile = (HANDLE)f->internal;
    switch(action)
    {
        case IO_HANDLER_CLOSE:
            return !CloseHandle(hFile) ? -1 : 0;
            break;
        case IO_HANDLER_WRITE:
        {
            uint32_t written;
            WriteFile(hFile, buf, count, (LPDWORD)&written, NULL);
            return written;
            break;
        }
        case IO_HANDLER_READ:
        {
            uint32_t read;
            ReadFile(hFile, buf, count, (LPDWORD)&read, NULL);
            return read;
            break;
        }
        default:
            return -1;
    }
}

int io_open(char *pathname, int flags, mode_t mode)
{
    // NOT IMPLEMENTED YET!
    flags = GENERIC_WRITE | GENERIC_READ;
    int fd = -1;
    HANDLE w_fd = CreateFile(pathname, flags, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
    if(w_fd != INVALID_HANDLE_VALUE && (fd = io_free_file()) != -1)
    {
        io_files[fd].state = IO_STATE_OPENED;
        io_files[fd].internal = (void*)w_fd;
        io_files[fd].handler = win32_handler;
    }
    return fd;
}

int io_creat(char *pathname, mode_t mode)
{
    // NOT IMPLEMENTED YET!
    HANDLE hFile = CreateFile(pathname, GENERIC_WRITE | GENERIC_READ, 0, NULL,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE || CloseHandle(hFile) == 0) return -1;
    else return 0;
}

int io_link(char *oldpath, char *newpath)
{
    // Windows & NTFS has an awkward mechanism of symlinks
    // Situation changed with Win Vista, but I still have WinXP so can't test CreateSymbolicLink :(
    //return !CreateSymbolicLink(oldpath, newpath) ? -1: 0;
    return -1;
}

struct X86_FILE win32_stdin, win32_stdout, win32_stderr;

void platform_init()
{
    /* firstly, let's initialize and open first-three-canonical files
       (stdin, stdout, stderr -- shame on you if you didn't know!) */
    win32_stdin.state = IO_STATE_OPENED;
    win32_stdin.internal = (void*)GetStdHandle(STD_INPUT_HANDLE);
    win32_stdin.handler = win32_handler;
    set_fd(0, win32_stdin);
    win32_stdout.state = IO_STATE_OPENED;
    win32_stdout.internal = (void*)GetStdHandle(STD_OUTPUT_HANDLE);
    win32_stdout.handler = win32_handler;
    set_fd(1, win32_stdout);
    win32_stderr.state = IO_STATE_OPENED;
    win32_stderr.internal = (void*)GetStdHandle(STD_ERROR_HANDLE);
    win32_stderr.handler = win32_handler;
    set_fd(2, win32_stderr);
    // randomize
    srand(time(NULL));
}
