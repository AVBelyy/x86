#include "libs.h"

uint16_t flags;
int pid_counter;
int reg_size[]    = {8,8,8,8,8,8,8,8,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1};
int pusha_order[] = {0,2,3,1,7,6,4,5};
handler inttable[INTTABLE_MAX];

struct STACK *head = NULL;
struct SIGNALS *sig_head = NULL;

struct CODE *code_load(char *fname)
{
    int i, shared_cnt;
    uint8_t header[8];
    uint64_t *shared;
    struct STACK *new = malloc(sizeof(struct STACK));
    int fd = io_open(fname, X86_RDONLY, X86_RDONLY);
    int sz = io_size(fd) - 8;
    struct CODE *p = malloc(sizeof(struct CODE));
    // read 'header' and 'text' sections
    io_read(fd, &header, 8);
    p->regs = malloc(8*sizeof(int64_t));
    p->pid = header[1] & 0x80 ? header[1] & 0x7F : 64 + pid_counter++;
    shared_cnt = header[2] + (header[3] << 8);
    sz -= shared_cnt*sizeof(int64_t);
    shared = malloc(shared_cnt*sizeof(int64_t));
    io_read(fd, shared, shared_cnt*sizeof(int64_t));
    for(i = 0; i < 8; i++) p->regs[i] = 0;
    p->text = malloc(p->regs[7] = (1<<((header[0]>>4)+7))+sz);
    io_read(fd, p->text, sz);
    for(i = 0; i < shared_cnt; i++) p->text[shared[i]] = 0x80|p->pid;
    free(shared);
    io_close(fd);
    --p->regs[7];
    p->regs[7] += (uint64_t)(0x80|p->pid)<<56;
    p->pc = p->text;
    p->cmp_flag = FALSE;
    p->ret_count = 0;
    new->item = p;
    new->next = head;
    head = new;
    return p;
}

uint8_t *search(int pid)
{
    struct STACK *cur = head;
    while(cur != NULL)
    {
        if(cur->item->pid == pid)
            return cur->item->text;
        cur = cur->next;
    }
    return NULL;
}

void sig_attach(int sig, sighandler h, void *data)
{
    struct SIGNALS *new = malloc(sizeof(struct SIGNALS));
    new->sig = sig;
    new->h = h;
    new->data = data;
    new->next = sig_head;
    sig_head = new;
}

void sig_raise(int sig, void *p)
{
    struct SIGNALS *cur = sig_head;
    while(cur != NULL)
    {
        if(cur->sig == sig)
            cur->h(p, cur->data);
        cur = cur->next;
    }
}

void code_sighandler(void *zero, void *data)
{
    struct CODE *p;
    struct STACK *cur = head;
    while(cur != NULL)
    {
        if(cur->item->pid == ((code_sigh*)data)->pid)
        {
            p = cur->item;
            break;
        }
        cur = cur->next;
    }
    free(data);
    uint64_t *regs = p->regs;
    uint8_t  *text = p->text;
    uint64_t buf   = ((uint64_t)(p->pid)<<56)+(p->pc)-text;
    push(&buf, 8);
    p->pc = text+RCX;
}

void int32_handler(void *p)
{
    uint64_t *regs = caller->regs;
    uint8_t  *text = caller->text;
    switch(RAX)
    {
        case 0x01: // exit
            sig_raise(X86_EXIT, p);
            break;
        case 0x03: // read
        {
            /* rbx - int fd;
               rcx - void *buf;
               rdx - size_t count; */
            uint8_t *buf;
            calc_mem(buf, RCX);
            RAX = io_read(RBX, buf, RDX);
            break;
        }
        case 0x04: // write
        {
            /* rbx - int fd;
               rcx - void *buf;
               rdx - size_t count; */
            uint8_t *buf;
            calc_mem(buf, RCX);
            RAX = io_write(RBX, buf, RDX);
            break;
        }
        case 0x05: // open
        {
            /* rbx - char *pathname;
               rcx - int flags;
               rdx - mode_t mode; */
            uint8_t *pathname;
            calc_mem(pathname, RBX);
            RAX = io_open(pathname, RCX, RDX);
            break;
        }
        case 0x06: // close
        {
            /* rbx - int fd; */
            RAX = io_close(RBX);
            break;
        }
        case 0x08: // creat
        {
            /* rbx - char *pathname;
               rcx - mode_t mode; */
            uint8_t *pathname;
            calc_mem(pathname, RBX);
            RAX = io_creat(pathname, RCX);
            break;
        }
        case 0x09: // link
        {
            /* rbx - const char *oldpath;
               rcx - const char *newpath; */
            uint8_t *oldpath, *newpath;
            calc_mem(oldpath, RBX);
            calc_mem(newpath, RCX);
            RAX = io_link(oldpath, newpath);
            break;
        }
        case 0xC0: // malloc
        {
            /* rbx - size_t size; */
            RAX = memmgr_alloc(RBX);
            break;
        }
        case 0xC1: // free
        {
            /* ebx - void *ptr; */
            memmgr_free(RBX);
            break;
        }
        case 0xC2: // memcpy
        {
            /* rbx - size_t num;
               rsi - void *src;
               rdi - void *dst; */
            uint8_t *src, *dst;
            calc_mem(src, RSI);
            calc_mem(dst, RDI);
            memcpy(dst, src, RBX);
            break;
        }
        case 0xC3: // memset
        {
            /* rbx - void *ptr;
               rcx - int value;
               rdx - size_t num; */
            uint8_t *ptr;
            calc_mem(ptr, RBX);
            memset(ptr, RCX, RDX);
            break;
        }
        case 0xC4: // rand
        {
            /* rbx - int rand_max (UINT64_MAX if 0); */
            RAX = rand() % (RBX ? RBX : UINT64_MAX);
            break;
        }
        case 0xC5: // sig_attach
        {
            /* rbx - int sig_id;
               rcx - void *ptr; */
            code_sigh *cs = malloc(sizeof(code_sigh));
            cs->pid = caller->pid;
            cs->offset = RCX;
            sig_attach(RBX, code_sighandler, cs);
        }
    }
}

