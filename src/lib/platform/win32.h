#ifndef __WIN32_H
#define __WIN32_H

#include <windows.h>

int io_open(char*, int, mode_t);
int io_creat(char*, mode_t);
int io_link(char*, char*);

#endif
