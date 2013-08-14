include "libc.obj"

@itoa_buf   dq      0,0
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
            mov     rax,12345678
            mov     rbx,87654321
            call    gcd
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
