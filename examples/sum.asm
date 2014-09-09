include "libc.obj"

@itoa_buf   dq      0,0,0

_start:     mov     rax,0
            mov     rcx,100000000
.loop:      lea     rax,[rax+rcx]
            loop    .loop
            push    qword 10
            push    @itoa_buf
            push    rax
            call    itoa
            add     rsp,24
            push    rax
            call    printf
            add     rsp,8
            ret
