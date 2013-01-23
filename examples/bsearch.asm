include "libc.obj" 

@haystack   dd  1,1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,4181,6765,10946,17711,28657,46368,75025,121393,196418,317811,514229,832040,1346269,2178309,3524578,5702887,9227465,14930352,24157817,39088169
haystack_size = ($-haystack)/4
needle = 377

@buf        dq      0,0,0
@msg        db      "Needle index: ",0

@compare:   enter
            mov     rax,0
            mov     eax,dword [rbp+16]
            mov     rdx,[rbp+24]
            cmp     eax,dword [rdx]
            leave
            ret

_start:
    ;; Search the key
            push    @compare
            push    qword 4
            push    qword haystack_size
            push    @haystack
            push    qword needle
            call    bsearch
            add     rsp,40
            mov     rsi,rax
            test    rsi,rsi
            jz      .output
            sub     rsi,@haystack
            shr     rsi,2
    ;; Output the result
.output:    push    @msg
            call    printf
            add     rsp,8
            push    qword 10
            push    @buf
            push    rsi
            call    itoa
            add     rsp,24
            push    rax
            call    printf
            add     rsp,8
            push    byte '\n'
            call    putc
            inc     rsp
            ret
