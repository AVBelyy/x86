    ;; (C) Anton Belyy, 2011

include     "libc.obj"

    ;; Some constant values for faster MD5 evaluation
r           dd      7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22
            dd      5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20
            dd      4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23
            dd      6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21

k           dd      0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee
            dd      0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501
            dd      0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be
            dd      0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821
            dd      0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa
            dd      0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8
            dd      0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed
            dd      0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a
            dd      0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c
            dd      0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70
            dd      0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05
            dd      0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665
            dd      0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039
            dd      0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1
            dd      0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1
            dd      0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391

h0          dd      0x67452301
h1          dd      0xefcdab89
h2          dd      0x98badcfe
h3          dd      0x10325476


    ;; Working variables
w           dd      0, 0, 0, 0, 0, 0, 0, 0
            dd      0, 0, 0, 0, 0, 0, 0, 0
a           dd      0
b           dd      0
c           dd      0
d           dd      0

^stream     db      "1"
stream_len  = $-stream
^itoa_buf   dd      0, 0, 0
md5_msg1    db      'md5("'
md5_msg2    db      '") => '
newline     db      0xA

_start:     mov     esi,^stream
            mov     ebx,stream_len

    ;; ESI - far pointer to data stream
    ;; EBX - stream length

            push    ebx             ; temporarily save stream length

    ;; Allocate buffer with size L' = 64*(L div 64+1) bytes
            shr     ebx,6
            inc     ebx
            shl     ebx,6
            mov     eax,0xc0
            int     0x32            ; allocate working buffer
            mov     edi,eax
            mov     edx,ebx
            mov     ebx,eax
            mov		eax,0xc3
            mov     ecx,0
            int		0x32            ; fill allocated buffer with "0" byte
            mov     eax,0xc2
            pop     ebx
            int     0x32            ; copy input stream to working buffer
            mov     [edi+ebx],0x80  ; append "1" bit to message
            shl     ebx,3
            mov     [edi+edx-8],ebx ; append bit length of unpadded message
    ;; Process the message in successive 512-bit chunks
            shr     edx,6           ; EDX now stores chunks count
            mov     ecx,16
.split_ch:  dec     ecx             ; break chunk into 16 32-bit words w[0..15]
            mov     dword [w+ecx*4],[ecx*4+edi]
            jnz     .split_ch
            mov     dword [a],[h0]
            mov     dword [b],[h1]
            mov     dword [c],[h2]
            mov     dword [d],[h3]
    ;; Main loop
.loop:      mov     edx,ecx
            shr     edx,4
            jz      .0_15           ; 0 ≤ i ≤ 15
            cmp     edx,1
            jz      .16_31          ; 16 ≤ i ≤ 31
            cmp     edx,2
            jz      .32_47          ; 32 ≤ i ≤ 47
            mov     eax,[d]         ; 48 ≤ i ≤ 63
            not     eax
            or      eax,[b]
            xor     eax,[c]         ; f = c xor (b or (not d))
            mov     ebx,ecx
            mul     ebx,7
            mod     ebx,16          ; g = (7*i) mod 16
.loop_ret:  push    dword [d]
            mov     dword [d],[c]
            mov     dword [c],[b]
            mov     edx,[a]
            add     edx,eax
            add     edx,[k+ecx*4]
            add     edx,[w+ebx*4]   ; edx = a + f + k[i] + w[g]
            mov     eax,edx
            shl     eax,[r+ecx*4]   ; eax = edx << r[i]
            mov     ebx,32
            sub     ebx,[r+ecx*4]
            shr     edx,ebx
            or      eax,edx         ; eax = eax or (edx >> (32-r[i]))
            add     dword [b],eax
            pop     dword [a]
            inc     ecx
            cmp     ecx,64
            jnz     .loop
            jmp     .chnks_ret
.0_15:      mov     eax,[c]
            xor     eax,[d]
            and     eax,[b]
            xor     eax,[d]         ; f = d xor (b and (c xor d))
            mov     ebx,ecx         ; g = i
            jmp     .loop_ret
.16_31:     mov     eax,[b]
            xor     eax,[c]
            and     eax,[d]
            xor     eax,[c]         ; f = c xor (d and (b xor c))
            mov     ebx,ecx
            mul     ebx,5
            inc     ebx
            mod     ebx,16          ; g = (5*i + 1) mod 16
            jmp     .loop_ret
.32_47:     mov     eax,[b]
            xor     eax,[c]
            xor     eax,[d]         ; f = b xor c xor d
            mov     ebx,ecx
            mul     ebx,3
            add     ebx,5
            mod     ebx,16          ; g = (3*i + 5) mod 16            
            jmp     .loop_ret
.chnks_ret: add     dword [h0],[a]
            add     dword [h1],[b]
            add     dword [h2],[c]
            add     dword [h3],[d]
    ;; Build result
            mov     edi,0
            mov     eax,4
            mov     ebx,1
            mov     ecx,md5_msg1
            mov     edx,5
            int     0x32
            mov     eax,4
            mov     ecx,stream
            mov     edx,stream_len
            int     0x32
            mov     eax,4
            mov     ecx,md5_msg2
            mov     edx,6
            int     0x32
.res_loop:  push    dword 16
            push    ^itoa_buf
            push    dword [h0+edi*4]
            call    itoa
            add     esp,12
            mov     ecx,eax
            mov     edx,ebx
            mov     eax,4
            mov     ebx,1
            int     0x32
            inc     edi
            cmp     edi,4
            jnz     .res_loop
            mov     eax,4
            mov     ebx,1
            mov     ecx,newline
            mov     edx,1
            int     0x32
            ret
