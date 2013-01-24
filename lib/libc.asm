;  (C) Anton Belyy, 2011, 2012, 2013
;  quicksort implementation by Chuck Liang, Anton Belyy

section header
    pid = 0x05

section text

itoa_table  db      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"


    ;;  Copies string from src to dst
    ;;  char *strcpy(char *dst, char *src)
@strcpy:    enter
            mov     rcx,[rbp+16]
            mov     rdx,[rbp+24]
            mov     rax,rcx             ;  save current dst value
.loop:      mov     [rcx],[rdx]
            cmp     [rcx],'\0'
            je      .return
            inc     rcx
            inc     rdx
            jmp     .loop
.return:    leave
            ret


    ;;  Converts 64-bit integer into string
    ;;  char *itoa(int value, char *str, int base)
@itoa:      enter
            push    rdi
            mov     rdi,[rbp+32]
            mov     rax,[rbp+24]
            push    rax
            mov     rcx,[rbp+16]
            test    rcx,1<<63
            jz      .init
            neg     rcx
            mov     [rax],'-'
            inc     rax
.init:      mov     rdx,rcx
.count:     inc     rax
            div     rdx,rdi
            jnz     .count
            mov     [rax],'\0'
            mov     rbx,rax
.loop:      dec     rax
            mov     rdx,rcx
            mod     rdx,rdi
            div     rcx,rdi
            mov     [rax],[itoa_table+rdx]
            test    rcx,rcx
            jnz     .loop
            pop     rax
            pop     rdi
            sub     rbx,rax
            leave
            ret


    ;;  Searches the sorted array
    ;;  void *bsearch(void *key, void *base, int num, int size, int (*compar)(void*, void*))
@bsearch:   enter
            push    rdi
            mov     rcx,0
            mov     rdx,[rbp+32]
            dec     rdx
.loop:      cmp     rcx,rdx
            jg      .failure
            lea     rdi,[rcx+rdx]
            shr     rdi,1
            mov     rax,rdi
            mul     rax,[rbp+40]
            add     rax,[rbp+24]
            pusha
            push    rax
            push    qword [rbp+16]
            call    qword [rbp+48]
            jl      .smaller
            jg      .larger
            jmp     .success
.smaller:   add     rsp,16
            popa
            mov     rdx,rdi
            dec     rdx
            jmp     .loop
.larger:    add     rsp,16
            popa
            mov     rcx,rdi
            inc     rcx
            jmp     .loop
.failure:   mov     rax,0
            jmp     .exit
.success:   add     rsp,16
            popa
.exit:      pop     rdi
            leave
            ret


_quickaux:  enter
            pusha
            mov     rdx,[rbp+16]    ;  save start of partition in rdx
            mov     rdi,[rbp+24]    ;  end of partition in rdi
            mov     rax,[rbp+32]    ;  and cell size in rax
            mov     rbx,rdx
            mov     rcx,rdi
    ;;  find pivot index:
.ploop:     cmp     rbx,rdi         ;  last cell of partition?
            jae     .qretrn         ;  no pivot, so exit
            cmp     qword [rbx+rax],qword [rbx]
            jl      .pfound         ;  A[i+1]<A[i], pivot found
            lea     rbx,[rbx+rax]   ;  next cell
            jmp     .ploop
.pfound:
    ;;  at this point, rbx holds mem address of pivot
    ;;  now partition into < and >= pivot via repeated swapping.
    ;;  use two counters: rbx and rsi. rbx always points to the
    ;;  first cell of second partition (what's >= pivot)
            mov     rcx,[rbx]       ;  save pivot in rcx
            lea     rsi,[rbx+rax]   ;  next cell
.tloop:                             ;  partitioning loop
            cmp     rcx,[rsi]       ;  compare pivot against element
            jle     .noswap         ;  no swap if element >=pivot
    ;;  swap [rbx] and [rsi], advance both
            xchg    qword [rbx],qword [rsi]
            lea     rbx,[rbx+rax]   ;  next cell must still be >= pivot
.noswap:    lea     rsi,[rsi+rax]   ;  goto next cell, preserve rbx
            cmp     rsi,rdi         ;  end of partition?
            jbe     .tloop          ;  next iteration of partition loop
    ;;  at this point, rbx holds start addr of second partition
    ;;  (could be pivot itself).
    ;;  make recursive calls to quickaux

    ;;  first partition:
            push    rax             ;  size of cell
            sub     rbx,rax
            push    rbx             ;  end of first paritition
            push    rdx             ;  start of first partition
            call    _quickaux
            add     rsp,24          ;  deallocate params
    ;;  second partition:
            push    rax             ;  size of cell
            push    rdi             ;  end of second partition
            lea     rbx,[rbx+rax]
            push    rbx             ;  start of second partition
            call    _quickaux
            add     rsp,24
.qretrn:    popa
            leave
            ret


    ;;  the qsort procedure is just a wrapper around quickaux,
    ;;  for ease of integration into high level language.
    ;;  void qsort(void *base, int num/*, int size, int (*compar)(void*, void*)*/)
@qsort:     enter
            mov     rcx,[rbp+16]    ;  start addr of array
            mov     rax,[rbp+24]    ;  end index of partition
            dec     rax
            lea     rax,[8*rax+rcx] ;  multiply by 8: sizeof(int)==8
                                    ;  rax holds end addr of partition
            push    qword 8         ;  quickaux expects size of cell,
            push    rax             ;  as well as start and end
            push    rcx             ;  addresses of partition as arguments.
            call    _quickaux
            add     rsp,24
            leave
            ret


    ;;  writes a character to the stream
    ;;  returns 1 on success, otherwise 0
    ;;  int fputc(char character, fd stream)
@fputc:     enter
            pusha
            mov     rax,4
            mov     rbx,[rbp+17]
            lea     rcx,[rbp+16]
            mov     rdx,1
            int     0x32
            popa
            leave
            ret


    ;;  writes a character to the standart output
    ;;  returns 1 on success, otherwise 0
    ;;  int putc(char character)
@putc:      enter
            pusha
            mov     rax,4
            mov     rbx,1
            lea     rcx,[rbp+16]
            mov     rdx,rbx
            int     0x32
            popa
            leave
            ret


    ;;  writes formatted data to any place
    ;;  int _uprintf(void (*callback)(char character, void *additional), void *additional, void *garbage, char *format, ...)
_uprintf:   enter
            pusha
            mov     rax,[rbp+16]
            mov     rbx,[rbp+24]
            mov     rcx,[rbp+40]
.loop:      cmp     byte [rcx],'\0'
            je      .finally
            push    rbx
            push    byte [rcx]
            call    rax
            add     rsp,9
            inc     rcx
            jmp     .loop
.finally:   popa
            leave
            ret


    ;;  prints formatted data to the standart output
    ;;  int printf(char *format, ...)
@printf:    push    qword 0 
            push    qword putc
            call    _uprintf
            add     rsp,16
            ret


_start:     ret
