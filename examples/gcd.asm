include "libc.obj"

^itoa_buf   dq      0,0,0
newline     db      0xA


    ;; Greatest Common Divisor
    ;; int gcd(int a, int b)
gcd:        enter
            mov     rax,[rbp+24]
            mov     rbx,[rbp+16]
            mod     rbx,rax
            jz      .return
            push    rbx
            push    rax
            call    gcd
            add     rsp,8
.return:    leave
            ret


_start:     
    ;; Calculation part
            push    qword 2**30-1
            push    qword 2**30
            call    gcd
            add     rsp,16
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
