#ifndef __LIBS_H
#define __LIBS_H

#include "x86.h"

#include "lib/core/io.h"

#if defined(_AVR)
    #include "lib/platform/avr.h"
#elif defined(_WIN32) || defined(WIN32)
    #include "lib/platform/win32.h"
#else
    #include "lib/platform/linux.h"
#endif // _WIN32

#include "lib/core/memmgr.h"

#endif // __LIBS_H
