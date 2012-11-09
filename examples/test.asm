;  (C) Anton Belyy, 2011, 2012

msg         db      "Hello, world!", 0xA
msg_len = $ - msg
a = -100500

_start:     mov     rax,4
            mov     rbx,1
            mov     rcx,msg
            mov     rdx,msg_len
            int     0x32
            ret