int code_exec(struct CODE *p)
{
    uint64_t *regs = p->regs;
    uint8_t *pc = p->pc, *text = p->text, cmd;
    int pid = p->pid;
    for(;;)
    {
        cmd = get();
        switch(cmd)
        {
            case 0x01:  // DBG
            {
                regs_out();
                flags_out();
                break;
            }
            case 0x02:  // MOV REG REG
            {
                int x = get(), y = get();
                set_reg(x, get_reg(y));
                break;
            }
            case 0x03:  // MOV REG CONST
            {
                int reg = get(), size = get();
                memcpy(get_reg_ptr(reg), pc, size);
                skip(size);
                break;
            }
            case 0x04:  // MOV REG MEM
            {
                int mem, size, base, reg = get();
                uint64_t rel_addr = 0;
                uint8_t *addr;
                get_mem_ptr();
                memcpy(get_reg_ptr(reg), addr, size);
                break;
            }
            case 0x05:  // MOV MEM REG
            {
                int reg, mem, base, size;
                uint64_t rel_addr = 0;
                uint8_t *addr;
                get_mem_ptr();
                reg = get();
                memcpy(addr, get_reg_ptr(reg), size);
                break;
            }
            case 0x06:  // MOV MEM CONST
            {
                int mem, size, base;
                uint64_t rel_addr = 0;
                uint8_t *addr;
                get_mem_ptr();
                skip(1);
                memcpy(addr, pc, size);
                skip(size);
                break;
            }
            case 0x07:  // MOV MEM MEM
            {
                int mem, size, base;
                uint64_t rel_addr = 0;
                uint8_t *addr, *dest;
                get_mem_ptr();
                dest = addr; rel_addr = 0;
                get_mem_ptr();
                memcpy(dest, addr, size);
                break;
            }
            case 0x08:  // XCHG REG REG
            {
                int x = get(), y = get(), z = get_reg(x); // z <- x
                set_reg(x, get_reg(y)); // x <- y
                set_reg(y, z); // y <- z
                break;
            }
            case 0x09:  // XCHG REG MEM
            {
                int mem, size, base, reg = get();
                uint64_t rel_addr = 0, *pt = get_reg_ptr(reg), z = *pt; // z <- x
                uint8_t *addr;
                get_mem_ptr();
                memcpy(pt, addr, size); // x <- y
                memcpy(addr, &z, size); // y <- z
                break;
            }
            case 0x0A:   // XCHG MEM MEM
            {
                int mem, size, base;
                uint64_t rel_addr = 0, z = 0;
                uint8_t *addr, *dest;
                get_mem_ptr(); // read X
                dest = addr;
                memcpy(&z, dest, size); // z <- x
                rel_addr = 0; get_mem_ptr(); // read Y
                memcpy(dest, addr, size); // x <- y
                memcpy(addr, &z, size); // y <- z
                break;
            }
            case 0x0B:  // PUSHA
            {
                int i;
                uint64_t saved = RSP;
                for(i = 0; i < 8; i++)
                    if(i == 4)
                    {
                        push(&saved, 8);
                    } else {
                        push(&regs[pusha_order[i]], 8);
                    }
                break;
            }
            case 0x0C:  // POPA
            {
                int i;
                uint64_t stub;
                for(i = 7; i >=0; i--)
                    if(i == 4)
                    {
                        pop(&stub, 8);
                    } else {
                        pop(&regs[pusha_order[i]], 8);
                    }
                break;
            }
            case 0x0D:  // PUSH REG
            {
                int reg = get(), size = reg_size[reg];
                push(get_reg_ptr(reg), size);
                break;
            }
            case 0x0E:  // PUSH CONST
            {
                int size = get();
                push(pc, size);
                skip(size);
                break;
            }
            case 0x0F:  // PUSH MEM
            {
                int mem, size, base;
                uint64_t rel_addr = 0;
                uint8_t *addr;
                get_mem_ptr();
                push(addr, size);
                break;
            }
            case 0x10:  // POP REG
            {
                int reg = get(), size = reg_size[reg];
                pop(get_reg_ptr(reg), size);
                break;
            }
            case 0x11:  // POP MEM
            {
                int mem, size, base;
                uint64_t rel_addr = 0;
                uint8_t *addr;
                get_mem_ptr();
                pop(addr, size);
                break;
            }
            case 0x12:  // JMP REG
            {
                int reg;
            jmp_reg:
                reg = get();
                pc = 0;
                memcpy(&pc, get_reg_ptr(reg), reg_size[reg]);
                pc += (sizeof(void*) == 4) ? (uint32_t)text : (uint64_t)text;
                break;
             }
            case 0x13:  // JMP CONST
            {
                int size;
                uint8_t *addr;
            jmp_const:
                size = get(); addr = pc;
                pc = 0;
                memcpy(&pc, addr, size);
                pc += (sizeof(void*) == 4) ? (uint32_t)text : (uint64_t)text;
                break;
            }
            case 0x14:  // JMP MEM
            {
                int mem, size, base;
                uint64_t rel_addr;
                uint8_t *addr;
            jmp_mem:
                rel_addr = 0;
                get_mem_ptr();
                pc = 0;
                memcpy(&pc, addr, size);
                pc += (sizeof(void*) == 4) ? (uint32_t)text : (uint64_t)text;
                break;
            }
            case 0x15:  // EXIT
            {
                return TRUE;
            }
            case 0x16:  // INT REG
            {
                int reg = get();
                intr(get_reg(reg), (void*)p);
                break;
            }
            case 0x17:  // INT CONST
            {
                int size = get();
                uint8_t buf = 0;
                memcpy(&buf, pc, size);
                skip(size);
                intr(buf, (void*)p);
                break;
            }
            case 0x18:  // INT MEM
            {
                int mem, size, base;
                uint64_t rel_addr = 0, buf = 0;
                uint8_t *addr;
                get_mem_ptr();
                memcpy(&buf, addr, size);
                intr(buf, (void*)p);
                break;
            }
            case 0x19:  // INC REG
            case 0x1B:  // DEC REG
            {
                int reg = get(), size = reg_size[reg], power, tmp;
                uint64_t buf = 0, *ptr = get_reg_ptr(reg), sign;
                bclear(PF|ZF|SF|OF);
                power = size*8-1;
                memcpy(&buf, ptr, size);
                sign = 1LL<<power&buf;
                if(cmd == 0x19) ++buf; else --buf;
                memcpy(ptr, &buf, size);                
                tmp = buf & 0xFF;
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ (1LL<<(power+1)&buf)) bset(OF);
                break;
            }
            case 0x1A:  // INC MEM
            case 0x1C:  // DEC MEM
            {
                int mem, size, base, power, tmp;
                uint64_t rel_addr = 0, buf = 0, sign;
                uint8_t *addr;
                bclear(PF|ZF|SF|OF);
                get_mem_ptr();
                power = size*8-1;
                memcpy(&buf, addr, size);
                sign = 1LL<<power&buf;
                if(cmd == 0x1A) ++buf; else --buf;
                memcpy(addr, &buf, size);                
                tmp = buf & 0xFF;
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ (1LL<<(power+1)&buf)) bset(OF);
                break;
            }
            case 0x1D:  // ADD REG REG
            case 0x1E:  // SUB REG REG
            case 0x1F:  // MOD REG REG
            case 0x20:  // MUL REG REG
            case 0x21:  // DIV REG REG
            case 0x22:  // XOR REG REG
            case 0x23:  // OR  REG REG
            case 0x24:  // AND REG REG
            case 0x25:  // SHL REG REG
            case 0x26:  // SHR REG REG
            {
                int x, y, tmp, power, size;
                uint64_t *ptr, val, sign, buf;
            reg_reg:
                x = get(); y = get(); size = reg_size[x]; power = size*8-1;
                ptr = get_reg_ptr(x); buf = *ptr; val = get_reg(y);
                bclear(CF|PF|ZF|SF|OF);
                sign = (1LL<<power&buf) ^ (1LL<<power&val);
                switch(cmd)
                {
                    case 0x1D:  buf +=  val; break;
                    case 0x1E:  buf -=  val; break;
                    case 0x1F:  buf %=  val; break;
                    case 0x20:  buf *=  val; break;
                    case 0x21:  buf /=  val; break;
                    case 0x22:  buf ^=  val; break;
                    case 0x23:  buf |=  val; break;
                    case 0x24:  buf &=  val; break;
                    case 0x25:  buf <<= val; break;
                    case 0x26:  buf >>= val; break;
                }
                if(cmd == 0x1D) {
                    int a = *ptr>>power, b = val>>power, c = buf>>power;
                    if(a&&b || a&&!c || b&&!c) bset(CF);
                } else if(cmd == 0x1E) {
                    if(*ptr < val) bset(CF);
                } else if(cmd == 0x20 && val) {
                    if(*ptr > UINT64_MAX / val) bset(CF);
                }
                if(!p->cmp_flag) memcpy(ptr, &buf, size);
                tmp = buf & 0xFF;
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if((bget(SF)>>7 ^ sign>>power) ^ bget(CF)) bset(OF);
                p->cmp_flag = FALSE;
                break;
            }
            case 0x27:  // ADD REG CONST
            case 0x28:  // SUB REG CONST
            case 0x29:  // MOD REG CONST
            case 0x2A:  // MUL REG CONST
            case 0x2B:  // DIV REG CONST
            case 0x2C:  // XOR REG CONST
            case 0x2D:  // OR  REG CONST
            case 0x2E:  // AND REG CONST
            case 0x2F:  // SHL REG CONST
            case 0x30:  // SHR REG CONST
            {
                int reg, size, power, tmp;
                uint64_t val, *ptr, prev, sign, buf;
            reg_const:
                reg = get(); size = get();
                power = reg_size[reg]*8-1;
                val = 0; ptr = get_reg_ptr(reg); buf = *ptr;
                bclear(CF|PF|ZF|SF|OF);
                memcpy(&val, pc, size);
                sign = (1LL<<power&buf) ^ (1LL<<power&val);
                skip(size);
                switch(cmd)
                {
                    case 0x27:  buf +=  val; break;
                    case 0x28:  buf -=  val; break;
                    case 0x29:  buf %=  val; break;
                    case 0x2A:  buf *=  val; break;
                    case 0x2B:  buf /=  val; break;
                    case 0x2C:  buf ^=  val; break;
                    case 0x2D:  buf |=  val; break;
                    case 0x2E:  buf &=  val; break;
                    case 0x2F:  buf <<= val; break;
                    case 0x30:  buf >>= val; break;
                }
                if(cmd == 0x27) {
                    int a = *ptr>>power, b = val>>power, c = buf>>power;
                    if(a&&b || a&&!c || b&&!c) bset(CF);
                } else if(cmd == 0x28) {
                    if(*ptr < val) bset(CF);
                } else if(cmd == 0x2A && val) {
                    if(*ptr > UINT64_MAX / val) bset(CF);
                }
                if(!p->cmp_flag) memcpy(ptr, &buf, size);
                tmp = buf & 0xFF;
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if((bget(SF)>>7 ^ sign>>power) ^ bget(CF)) bset(OF);
                p->cmp_flag = FALSE;
                break;
            }
            case 0x31:  // ADD REG MEM
            case 0x32:  // SUB REG MEM
            case 0x33:  // MOD REG MEM
            case 0x34:  // MUL REG MEM
            case 0x35:  // DIV REG MEM
            case 0x36:  // XOR REG MEM
            case 0x37:  // OR  REG MEM
            case 0x38:  // AND REG MEM
            case 0x39:  // SHL REG MEM
            case 0x3A:  // SHR REG MEM
            {
                int reg, mem, size, base, power, tmp;
                uint8_t *addr;
                uint64_t rel_addr, val, *ptr, prev, sign, buf;
            reg_mem:
                reg = get(); power = reg_size[reg]*8-1;
                rel_addr = 0; val = 0; ptr = get_reg_ptr(reg); buf = *ptr;
                bclear(CF|PF|ZF|SF|OF);
                get_mem_ptr();
                memcpy(&val, addr, size);
                sign = (1LL<<power&buf) ^ (1LL<<power&val);
                switch(cmd)
                {
                    case 0x31:  buf +=  val; break;
                    case 0x32:  buf -=  val; break;
                    case 0x33:  buf %=  val; break;
                    case 0x34:  buf *=  val; break;
                    case 0x35:  buf /=  val; break;
                    case 0x36:  buf ^=  val; break;
                    case 0x37:  buf |=  val; break;
                    case 0x38:  buf &=  val; break;
                    case 0x39:  buf <<= val; break;
                    case 0x3A:  buf >>= val; break;
                }
                if(cmd == 0x31) {
                    int a = *ptr>>power, b = val>>power, c = buf>>power;
                    if(a&&b || a&&!c || b&&!c) bset(CF);
                } else if(cmd == 0x32) {
                    if(*ptr < val) bset(CF);
                } else if(cmd == 0x34 && val) {
                    if(*ptr > UINT64_MAX / val) bset(CF);
                }
                if(!p->cmp_flag) memcpy(ptr, &buf, size);
                tmp = buf & 0xFF;
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if((bget(SF)>>7 ^ sign>>power) ^ bget(CF)) bset(OF);
                p->cmp_flag = FALSE;
                break;
            }
            case 0x3B:  // ADD MEM REG
            case 0x3C:  // SUB MEM REG
            case 0x3D:  // MOD MEM REG
            case 0x3E:  // MUL MEM REG
            case 0x3F:  // DIV MEM REG
            case 0x40:  // XOR MEM REG
            case 0x41:  // OR  MEM REG
            case 0x42:  // AND MEM REG
            case 0x43:  // SHL MEM REG
            case 0x44:  // SHR MEM REG
            {
                int reg, mem, size, base, tmp, power;
                uint8_t *addr;
                uint64_t rel_addr, prev, val, sign, buf, _buf;
             mem_reg:
                rel_addr = 0; buf = 0;
                bclear(CF|PF|ZF|SF|OF);
                get_mem_ptr();
                power = size*8-1;
                reg = get();
                val = get_reg(reg);
                memcpy(&buf, addr, size);
                _buf = buf;
                sign = (1LL<<power&buf) ^ (1LL<<power&val);
                switch(cmd)
                {
                    case 0x3B:  buf +=  val; break;
                    case 0x3C:  buf -=  val; break;
                    case 0x3D:  buf %=  val; break;
                    case 0x3E:  buf *=  val; break;
                    case 0x3F:  buf /=  val; break;
                    case 0x40:  buf ^=  val; break;
                    case 0x41:  buf |=  val; break;
                    case 0x42:  buf &=  val; break;
                    case 0x43:  buf <<= val; break;
                    case 0x44:  buf >>= val; break;
                }
                if(cmd == 0x3B) {
                    int a = _buf>>power, b = val>>power, c = buf>>power;
                    if(a&&b || a&&!c || b&&!c) bset(CF);
                } else if(cmd == 0x3C) {
                    if(_buf < val) bset(CF);
                } else if(cmd == 0x3E && val) {
                    if(_buf > UINT64_MAX / val) bset(CF);
                }
                if(!p->cmp_flag) memcpy(addr, &buf, size);
                tmp = buf & 0xFF;
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if((bget(SF)>>7 ^ sign>>power) ^ bget(CF)) bset(OF);
                p->cmp_flag = FALSE;
                break;
            }
            case 0x45:  // ADD MEM CONST
            case 0x46:  // SUB MEM CONST
            case 0x47:  // MOD MEM CONST
            case 0x48:  // MUL MEM CONST
            case 0x49:  // DIV MEM CONST
            case 0x4A:  // XOR MEM CONST
            case 0x4B:  // OR  MEM CONST
            case 0x4C:  // AND MEM CONST
            case 0x4D:  // SHL MEM CONST
            case 0x4E:  // SHR MEM CONST
            {
                int mem, size, base, const_size, power, tmp;
                uint8_t *addr;
                uint64_t rel_addr, const_buf, sign, mem_buf, _mem_buf;
            mem_const:
                rel_addr = 0; mem_buf = 0; const_buf = 0;
                bclear(CF|PF|ZF|SF|OF);
                get_mem_ptr();
                power = size*8-1;
                memcpy(&mem_buf, addr, size);
                _mem_buf = mem_buf;
                const_size = get();
                memcpy(&const_buf, pc, const_size);
                sign = (1LL<<power&mem_buf) ^ (1LL<<(const_size*8-1)&const_buf);
                switch(cmd)
                {
                    case 0x45:  mem_buf +=  const_buf; break;
                    case 0x46:  mem_buf -=  const_buf; break;
                    case 0x47:  mem_buf %=  const_buf; break;
                    case 0x48:  mem_buf *=  const_buf; break;
                    case 0x49:  mem_buf /=  const_buf; break;
                    case 0x4A:  mem_buf ^=  const_buf; break;
                    case 0x4B:  mem_buf |=  const_buf; break;
                    case 0x4C:  mem_buf &=  const_buf; break;
                    case 0x4D:  mem_buf <<= const_buf; break;
                    case 0x4E:  mem_buf >>= const_buf; break;
                }
                if(cmd == 0x45) {
                    int a = _mem_buf>>power, b = const_buf>>power, c = mem_buf>>power;
                    if(a&&b || a&&!c || b&&!c) bset(CF);
                } else if(cmd == 0x46) {
                    if(_mem_buf < const_buf) bset(CF);
                } else if(cmd == 0x48 && const_buf) {
                    if(_mem_buf > UINT64_MAX / const_buf) bset(CF);
                }
                if(!p->cmp_flag) memcpy(addr, &mem_buf, size);
                skip(const_size);
                tmp = mem_buf & 0xFF;
                try_PF(tmp);
                if(!mem_buf) bset(ZF);
                if(1LL<<power&mem_buf) bset(SF);
                if((bget(SF)>>7 ^ sign>>power) ^ bget(CF)) bset(OF);
                p->cmp_flag = FALSE;
                break;
            }
            case 0x4F:  // ADD MEM MEM
            case 0x50:  // SUB MEM MEM
            case 0x51:  // MOD MEM MEM
            case 0x52:  // MUL MEM MEM
            case 0x53:  // DIV MEM MEM
            case 0x54:  // XOR MEM MEM
            case 0x55:  // OR  MEM MEM
            case 0x56:  // AND MEM MEM
            case 0x57:  // SHL MEM MEM
            case 0x58:  // SHR MEM MEM
            {
                int mem, size, base, power, tmp;
                uint8_t *addr, *dest;
                uint64_t rel_addr, src_buf, sign, dest_buf, _dest_buf;
            mem_mem:
                bclear(CF|PF|ZF|SF|OF);
                rel_addr = 0; dest_buf = 0; src_buf = 0;
                get_mem_ptr();
                power = size*8-1;
                memcpy(&dest_buf, addr, size);
                _dest_buf = dest_buf;
                dest = addr; rel_addr = 0;
                get_mem_ptr();
                memcpy(&src_buf, addr, size);
                sign = (1LL<<power&dest_buf) ^ (1LL<<power&src_buf);
                switch(cmd)
                {
                    case 0x4F:  dest_buf +=  src_buf; break;
                    case 0x50:  dest_buf -=  src_buf; break;
                    case 0x51:  dest_buf %=  src_buf; break;
                    case 0x52:  dest_buf *=  src_buf; break;
                    case 0x53:  dest_buf /=  src_buf; break;
                    case 0x54:  dest_buf ^=  src_buf; break;
                    case 0x55:  dest_buf |=  src_buf; break;
                    case 0x56:  dest_buf &=  src_buf; break;
                    case 0x57:  dest_buf <<= src_buf; break;
                    case 0x58:  dest_buf >>= src_buf; break;
                }
                if(cmd == 0x4F) {
                    int a = _dest_buf>>power, b = src_buf>>power, c = dest_buf>>power;
                    if(a&&b || a&&!c || b&&!c) bset(CF);
                } else if(cmd == 0x50) {
                    if(_dest_buf < src_buf) bset(CF);
                } else if(cmd == 0x52 && src_buf) {
                    if(_dest_buf > UINT64_MAX / src_buf) bset(CF);
                }
                if(!p->cmp_flag) memcpy(dest, &dest_buf, size);
                tmp = dest_buf & 0xFF;
                try_PF(tmp);
                if(!dest_buf) bset(ZF);
                if(1LL<<power&dest_buf) bset(SF);
                if((bget(SF)>>7 ^ sign>>power) ^ !!bget(CF)) bset(OF);
                p->cmp_flag = FALSE;
                break;
            }
            case 0x59:  // XLAT
            {
                *((uint8_t*)regs) = *(text+RBX+*((uint8_t*)regs));
                break;
            }
            case 0x5A:  // CMP REG REG
            {
                cmd = 0x1E;
                p->cmp_flag = TRUE;
                goto reg_reg;
            }
            case 0x5B:  // CMP REG CONST
            {
                cmd = 0x28;
                p->cmp_flag = TRUE;
                goto reg_const;
            }
            case 0x5C:  // CMP REG MEM
            {
                cmd = 0x32;
                p->cmp_flag = TRUE;
                goto reg_mem;
            }
            case 0x5D:  // CMP MEM REG
            {
                cmd = 0x3C;
                p->cmp_flag = TRUE;
                goto mem_reg;
            }
            case 0x5E:  // CMP MEM CONST
            {
                cmd = 0x46;
                p->cmp_flag = TRUE;
                goto mem_const;
            }
            case 0x5F:  // CMP MEM MEM
            {
                cmd = 0x50;
                p->cmp_flag = TRUE;
                goto mem_mem;
            }
            case 0x60:  // TEST REG REG
            {
                cmd = 0x24;
                p->cmp_flag = TRUE;
                goto reg_reg;
            }
            case 0x61:  // TEST REG CONST
            {
                cmd = 0x2E;
                p->cmp_flag = TRUE;
                goto reg_const;
            }
            case 0x62:  // TEST REG MEM
            {
                cmd = 0x38;
                p->cmp_flag = TRUE;
                goto reg_mem;
            }
            case 0x63:  // TEST MEM REG
            {
                cmd = 0x42;
                p->cmp_flag = TRUE;
                goto mem_reg;
            }
            case 0x64:  // TEST MEM CONST
            {
                cmd = 0x4C;
                p->cmp_flag = TRUE;
                goto mem_const;
            }
            case 0x65:  // TEST MEM MEM
            {
                cmd = 0x56;
                p->cmp_flag = TRUE;
                goto mem_mem;
            }
            case 0x66:  // LOOP REG
                if(--RCX) goto jmp_reg; else { skip_reg(); break; };
            case 0x67:  // LOOP CONST
                if(--RCX) goto jmp_const; else { skip_const(); break; };
            case 0x68:  // LOOP MEM
                if(--RCX) goto jmp_mem; else { skip_mem(); break; };
            case 0x69:  // JA,JNBE REG
                if(!bget(CF) && !bget(ZF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x6A:  // JE,JNBE CONST
                if(!bget(CF) && !bget(ZF)) goto jmp_const; else { skip_const(); break; };
            case 0x6B:  // JA,JNBE MEM
                if(!bget(CF) && !bget(ZF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x6C:  // JAE,JNB,JNC REG
                if(!bget(CF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x6D:  // JAE,JNB,JNC CONST
                if(!bget(CF)) goto jmp_const; else { skip_const(); break; };
            case 0x6E:  // JAE,JNB,JNC MEM
                if(!bget(CF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x6F:  // JB,JNAE,JC REG
                if(bget(CF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x70:  // JB,JNAE,JC CONST
                if(bget(CF)) goto jmp_const; else { skip_const(); break; };
            case 0x71:  // JB,JNAE,JC MEM
                if(bget(CF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x72:  // JBE,JNA REG
                if(bget(CF) || bget(ZF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x73:  // JBE,JNA CONST
                if(bget(CF) || bget(ZF)) goto jmp_const; else { skip_const(); break; };
            case 0x74:  // JBE,JNA MEM
                if(bget(CF) || bget(ZF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x75:  // JE,JZ REG
                if(bget(ZF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x76:  // JE,JZ CONST
                if(bget(ZF)) goto jmp_const; else { skip_const(); break; };
            case 0x77:  // JE,JZ MEM
                if(bget(ZF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x78:  // JG,JNLE REG
                if(!bget(ZF) && ((bget(SF)>>7) == (bget(OF)>>11))) goto jmp_reg; else { skip_reg(); break; };
            case 0x79:  // JG,JNLE CONST
                if(!bget(ZF) && ((bget(SF)>>7) == (bget(OF)>>11))) goto jmp_const; else { skip_const(); break; };
            case 0x7A:  // JG,JNLE MEM
                if(!bget(ZF) && ((bget(SF)>>7) == (bget(OF)>>11))) goto jmp_mem; else { skip_mem(); break; };
            case 0x7B:  // JGE,JNL REG
                if((bget(SF)>>7) == (bget(OF)>>11)) goto jmp_reg; else { skip_reg(); break; };
            case 0x7C:  // JGE,JNL CONST
                if((bget(SF)>>7) == (bget(OF)>>11)) goto jmp_const; else { skip_const(); break; };
            case 0x7D:  // JGE,JNL MEM
                if((bget(SF)>>7) == (bget(OF)>>11)) goto jmp_mem; else { skip_mem(); break; };
            case 0x7E:  // JL,JNGE REG
                if((bget(SF)>>7) != (bget(OF)>>11)) goto jmp_reg; else { skip_reg(); break; };
            case 0x7F:  // JL,JNGE CONST
                if((bget(SF)>>7) != (bget(OF)>>11)) goto jmp_const; else { skip_const(); break; };
            case 0x80:  // JL,JNGE MEM
                if((bget(SF)>>7) != (bget(OF)>>11)) goto jmp_mem; else { skip_mem(); break; };
            case 0x81:  // JLE,JNG REG
                if(bget(ZF) || ((bget(SF)>>7) != (bget(OF)>>11))) goto jmp_reg; else { skip_reg(); break; };
            case 0x82:  // JLE,JNG CONST
                if(bget(ZF) || ((bget(SF)>>7) != (bget(OF)>>11))) goto jmp_const; else { skip_const(); break; };
            case 0x83:  // JLE,JNG MEM
                if(bget(ZF) || ((bget(SF)>>7) != (bget(OF)>>11))) goto jmp_mem; else { skip_mem(); break; };
            case 0x84:  // JNE,JNZ REG
                if(!bget(ZF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x85:  // JNE,JNZ CONST
                if(!bget(ZF)) goto jmp_const; else { skip_const(); break; };
            case 0x86:  // JNE,JNZ MEM
                if(!bget(ZF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x87:  // JNO REG
                if(!bget(OF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x88:  // JNO CONST
                if(!bget(OF)) goto jmp_const; else { skip_const(); break; };
            case 0x89:  // JNO MEM
                if(!bget(OF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x8A:  // JO REG
                if(bget(OF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x8B:  // JO CONST
                if(bget(OF)) goto jmp_const; else { skip_const(); break; };
            case 0x8C:  // JO MEM
                if(bget(OF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x8D:  // JNP,JPO REG
                if(!bget(PF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x8E:  // JNP,JPO CONST
                if(!bget(PF)) goto jmp_const; else { skip_const(); break; };
            case 0x8F:  // JNP,JPO MEM
                if(!bget(PF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x90:  // JP,JPE REG
                if(bget(PF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x91:  // JP,JPE CONST
                if(bget(PF)) goto jmp_const; else { skip_const(); break; };
            case 0x92:  // JP,JPE MEM
                if(bget(PF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x93:  // JNS REG
                if(!bget(SF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x94:  // JNS CONST
                if(!bget(SF)) goto jmp_const; else { skip_const(); break; };
            case 0x95:  // JNS MEM
                if(!bget(SF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x96:  // JS REG
                if(bget(SF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x97:  // JS CONST
                if(bget(SF)) goto jmp_const; else { skip_const(); break; };
            case 0x98:  // JS MEM
                if(bget(SF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x99:  // LEA REG MEM
            {
                int reg = get(), mem, size, base;
                uint64_t rel_addr = 0;
                get_mem();
                memcpy(get_reg_ptr(reg), &rel_addr, size);
                break;
            }
            case 0x9A:  // NEG REG
            {
                int reg = get(), tmp;
                int size = reg_size[reg]; int power = size*8-1;
                uint64_t *ptr = get_reg_ptr(reg), buf = 0, sign;
                bclear(CF|PF|ZF|SF|OF);
                memcpy(&buf, ptr, size);
                sign = 1LL<<power&buf;
                buf = ~buf + 1;
                memcpy(ptr, &buf, size);
                tmp = buf & 0xFF;
                if(buf) bset(CF); else bset(ZF);
                try_PF(tmp);
                if(1LL<<power&buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ bget(CF)) bset(OF);
                break;
            }
            case 0x9B:  // NEG MEM
            {
                int mem, size, base, tmp, power;
                uint8_t *addr;
                uint64_t rel_addr = 0, buf = 0, sign;
                bclear(CF|PF|ZF|SF|OF);
                get_mem_ptr();
                memcpy(&buf, addr, size);
                power = size*8-1;
                sign = 1LL<<power&buf;
                buf = ~buf + 1;
                memcpy(addr, &buf, size);
                tmp = buf & 0xFF;
                if(buf) bset(CF); else bset(ZF);
                try_PF(tmp);
                if(1LL<<power&buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ bget(CF)) bset(OF);
                break;
            }
            case 0x9C:  // JECXZ REG
                if(!(RCX & 0xFFFFFFFF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x9D:  // JECXZ CONST
                if(!(RCX & 0xFFFFFFFF)) goto jmp_const; else { skip_const(); break; };
            case 0x9E:  // JECXZ MEM
                if(!(RCX & 0xFFFFFFFF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x9F:  // JRCXZ REG
                if(!RCX) goto jmp_reg; else { skip_reg(); break; };
            case 0xA0:  // JRCXZ CONST
                if(!RCX) goto jmp_const; else { skip_const(); break; };
            case 0xA1:  // JRCXZ MEM
                if(!RCX) goto jmp_mem; else { skip_mem(); break; };
            case 0xA2:  // NOT REG
            {
                int reg = get(), size = reg_size[reg];
                uint64_t *ptr = get_reg_ptr(reg), buf = 0;
                memcpy(&buf, ptr, size);
                buf = ~buf;
                memcpy(ptr, &buf, size);
                break;
            }
            case 0xA3:  // NOT MEM
            {
                int mem, size, base;
                uint8_t *addr;
                uint64_t rel_addr = 0, buf = 0;
                get_mem_ptr();
                memcpy(&buf, addr, size);
                buf = ~buf;
                memcpy(addr, &buf, size);
                break;
            }
            case 0xA4:  // CALL REG
            {
                int reg = get();
                uint64_t buf = ((uint64_t)pid<<56)|(pc-text);
                int64_t temp = 0;
                push(&buf, 8);
                memcpy(&temp, get_reg_ptr(reg), reg_size[reg]);
                if(temp < 0) // external call flag
                {
                    pid  = temp>>56&0x7F;
                    text = search(pid);
                }
                pc=text+(temp&0xFFFFFFFFFFFFFFLL);
                ++p->ret_count;
                break;
             }
            case 0xA5:  // CALL CONST
            {
                int size = get();
                uint8_t *addr = pc;
                uint64_t buf = ((uint64_t)pid<<56)|(pc-text+size);
                int64_t temp = 0;
                push(&buf, 8);
                memcpy(&temp, addr, size);
                if(temp < 0) // external call flag
                {
                    pid  = temp>>56&0x7F;
                    text = search(pid);
                }
                pc=text+(temp&0xFFFFFFFFFFFFFFLL);
                ++p->ret_count;
                break;
            }
            case 0xA6:  // CALL MEM
            {
                int mem, size, base;
                uint8_t *addr;
                uint64_t rel_addr = 0, buf;
                int64_t temp = 0;
                get_mem_ptr();
                buf = ((uint64_t)pid<<56)|(pc-text);
                push(&buf, 8);
                memcpy(&temp, addr, size);
                if(temp < 0) // external call flag
                {
                    pid  = temp>>56&0x7F;
                    text = search(pid);
                }
                pc=text+(temp&0xFFFFFFFFFFFFFFLL);
                ++p->ret_count;
                break;
            }
            case 0xA7:  // RET
            {
                uint64_t temp = 0;
            ret:
                if(!p->ret_count--) return TRUE;
                pop(&temp, 8);
                pid  = temp>>56;
                text = search(pid);
                pc=text+(temp&0xFFFFFFFFFFFFFFLL);
                break;
            }
            case 0xA8:  // RET CONST
            {
                int size = get();
                uint64_t buf = 0;
                memcpy(&buf, pc, size);
                skip(size);
                RSP += buf;
                goto ret;
            }
            case 0xA9:  // ENTER
            {
                push(&RBP, 8);
                RBP = RSP;
                break;
            }
            case 0xAA:  // ENTER CONST [CONST]
            {
                int size;
                uint64_t buf = 0;
                push(&RBP, 8);
                RBP = RSP;
                size = get();
                memcpy(&buf, pc, size);
                RSP -= buf;
                skip(size);
                break;
            }
            case 0xAB:  // LEAVE
            {
                RSP = RBP;
                pop(&RBP, 8);
                break;
            }
            case 0xAC:  // LEA MEM MEM
            {
                int mem, size, base;
                uint64_t rel_addr = 0;
                uint8_t *addr;
                get_mem_ptr();
                rel_addr = 0;
                get_mem();
                memcpy(addr, &rel_addr, size);
                break;
            }
        }
        //p->pc = pc;
    }
}

int code_free(struct CODE *p)
{
    free(p->regs);
    free(p->text);
    free(p);
    return TRUE;
}

void x86_exit(void *p)
{
    fprintf(stderr, ":: raised X86_EXIT signal\n");
    // destroy all program's data
    struct STACK *prev, *cur = head;
    while(cur != NULL)
    {
        prev = cur;
        cur  = cur->next;
        code_free(prev->item);
        free(prev);
    }
    // destroy signals' stack
    struct SIGNALS *sig_prev, *sig_cur = sig_head;
    while(sig_cur != NULL)
    {
        sig_prev = sig_cur;
        sig_cur  = sig_cur->next;
        free(sig_prev);
    }
    // signal action
    exit(0);
}

int main(int argc, char **argv)
{
    // set X86_EXIT handler
    sig_attach(X86_EXIT, x86_exit, NULL);

    // init internal kernel parts (platform abstraction layer, libs, drivers, etc)
    io_init();
    memmgr_init();
    platform_init();
    // set interrupt handler
    set_intr(0x32, int32_handler);
    // load libc
    code_load("libc.bin");

    // load application
    if(argc < 2)
    {
        fprintf(stderr, "No application specified\n");
    } else {
        struct CODE *test = code_load(argv[1]);
        code_exec(test);
    }

    // raise X86_EXIT
    sig_raise(X86_EXIT, NULL);
    return 0;
}
