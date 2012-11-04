#ifndef __AVR_H
#define __AVR_H

#define X86_CREAT       0
#define X86_RDONLY      1
#define X86_RDWR        2
#define X86_WRONLY      3

int io_open(char*, int, int);
int io_creat(char*, int);
int io_link(char*, char*);

#endif
