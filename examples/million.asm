include "libc.obj"

@itoa_buf       dq      0

million = 1000000

_start:         mov     rsi,1
                mov     rdi,10
                mov     rcx,million
.loop:          push    rcx
                push    rdi
                push    @itoa_buf
                push    rsi
                call    itoa
                add     rsp,24
                pop     rcx
                push    rax
                call    printf
                add     rsp,8
                push    byte '\n'
                call    putc
                inc     rsp
                inc     rsi
                loop    .loop
                ret
