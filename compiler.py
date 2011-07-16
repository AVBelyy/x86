#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os, re, math

class _:
    BYTE    =    1
    WORD    =    2
    DWORD   =    4

    registers = {
    #   regname         id    size
        "eax":     (    0,    DWORD   ),
        "ebx":     (    1,    DWORD   ),
        "ecx":     (    2,    DWORD   ),
        "edx":     (    3,    DWORD   ),
        "esi":     (    4,    DWORD   ),
        "edi":     (    5,    DWORD   ),
        "ebp":     (    6,    DWORD   ),
        "esp":     (    7,    DWORD   ),
        "ax":      (    8,    WORD    ),
        "bx":      (    9,    WORD    ),
        "cx":      (    10,   WORD    ),
        "dx":      (    11,   WORD    ),
        "si":      (    12,   WORD    ),
        "di":      (    13,   WORD    ),
        "bp":      (    14,   WORD    ),
        "sp":      (    15,   WORD    ),
        "al":      (    16,   BYTE    ),
        "bl":      (    17,   BYTE    ),
        "cl":      (    18,   BYTE    ),
        "dl":      (    19,   BYTE    ),
        "ah":      (    20,   BYTE    ),
        "bh":      (    21,   BYTE    ),
        "ch":      (    22,   BYTE    ),
        "dh":      (    23,   BYTE    )
    }

    sizes = {
    #    sizename        id        size
        "byte":     (    BYTE,     BYTE,    ),
        "word":     (    WORD,     WORD,    ),
        "dword":    (    DWORD,    DWORD,   )
    }

    datadirs = {
    #    dirname    size
        "db":       BYTE,
        "dw":       WORD,
        "du":       WORD,
        "dd":       DWORD
    }

    opcodes = (
            #        cmd        #1        #2        #3
        (   0x01,    "dbg",     "",       "",       ""    ),    # (√)
        (   0x02,    "mov",     "reg",    "reg",    ""    ),    # (√)
        (   0x03,    "mov",     "reg",    "const",  ""    ),    # (√)
        (   0x04,    "mov",     "reg",    "mem",    ""    ),    # (√)
        (   0x05,    "mov",     "mem",    "reg",    ""    ),    # (√)
        (   0x06,    "mov",     "mem",    "const",  ""    ),    # (√)
        (   0x07,    "mov",     "mem",    "mem",    ""    ),    # (√)
        (   0x08,    "xchg",    "reg",    "reg",    ""    ),    # (√)
        (   0x09,    "xchg",    "reg",    "mem",    ""    ),    # (√)
        (   0x09,    "xchg",    "mem",    "reg",    ""    ),    # (√)
        (   0x0A,    "xchg",    "mem",    "mem",    ""    ),    # (√)
        (   0x0B,    "pusha",   "",       "",       ""    ),    # (√)
        (   0x0B,    "pushad",  "",       "",       ""    ),    # (√)
        (   0x0C,    "popa",    "",       "",       ""    ),    # (√)
        (   0x0C,    "popad",   "",       "",       ""    ),    # (√)
        (   0x0D,    "push",    "reg",    "",       ""    ),    # (√)
        (   0x0E,    "push",    "const",  "",       ""    ),    # (√)
        (   0x0F,    "push",    "mem",    "",       ""    ),    # (√)
        (   0x10,    "pop",     "reg",    "",       ""    ),    # (√)
        (   0x11,    "pop",     "mem",    "",       ""    ),    # (√)
        (   0x12,    "jmp",     "reg",    "",       ""    ),    # (√)
        (   0x13,    "jmp",     "const",  "",       ""    ),    # (√)
        (   0x14,    "jmp",     "mem",    "",       ""    ),    # (√)
        (   0x15,    "exit",    "",       "",       ""    ),    # (√)
        (   0x16,    "int",     "reg",    "",       ""    ),    # (√)
        (   0x17,    "int",     "const",  "",       ""    ),    # (√)
        (   0x18,    "int",     "mem",    "",       ""    ),    # (√)
        (   0x19,    "inc",     "reg",    "",       ""    ),    # (√)
        (   0x1A,    "inc",     "mem",    "",       ""    ),    # (√)
        (   0x1B,    "dec",     "reg",    "",       ""    ),    # (√)
        (   0x1C,    "dec",     "mem",    "",       ""    ),    # (√)
        (   0x1D,    "add",     "reg",    "reg",    ""    ),    # (√)
        (   0x1E,    "sub",     "reg",    "reg",    ""    ),    # (√)
        (   0x1F,    "mod",     "reg",    "reg",    ""    ),    # (√)
        (   0x20,    "mul",     "reg",    "reg",    ""    ),    # (√)
        (   0x21,    "div",     "reg",    "reg",    ""    ),    # (√)
        (   0x22,    "xor",     "reg",    "reg",    ""    ),    # (√)
        (   0x23,    "or",      "reg",    "reg",    ""    ),    # (√)
        (   0x24,    "and",     "reg",    "reg",    ""    ),    # (√)
        (   0x25,    "shl",     "reg",    "reg",    ""    ),    # (√)
        (   0x25,    "sal",     "reg",    "reg",    ""    ),    # (√)
        (   0x26,    "shr",     "reg",    "reg",    ""    ),    # (√)
        (   0x27,    "add",     "reg",    "const",  ""    ),    # (√)
        (   0x28,    "sub",     "reg",    "const",  ""    ),    # (√)
        (   0x29,    "mod",     "reg",    "const",  ""    ),    # (√)
        (   0x2A,    "mul",     "reg",    "const",  ""    ),    # (√)
        (   0x2B,    "div",     "reg",    "const",  ""    ),    # (√)
        (   0x2C,    "xor",     "reg",    "const",  ""    ),    # (√)
        (   0x2D,    "or",      "reg",    "const",  ""    ),    # (√)
        (   0x2E,    "and",     "reg",    "const",  ""    ),    # (√)
        (   0x2F,    "shl",     "reg",    "const",  ""    ),    # (√)
        (   0x2F,    "sal",     "reg",    "const",  ""    ),    # (√)
        (   0x30,    "shr",     "reg",    "const",  ""    ),    # (√)
        (   0x31,    "add",     "reg",    "mem",    ""    ),    # (√)
        (   0x32,    "sub",     "reg",    "mem",    ""    ),    # (√)
        (   0x33,    "mod",     "reg",    "mem",    ""    ),    # (√)
        (   0x34,    "mul",     "reg",    "mem",    ""    ),    # (√)
        (   0x35,    "div",     "reg",    "mem",    ""    ),    # (√)
        (   0x36,    "xor",     "reg",    "mem",    ""    ),    # (√)
        (   0x37,    "or",      "reg",    "mem",    ""    ),    # (√)
        (   0x38,    "and",     "reg",    "mem",    ""    ),    # (√)
        (   0x39,    "shl",     "reg",    "mem",    ""    ),    # (√)
        (   0x39,    "sal",     "reg",    "mem",    ""    ),    # (√)
        (   0x3A,    "shr",     "reg",    "mem",    ""    ),    # (√)
        (   0x3B,    "add",     "mem",    "reg",    ""    ),    # (√)
        (   0x3C,    "sub",     "mem",    "reg",    ""    ),    # (√)
        (   0x3D,    "mod",     "mem",    "reg",    ""    ),    # (√)
        (   0x3E,    "mul",     "mem",    "reg",    ""    ),    # (√)
        (   0x3F,    "div",     "mem",    "reg",    ""    ),    # (√)
        (   0x40,    "xor",     "mem",    "reg",    ""    ),    # (√)
        (   0x41,    "or",      "mem",    "reg",    ""    ),    # (√)
        (   0x42,    "and",     "mem",    "reg",    ""    ),    # (√)
        (   0x43,    "shl",     "mem",    "reg",    ""    ),    # (√)
        (   0x43,    "sal",     "mem",    "reg",    ""    ),    # (√)
        (   0x44,    "shr",     "mem",    "reg",    ""    ),    # (√)
        (   0x45,    "add",     "mem",    "const",  ""    ),    # (√)
        (   0x46,    "sub",     "mem",    "const",  ""    ),    # (√)
        (   0x47,    "mod",     "mem",    "const",  ""    ),    # (√)
        (   0x48,    "mul",     "mem",    "const",  ""    ),    # (√)
        (   0x49,    "div",     "mem",    "const",  ""    ),    # (√)
        (   0x4A,    "xor",     "mem",    "const",  ""    ),    # (√)
        (   0x4B,    "or",      "mem",    "const",  ""    ),    # (√)
        (   0x4C,    "and",     "mem",    "const",  ""    ),    # (√)
        (   0x4D,    "shl",     "mem",    "const",  ""    ),    # (√)
        (   0x4D,    "sal",     "mem",    "const",  ""    ),    # (√)
        (   0x4E,    "shr",     "mem",    "const",  ""    ),    # (√)
        (   0x4F,    "add",     "mem",    "mem",    ""    ),    # (√)
        (   0x50,    "sub",     "mem",    "mem",    ""    ),    # (√)
        (   0x51,    "mod",     "mem",    "mem",    ""    ),    # (√)
        (   0x52,    "mul",     "mem",    "mem",    ""    ),    # (√)
        (   0x53,    "div",     "mem",    "mem",    ""    ),    # (√)
        (   0x54,    "xor",     "mem",    "mem",    ""    ),    # (√)
        (   0x55,    "or",      "mem",    "mem",    ""    ),    # (√)
        (   0x56,    "and",     "mem",    "mem",    ""    ),    # (√)
        (   0x57,    "shl",     "mem",    "mem",    ""    ),    # (√)
        (   0x57,    "sal",     "mem",    "mem",    ""    ),    # (√)
        (   0x58,    "shr",     "mem",    "mem",    ""    ),    # (√)
        (   0x59,    "xlat",    "",       "",       ""    ),    # (√)
        (   0x5A,    "cmp",     "reg",    "reg",    ""    ),    # (√)
        (   0x5B,    "cmp",     "reg",    "const",  ""    ),    # (√)
        (   0x5C,    "cmp",     "reg",    "mem",    ""    ),    # (√)
        (   0x5D,    "cmp",     "mem",    "reg",    ""    ),    # (√)
        (   0x5E,    "cmp",     "mem",    "const",  ""    ),    # (√)
        (   0x5F,    "cmp" ,    "mem",    "mem",    ""    ),    # (√)
        (   0x60,    "test",    "reg",    "reg",    ""    ),    # (√)
        (   0x61,    "test",    "reg",    "const",  ""    ),    # (√)
        (   0x62,    "test",    "reg",    "mem",    ""    ),    # (√)
        (   0x63,    "test",    "mem",    "reg",    ""    ),    # (√)
        (   0x64,    "test",    "mem",    "const",  ""    ),    # (√)
        (   0x65,    "test",    "mem",    "mem",    ""    ),    # (√)
        (   0x66,    "loop",    "reg",    "",       ""    ),    # (√)
        (   0x67,    "loop",    "const",  "",       ""    ),    # (√)
        (   0x68,    "loop",    "mem",    "",       ""    ),    # (√)
        (   0x69,    "ja",      "reg",    "",       ""    ),    # (√)
        (   0x6A,    "ja",      "const",  "",       ""    ),    # (√)
        (   0x6B,    "ja",      "mem",    "",       ""    ),    # (√)
        (   0x69,    "jbe",     "reg",    "",       ""    ),    # (√)
        (   0x6A,    "jbe",     "const",  "",       ""    ),    # (√)
        (   0x6B,    "jbe",     "mem",    "",       ""    ),    # (√)
        (   0x6C,    "jae",     "reg",    "",       ""    ),    # (√)
        (   0x6D,    "jae",     "const",  "",       ""    ),    # (√)
        (   0x6E,    "jae",     "mem",    "",       ""    ),    # (√)
        (   0x6C,    "jnb",     "reg",    "",       ""    ),    # (√)
        (   0x6D,    "jnb",     "const",  "",       ""    ),    # (√)
        (   0x6E,    "jnb",     "mem",    "",       ""    ),    # (√)
        (   0x6C,    "jnc",     "reg",    "",       ""    ),    # (√)
        (   0x6D,    "jnc",     "const",  "",       ""    ),    # (√)
        (   0x6E,    "jnc",     "mem",    "",       ""    ),    # (√)
        (   0x6F,    "jb",      "reg",    "",       ""    ),    # (√)
        (   0x70,    "jb",      "const",  "",       ""    ),    # (√)
        (   0x71,    "jb",      "mem",    "",       ""    ),    # (√)
        (   0x6F,    "jnae",    "reg",    "",       ""    ),    # (√)
        (   0x70,    "jnae",    "const",  "",       ""    ),    # (√)
        (   0x71,    "jnae",    "mem",    "",       ""    ),    # (√)
        (   0x6F,    "jc",      "reg",    "",       ""    ),    # (√)
        (   0x70,    "jc",      "const",  "",       ""    ),    # (√)
        (   0x71,    "jc",      "mem",    "",       ""    ),    # (√)
        (   0x72,    "jbe",     "reg",    "",       ""    ),    # (√)
        (   0x73,    "jbe",     "const",  "",       ""    ),    # (√)
        (   0x74,    "jbe",     "mem",    "",       ""    ),    # (√)
        (   0x72,    "jna",     "reg",    "",       ""    ),    # (√)
        (   0x73,    "jna",     "const",  "",       ""    ),    # (√)
        (   0x74,    "jna",     "mem",    "",       ""    ),    # (√)
        (   0x75,    "je",      "reg",    "",       ""    ),    # (√)
        (   0x76,    "je",      "const",  "",       ""    ),    # (√)
        (   0x77,    "je",      "mem",    "",       ""    ),    # (√)
        (   0x75,    "jz",      "reg",    "",       ""    ),    # (√)
        (   0x76,    "jz",      "const",  "",       ""    ),    # (√)
        (   0x77,    "jz",      "mem",    "",       ""    ),    # (√)
        (   0x78,    "jg",      "reg",    "",       ""    ),    # (√)
        (   0x79,    "jg",      "const",  "",       ""    ),    # (√)
        (   0x7A,    "jg",      "mem",    "",       ""    ),    # (√)
        (   0x78,    "jnle",    "reg",    "",       ""    ),    # (√)
        (   0x79,    "jnle",    "const",  "",       ""    ),    # (√)
        (   0x7A,    "jnle",    "mem",    "",       ""    ),    # (√)
        (   0x7B,    "jge",     "reg",    "",       ""    ),    # (√)
        (   0x7C,    "jge",     "const",  "",       ""    ),    # (√)
        (   0x7D,    "jge",     "mem",    "",       ""    ),    # (√)
        (   0x7B,    "jnl",     "reg",    "",       ""    ),    # (√)
        (   0x7C,    "jnl",     "const",  "",       ""    ),    # (√)
        (   0x7D,    "jnl",     "mem",    "",       ""    ),    # (√)
        (   0x7E,    "jl",      "reg",    "",       ""    ),    # (√)
        (   0x7F,    "jl",      "const",  "",       ""    ),    # (√)
        (   0x80,    "jl",      "mem",    "",       ""    ),    # (√)
        (   0x7E,    "jnge",    "reg",    "",       ""    ),    # (√)
        (   0x7F,    "jnge",    "const",  "",       ""    ),    # (√)
        (   0x80,    "jnge",    "mem",    "",       ""    ),    # (√)
        (   0x81,    "jle",     "reg",    "",       ""    ),    # (√)
        (   0x82,    "jle",     "const",  "",       ""    ),    # (√)
        (   0x83,    "jle",     "mem",    "",       ""    ),    # (√)
        (   0x81,    "jng",     "reg",    "",       ""    ),    # (√)
        (   0x82,    "jng",     "const",  "",       ""    ),    # (√)
        (   0x83,    "jng",     "mem",    "",       ""    ),    # (√)
        (   0x84,    "jne",     "reg",    "",       ""    ),    # (√)
        (   0x85,    "jne",     "const",  "",       ""    ),    # (√)
        (   0x86,    "jne",     "mem",    "",       ""    ),    # (√)
        (   0x84,    "jnz",     "reg",    "",       ""    ),    # (√)
        (   0x85,    "jnz",     "const",  "",       ""    ),    # (√)
        (   0x86,    "jnz",     "mem",    "",       ""    ),    # (√)
        (   0x87,    "jno",     "reg",    "",       ""    ),    # (√)
        (   0x88,    "jno",     "const",  "",       ""    ),    # (√)
        (   0x89,    "jno",     "mem",    "",       ""    ),    # (√)
        (   0x8A,    "jo",      "reg",    "",       ""    ),    # (√)
        (   0x8B,    "jo",      "const",  "",       ""    ),    # (√)
        (   0x8C,    "jo",      "mem",    "",       ""    ),    # (√)
        (   0x8D,    "jnp",     "reg",    "",       ""    ),    # (√)
        (   0x8E,    "jnp",     "const",  "",       ""    ),    # (√)
        (   0x8F,    "jnp",     "mem",    "",       ""    ),    # (√)
        (   0x8D,    "jpo",     "reg",    "",       ""    ),    # (√)
        (   0x8E,    "jpo",     "const",  "",       ""    ),    # (√)
        (   0x8F,    "jpo",     "mem",    "",       ""    ),    # (√)
        (   0x90,    "jp",      "reg",    "",       ""    ),    # (√)
        (   0x91,    "jp",      "const",  "",       ""    ),    # (√)
        (   0x92,    "jp",      "mem",    "",       ""    ),    # (√)
        (   0x90,    "jpe",     "reg",    "",       ""    ),    # (√)
        (   0x91,    "jpe",     "const",  "",       ""    ),    # (√)
        (   0x92,    "jpe",     "mem",    "",       ""    ),    # (√)
        (   0x93,    "jns",     "reg",    "",       ""    ),    # (√)
        (   0x94,    "jns",     "const",  "",       ""    ),    # (√)
        (   0x95,    "jns",     "mem",    "",       ""    ),    # (√)
        (   0x96,    "js",      "reg",    "",       ""    ),    # (√)
        (   0x97,    "js",      "const",  "",       ""    ),    # (√)
        (   0x98,    "js",      "mem",    "",       ""    ),    # (√)
        (   0x99,    "lea",     "reg",    "mem",    ""    ),    # (√)
        (   0x9A,    "neg",     "reg",    "",       ""    ),    # (√)
        (   0x9B,    "neg",     "mem",    "",       ""    ),    # (√)
        (   0x9C,    "jcxz",    "reg",    "",       ""    ),    # (√)
        (   0x9D,    "jcxz",    "const",  "",       ""    ),    # (√)
        (   0x9E,    "jcxz",    "mem",    "",       ""    ),    # (√)
        (   0x9F,    "jecxz",   "reg",    "",       ""    ),    # (√)
        (   0xA0,    "jecxz",   "const",  "",       ""    ),    # (√)
        (   0xA1,    "jecxz",   "mem",    "",       ""    ),    # (√)
        (   0xA2,    "not",     "reg",    "",       ""    ),    # (√)
        (   0xA3,    "not",     "mem",    "",       ""    ),    # (√)
        (   0xA4,    "call",    "reg",    "",       ""    ),    # (√)
        (   0xA5,    "call",    "const",  "",       ""    ),    # (√)
        (   0xA6,    "call",    "mem",    "",       ""    ),    # (√)
        (   0xA7,    "ret",     "",       "",       ""    ),    # (√)
        (   0xA8,    "ret",     "const",  "",       ""    ),    # (√)
        (   0xA9,    "enter",   "",       "",       ""    ),    # (√)
        (   0xAA,    "enter",   "const",  "const",  ""    ),    # (√)
        (   0xAB,    "leave",   "",       "",       ""    )     # (√)
    )

    decode = dict( sizes.values() )

    # exception classes
    class BaseException( Exception ):
        def __init__( self, *data ):
            self.instance, self.value = data

        def __str__( self ):
            return "at %s [%s]:\n" % ( os.path.basename( self.instance.f.name ), self.instance.curln+len( Parser.prologue)-1 ) + \
                   "\t%s\nerror: %s" % ( self.instance.lines[self.instance.curln].strip(), self.value )

    class SyntaxError    ( BaseException ):    pass
    class TypeError      ( BaseException ):    pass
    class NameError      ( BaseException ):    pass
    class ValueError     ( BaseException ):    pass

    # some static methods
    @staticmethod
    def split( ln, symbol=",", regexp="" ):
        if regexp:
            return [x.strip() for x in re.split( "(%s)\s*(?:(?:\%s)\s*(%s)\s*)?" % (regexp, symbol, regexp), ln ) if x and x != symbol]
        else:
            return [x.strip() for x in re.split( '%s(?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)' % symbol, ln ) if x.strip()]

    @staticmethod
    def opcode( cmd, op1="", op2="", op3="" ):
        try:    return [x[0] for x in _.opcodes if x[1:] == (cmd.lower(), op1, op2, op3)][0]
        except: pass

    @staticmethod
    def search_source( path ):
        if path: return path

