include "libc.obj"

^itoa_buf   dd      0, 0, 0, 0
newline     db      0xA

_start:
    ;; Calculation part
            mov     eax,1
            mov     ecx,10
.loop:      mul     eax,ecx
            loop    .loop
    ;; Output part
            push    dword 10
            push    ^itoa_buf
            push    eax
            call    itoa
            add     esp,12
            mov     ecx,eax
            mov     edx,ebx
            mov     eax,4
            mov     ebx,1
            int     0x32
            mov     eax,4
            mov     ecx,newline
            mov     edx,1
            int     0x32
            ret
