#ifndef __LINUX_H
#define __LINUX_H

#include <fcntl.h>
#include <sys/stat.h>

// directly use Linux I/O API for these functions
#define io_creat        creat
#define io_link         link

#define X86_CREAT       O_CREAT
#define X86_RDONLY      O_RDONLY
#define X86_RDWR        O_RDWR
#define X86_WRONLY      O_WRONLY

int io_open(char*, int, mode_t);

#endif
