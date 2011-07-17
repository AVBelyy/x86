section .header
    pid = 0x01

section .text

    ;;  Copies string from src to dst
    ;;  char *strcpy(char *dst, const char *src)
@strcpy:
            enter
            mov     ecx,[ebp+8]
            mov     edx,[ebp+12]
            mov     eax,ecx             ;  save current dst value
.loop:      mov     [ecx],[edx]
            cmp     [ecx],'\0'
            je      .return
            inc     ecx
            inc     edx
            jmp     .loop
.return:    leave
            ret


    ;;  Converts 32-bit integer to string
    ;;  char *itoa(int value, char *str)
@itoa:      enter
            mov     eax,[ebp+12]
            mov     ecx,[ebp+8]
            mov     edx,ecx
.count:     inc     eax
            div     edx,10
            jnz     .count
            mov     [eax],'\0'
            mov     ebx,eax
.loop:      dec     eax
            mov     edx,ecx
            mod     edx,10
            add     dl,'0'
            div     ecx,10
            mov     [eax],dl
            test    ecx,ecx
            jnz     .loop
            sub     ebx,eax
            leave
            ret


quickaux:
            enter
            pusha
            mov     ebx,[ebp+8]     ;  start of partition
    ;;  find pivot index:
.ploop:
            cmp     ebx,[ebp+12]    ;  last cell of partition?
            jge     .qretrn         ;  no pivot, so exit
            cmp     dword [ebx+4],dword [ebx]
            jl      .pfound         ;  A[i]>A[i+1], pivot found
            add     ebx,4           ;  next cell
            jmp     .ploop
.pfound:
    ;;  at this point, ebx holds mem address of pivot
    ;;  now partition into < and >= pivot via repeated swapping.
    ;;  use two counters: ebx and esi. ebx always points to the
    ;;  first cell of second partition (what's >= pivot)
            mov     ecx,[ebx]       ;  save pivot in ecx
            lea     esi,[ebx+4]     ;  next cell
.tloop:                             ;  partitioning loop
            cmp     ecx,[esi]       ;  compare pivot against element
            jle     .noswap         ;  no swap if element >=pivot
    ;;  swap [ebx] and [esi], advance both
            xchg    dword [ebx],dword [esi]
            add     ebx,4           ;  next cell must still be >= pivot
.noswap:
            add     esi,4           ;  goto next cell, preserve ebx
            cmp     esi,[ebp+12]    ;  end of partition?
            jle     .tloop          ;  next iteration of partition loop
    ;;  at this point, ebx holds start addr of second partition
    ;;  (could be pivot itself).
    ;;  make recursive calls to quickaux:

    ;;  first partition:
            sub     ebx,4
            push    ebx             ;  end of first paritition
            mov     eax,[ebp+8]
            push    eax             ;  start of first partition
            call    quickaux
            add     esp,8           ;  deallocate params
    ;;  second partition:
            mov     eax,[ebp+12]
            push    eax             ;  end of second partition
            add     ebx,4
            push    ebx             ;  start of second partition
            call    quickaux
            add     esp,8
.qretrn:
            popa
            leave
            ret

    ;;  the qsort procedure is just a wrapper around quickaux,
    ;;  for ease of integration into high level language.
    ;;  void qsort(int *A, int start, int end)
@qsort:
            enter
            pusha
            mov     ebx,[ebp+8]     ;  start addr of array
            mov     eax,[ebp+16]    ;  end index of partition
            lea     eax,[4*eax+ebx] ;  multiply by 4: sizeof(int)==4
                                    ;  eax holds end addr of partition
            mov     ecx,[ebp+12]    ;  start index of partition
            lea     ecx,[4*ecx+ebx] ;  start addr of partition
            push    eax             ;  quickaux expects start and end 
            push    ecx             ;  addresses of partition as arguments.
            call    quickaux
            add     esp,8
            popa
            leave
            ret

_start:     ret
