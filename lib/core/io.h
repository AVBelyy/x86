// Config
#define IO_MAX_FILES        40 // not less than 8!

// X86_FILE statuses
#define IO_STATE_CLOSED     0
#define IO_STATE_OPENED     1

// X86_FILE handler operations
#define IO_HANDLER_CLOSE    0
#define IO_HANDLER_WRITE    1
#define IO_HANDLER_READ     2

// Macroses
#define set_fd(f,s)         io_files[f]=s

struct X86_FILE
{
    int state;
    void *internal;
    int(*handler)(int, struct X86_FILE*, void*, int);
};

int io_write(int, void*, int);

extern int io_opened_files;
extern struct X86_FILE io_files[IO_MAX_FILES];
