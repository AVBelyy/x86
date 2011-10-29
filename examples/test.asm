msg         db      "Hello, world!", 0xA
msg_len = $ - msg

_start:     mov     eax,4
            mov     ebx,1
            mov     ecx,msg
            mov     edx,msg_len
            int     0x32
            ret
