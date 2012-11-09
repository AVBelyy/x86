include "libc.obj"

^itoa_buf   dq      0,0,0
newline     db      0xA


    ;; Greatest Common Divisor
    ;; fastcall int gcd(int a(rax), int b(rbx))
gcd:        enter
.loop:      mod     rax,rbx
            xchg    rax,rbx
            jnz     .loop
            leave
            ret


_start:     
    ;; Calculation part
            mov     eax,2**30-1
            mov     ebx,2**30
            call    gcd
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
            mov     rbx,1
            mov     rcx,newline
            mov     rdx,1
            int     0x32
            ret
