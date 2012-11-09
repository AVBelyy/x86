include "libc.obj"

^haystack   dq  1,1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,4181,6765,10946,17711,28657,46368,75025,121393,196418,317811,514229,832040,1346269,2178309,3524578,5702887,9227465,14930352,24157817,39088169
haystack_len = ($-haystack)/8
needle = 14930352

^buf        dq  0,0,0
msg         db  "Search result: "
msg_len = $-msg
newline     db  0xA

_start:
    ;; Search the key
            push    qword haystack_len
            push    ^haystack
            push    qword needle
            call    bsearch
            add     rsp,24
            mov     rdi,rax
    ;; Output the result
            mov     rax,4
            mov     rbx,1
            mov     rcx,msg
            mov     rdx,msg_len
            int     0x32
            push    qword 10
            push    ^buf
            push    rdi
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
