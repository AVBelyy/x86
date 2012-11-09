;  (C) Anton Belyy, 2012

include "libc.obj"

^itoa_buf   dd      0, 0, 0, 0
newline     db      0xA

_start:
    ;; Calculation part
            mov     rax,1
            mov     rcx,10
.loop:      mul     rax,rcx
            loop    .loop
    ;; Output part
            push    qword 10
            push    ^itoa_buf
            push    rax
            call    itoa
            add     rsp,24
            mov     rcx,rax
            mov     rdx,rbx
            mov     rax,4
            mov     rbx,1
            int     0x32
            mov     rax,4
            mov     rcx,newline
            mov     rdx,1
            int     0x32
            ret
