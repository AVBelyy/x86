include "libc.obj"

^itoa_buf       dq      0
newline         db      0xA

million = 1000000

_start:         mov     rsi,1
                mov     rdi,10
.loop:          push    rdi
                push    ^itoa_buf
                push    rsi
                call    itoa
                add     rsp,24
                mov     rcx,rax
                mov     rdx,rbx
                mov     rax,4
                mov     rbx,1
                int     0x32
                mov     rax,4
                mov     rcx,newline
                mov     rdx,rbx
                int     0x32
                inc     rsi
                cmp     rsi,million
                jle     .loop
                ret
