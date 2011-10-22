#include "libs.h"

uint16_t flags;
int pid_counter;
int reg_size[]    = {4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1};
int pusha_order[] = {0,2,3,1,7,6,4,5};
handler inttable[INTTABLE_MAX];

struct STACK *head = NULL;
struct SIGNALS *sig_head = NULL;

struct CODE *code_load(char *fname)
{
    int i, shared_cnt;
    uint8_t header[8];
    uint32_t *shared;
    struct STACK *new = malloc(sizeof(struct STACK));
    FILE *f = fopen(fname, "rb");
    // get file size
    fseek(f, 0, SEEK_END);
    int sz = ftell(f) - 8;
    fseek(f, 0, SEEK_SET);
    struct CODE *p = malloc(sizeof(struct CODE));
    // read 'header' and 'text' sections
    fread(&header, 1, 8, f);
    p->regs = malloc(8*4);
    p->pid = header[1] & 0x80 ? header[1] & 0x7F : 64 + pid_counter++;
    shared_cnt = header[2] + (header[3] << 8);
    sz -= shared_cnt*4;
    shared = malloc(shared_cnt*4);
    fread(shared, 4, shared_cnt, f);
    for(i = 0; i < 8; i++) p->regs[i] = 0;
    p->text = malloc(p->regs[7] = (1<<((header[0]>>4)+5))+sz);
    fread(p->text, sz, 1, f);
    for(i = 0; i < shared_cnt; i++) p->text[shared[i]] = 0x80|p->pid;
    fclose(f);  
    --p->regs[7];
    p->regs[7] += (0x80|p->pid)<<24;
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

void sig_handler(void *p)
{
    // EAX: action (0 - raise, 1 - attach)
    // EBX: sig number
    // ECX(optional): handler ptr
}

void sig_attach(int sig, handler h)
{
    struct SIGNALS *new = malloc(sizeof(struct SIGNALS));
    new->sig = sig;
    new->h = h;
    new->next = sig_head;
    sig_head = new;
}

void sig_raise(int sig, void *p)
{
    struct SIGNALS *cur = sig_head;
    while(cur != NULL)
    {
        if(cur->sig == sig)
            cur->h(p);
        cur = cur->next;
    }
}

int code_exec(struct CODE *p)
{
    uint32_t *regs = p->regs;
    uint8_t *pc = p->pc, *text = p->text, cmd;
    int pid = p->pid;
    for(;;)
    {
        cmd = get();
        switch(cmd)
        {
            case 0x01:  // DBG
            {
                fprintf(stderr, ":: EAX=%d EBX=%d ECX=%d EDX=%d ; ", EAX, EBX, ECX, EDX);
                fprintf(stderr, ":: CF=%d PF=%d ZF=%d SF=%d OF=%d\n", bget(CF), bget(PF), bget(ZF), bget(SF), bget(OF));
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
                uint32_t rel_addr = 0;
                uint8_t *addr;
                get_mem_ptr();
                memcpy(get_reg_ptr(reg), addr, size);
                break;
            }
            case 0x05:  // MOV MEM REG
            {
                int reg, mem, base, size;
                uint32_t rel_addr = 0;
                uint8_t *addr;
                get_mem_ptr();
                reg = get();
                memcpy(addr, get_reg_ptr(reg), size);
                break;
            }
            case 0x06:  // MOV MEM CONST
            {
                int mem, size, base;
                uint32_t rel_addr = 0;
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
                uint32_t rel_addr = 0;
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
                uint32_t rel_addr = 0, *pt = get_reg_ptr(reg), z = *pt; // z <- x
                uint8_t *addr;
                get_mem_ptr();
                memcpy(pt, addr, size); // x <- y
                memcpy(addr, &z, size); // y <- z
                break;
            }
            case 0x0A:   // XCHG MEM MEM
            {
                int mem, size, base;
                uint32_t rel_addr = 0, z = 0;
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
                uint32_t saved = ESP;
                for(i = 0; i < 8; i++)
                    if(i == 4)
                    {
                        push(&saved, 4);
                    } else {
                        push(&regs[pusha_order[i]], 4);
                    }
                break;
            }
            case 0x0C:  // POPA
            {
                int i;
                uint32_t stub;
                for(i = 7; i >=0; i--)
                    if(i == 4)
                    {
                        pop(&stub, 4);
                    } else {
                        pop(&regs[pusha_order[i]], 4);
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
                uint32_t rel_addr = 0;
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
                uint32_t rel_addr = 0;
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
                pc+=(uint32_t)text;
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
                pc+=(uint32_t)text;
                break;
            }
            case 0x14:  // JMP MEM
            {
                int mem, size, base;
                uint32_t rel_addr;
                uint8_t *addr;
            jmp_mem:
                rel_addr = 0;
                get_mem_ptr();
                pc = 0;
                memcpy(&pc, addr, size);
                pc+=(uint32_t)text;
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
                uint32_t rel_addr = 0, buf = 0;
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
                uint32_t buf = 0, *ptr = get_reg_ptr(reg);
                uint64_t sign;
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
                uint32_t rel_addr = 0, buf = 0;
                uint8_t *addr;
                uint64_t sign;
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
                uint32_t *ptr, val;
                uint64_t sign, buf;
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
                if(!p->cmp_flag) memcpy(ptr, &buf, size);
                tmp = buf & 0xFF;
                if(1LL<<(power+1)&buf) bset(CF);
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ bget(CF)) bset(OF);
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
                uint32_t val, *ptr, prev;
                uint64_t sign, buf;
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
                if(!p->cmp_flag) memcpy(ptr, &buf, size);
                tmp = buf & 0xFF;
                if(1LL<<(power+1)&buf) bset(CF);
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ bget(CF)) bset(OF);
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
                uint32_t rel_addr, val, *ptr, prev;
                uint64_t sign, buf;
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
                if(!p->cmp_flag) memcpy(ptr, &buf, size);
                tmp = buf & 0xFF;
                if(1LL<<(power+1)&buf) bset(CF);
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ bget(CF)) bset(OF);
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
                uint32_t rel_addr, prev, val;
                uint64_t sign, buf;
             mem_reg:
                rel_addr = 0; buf = 0;
                bclear(CF|PF|ZF|SF|OF);
                get_mem_ptr();
                power = size*8-1;
                reg = get();
                val = get_reg(reg);
                memcpy(&buf, addr, size);
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
                if(!p->cmp_flag) memcpy(addr, &buf, size);
                tmp = buf & 0xFF;
                if(1LL<<(power+1)&buf) bset(CF);
                try_PF(tmp);
                if(!buf) bset(ZF);
                if(1LL<<power&buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ bget(CF)) bset(OF);
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
                uint32_t rel_addr, const_buf;
                uint64_t sign, mem_buf;
            mem_const:
                rel_addr = 0; mem_buf = 0; const_buf = 0;
                bclear(CF|PF|ZF|SF|OF);
                get_mem_ptr();
                power = size*8-1;
                memcpy(&mem_buf, addr, size);
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
                if(!p->cmp_flag) memcpy(addr, &mem_buf, size);
                skip(const_size);
                tmp = mem_buf & 0xFF;
                if(1LL<<(power+1)&mem_buf) bset(CF);
                try_PF(tmp);
                if(!mem_buf) bset(ZF);
                if(1LL<<power&mem_buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ bget(CF)) bset(OF);
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
                uint32_t rel_addr, src_buf;
                uint64_t sign, dest_buf;
            mem_mem:
                bclear(CF|PF|ZF|SF|OF);
                rel_addr = 0; dest_buf = 0; src_buf = 0;
                get_mem_ptr();
                power = size*8-1;
                memcpy(&dest_buf, addr, size);
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
                if(!p->cmp_flag) memcpy(dest, &dest_buf, size);
                tmp = dest_buf & 0xFF;
                if(1LL<<(power+1)&dest_buf) bset(CF);
                try_PF(tmp);
                if(!dest_buf) bset(ZF);
                if(1LL<<power&dest_buf) bset(SF);
                if(((bget(SF) ^ sign)>>7) ^ bget(CF)) bset(OF);
                p->cmp_flag = FALSE;
                break;
            }
            case 0x59:  // XLAT
            {
                *((uint8_t*)regs) = *(text+EBX+*((uint8_t*)regs));
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
                if(--ECX) goto jmp_reg; else { skip_reg(); break; };
            case 0x67:  // LOOP CONST
                if(--ECX) goto jmp_const; else { skip_const(); break; };
            case 0x68:  // LOOP MEM
                if(--ECX) goto jmp_mem; else { skip_mem(); break; };
            case 0x69:  // JA,JBE REG
                if(!bget(CF)&&!bget(ZF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x6A:  // JE,JBE CONST
                if(!bget(CF)&&!bget(ZF)) goto jmp_const; else { skip_const(); break; };
            case 0x6B:  // JA,JBE MEM
                if(!bget(CF)&&!bget(ZF)) goto jmp_mem; else { skip_mem(); break; };
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
                if(bget(CF)||bget(ZF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x73:  // JBE,JNA CONST
                if(bget(CF)||bget(ZF)) goto jmp_const; else { skip_const(); break; };
            case 0x74:  // JBE,JNA MEM
                if(bget(CF)||bget(ZF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x75:  // JE,JZ REG
                if(bget(ZF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x76:  // JE,JZ CONST
                if(bget(ZF)) goto jmp_const; else { skip_const(); break; };
            case 0x77:  // JE,JZ MEM
                if(bget(ZF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x78:  // JG,JNLE REG
                if(!bget(ZF)&&((bget(SF)>>7)==(bget(OF)>>11))) goto jmp_reg; else { skip_reg(); break; };
            case 0x79:  // JG,JNLE CONST
                if(!bget(ZF)&&((bget(SF)>>7)==(bget(OF)>>11))) goto jmp_const; else { skip_const(); break; };
            case 0x7A:  // JG,JNLE MEM
                if(!bget(ZF)&&((bget(SF)>>7)==(bget(OF)>>11))) goto jmp_mem; else { skip_mem(); break; };
            case 0x7B:  // JGE,JNL REG
                if((bget(SF)>>7)==(bget(OF)>>11)) goto jmp_reg; else { skip_reg(); break; };
            case 0x7C:  // JGE,JNL CONST
                if((bget(SF)>>7)==(bget(OF)>>11)) goto jmp_const; else { skip_const(); break; };
            case 0x7D:  // JGE,JNL MEM
                if((bget(SF)>>7)==(bget(OF)>>11)) goto jmp_mem; else { skip_mem(); break; };
            case 0x7E:  // JL,JNGE REG
                if((bget(SF)>>7)!=(bget(OF)>>11)) goto jmp_reg; else { skip_reg(); break; };
            case 0x7F:  // JL,JNGE CONST
                if((bget(SF)>>7)!=(bget(OF)>>11)) goto jmp_const; else { skip_const(); break; };
            case 0x80:  // JL,JNGE MEM
                if((bget(SF)>>7)!=(bget(OF)>>11)) goto jmp_mem; else { skip_mem(); break; };
            case 0x81:  // JLE,JNG REG
                if(bget(ZF)||((bget(SF)>>7)!=(bget(OF)>>11))) goto jmp_reg; else { skip_reg(); break; };
            case 0x82:  // JLE,JNG CONST
                if(bget(ZF)||((bget(SF)>>7)!=(bget(OF)>>11))) goto jmp_const; else { skip_const(); break; };
            case 0x83:  // JLE,JNG MEM
                if(bget(ZF)||((bget(SF)>>7)!=(bget(OF)>>11))) goto jmp_mem; else { skip_mem(); break; };
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
                uint32_t rel_addr = 0;
                get_mem();
                memcpy(get_reg_ptr(reg), &rel_addr, size);
                break;
            }
            case 0x9A:  // NEG REG
            {
                int reg = get(), tmp;
                int size = reg_size[reg]; int power = size*8-1;
                uint32_t *ptr = get_reg_ptr(reg), buf = 0;
                uint64_t sign;
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
                uint32_t rel_addr = 0, buf = 0;
                uint64_t sign;
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
            case 0x9C:  // JCXZ REG
                if(!(ECX & 0xFFFF)) goto jmp_reg; else { skip_reg(); break; };
            case 0x9D:  // JCXZ CONST
                if(!(ECX & 0xFFFF)) goto jmp_const; else { skip_const(); break; };
            case 0x9E:  // JCXZ MEM
                if(!(ECX & 0xFFFF)) goto jmp_mem; else { skip_mem(); break; };
            case 0x9F:  // JECXZ REG
                if(!ECX) goto jmp_reg; else { skip_reg(); break; };
            case 0xA0:  // JECXZ CONST
                if(!ECX) goto jmp_const; else { skip_const(); break; };
            case 0xA1:  // JECXZ MEM
                if(!ECX) goto jmp_mem; else { skip_mem(); break; };
            case 0xA2:  // NOT REG
            {
                int reg = get(), size = reg_size[reg];
                uint32_t *ptr = get_reg_ptr(reg), buf = 0;
                memcpy(&buf, ptr, size);
                buf = ~buf;
                memcpy(ptr, &buf, size);
                break;
            }
            case 0xA3:  // NOT MEM
            {
                int mem, size, base;
                uint8_t *addr;
                uint32_t rel_addr = 0, buf = 0;
                get_mem_ptr();
                memcpy(&buf, addr, size);
                buf = ~buf;
                memcpy(addr, &buf, size);
                break;
            }
            case 0xA4:  // CALL REG
            {
                int reg = get();
                uint32_t buf = (uint32_t)((pid<<24)+pc-text);
                int32_t temp = 0;
                push(&buf, 4);
                memcpy(&temp, get_reg_ptr(reg), reg_size[reg]);
                if(temp < 0) // external call flag
                {
                    pid  = temp>>24&0x7F;
                    text = search(pid);
                }
                pc=text+(temp&0xFFFFFF);
                ++p->ret_count;
                break;
             }
            case 0xA5:  // CALL CONST
            {
                int size = get();
                uint8_t *addr = pc;
                uint32_t buf = (uint32_t)((pid<<24)+pc-text+size);
                int32_t temp = 0;
                push(&buf, 4);
                memcpy(&temp, addr, size);
                if(temp < 0) // external call flag
                {
                    pid  = temp>>24&0x7F;
                    text = search(pid);
                }
                pc=text+(temp&0xFFFFFF);
                ++p->ret_count;
                break;
            }
            case 0xA6:  // CALL MEM
            {
                int mem, size, base;
                uint8_t *addr;
                uint32_t rel_addr = 0, buf;
                int32_t temp = 0;
                get_mem_ptr();
                buf = (uint32_t)((pid<<24)+pc-text);
                push(&buf, 4);
                memcpy(&temp, addr, size);
                if(temp < 0) // external call flag
                {
                    pid  = temp>>24&0x7F;
                    text = search(pid);
                }
                pc=text+(temp&0xFFFFFF);
                ++p->ret_count;
                break;
            }
            case 0xA7:  // RET
            {
                uint32_t temp = 0;
            ret:
                if(!p->ret_count--) return TRUE;
                pop(&temp, 4);
                pid  = temp>>24;
                text = search(pid);
                pc=text+(temp&0xFFFFFF);
                break;
            }
            case 0xA8:  // RET CONST
            {
                int size = get();
                uint32_t buf = 0;
                memcpy(&buf, pc, size);
                skip(size);
                ESP += buf;
                goto ret;
            }
            case 0xA9:  // ENTER
            {
                push(&EBP, 4);
                EBP = ESP;
                break;
            }
            case 0xAA:  // ENTER CONST [CONST]
            {
                int size;
                uint32_t buf = 0;
                push(&EBP, 4);
                EBP = ESP;
                size = get();
                memcpy(&buf, pc, size);
                ESP -= buf;
                skip(size);
                break;
            }
            case 0xAB:  // LEAVE
            {
                ESP = EBP;
                pop(&EBP, 4);
                break;
            }
        }
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
    struct STACK *cur = head;
    while(cur != NULL)
    {
        code_free(cur->item);
        cur = cur->next;
    }

    // signal action
    exit(0);
}

int main(int argc, char **argv)
{
    char *app_path;

    // init signal interrupt
    set_intr(0, sig_handler);

    // set X86_EXIT handler
    sig_attach(X86_EXIT, x86_exit);

    // init libraries
    #if defined(_WIN32) || defined(WIN32)
        win32_init();
    #else
        linux_init();
    #endif
    // load libc
    code_load("libc.bin");

    // load application
    if(argc < 2)
    {
        app_path = malloc(9);
        strcpy(app_path, "test.bin");
    } else {
        app_path = malloc(strlen(argv[1])+1);
        strcpy(app_path, argv[1]);
    }
    struct CODE *test = code_load(app_path);
    free(app_path);
    code_exec(test);

    // raise X86_EXIT
    sig_raise(X86_EXIT, NULL);
    return 0;
}
