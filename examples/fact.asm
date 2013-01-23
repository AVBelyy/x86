include "libc.obj"

@itoa_buf   dq      0,0,0

_start:
    ;; Calculation part
            mov     rax,1
            mov     rcx,10
.loop:      mul     rax,rcx
            loop    .loop
    ;; Output part
            push    qword 10
            push    @itoa_buf
            push    rax
            call    itoa
            add     rsp,24
            push    rax
            call    printf
            add     rsp,8
            push    byte '\n'
            call    putc
            inc     rsp
            ret
