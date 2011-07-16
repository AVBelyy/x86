#ifndef __LIBS_H
#define __LIBS_H

#include "x86.h"

#if defined(_WIN32) || defined(WIN32)
    #include "lib/win32.h"
#else
    #include "lib/linux.h"
#endif // _WIN32

#endif // __LIBS_H
