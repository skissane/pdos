/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

/* For internal use in as_i386.c. */

#define REGISTER_PREFIX  '%'
#define IMMEDIATE_PREFIX '$'

#define BYTE_SUFFIX  'b'
#define WORD_SUFFIX  'w'
#define DWORD_SUFFIX 'l'

#define MAX_OPERANDS 2

#define TWOBYTE_OPCODE 0x0F

#define ESP_REGISTER_NUMBER 4
#define EBP_REGISTER_NUMBER 5

#define MODRM_REGMEM_TWO_BYTE_ADDRESSING ESP_REGISTER_NUMBER
#define SIB_INDEX_NO_INDEX_REGISTER ESP_REGISTER_NUMBER

typedef struct {
    const char *name;

    unsigned int operands;

    unsigned int base_opcode;

    unsigned int extension_opcode;
#define None 0xFFFF

    unsigned int opcode_modifier;
#define W 0x1 /* Operands can be words/dwords
              after adding 1 to base_opcode. */
#define D 0x2 /* OR 0x2 into base_opcode to reverse direction.
               * Must be 0x2. */
#define Modrm 0x4
#define ShortForm 0x8 /* Register is encoded in low 3 bits of opcode. */
#define Jump 0x10 /* Variable length jump instruction. */
#define Call 0x20 /* call. */
#define No_bSuf 0x40 /* b suffix is not allowed. */
#define No_wSuf 0x80 /* w suffix is not allowed. */
#define No_lSuf 0x100 /* l suffix is not allowed. */

    unsigned int operand_types[MAX_OPERANDS];
/* register */
#define Reg8 0x1
#define Reg16 0x2
#define Reg32 0x4
#define Reg (Reg8 | Reg16 | Reg32)
#define WordReg (Reg16 | Reg32)
/* immediate */
#define Imm8  0x10 /* 8 bit immediate. */
#define Imm8S 0x20 /* 8 bit immediate sign extended. */
#define Imm16 0x40 /* 16 bit immediate. */
#define Imm32 0x80 /* 32 bit immediate. */
#define Imm (Imm8 | Imm8S | Imm16 | Imm32)
/* displacement */
#define Disp8  0x100 /* 8 bit displacement. */
#define Disp16 0x200 /* 16 bit displacement. */
#define Disp32 0x400 /* 32 bit displacement */
#define Disp (Disp8 | Disp16 | Disp32)

#define BaseIndex 0x1000
#define AnyMem (Disp8 | Disp16 | Disp32 | BaseIndex)

#define Acc 0x2000 /* Accumulator (%al, %ax, %eax). */
#define ShiftCount 0x4000 /* Register to hold the shift count (%cl). */

#define ImplicitRegister (ShiftCount | Acc)
} template;

typedef struct {
    const char *name;
    unsigned int type;
    unsigned int number;
} reg_entry;

typedef struct {
    unsigned int regmem;
    unsigned int reg;
    unsigned int mode;
} modrm_byte;

/* Scaled Index Byte. */
typedef struct {
    unsigned int base;
    unsigned int index;
    unsigned int scale;
} sib_byte;

