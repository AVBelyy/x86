include "libc.obj"

bf_mem = 30000

bf_program      db      "++++[>+++++<-]>[<+++++>-]+<+[>[>+>+<<-]++>>[<<+>>-]>>>[-]++>[-]+>>>+[[-]++++++>>>]<<<[[<++++++++<++>>-]+<.<[>----<-]<]<<[>>>>>[>>>[-]+++++++++<[>-<-]+++++++++>[-[<->-]+[<<<]]<[>+<-]>]<<-]<<-]", 0

_start:
        ;; registry overview:
        ;;     RDX -- loop depth
        ;;     RSI -- pointer to bf cmds
        ;;     RDI -- pointer to bf cells

        ;; allocate brainfuck cells
                mov     rax, 0xC0
                mov     rbx, bf_mem
                int     0x32
                mov     rdi, rax
        ;; execute program
                mov     rsi, bf_program
    .loop:      cmp     byte ptr rsi, '>'
                je      .fwd_cell
                cmp     byte ptr rsi, '<'
                je      .bck_cell
                cmp     byte ptr rsi, '+'
                je      .inc_cell
                cmp     byte ptr rsi, '-'
                je      .dec_cell
                cmp     byte ptr rsi, '.'
                je      .put_cell
                cmp     byte ptr rsi, ','
                je      .get_cell
                cmp     byte ptr rsi, '['
                je      .new_loop
                cmp     byte ptr rsi, ']'
                je      .end_loop
                cmp     byte ptr rsi, 0
                je      .exit
    .continue:  inc     rsi
                jmp     .loop

        ;; at this point, our program reached its end
        ;; so be it. let's just silently terminate our execution

    .exit:
        ;; deallocate brainfuck cells
                mov     rax, 0xC1
                mov     rbx, rdi
                int     0x32
                ret

        ;; bf cmd handlers

    .fwd_cell:  inc     rdi
                jmp     .continue

    .bck_cell:  dec     rdi
                jmp     .continue

    .inc_cell:  inc     byte ptr rdi
                jmp     .continue

    .dec_cell:  dec     byte ptr rdi
                jmp     .continue

    .put_cell:  push    byte ptr rdi
                call    putc
                inc     rsp
                jmp     .continue

    .get_cell:
        ;; not implemented yet
                jmp     .continue

    .new_loop:  cmp     byte ptr rdi, 0
                jne     .continue
                inc     rdx
    .new_strip: test    rdx, rdx
                je      .continue
                inc     rsi
                cmp     byte ptr rsi, '['
                je      .new_deep
                cmp     byte ptr rsi, ']'
                jne     .new_strip
                dec     rdx
                jmp     .new_strip
    .new_deep:  inc     rdx
                jmp     .new_strip

    .end_loop:  cmp     byte ptr rdi, 0
                je      .continue
                dec     rdx
    .end_strip: test    rdx, rdx
                je      .continue
                dec     rsi
                cmp     byte ptr rsi, '['
                je      .end_deep
                cmp     byte ptr rsi, ']'
                jne     .end_strip
                dec     rdx
                jmp     .end_strip
    .end_deep:  inc     rdx
                jmp     .end_strip
