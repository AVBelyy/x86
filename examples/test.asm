;  (C) Anton Belyy, 2011, 2012, 2013

include "libc.obj"

^msg        db      "Hello, world!\n", 0

_start:     push    ^msg
            call    printf
            add     rsp,8
            ret