class Parser:
    # regexp
    t_int           =    r"(?i)0x[\dabcdef]+|[\dabcdef]+h|[01]+b|\d+"
    t_string        =    r"\'.*?\'|\".*?\""
    t_name          =    r"[_a-zA-Z\$\.@\^][\w\.@:]*"
    t_operator      =    r"<<|>>|\+|-|\*\*|\*|/|%|\||&|\^"
    t_operand       =    r"|".join( ( t_name, t_string, t_int ) )
    t_term          =    r"(\(\s*)*(?:(\~)\s*)?(%s)(\s*\))*" % t_operand
    t_expr          =    r"(?:%s)?\s*(?:(%s)\s*(?:%s)\s*)?" % ( t_term, t_operator, t_term )
    t_term_match    =    r"(?:\(\s*)*(?:\~\s*)?\s*(?:%s)(?:\s*\))*" % t_operand
    t_expr_match    =    r"(?:%s)\s*(?:(?:%s)\s*(?:%s)\s*)*" % ( t_term_match, t_operator, t_term_match )
    t_size          =    r"|".join( _.sizes.keys() )
    t_reg           =    r"(?i)" + "|".join( _.registers.keys() )
    t_const         =    r"(?i)(%s)?\s*(%s)" % ( t_size, t_expr_match )
    t_const_match   =    r"(?i)(?:%s)?\s*(?:%s)" % ( t_size, t_expr_match )
    t_mem           =    r"(?i)(%s)?\s*(?:ptr\s*(%s)|\[\s*(%s)\s*\])" % ( t_size, "|".join( ( t_expr_match, t_reg ) ), "|".join( ( t_expr_match, t_reg ) ) )
    t_mem_match     =    r"(?i)(?:%s)?\s*(?:ptr\s*(?:%s)|\[\s*(?:%s)\s*\])" % ( t_size, "|".join( ( t_expr_match, t_reg ) ), "|".join( ( t_expr_match, t_reg ) ) )
    t_dest          =    r"|".join( ( t_mem_match, t_reg ) )
    t_source        =    r"|".join( ( t_mem_match, t_const_match, t_reg ) )
    t_comment       =    r";.*"
    t_datadir       =    r"(?i)" + "|".join( _.datadirs.keys() )
    t_dataop        =    t_expr_match
    t_data          =    r"(?:(%s)\s*)?(%s)?\s*(%s)?\s*(?:,\s*(%s)\s*)?(?:\s*(?:%s))?" % ( t_name, t_datadir, t_dataop, t_dataop, t_comment )
    t_data_match    =    r"(?:%s\s+)?(?:%s)\s+(?:%s)\s*(?:,\s*(?:%s)\s*)*\s*(?:%s)?" % ( t_name, t_datadir, t_dataop, t_dataop, t_comment )
    t_open          =    r"(?i)(?:(%s)\s*)?(macro|struc|proc)?\s*(%s)?\s*(?:,\s*(%s)\s*)?(?:\s*(?:%s))?" % ( t_name, t_name, t_name, t_comment )
    t_open_match    =    r"(?i)%s\s+(?:macro|struc|proc)(?:\s+(?:%s)\s*(?:,\s*(?:%s)\s*)*\s*(?:%s)?)?" % ( t_name, t_name, t_name, t_comment )
    t_close         =    r"(?:(%s)\s+)?(endm|ends|endp)\s*(?:%s)?" % ( t_name, t_comment ) 
    t_jmp_stub      =    r"near|short|far"
    t_section       =    r"(?i)section\s+\.(%s)\s*(?:%s)?" % ( t_name, t_comment )
    t_label         =    r"(%s)\s*:\s*(.*)\s*(?:%s)?" % ( t_name, t_comment )
    t_assign        =    r"(?i)(%s)\s*\=\s*(%s)\s*(?:%s)?" % ( t_name, t_expr_match, t_comment )
    t_equ           =    r"(?i)(%s)\s*equ\s*(.*)\s*(?:%s)?" % ( t_name, t_comment )
    t_cmd           =    r"|".join( sorted( list( set( map( lambda x: x[1], _.opcodes ) ) ), reverse=True ) )
    t_cmd0          =    r"(?i)(%s)\s*(?:%s)?" % ( t_cmd, t_comment )
    t_cmd1          =    r"(?i)(%s)\s+(?:%s\s+)?(%s)\s*(?:%s)?" % ( t_cmd, t_jmp_stub, t_source, t_comment )
    t_cmd2          =    r"(?i)(%s)\s+(%s)\s*\,\s*(%s)\s*(?:%s)?" % ( t_cmd, t_source, t_source, t_comment )
    t_include       =    r"(?i)include\s+(%s)\s*(?:%s)?" % ( t_string, t_comment )
    t_macro         =    r"(%s)\s*(.*)" % t_name
    t_reserved      =    r"|".join( ( t_reg, t_cmd, t_size, t_jmp_stub ) )
    
    # detect rules
    d_operand       =    ( ( t_int, "int" ), ( t_name, "name" ) )
    d_dest          =    ( ( t_mem, "mem" ), ( t_reg, "reg" ) )
    d_source        =    ( ( t_mem, "mem" ), ( t_const, "const" ), ( t_reg, "reg" ) )
    
    prologue = [
        "jmp _start"
    ]

    class Deferred:
        default_size = _.DWORD    # deferred expressions are 32 bits
        def __init__( self, parser, expr, last_label, curln ):
            self.parser = parser
            self.expr = expr
            self.last_label = last_label
            self.curln = curln
        def calculate( self ):
            self.parser.last_label = self.last_label
            self.parser.curln = self.curln
            return self.parser.calculate( self.expr )

    def check_reserved( self, ln ):
        match = re.match( Parser.t_reserved, ln )
        if match and match.group( 0 ) == ln.strip():
            raise _.NameError, (self, "reserved word used as symbol")

    def replace_sc( self, ln ):
        flag = True
        out = " %s " % ln
        symbol = re.compile(r"[^\w\.]")
        for key, value in self.sconstants.items():
            l, ln, out, x = len( key ), out, "", 0
            while x < len( ln ):
                if ln[x] in ('"', "'"):
                    flag = not flag
                if ln[x:x+l] == key and flag \
                and symbol.match( ln[x+l] ) and symbol.match( ln[x-1] ):
                    out += value
                    x += l
                else:
                    out += ln[x]
                    x += 1
        return out

    def get_int( self, op ):
        op = op.lower()
        if   op.startswith( "0x" ):    op = int( op[2:],  16 )
        elif op.endswith("h"):         op = int( op[:-1], 16 )
        elif op.endswith("b"):         op = int( op[:-1], 2 )
        else:
            try:    op = int( op )
            except: raise _.TypeError, (self, "not an int: '%s'" % op)
        return str( op )

    def get_string( self, op ):
        try:
            return op[1:-1].decode("string_escape")
        except:
            raise _.SyntaxError, (self, "invalid string format")

    def tokenize( self, expr, skip_regs=False ):
        find     =    re.findall( Parser.t_expr, expr )
        tokens   =    [x.strip() for x in reduce( lambda x, y: x+y, find ) if x]
        priority = {
            # operand    priority
            "**"    :    8,
            "~"     :    7,
            "*"     :    6,
            "/"     :    6,
            "%"     :    6,
            "+"     :    5,
            "-"     :    5,
            "<<"    :    4,
            ">>"    :    4,
            "&"     :    3,
            "^"     :    2,
            "|"     :    1,
        }
        out, pr_list, pr, index, delta = [], [], 0, 0, 10**6.0
        for t in tokens:
            delta -= 1
            if not t: continue
            if t[0] == ".": t = self.last_label + t
            if t in self.constants:
                 t = self.constants[t]
            if   t == "(": pr += 100
            elif t == ")": pr -= 100
            elif t in priority.keys():
                out.append( t )
                pr_list.append( ( pr+priority[t]+delta/10**6, index ) )
                index += 1
            else:
                int_match = re.match( Parser.t_int, t )
                str_match = re.match( Parser.t_string, t )
                if   int_match:
                    t = self.get_int( t )
                elif str_match:
                     t = str( ord( self.get_string( str_match.group( 0 ) )[0] ) )
                elif not skip_regs:
                    if self.allow_def:
                        # try to use lazy evaluation
                        return Parser.Deferred( self, expr, self.last_label, self.curln ), []
                    else:
                        raise _.SyntaxError, (self, "invalid operand: '%s'" % t)
                out.append( t )
                index += 1
        pr_list.sort( reverse=True )
        return out, [x[1] for x in pr_list]

    def calculate( self, expr,skip_regs=False ):
        action = {
            "**"    :    lambda x, y: x**y,
            "*"     :    lambda x, y: x*y,
            "/"     :    lambda x, y: x/y,
            "%"     :    lambda x, y: x%y,
            "+"     :    lambda x, y: x+y,
            "-"     :    lambda x, y: x-y,
            "<<"    :    lambda x, y: x<<y,
            ">>"    :    lambda x, y: x>>y,
            "&"     :    lambda x, y: x&y,
            "^"     :    lambda x, y: x^y,
            "|"     :    lambda x, y: x|y,
        }
        tokens, priority = self.tokenize( expr, skip_regs )
        if isinstance( tokens, Parser.Deferred ): return tokens
        pr_count = len( priority )
        for i in xrange( len( priority ) ):
            pr = priority[i]
            op = tokens[pr]
            if op == "~":
                if not tokens[pr+1].isdigit(): continue
                tokens[pr] = ~int( tokens[pr+1] )
                del tokens[pr+1]
                delta = 1
            else:
                x, y = tokens[pr-1], tokens[pr+1]
                if not (str( x ).isdigit() and str( y ).isdigit()): continue
                tokens[pr] = action[op]( int( x ), int( y ) )
                pr_count -= 1
                del tokens[pr-1], tokens[pr]
                delta = 2
            # change indexes in priority
            priority = [x-delta if x>pr else x for x in priority]
        if skip_regs:
            return map( str, tokens ), priority[:pr_count]
        else:
            return int( tokens[0] )

    def detect( self, rules, src ):
        src = src.strip()
        detected = None
        for reg, ret in rules:
            match = re.match( reg, src )
            if match and match.group(0) == src:
                detected = ret, match
        return detected

    def pack( self, num, size=0 ):
        num = int( num )
        if not size:
            for sz in sorted( map( lambda x: x[1], _.sizes.values() ) ):
                if num < 2**(8*sz): return self.pack( num, sz )
        if num >= 2**(8*size):
            raise _.ValueError, (self, "int value too big: '%s'" % num)
        buf = []
        for bit in xrange( size ):
            buf.append( num >> (bit * 8) & 0xFF )
        return buf

    def push( self, bytes ):
        if self.cur != "text":
            raise _.SyntaxError, (self, "assembling to incorrect section: '%s'" % self.cur)
        self.sections[self.cur][0] += bytes
        self.sections[self.cur][1] += len(bytes)
        length = self.sections[self.cur][1]
        self.constants["$"] = str( length )
        return length

    def compile( self, token ):
        if not isinstance( token, tuple ) or not (len( token ) == 2): return
        type, match = token
        if   type == "reg":
            id, size = _.registers[match.group( 0 ).lower()]
            return "reg", size, [id]
        elif type == "const":
            size, const = match.groups()
            const = self.calculate( const )
            if isinstance(const, Parser.Deferred): # deferred expression
                size = _.decode[Parser.Deferred.default_size]
                return "const", size, [const] + [0]*(size-1)
            else:
                size = _.sizes[size.lower()][1] if size else 0
                packed = self.pack( const, size )
                return "const", _.decode[len( packed )], packed
        elif type == "mem":
            size, addr1, addr2 = match.groups()
            addr = addr1 or addr2
            regs = [key for key in _.registers if key[0] == "e"]
            tokens, pr = self.calculate( addr, True )
            flags = 0
            p = [0,0,0,0]
            def mark( check, *flist ):
                F = flags
                for f in flist:
                    if F & (1<<f) and check: # if already marked
                       raise _.SyntaxError, (self, "invalid address: '%s'" % addr)
                    F |= 1<<f
                return F
            for i in pr:
                # make correct order
                x, op, y = tokens[i-1], tokens[i], tokens[i+1]
                if op == "*" and y.lower() in regs:
                    tokens[i-1], tokens[i+1] = tokens[i+1], tokens[i-1]
            if len(tokens) == 1: # single operand
                op = tokens[0]
                if   op.lower() in regs:
                    p[2] = _.registers[op.lower()][0]
                    flags = mark(False, 2)
                elif op.isdigit():
                    p[3] = self.pack( int( op ) )
                    flags = mark(False, 3)
                else:
                    raise _.SyntaxError, (self, "invalid operand in address: '%s'" % op)
            for i in pr:
                x, op, y = tokens[i-1], tokens[i], tokens[i+1]
                if op in ("+", "-", "*"):
                    if   (x.lower() in regs) and (op == "*") and y in ("1", "2", "4", "8"):
                        p[0] = p[0] if flags&(1<<0) else int( math.log( int( y ), 2) )
                        p[1] = p[1] if flags&(1<<1) else _.registers[x.lower()][0]
                        tokens[i+1] = x
                        flags = mark(True, 0, 1)
                    elif (x.lower() in regs) and (op == "*") and y in ("3", "5", "9"):
                        p[0] = p[0] if flags&(1<<0) else int( math.log( int( y )-1, 2) )
                        p[1] = p[1] if flags&(1<<1) else _.registers[x.lower()][0]
                        p[2] = p[2] if flags&(1<<2) else _.registers[x.lower()][0]
                        tokens[i+1] = x
                        flags = mark(True, 0, 1, 2)
                    elif (x.lower() in regs) and (op  == "+") and (y.lower() in regs):
                        p[1] = p[1] if flags&(1<<1) else _.registers[x.lower()][0]
                        p[2] = p[2] if flags&(1<<2) else _.registers[y.lower()][0]
                        flags = mark(False, 0, 1)
                        flags = mark(True, 2)
                    elif (x.isdigit() and (op in ("+", "-")) and (y.lower() in regs)) \
                      or ((x.lower() in regs) and (op in ("+", "-")) and y.isdigit()):
                        if x.isdigit(): x, y = y, x
                        y, int_size = int( y ), 0
                        if op == "-": y = -y
                        if y < 0:    int_size = 4
                        p[1] = p[1] if flags&(1<<1) else _.registers[x.lower()][0]
                        p[3] = p[3] if flags&(1<<3) else  self.pack( y, int_size )
                        flags = mark(False, 1, 3)
                    else:
                        raise _.SyntaxError, (self, "invalid address: '%s'" % addr)
                else:
                    raise _.SyntaxError, (self, "invalid operator in address: '%s'" % op)
            # compile mem pointer
            size = _.sizes[size.lower()][1] if size else 0
            compiled = []
            if flags&7: compiled.append((p[0]<<6)+(p[1]<<3)+p[2])
            if flags&8: compiled.extend([len( p[3] )] + p[3])
            return "mem", size, [(flags<<4) + size] + compiled

    def check_ln( self, regexp, ln ):
        match = re.match( regexp, ln )
        if match and match.group( 0 ) == ln: return match

    def parse( self, ln ):
        ln = ln.strip()
        if self.hooks and not self.check_ln( Parser.t_close, ln ):
            block, name = self.hooks[len(self.hooks)-1]
            if   block == "macro":
                self.macros[name][1].append( ln )
            elif block == "struc":
                raw, parsed = re.split(r"\s+", ln, 2), []
                if len( raw ) > 1 and raw[1] in self.structs:
                    self.structs[name][0][raw[0]] = self.structs[name][2]
                    struct = self.structs[raw[1]]
                    for item, offset in struct[0].items():
                        self.structs[name][0]["%s.%s" % (raw[0], item)] = self.structs[name][2] + offset
                    parsed = struct[1]
                elif not self.check_ln( Parser.t_data_match, ln ):
                    raise _.SyntaxError, (self, "invalid data inside structure definition")
                else:
                    find        = re.findall( Parser.t_data, ln )
                    operands    = [x.strip() for x in reduce( lambda x, y: x+y, find ) if x]
                    if re.match( Parser.t_datadir, operands[0] ):
                        dir = operands[0]
                        ops = operands[1:]
                    else:
                        dir = operands[1]
                        ops = operands[2:]
                        self.structs[name][0][operands[0]] = self.structs[name][2]
                    size    = _.datadirs[dir]
                    for x in ops:
                        if re.match( Parser.t_string, x ):
                            chunk = map( ord, self.get_string( x ) )
                            map( lambda x: parsed.extend( self.pack(x, size) ), chunk )
                        else:
                            chunk = self.calculate( x )
                            if isinstance(chunk, Parser.Deferred): # deferred expression
                                sz = _.decode[Parser.Deferred.default_size]
                                parsed.extend( [chunk] + [0]*(sz-1) )
                            else:
                                parsed.extend( self.pack( chunk, size ) )
                self.structs[name][1].extend( parsed )
                self.structs[name][2] += len( parsed )
            elif block == "proc":
                hooks = self.hooks
                self.hooks = []
                self.parse( ln )
                self.hooks = hooks
        ln = self.replace_sc( ln ).strip()
        comment = self.check_ln( Parser.t_comment, ln )     # comment?
        include = self.check_ln( Parser.t_include, ln )     # include?
        equ     = self.check_ln( Parser.t_equ, ln )         # symbol constant assign?
        _open   = self.check_ln( Parser.t_open_match, ln )  # block open bracket?
        _close  = self.check_ln( Parser.t_close, ln )       # block close bracket?
        if comment:
            return
        if include:
            path = _.search_source( self.get_string( include.groups()[0] ) )
            f_saved, lines_saved, ln_saved = self.f, self.lines, self.curln
            self.f = open( path )
            self.lines = [""] * len( Parser.prologue) + self.f.readlines()
            for self.curln, ln in enumerate( self.lines ):
                self.parse( ln )
            self.f, self.lines, self.curln = f_saved, lines_saved, ln_saved
            return
        if equ:
            key, value = equ.groups()
            self.sconstants[key] = value
            return
        elif _open:
            find     = re.findall( Parser.t_open, ln )
            operands = [x.strip() for x in reduce( lambda x, y: x+y, find ) if x]
            name, block, args = operands[0], operands[1].lower(), operands[2:]
            if name[0] == ".": name = self.last_label + name
            if block == "macro":
                self.macros[name] = args, [], self.curln+1
            elif block == "proc":
                if self.constants.has_key(name):
                    raise _.NameError, (self, "symbol '%s' already defined" % name)
                if "@" in name: # exportable label
                    local, export = name.split("@")
                    if not export:
                        raise _.NameError, (self, "invalid exportable label format")
                    self.sections["export"][self.constants["$"]] = export
                    name = local or export
                for arg in args:
                    arg = arg.split(":")
                    if len( arg ) > 1:
                        key, value = arg
                    else:
                        key, value = arg[0], ""
                    key, value = key.strip().lower(), value.strip()
                    if key == "export":
                        self.sections["export"][self.constants["$"]] = value or name
                if name[0] == ".":
                    name = self.last_label + name
                else:
                    self.last_label = name
                self.constants[name] = self.constants["$"]
            elif block == "struc":
                self.structs[name] = [ {}, [], 0 ]
            self.hooks.append( (block, name) )
        elif _close:
            name, block = _close.groups()
            if (not self.hooks) or (name and name != self.hooks[len(self.hooks)-1][1]) \
            or (block.lower() != "end" + self.hooks[len(self.hooks)-1][0][0]):
                raise _.SyntaxError, (self, "invalid close bracket")
            self.hooks.pop()
            return
        if self.hooks: return
        label = re.match( Parser.t_label, ln )
        # if line starts with label
        if label:
            l_name, ln = label.groups()
            if self.constants.has_key(l_name):
                raise _.NameError, (self, "symbol '%s' already defined" % l_name)
            if "@" in l_name: # exportable label
                local, export = l_name.split("@")
                if not export:
                    raise _.NameError, (self, "invalid exportable label format")
                self.sections["export"][self.constants["$"]] = export
                l_name = local or export
            if l_name[0] == ".":
                l_name = self.last_label + l_name
            else:
                self.last_label = l_name
            self.constants[l_name] = self.constants["$"]
        comment = self.check_ln( Parser.t_comment, ln )     # comment?
        if not ln or comment: return
        section = self.check_ln( Parser.t_section, ln )     # section?
        assign  = self.check_ln( Parser.t_assign, ln )      # constant assign?
        data    = self.check_ln( Parser.t_data_match, ln )  # data?
        cmd0    = self.check_ln( Parser.t_cmd0, ln )        # 0-operand command?
        cmd1    = self.check_ln( Parser.t_cmd1, ln )        # 1-operand command?
        cmd2    = self.check_ln( Parser.t_cmd2, ln )        # 2-operand command?
        if section:
            sect = (section.group(1) or section.group(2)).lower()
            if sect not in ("header", "text"):
                raise _.SyntaxError, (self, "unknown section: '%s'" % sect)
            self.cur = sect
        elif assign:
            key, value = assign.groups()
            self.check_reserved( key )
            calc = self.calculate( value )
            if type( calc ) not in (int, long): # deffered
                raise _.ValueError, (self, "unknown constant value: '%s'" % value)
            if self.cur == "header":
                key = key.lower()
                if key == "stack_size":
                    for x in xrange(16):
                        if calc == (1<<x)*32: size = x
                    try: size
                    except:
                        raise _.SyntaxError, (self, "incorrect stack size: '%s'" % calc)
                    self.sections["header"][0] &= ~(0xF<<4)
                    self.sections["header"][0] |=  size<<4
                elif key == "pid":
                    if calc > 127:
                        raise _.SyntaxError, (self, "pid must be in range(128)")
                    self.sections["header"][1] = 0x80|calc
            elif self.cur == "text":
                self.constants[key] = str( calc )
        elif data:
            find        = re.findall( Parser.t_data, ln )
            operands    = [x.strip() for x in reduce( lambda x, y: x+y, find ) if x]
            if re.match( Parser.t_datadir, operands[0] ):
                name = ""
                dir  = operands[0]
                ops  = operands[1:]
            else:
                name = operands[0]
                addr = self.constants["$"]
                dir  = operands[1]
                ops  = operands[2:]
            size     = _.datadirs[dir]
            parsed   = []
            for x in ops:
                if re.match( Parser.t_string, x ):
                    parsed += map( ord, self.get_string( x ) )
                else:
                    parsed.append( self.calculate( x ) )
            if name.startswith("^"):
                name = name[1:]
                addr = str( int( addr )+4 )
                self.sconstants["^"+name] = "dword[%s-4]" % name
                self.sections["shared"].append( str( int( addr )-1 ) )
                self.push( self.pack( addr, 3 )+[0] )
            for x in parsed:
                if isinstance(x, Parser.Deferred): # deferred expression
                    size = _.decode[Parser.Deferred.default_size]
                    self.push( [x] + [0]*(size-1) )
                else:
                    self.push( self.pack( x, size ) )
            if name: self.constants[name] = addr
        elif cmd2:
            cmd, x, y = cmd2.groups()
            cmd, x, y = cmd.lower(), self.detect( Parser.d_source, x ), self.detect( Parser.d_source, y )
            (s_type, s_size, s_code), (d_type, d_size, d_code) = self.compile( y ), self.compile( x )
            d_size = d_size or s_size
            s_size = s_size or d_size
            # use 'byte' if operands type not specified
            if not d_size + s_size: d_size = s_size = _.BYTE
            # write opcode
            self.push( [_.opcode( cmd, d_type, s_type )] )
            # 1 - reg, mem ; 2 - reg, const, mem
            if cmd in ("mov", "add", "sub", "mod", "mul", "div", "xor", "or",
                         "and", "shl", "sal", "shr", "cmp", "test"):
                if (d_type, s_type) in ( ("reg", "const"), ("mem", "const") ):
                    if not (d_code[0]&0xF or d_type == "reg"): d_code[0] |= s_size
                    if d_size > s_size:
                        s_code += [0]*(d_size-s_size)
                        s_size = d_size
                    self.push( d_code + [s_size] + s_code )
                elif (d_type, s_type) in ( ("reg", "reg"), ("reg", "mem"), ("mem", "reg"), ("mem", "mem") ):
                    if not (d_code[0]&0xF or d_type == "reg"): d_code[0] |= s_size
                    if not (s_code[0]&0xF or s_type == "reg"): s_code[0] |= d_size
                    self.push( d_code + s_code )
            # 1 - reg, mem ; 2 - reg, mem
            elif cmd == "xchg":
                if (d_type, s_type) in ( ("reg", "reg"), ("reg", "mem"), ("mem", "reg"), ("mem", "mem") ):
                    if not (d_code[0]&0xF or d_type == "reg"): d_code[0] |= s_size
                    if not (s_code[0]&0xF or s_type == "reg"): s_code[0] |= d_size
                    if cmd == "xchg" and (d_type, s_type) == ("mem", "reg"):
                        self.push( s_code + d_code )
                    else:
                        self.push( d_code + s_code )
                else:
                    raise _.SyntaxError, (self, "invalid operand: '%s'" % source[1].group( 0 ))
            # 1 - reg ; 2 - mem
            elif cmd == "lea":
                if (d_type, s_type) == ("reg", "mem"):
                    if not s_code[0]&0xF: s_code[0] |= d_size
                    self.push( d_code + s_code )
                else:
                    raise _.SyntaxError, (self, "invalid operand: '%s'" % source[1].group( 0 ))
            # 1 - const [; 2 - const]
            elif cmd == "enter":
                self.push( [d_size] + d_code )
            if (d_size != s_size):
                raise _.SyntaxError, (self, "operand sizes do not match")
        elif cmd1:
            cmd, op = cmd1.groups()
            op_type, op_size, op_code = self.compile( self.detect( Parser.d_source, op ) )
            self.push( [_.opcode( cmd, op_type )] ) # write opcode
            # 1 - reg, mem, const
            if cmd in ("push", "jmp", "loop", "int", "ja", "jbe", "jae", "jnb", "jnc", "jb",
                         "jnae", "jc", "jbe", "jna", "je", "jz", "jg", "jnle", "jge", "jnl",
                         "jl", "jnge", "jle", "jng", "jne", "jnz", "jno", "jo", "jnp", "jpo",
                         "jp", "jpe", "jns", "js", "jcxz", "jecxz", "call"):
                if op_type in ("reg", "mem"):
                    if not (op_code[0]&0xF or op_type == "reg"): op_code[0] |= _.BYTE
                    self.push( op_code )
                elif op_type == "const":
                    self.push( [op_size] + op_code )
            # 1 - reg, mem
            elif cmd in ("pop", "inc", "dec", "neg", "not"):
                if op_type in ("reg", "mem"):
                    if not (op_code[0]&0xF or op_type == "reg"): op_code[0] |= _.BYTE
                    self.push( op_code )
                elif op_type == "const":
                    raise _.SyntaxError, (self, "invalid operand: '%s'" % op)
            # 1 - const
            elif cmd == "ret":
                self.push( [op_size] + op_code )
        elif cmd0:
            (cmd,) = cmd0.groups()
            self.push( [_.opcode( cmd )] )
        elif self.check_ln( Parser.t_macro, ln ):
            find        = re.findall( Parser.t_macro, ln )
            operands    = [x.strip() for x in reduce( lambda x, y: x+y, find ) if x]
            name = operands[0]
            if name[0] == ".": name = self.last_label + name
            try:    args = _.split( operands[1] )
            except: args = []
            convert = re.compile( "(%s)(?:@(%s)$)?" % ("|".join(self.structs.keys()), Parser.t_reg) )
            if len( args ) == 1 and convert.match( args[0] ):
                sname, reg = convert.match( args[0] ).groups()
                addr   = self.constants["$"]
                struct = self.structs[sname]
                if reg:
                    for key, value in struct[0].items():
                        self.sconstants["%s.%s" % (name, key)] = "[%s+%s]" % (reg, value)
                else:
                    if name.startswith("^"):
                        name = name[1:]
                        addr = str( int( addr )+4 )
                        self.sconstants["^"+name] = "dword[%s-4]" % name
                        self.sections["shared"].append( str( int( addr )-1 ) )
                        self.push( self.pack( addr, 3 )+[0] )
                    for key, value in struct[0].items():
                        self.sconstants["%s.%s" % (name, key)] = "%s+%s" % (name, value)
                    self.constants[name] = addr
                    self.push( struct[1] )
            elif name in self.macros:
                macro = self.macros[name]
                if len( args ) < len( macro[0] ):
                    raise _.SyntaxError, (self, "too few args to '%s': %s given, %s expected" % (name, len(args), len(macro[0])))
                elif len( args ) > len( macro[0] ):
                    raise _.SyntaxError, (self, "too many args to '%s': %s given, %s expected" % (name, len(args), len(macro[0])))
                for arg, value in zip( macro[0], args ):
                    self.sconstants[arg] = value
                for x, ln in enumerate( macro[1] ):
                    self.curln = macro[2]+x
                    self.parse( ln )
                for arg in macro[0]:
                    del self.sconstants[arg]
            else:
                raise _.SyntaxError, (self, "unknown command or invalid syntax")
        else:
            raise _.SyntaxError, (self, "unknown command or invalid syntax")

    def __init__( self, path ):
        self.sections = {
            "header"    :    [176, 0, 0, 0, 0, 0, 0, 0],
            "export"    :    {},
            "shared"    :    [],
            "text"      :    [[], 0]
        }
        self.path         =    _.search_source( path )
        self.cur          =    "text"
        self.constants    =    { "$": "0" }
        self.sconstants   =    {}
        self.macros       =    {}
        self.structs      =    {}
        self.labels       =    {}
        self.last_label   =    ""
        self.allow_def    =    True
        self.hooks        =    []
        try:
            self.f = open( self.path + ".asm" )
            self.lines = Parser.prologue + self.f.readlines()
        except:
            raise IOError, "incorrect source filename: '%s'" % path
        # first pass
        for self.curln, ln in enumerate( self.lines ):
            self.parse( ln )
        # second pass
        self.allow_def = False
        # compile deferred expressions
        for i, byte in enumerate( self.sections["text"][0] ):
            if isinstance( byte, Parser.Deferred ):
                for j, x in enumerate( self.pack( byte.calculate() ) ):
                    self.sections["text"][0][i+j] = x
        # parse 'shared' section
        shared = self.sections["shared"]
        self.sections["header"][2:4] = self.pack( len( shared ), 2 )
        if len( shared ) > 0xFFFF:
            raise SyntaxError, "too many shared data"
        s_shared = []
        for x in shared:
            s_shared.extend( self.pack( x, 4 ) )
        # parse 'export' section
        export = self.sections["export"]
        if len ( export ):
            inc = open( self.path+".obj", "w" )
            maxlen = max( map( len, export.values() ) )
            items = sorted( [(int( k ),v) for k,v in export.items()] )
            for addr, name in items:
                # add address to .obj file
                address = (int( self.sections["header"][1] )<<24) + (addr&0xFFFFFF)
                inc.write( "%s%s= 0x%X\n" % (name, " "*(maxlen-len( name )+1), address) )
            inc.close()
        # write 'header' and 'text' sections
        try:
            s_header    =    map( chr, self.sections["header"]  )
            s_shared    =    map( chr, s_shared )
            s_text      =    map( chr, self.sections["text"][0] )
        except:
            raise SyntaxError, "unknown error"
        file(self.path+".bin", "wb").write( "".join( s_header + s_shared + s_text ) )        

try:
    source = sys.argv[1]
except IndexError:
    source = None
lexer = Parser( source )
