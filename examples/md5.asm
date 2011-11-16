    ;; (C) Anton Belyy, 2011

include     "libc.obj"

stream      db      "Isud vinum, bonum vinum"
stream_len  = $-stream

_start:
            mov     eax,stream
            mov     ebx,stream_len
    ;; EAX - pointer to data stream
    ;; EBX - stream length
.step1:     inc     ebx
            mov     ecx,ebx
            mod     ecx,64
            cmp     ecx,56
            jnz     .step1 
            ret