const template template_table[] = {
    /* Move instructions. */
    {"mov", 2, 0xA0, None, D | W, {Disp16 | Disp32, Acc}},
    {"mov", 2, 0x88, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"mov", 2, 0xB0, None, W | ShortForm, {Imm, Reg8 | Reg16 | Reg32}},
    {"mov", 2, 0xC6, None, W | Modrm, {Imm, Reg | AnyMem}},

    /* Push instructions. */
    {"push", 1, 0x50, None, ShortForm, {WordReg, 0}},
    {"push", 1, 0xFF, 6, Modrm, {WordReg | AnyMem}},
    {"push", 1, 0x6A, None, 0, {Imm8S, 0}},
    {"push", 1, 0x68, None, 0, {Imm16 | Imm32, 0}},

    {"pusha", 0, 0x60, None, 0, {0, 0}},

    /* Pop instructions. */
    {"pop", 1, 0x58, None, ShortForm, {WordReg, 0}},
    {"pop", 1, 0xF8, 0, Modrm, {WordReg | AnyMem}},

    {"popa", 0, 0x61, None, 0, {0, 0}},

    /* Exchange instructions. */
    {"xchg", 2, 0x90, None, ShortForm, {WordReg, Acc}},
    {"xchg", 2, 0x90, None, ShortForm, {Acc, WordReg}},
    {"xchg", 2, 0x86, None, W | Modrm, {Reg, Reg | AnyMem}},
    {"xchg", 2, 0x86, None, W | Modrm, {Reg | AnyMem, Reg}},

    /* Flags register instructions. */
    {"cmc", 0, 0xF5, None, 0, {0, 0}},
    {"clc", 0, 0xF8, None, 0, {0, 0}},
    {"stc", 0, 0xF9, None, 0, {0, 0}},
    {"cli", 0, 0xFA, None, 0, {0, 0}},
    {"cld", 0, 0xFC, None, 0, {0, 0}},
    {"std", 0, 0xFD, None, 0, {0, 0}},
    {"sti", 0, 0xFB, None, 0, {0, 0}},
    {"lahf", 0, 0x9F, None, 0, {0, 0}},
    {"sahf", 0, 0x9E, None, 0, {0, 0}},
    {"pushf", 0, 0x9C, None, 0, {0, 0}},
    {"pushf", 0, 0x9D, None, 0, {0, 0}},

    /* Load effective address. */
    {"lea", 2, 0x8D, None, Modrm, {AnyMem, WordReg}},

    /* Arithmetic instructions. */
    {"add", 2, 0x00, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"add", 2, 0x83, 0, No_bSuf | Modrm, {Imm8S, WordReg | AnyMem}},
    {"add", 2, 0x04, None, W, {Imm, Acc}},
    {"add", 2, 0x80, 0, W | Modrm, {Imm, Reg | AnyMem}},

    {"inc", 1, 0x40, None, No_bSuf | ShortForm, {WordReg, 0}},
    {"inc", 1, 0xFE, 0, W | Modrm, {Reg | AnyMem, 0}},

    {"sub", 2, 0x28, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"sub", 2, 0x83, 5, No_bSuf | Modrm, {Imm8S, WordReg | AnyMem}},
    {"sub", 2, 0x2C, None, W, {Imm, Acc}},
    {"sub", 2, 0x80, 5, W | Modrm, {Imm, Reg | AnyMem}},

    {"dec", 1, 0x48, None, No_bSuf | ShortForm, {WordReg, 0}},
    {"dec", 1, 0xFE, 1, W | Modrm, {Reg | AnyMem, 0}},

    {"sbb", 2, 0x18, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"sbb", 2, 0x83, 3, No_bSuf | Modrm, {Imm8S, WordReg | AnyMem}},
    {"sbb", 2, 0x1c, None, W, {Imm, Acc}},
    {"sbb", 2, 0x80, 3, W | Modrm, {Imm, Reg | AnyMem}},

    {"cmp", 2, 0x38, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"cmp", 2, 0x83, 7, No_bSuf | Modrm, {Imm8S, WordReg | AnyMem}},
    {"cmp", 2, 0x3c, None, W, {Imm, Acc}},
    {"cmp", 2, 0x80, 7, W | Modrm, {Imm, Reg | AnyMem}},

    {"test", 2, 0x84, None, W | Modrm, {Reg | AnyMem, Reg}},
    {"test", 2, 0x84, None, W | Modrm, {Reg, Reg | AnyMem}},
    {"test", 2, 0xA8, None, W, {Imm, Acc}},
    {"test", 2, 0xF6, None, W | Modrm, {Imm, Reg | AnyMem}},

    {"and", 2, 0x20, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"and", 2, 0x83, 4, No_bSuf | Modrm, {Imm8S, WordReg | AnyMem}},
    {"and", 2, 0x24, None, W, {Imm, Acc}},
    {"and", 2, 0x80, 4, W | Modrm, {Imm, Reg | AnyMem}},

    {"or", 2, 0x08, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"or", 2, 0x83, 1, No_bSuf | Modrm, {Imm8S, WordReg | AnyMem}},
    {"or", 2, 0x0c, None, W, {Imm, Acc}},
    {"or", 2, 0x80, 1, W | Modrm, {Imm, Reg | AnyMem}},
    
    {"xor", 2, 0x30, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"xor", 2, 0x83, 6, No_bSuf | Modrm, {Imm8S, WordReg | AnyMem}},
    {"xor", 2, 0x34, None, W, {Imm, Acc}},
    {"xor", 2, 0x80, 6, W | Modrm, {Imm, Reg | AnyMem}},

    {"adc", 2, 0x10, None, D | W | Modrm, {Reg, Reg | AnyMem}},
    {"adc", 2, 0x83, 2, No_bSuf | Modrm, {Imm8S, WordReg | AnyMem}},
    {"adc", 2, 0x14, None, W, {Imm, Acc}},
    {"adc", 2, 0x80, 2, W | Modrm, {Imm, Reg | AnyMem}},

    {"neg", 1, 0xF6, 3, W | Modrm, {Reg | AnyMem, 0}},
    {"not", 1, 0xF6, 2, W | Modrm, {Reg | AnyMem, 0}},

    {"aaa", 0, 0x37, None, 0, {0, 0}},
    {"aas", 0, 0x3F, None, 0, {0, 0}},
    {"daa", 0, 0x27, None, 0, {0, 0}},
    {"das", 0, 0x2F, None, 0, {0, 0}},
    {"aad", 1, 0xD5, None, 0, {Imm8S, 0}},
    {"aam", 1, 0xD4, None, 0, {Imm8S, 0}},

    /* Conversion instructions. */
    {"cwde", 0, 0x98, None, 0, {0, 0}},
    {"cdq", 0, 0x99, None, 0, {0, 0}},

    {"cwtl", 0, 0x98, None, 0, {0, 0}},
    {"cltd", 0, 0x99, None, 0, {0, 0}},

    {"mul", 1, 0xF6, 4, W | Modrm, {Reg | AnyMem, 0}},
    {"imul", 1, 0xF6, 5, W | Modrm, {Reg | AnyMem, 0}},
    {"imul", 2, 0x0FAF, None, No_bSuf | Modrm, {WordReg | AnyMem, WordReg}},

    {"div", 1, 0xF6, 6, W | Modrm, {Reg | AnyMem, 0}},
    {"div", 2, 0xF6, 6, W | Modrm, {Reg | AnyMem, Acc}},
    {"idiv", 1, 0xF6, 7, W | Modrm, {Reg | AnyMem, 0}},
    {"idiv", 2, 0xF6, 7, W | Modrm, {Reg | AnyMem, Acc}},

    {"rol", 2, 0xC0, 0, W | Modrm, {Imm8, Reg | AnyMem}},
    {"rol", 2, 0xD2, 0, W | Modrm, {ShiftCount, Reg | AnyMem}},
    {"rol", 1, 0xD0, 0, W | Modrm, {Reg | AnyMem, 0}},

    {"ror", 2, 0xC0, 1, W | Modrm, {Imm8, Reg | AnyMem}},
    {"ror", 2, 0xD2, 1, W | Modrm, {ShiftCount, Reg | AnyMem}},
    {"ror", 1, 0xD0, 1, W | Modrm, {Reg | AnyMem, 0}},

    {"rcl", 2, 0xC0, 2, W | Modrm, {Imm8, Reg | AnyMem}},
    {"rcl", 2, 0xD2, 2, W | Modrm, {ShiftCount, Reg | AnyMem}},
    {"rcl", 1, 0xD0, 2, W | Modrm, {Reg | AnyMem, 0}},

    {"rcr", 2, 0xC0, 3, W | Modrm, {Imm8, Reg | AnyMem}},
    {"rcr", 2, 0xD2, 3, W | Modrm, {ShiftCount, Reg | AnyMem}},
    {"rcr", 1, 0xD0, 3, W | Modrm, {Reg | AnyMem, 0}},

    /* SAL and SHL are the same instruction. */
    {"sal", 2, 0xC0, 4, W | Modrm, {Imm8, Reg | AnyMem}},
    {"sal", 2, 0xD2, 4, W | Modrm, {ShiftCount, Reg | AnyMem}},
    {"sal", 1, 0xD0, 4, W | Modrm, {Reg | AnyMem, 0}},

    {"shl", 2, 0xC0, 4, W | Modrm, {Imm8, Reg | AnyMem}},
    {"shl", 2, 0xD2, 4, W | Modrm, {ShiftCount, Reg | AnyMem}},
    {"shl", 1, 0xD0, 4, W | Modrm, {Reg | AnyMem, 0}},
    
    {"shr", 2, 0xC0, 5, W | Modrm, {Imm8, Reg | AnyMem}},
    {"shr", 2, 0xD2, 5, W | Modrm, {ShiftCount, Reg | AnyMem}},
    {"shr", 1, 0xD0, 5, W | Modrm, {Reg | AnyMem, 0}},
    
    {"sar", 2, 0xC0, 7, W | Modrm, {Imm8, Reg | AnyMem}},
    {"sar", 2, 0xD2, 7, W | Modrm, {ShiftCount, Reg | AnyMem}},
    {"sar", 1, 0xD0, 7, W | Modrm, {Reg | AnyMem, 0}},
    
    /* Program control transfer instructions. */
    {"call", 1, 0xE8, None, Call, {Disp32, 0}},

#define PC_RELATIVE_JUMP 0xEB
    {"jmp", 1, 0xEB, None, No_bSuf | Jump, {Disp, 0}},

    {"ret", 0, 0xC3, None, 0, {0, 0}},

    {"leave", 0, 0xC9, None, 0, {0, 0}},

    /* Conditional jumps. */
    {"jo", 1, 0x70, None, Jump, {Disp, 0}},
    {"jno", 1, 0x71, None, Jump, {Disp, 0}},
    {"jb", 1, 0x72, None, Jump, {Disp, 0}},
    {"jc", 1, 0x72, None, Jump, {Disp, 0}},
    {"jnae", 1, 0x72, None, Jump, {Disp, 0}},
    {"jnb", 1, 0x73, None, Jump, {Disp, 0}},
    {"jnc", 1, 0x73, None, Jump, {Disp, 0}},
    {"jae", 1, 0x73, None, Jump, {Disp, 0}},
    {"je", 1, 0x74, None, Jump, {Disp, 0}},
    {"jz", 1, 0x74, None, Jump, {Disp, 0}},
    {"jne", 1, 0x75, None, Jump, {Disp, 0}},
    {"jnz", 1, 0x75, None, Jump, {Disp, 0}},
    {"jbe", 1, 0x76, None, Jump, {Disp, 0}},
    {"jna", 1, 0x76, None, Jump, {Disp, 0}},
    {"jnbe", 1, 0x77, None, Jump, {Disp, 0}},
    {"ja", 1, 0x77, None, Jump, {Disp, 0}},
    {"js", 1, 0x78, None, Jump, {Disp, 0}},
    {"jns", 1, 0x79, None, Jump, {Disp, 0}},
    {"jp", 1, 0x7A, None, Jump, {Disp, 0}},
    {"jpe", 1, 0x7A, None, Jump, {Disp, 0}},
    {"jnp", 1, 0x7B, None, Jump, {Disp, 0}},
    {"jpo", 1, 0x7B, None, Jump, {Disp, 0}},
    {"jl", 1, 0x7C, None, Jump, {Disp, 0}},
    {"jnge", 1, 0x7C, None, Jump, {Disp, 0}},
    {"jnl", 1, 0x7D, None, Jump, {Disp, 0}},
    {"jge", 1, 0x7D, None, Jump, {Disp, 0}},
    {"jle", 1, 0x7E, None, Jump, {Disp, 0}},
    {"jng", 1, 0x7E, None, Jump, {Disp, 0}},
    {"jnle", 1, 0x7F, None, Jump, {Disp, 0}},
    {"jg", 1, 0x7F, None, Jump, {Disp, 0}},

    /* */
    {"sete", 1, 0x0f94, 0, Modrm, {Reg8 | AnyMem, 0}},

    /* String manipulation instructions. */
    {"cmps", 0, 0xA6, None, 0, {0, 0}},
    {"scas", 0, 0xAE, None, 0, {0, 0}},

    /* Prefixes. */
#define DATA_PREFIX_OPCODE 0x66
    {"repe", 0, 0xF3, None, 0, {0, 0}},
    {"repne", 0, 0xF2, None, 0, {0, 0}},

    {"nop", 0, 0x90, None, 0, {0, 0}},

    {0, 0}
};

const reg_entry reg_table[] = {
    /* 8 bit registers. */
    {"al", Reg8 | Acc, 0},
    {"cl", Reg8 | ShiftCount, 1},
    {"dl", Reg8, 2},
    {"bl", Reg8, 3},
    {"ah", Reg8, 4},
    {"ch", Reg8, 5},
    {"dh", Reg8, 6},
    {"bh", Reg8, 7},

    /* 16 bit registers.*/
    {"ax", Reg16 | Acc, 0},
    {"cx", Reg16, 1},

    /* 32 bit registers. */
    {"eax", Reg32 | Acc, 0},
    {"ecx", Reg32, 1},
    {"edx", Reg32, 2},
    {"ebx", Reg32, 3},
    {"esp", Reg32, 4},
    {"ebp", Reg32, 5},
    {"esi", Reg32, 6},
    {"edi", Reg32, 7},
    
    {0, 0, 0}
};

