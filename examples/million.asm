include "libc.obj"

^itoa_buf       dq      0

million = 1000000

_start:         mov     rsi,1
                mov     rdi,10
                mov     rcx,million
                mov     bl,'\n'
.loop:          push    rdi
                push    ^itoa_buf
                push    rsi
                call    itoa
                add     rsp,24
                push    rax
                call    printf
                add     rsp,8
                push    bl
                call    putc
                inc     rsi
                loop    .loop
                ret
