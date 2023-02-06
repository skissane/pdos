/******************************************************************************
 * @file            assemble.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define     ABSOLUTE_PREFIX             '*'
#define     IMMEDIATE_PREFIX            '$'
#define     REGISTER_PREFIX             '%'

#define     BYTE_SUFFIX                 'b'
#define     WORD_SUFFIX                 'w'
#define     SHORT_SUFFIX                's'
#define     DWORD_SUFFIX                'l'
#define     QWORD_SUFFIX                'q'
/* Internal suffix for .intel_syntax. It cannot be directly used by the user. */
#define     INTEL_SUFFIX                '\1'

#define     MAX_OPERANDS                0x03
#define     MAX_REG_NAME_SIZE           8

/* Prefixes are emitted in the following order. */
#define     FWAIT_PREFIX                0x00
#define     REP_PREFIX                  0x01
#define     ADDR_PREFIX                 0x02
#define     DATA_PREFIX                 0x03
#define     SEGMENT_PREFIX              0x04
#define     MAX_PREFIXES                0x05

#define     MODRM_REGMEM_TWO_BYTE_ADDRESSING                0x04
#define     SIB_BASE_NO_BASE_REGISTER                       0x05
#define     SIB_BASE_NO_BASE_REGISTER_16                    0x06
#define     SIB_INDEX_NO_INDEX_REGISTER                     0x04

struct template {

    const char *name;
    
#define     NO_BSUF                     0x00000040
#define     NO_WSUF                     0x00000080
#define     NO_SSUF                     0x00000100
#define     NO_LSUF                     0x00200000
#define     NO_QSUF                     0x00400000
#define     NO_INTELSUF                 0x00800000
    
    int operands;
    unsigned int base_opcode;
    unsigned int extension_opcode;
    
#define     NONE                        0x0000FFFF
    
    unsigned int opcode_modifier;

#define     W                           0x00000001
#define     D                           0x00000002
    
#define     MODRM                       0x00000004
#define     SHORT_FORM                  0x00000008

#define     JUMP                        0x00000010
#define     CALL                        0x00000020
    
#define     IGNORE_SIZE                 0x00000200
#define     DEFAULT_SIZE                0x01000000
#define     SEGSHORTFORM                0x00040000

#define     FLOAT_D                     0x00000400          /* Must be 0x400. */
    
#define     JUMPINTERSEGMENT            0x00000800
#define     JUMPBYTE                    0x00001000
    
#define     SIZE16                      0x00002000
#define     SIZE32                      0x00004000
    
#define     IS_PREFIX                   0x00008000
#define     IS_STRING                   0x00010000
    
#define     REG_DUPLICATION             0x00020000
    
#define     FLOAT_MF                    0x00080000
#define     ADD_FWAIT                   0x00100000
    
    unsigned int operand_types[MAX_OPERANDS];
    
#define     REG8                        0x00000001
#define     REG16                       0x00000002
#define     REG32                       0x00000004
    
#define     REG                         (REG8 | REG16 | REG32)
#define     WORD_REG                    (REG16 | REG32)
    
#define     SEGMENT1                    0x00000008
#define     SEGMENT2                    0x00020000
#define     CONTROL                     0x00000010
#define     DEBUG                       0x00100000
#define     TEST                        0x00200000
#define     FLOAT_REG                   0x00400000
#define     FLOAT_ACC                   0x00800000
    
#define     IMM8                        0x00000020
#define     IMM8S                       0x00000040
#define     IMM16                       0x00000080
#define     IMM32                       0x00000100
    
#define     IMM                         (IMM8 | IMM8S | IMM16 | IMM32)
#define     ENCODABLEIMM                (IMM8 | IMM16 | IMM32)
    
#define     DISP8                       0x00000200
#define     DISP16                      0x00000400
#define     DISP32                      0x00000800
    
#define     DISP                        (DISP8 | DISP16 | DISP32)
#define     BASE_INDEX                  0x00001000
    
/*
 * INV_MEM is for instruction with modrm where general register
 * encoding is allowed only in modrm.regmem (control register move).
 * */
#define     INV_MEM                     0x00040000
#define     ANY_MEM                     (DISP8 | DISP16 | DISP32 | BASE_INDEX | INV_MEM)
    
#define     ACC                         0x00002000
#define     PORT                        0x00004000
#define     SHIFT_COUNT                 0x00008000
#define     JUMP_ABSOLUTE               0x00010000
    
#define     IMPLICIT_REGISTER           (SHIFT_COUNT | ACC)
    
    int minimum_cpu;

};

struct reg_entry {

    const char *name;
    unsigned int type, number;

};

struct modrm_byte {

    unsigned int regmem;
    unsigned int reg;
    unsigned int mode;

};

struct sib_byte {

    unsigned int base;
    unsigned int index;
    unsigned int scale;

};

#define     NO_SUF                      (NO_BSUF | NO_WSUF | NO_SSUF | NO_LSUF | NO_QSUF | NO_INTELSUF)
#define     B_SUF                       (NO_WSUF | NO_SSUF | NO_LSUF | NO_QSUF | NO_INTELSUF)
#define     W_SUF                       (NO_BSUF | NO_SSUF | NO_LSUF | NO_QSUF | NO_INTELSUF)
#define     L_SUF                       (NO_BSUF | NO_WSUF | NO_SSUF | NO_QSUF | NO_INTELSUF)
#define     Q_SUF                       (NO_BSUF | NO_WSUF | NO_SSUF | NO_LSUF | NO_INTELSUF)
#define     INTEL_SUF                   (NO_BSUF | NO_WSUF | NO_SSUF | NO_LSUF | NO_QSUF)

#define     BW_SUF                      (NO_SSUF | NO_LSUF | NO_QSUF | NO_INTELSUF)
#define     WL_SUF                      (NO_BSUF | NO_SSUF | NO_QSUF | NO_INTELSUF)
#define     BWL_SUF                     (NO_SSUF | NO_QSUF | NO_INTELSUF)
#define     SL_SUF                      (NO_BSUF | NO_WSUF | NO_QSUF | NO_INTELSUF)

/*
 * All instructions with the exact same name must be together without any other instructions mixed in
 * for the code searching the template table to work properly.
 * */
static const struct template template_table[] = {

    /* Move instructions. */
    { "mov", 2, 0xA0, NONE, BWL_SUF | D | W, { DISP16 | DISP32, ACC, 0 }, 0 },
    { "mov", 2, 0x88, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "mov", 2, 0xB0, NONE, BWL_SUF | W | SHORT_FORM, { ENCODABLEIMM, REG8 | REG16 | REG32, 0 }, 0 },
    { "mov", 2, 0xC6, NONE, BWL_SUF | D | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    /* Move instructions for segment registers. */
    { "mov", 2, 0x8C, NONE, WL_SUF | MODRM, { SEGMENT1, WORD_REG | INV_MEM, 0 }, 0 },
    { "mov", 2, 0x8C, NONE, W_SUF | MODRM | IGNORE_SIZE, { SEGMENT1, ANY_MEM, 0 }, 0 },
    { "mov", 2, 0x8C, NONE, WL_SUF | MODRM, { SEGMENT2, WORD_REG | INV_MEM, 0 }, 3 },
    { "mov", 2, 0x8C, NONE, W_SUF | MODRM | IGNORE_SIZE, { SEGMENT2, ANY_MEM, 0 }, 3 },
    { "mov", 2, 0x8E, NONE, WL_SUF | MODRM | IGNORE_SIZE, { WORD_REG | INV_MEM, SEGMENT1, 0 }, 0 },
    { "mov", 2, 0x8E, NONE, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, SEGMENT1, 0 }, 0 },
    { "mov", 2, 0x8E, NONE, WL_SUF | MODRM | IGNORE_SIZE, { WORD_REG | INV_MEM, SEGMENT2, 0 }, 3 },
    { "mov", 2, 0x8E, NONE, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, SEGMENT2, 0 }, 3 },
    
    /* Move instructions for control, debug and test registers. */
    { "mov", 2, 0x0F20, NONE, L_SUF | D | MODRM | IGNORE_SIZE, { CONTROL, REG32 | INV_MEM, 0 }, 3 },
    { "mov", 2, 0x0F21, NONE, L_SUF | D | MODRM | IGNORE_SIZE, { DEBUG, REG32 | INV_MEM, 0 }, 3 },
    { "mov", 2, 0x0F24, NONE, L_SUF | D | MODRM | IGNORE_SIZE, { TEST, REG32 | INV_MEM, 0 }, 3 },
    
    /* Move with sign extend. */
    /* "movsbl" and "movsbw" are not unified into "movsb" to prevent conflict with "movs". */
    { "movsbl", 2, 0x0FBE, NONE, NO_SUF | MODRM, { REG8 | ANY_MEM, REG32, 0 }, 3 },
    { "movsbw", 2, 0x0FBE, NONE, NO_SUF | MODRM, { REG8 | ANY_MEM, REG16, 0 }, 3 },
    { "movswl", 2, 0x0FBF, NONE, NO_SUF | MODRM, { REG16 | ANY_MEM, REG32, 0 }, 3 },
    
    /* Alternative syntax. */
    { "movsx", 2, 0x0FBE, NONE, BW_SUF | W | MODRM, { REG8 | REG16 | ANY_MEM, WORD_REG, 0 }, 3 },
    
    /* Move with zero extend. */
    { "movzb", 2, 0x0FB6, NONE, WL_SUF | MODRM, { REG8 | ANY_MEM, WORD_REG, 0 }, 3 },
    { "movzwl", 2, 0x0FB7, NONE, NO_SUF | MODRM, { REG16 | ANY_MEM, REG32, 0 }, 3 },
    
    /* Alternative syntax. */
    { "movzx", 2, 0x0FB6, NONE, BW_SUF | W | MODRM, { REG8 | REG16 | ANY_MEM, WORD_REG, 0 }, 3 },
    
    /* Push instructions. */
    { "push", 1, 0x50, NONE, WL_SUF | SHORT_FORM, { WORD_REG, 0, 0 }, 0 },
    { "push", 1, 0xFF, 6, WL_SUF | DEFAULT_SIZE | MODRM, { WORD_REG | ANY_MEM, 0, 0 }, 0 },
    { "push", 1, 0x6A, NONE, WL_SUF | DEFAULT_SIZE, { IMM8S, 0, 0 }, 1 },
    { "push", 1, 0x68, NONE, WL_SUF | DEFAULT_SIZE, { IMM16 | IMM32, 0, 0 }, 1 },
    
    { "push", 1, 0x06, NONE, WL_SUF | DEFAULT_SIZE | SEGSHORTFORM, { SEGMENT1, 0, 0 }, 0},
    { "push", 1, 0x0FA0, NONE, WL_SUF | DEFAULT_SIZE | SEGSHORTFORM, { SEGMENT2, 0, 0 }, 3},
    
    { "pusha", 0, 0x60, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 1 },
    
    /* Pop instructions. */
    { "pop", 1, 0x58, NONE, WL_SUF | SHORT_FORM, { WORD_REG, 0, 0 }, 0 },
    { "pop", 1, 0x8F, NONE, WL_SUF | DEFAULT_SIZE | MODRM, { WORD_REG | ANY_MEM, 0, 0 }, 0 },
    
#define     POP_SEGMENT_SHORT           0x07
    
    { "pop", 1, 0x07, NONE, WL_SUF | DEFAULT_SIZE | SEGSHORTFORM, { SEGMENT1, 0, 0 }, 0 },
    { "pop", 1, 0x0FA1, NONE, WL_SUF | DEFAULT_SIZE | SEGSHORTFORM, { SEGMENT2, 0, 0 }, 3 },
    
    { "popa", 0, 0x61, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 1 },
    
    /* Exchange instructions. */
    { "xchg", 2, 0x90, NONE, WL_SUF | SHORT_FORM, { WORD_REG, ACC, 0 }, 0 },
    { "xchg", 2, 0x90, NONE, WL_SUF | SHORT_FORM, { ACC, WORD_REG, 0 }, 0 },
    { "xchg", 2, 0x86, NONE, BWL_SUF | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "xchg", 2, 0x86, NONE, BWL_SUF | W | MODRM, { REG | ANY_MEM, REG, 0 }, 0 },
    
    /* In/out for ports. */
    { "in", 2, 0xE4, NONE, BWL_SUF | W, { IMM8, ACC, 0 }, 0 },
    { "in", 2, 0xEC, NONE, BWL_SUF | W, { PORT, ACC, 0 }, 0 },
    { "in", 1, 0xE4, NONE, BWL_SUF | W, { IMM8, 0, 0 }, 0 },
    { "in", 1, 0xEC, NONE, BWL_SUF | W, { PORT, 0, 0 }, 0 },
    
    { "out", 2, 0xE6, NONE, BWL_SUF | W, { ACC, IMM8, 0 }, 0 },
    { "out", 2, 0xEE, NONE, BWL_SUF | W, { ACC, PORT, 0 }, 0 },
    { "out", 2, 0xE6, NONE, BWL_SUF | W, { IMM8, 0, 0 }, 0 },
    { "out", 2, 0xEE, NONE, BWL_SUF | W, { PORT, 0, 0 }, 0 },
    
    /* Load effective address. */
    { "lea", 2, 0x8D, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 0 },
    
    /* Load far pointer from memory. */
    { "lds", 2, 0xC5, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 0 },
    { "les", 2, 0xC4, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 0 },
    { "lfs", 2, 0x0FB4, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 3 },
    { "lgs", 2, 0x0FB5, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 3 },
    { "lss", 2, 0x0FB2, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 3 },
    
    /* Flags register instructions. */
    { "cmc", 0, 0xF5, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "clc", 0, 0xF8, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "stc", 0, 0xF9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "cli", 0, 0xFA, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "sti", 0, 0xFB, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "cld", 0, 0xFC, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "std", 0, 0xFD, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "clts", 0, 0x0F06, NONE, NO_SUF, { 0, 0, 0 }, 2 },
    { "lahf", 0, 0x9F, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "sahf", 0, 0x9E, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "pushf", 0, 0x9C, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    { "popf", 0, 0x9D, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    
    /* Arithmetic instructions. */
    { "add", 2, 0x00, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "add", 2, 0x83, 0, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "add", 2, 0x04, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "add", 2, 0x80, 0, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "inc", 1, 0x40, NONE, WL_SUF | SHORT_FORM, { WORD_REG, 0, 0 }, 0 },
    { "inc", 1, 0xFE, 0, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "sub", 2, 0x28, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "sub", 2, 0x83, 5, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "sub", 2, 0x2C, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "sub", 2, 0x80, 5, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "dec", 1, 0x48, NONE, WL_SUF | SHORT_FORM, { WORD_REG, 0, 0 }, 0 },
    { "dec", 1, 0xFE, 1, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "sbb", 2, 0x18, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "sbb", 2, 0x83, 3, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "sbb", 2, 0x1C, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "sbb", 2, 0x80, 3, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "cmp", 2, 0x38, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "cmp", 2, 0x83, 7, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "cmp", 2, 0x3C, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "cmp", 2, 0x80, 7, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "test", 2, 0x84, NONE, BWL_SUF | W | MODRM, { REG | ANY_MEM, REG, 0 }, 0 },
    { "test", 2, 0x84, NONE, BWL_SUF | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "test", 2, 0xA8, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "test", 2, 0xF6, 0, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "and", 2, 0x20, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "and", 2, 0x83, 4, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "and", 2, 0x24, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "and", 2, 0x80, 4, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "or", 2, 0x08, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "or", 2, 0x83, 1, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "or", 2, 0x0C, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "or", 2, 0x80, 1, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "xor", 2, 0x30, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "xor", 2, 0x83, 6, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "xor", 2, 0x34, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "xor", 2, 0x80, 6, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "clr", 1, 0x30, NONE, BWL_SUF | W | MODRM | REG_DUPLICATION, { REG, 0, 0 }, 0 },
    
    { "adc", 2, 0x10, NONE, BWL_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "adc", 2, 0x83, 2, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "adc", 2, 0x14, NONE, BWL_SUF | W, { ENCODABLEIMM, ACC, 0 }, 0 },
    { "adc", 2, 0x80, 2, BWL_SUF | W | MODRM, { ENCODABLEIMM, REG | ANY_MEM, 0 }, 0 },
    
    { "neg", 1, 0xF6, 3, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    { "not", 1, 0xF6, 2, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "aaa", 0, 0x37, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "aas", 0, 0x3F, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    { "daa", 0, 0x27, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "das", 0, 0x2F, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    { "aad", 0, 0xD50A, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "aad", 1, 0xD5, NONE, NO_SUF, { IMM8, 0, 0 }, 0 },
    
    { "aam", 0, 0xD40A, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "aam", 1, 0xD4, NONE, NO_SUF, { IMM8, 0, 0 }, 0 },
    
    /* Conversion instructions. */
    { "cbw", 0, 0x98, NONE, NO_SUF | SIZE16, { 0, 0, 0 }, 0 },
    { "cwde", 0, 0x98, NONE, NO_SUF | SIZE32, { 0, 0, 0 }, 0 },
    { "cwd", 0, 0x99, NONE, NO_SUF | SIZE16, { 0, 0, 0 }, 0 },
    { "cdq", 0, 0x99, NONE, NO_SUF | SIZE32, { 0, 0, 0 }, 3 },
    
    /* Other naming. */
    { "cbtw", 0, 0x98, NONE, NO_SUF | SIZE16, { 0, 0, 0 }, 0 },
    { "cwtl", 0, 0x98, NONE, NO_SUF | SIZE32, { 0, 0, 0 }, 0 },
    { "cwtd", 0, 0x99, NONE, NO_SUF | SIZE16, { 0, 0, 0 }, 0 },
    { "cltd", 0, 0x99, NONE, NO_SUF | SIZE32, { 0, 0, 0 }, 3 },
    
    { "mul", 1, 0xF6, 4, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "imul", 1, 0xF6, 5, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    { "imul", 2, 0x0FAF, NONE, WL_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, 3 },
    { "imul", 3, 0x6B, NONE, WL_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, WORD_REG }, 1 },
    { "imul", 3, 0x69, NONE, WL_SUF | MODRM, { IMM16 | IMM32, WORD_REG | ANY_MEM, WORD_REG }, 1 },
    { "imul", 2, 0x6B, NONE, WL_SUF | MODRM | REG_DUPLICATION, { IMM8S, WORD_REG, 0 }, 1 },
    { "imul", 2, 0x69, NONE, WL_SUF | MODRM | REG_DUPLICATION, { IMM16 | IMM32, WORD_REG, 0 }, 1 },
    
    { "div", 1, 0xF6, 6, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    { "div", 2, 0xF6, 6, BWL_SUF | W | MODRM, { REG | ANY_MEM, ACC, 0 }, 0 },
    
    { "idiv", 1, 0xF6, 7, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    { "idiv", 2, 0xF6, 7, BWL_SUF | W | MODRM, { REG | ANY_MEM, ACC, 0 }, 0 },
    
    { "rol", 2, 0xC0, 0, BWL_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, 1 },
    { "rol", 2, 0xD2, 0, BWL_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "rol", 1, 0xD0, 0, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "ror", 2, 0xC0, 1, BWL_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, 1 },
    { "ror", 2, 0xD2, 1, BWL_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "ror", 1, 0xD0, 1, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "rcl", 2, 0xC0, 2, BWL_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, 1 },
    { "rcl", 2, 0xD2, 2, BWL_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "rcl", 1, 0xD0, 2, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "rcr", 2, 0xC0, 3, BWL_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, 1 },
    { "rcr", 2, 0xD2, 3, BWL_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "rcr", 1, 0xD0, 3, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "sal", 2, 0xC0, 4, BWL_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, 1 },
    { "sal", 2, 0xD2, 4, BWL_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "sal", 1, 0xD0, 4, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "shl", 2, 0xC0, 4, BWL_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, 1 },
    { "shl", 2, 0xD2, 4, BWL_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "shl", 1, 0xD0, 4, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "shr", 2, 0xC0, 5, BWL_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, 1 },
    { "shr", 2, 0xD2, 5, BWL_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "shr", 1, 0xD0, 5, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "sar", 2, 0xC0, 7, BWL_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, 1 },
    { "sar", 2, 0xD2, 7, BWL_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "sar", 1, 0xD0, 7, BWL_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "shld", 3, 0x0FA4, NONE, WL_SUF | MODRM, { IMM8, WORD_REG, WORD_REG | ANY_MEM }, 3 },
    { "shld", 3, 0x0FA5, NONE, WL_SUF | MODRM, { SHIFT_COUNT, WORD_REG, WORD_REG | ANY_MEM }, 3 },
    { "shld", 2, 0x0FA5, NONE, WL_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, 3 },
    
    { "shrd", 3, 0x0FAC, NONE, WL_SUF | MODRM, { IMM8, WORD_REG, WORD_REG | ANY_MEM }, 3 },
    { "shrd", 3, 0x0FAD, NONE, WL_SUF | MODRM, { SHIFT_COUNT, WORD_REG, WORD_REG | ANY_MEM }, 3 },
    { "shrd", 2, 0x0FAD, NONE, WL_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, 3 },
    
    /* Program control transfer instructions. */
    { "call", 1, 0xE8, NONE, WL_SUF | DEFAULT_SIZE | CALL, { DISP16 | DISP32, 0, 0 }, 0 },
    { "call", 1, 0xFF, 2, WL_SUF | DEFAULT_SIZE | MODRM, { WORD_REG | ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    { "call", 2, 0x9A, NONE, WL_SUF | DEFAULT_SIZE | JUMPINTERSEGMENT, { IMM16, IMM16 | IMM32, 0 }, 0 },
    { "call", 1, 0xFF, 3, INTEL_SUF | DEFAULT_SIZE | MODRM, { ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    
    /* Alternative syntax. */
    { "lcall", 2, 0x9A, NONE, WL_SUF | DEFAULT_SIZE | JUMPINTERSEGMENT, { IMM16, IMM16 | IMM32, 0 }, 0 },
    { "lcall", 1, 0xFF, 3, WL_SUF | DEFAULT_SIZE | MODRM, { ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    
#define     PC_RELATIVE_JUMP            0xEB
    
    { "jmp", 1, 0xEB, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jmp", 1, 0xFF, 4, WL_SUF | MODRM, { WORD_REG | ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    { "jmp", 2, 0xEA, NONE, WL_SUF | JUMPINTERSEGMENT, { IMM16, IMM16 | IMM32, 0 }, 0 },
    { "jmp", 1, 0xFF, 5, INTEL_SUF | MODRM, { ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    
    /* Alternative syntax. */
    { "ljmp", 2, 0xEA, NONE, WL_SUF | JUMPINTERSEGMENT, { IMM16, IMM16 | IMM32, 0 }, 0 },
    { "ljmp", 1, 0xFF, 5, WL_SUF | MODRM, { ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    
    { "ret", 0, 0xC3, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    { "ret", 1, 0xC2, NONE, WL_SUF | DEFAULT_SIZE, { IMM16, 0, 0 }, 0 },
    { "retf", 0, 0xCB, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    { "retf", 1, 0xCA, NONE, WL_SUF | DEFAULT_SIZE, { IMM16, 0, 0 }, 0 },
    { "lret", 0, 0xCB, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    { "lret", 1, 0xCA, NONE, WL_SUF | DEFAULT_SIZE, { IMM16, 0, 0 }, 0 },
    { "enter", 2, 0xC8, NONE, WL_SUF | DEFAULT_SIZE, { IMM16, IMM8, 0 }, 1 },
    { "leave", 0, 0xC9, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 1 },
    
    /* Conditional jumps. */
    { "jo", 1, 0x70, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jno", 1, 0x71, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jb", 1, 0x72, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jc", 1, 0x72, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnae", 1, 0x72, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnb", 1, 0x73, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnc", 1, 0x73, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jae", 1, 0x73, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "je", 1, 0x74, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jz", 1, 0x74, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jne", 1, 0x75, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnz", 1, 0x75, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jbe", 1, 0x76, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jna", 1, 0x76, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "ja", 1, 0x77, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnbe", 1, 0x77, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "js", 1, 0x78, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jns", 1, 0x79, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jp", 1, 0x7A, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jpe", 1, 0x7A, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnp", 1, 0x7B, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jpo", 1, 0x7B, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jl", 1, 0x7C, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnge", 1, 0x7C, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jge", 1, 0x7D, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnl", 1, 0x7D, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jle", 1, 0x7E, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jng", 1, 0x7E, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jg", 1, 0x7F, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jnle", 1, 0x7F, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    
    { "jcxz", 1, 0xE3, NONE, NO_SUF | JUMPBYTE | SIZE16, { DISP, 0, 0 }, 0 },
    { "jecxz", 1, 0xE3, NONE, NO_SUF | JUMPBYTE | SIZE32, { DISP, 0, 0 }, 0 },
    
    /* Loop instructions. */
    { "loop", 1, 0xE2, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    { "loopz", 1, 0xE1, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    { "loope", 1, 0xE1, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    { "loopnz", 1, 0xE0, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    { "loopne", 1, 0xE0, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    
    /* Set byte on flag instructions. */
    { "seto", 1, 0x0F90, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setno", 1, 0x0F91, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setb", 1, 0x0F92, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setc", 1, 0x0F92, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnae", 1, 0x0F92, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnb", 1, 0x0F93, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnc", 1, 0x0F93, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setae", 1, 0x0F93, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "sete", 1, 0x0F94, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setz", 1, 0x0F94, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setne", 1, 0x0F95, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnz", 1, 0x0F95, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setbe", 1, 0x0F96, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setna", 1, 0x0F96, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnbe", 1, 0x0F97, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "seta", 1, 0x0F97, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "sets", 1, 0x0F98, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setns", 1, 0x0F99, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setp", 1, 0x0F9A, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setpe", 1, 0x0F9A, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnp", 1, 0x0F9B, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setpo", 1, 0x0F9B, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setl", 1, 0x0F9C, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnge", 1, 0x0F9C, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnl", 1, 0x0F9D, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setge", 1, 0x0F9D, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setle", 1, 0x0F9E, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setng", 1, 0x0F9E, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setnle", 1, 0x0F9F, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    { "setg", 1, 0x0F9F, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, 3 },
    
    /* String manipulation instructions. */
    { "cmps", 0, 0xA6, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "scmp", 0, 0xA6, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "ins", 0, 0x6C, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 1 },
    { "outs", 0, 0x6E, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 1 },
    { "lods", 0, 0xAC, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "slod", 0, 0xAC, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "movs", 0, 0xA4, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "smov", 0, 0xA4, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "scas", 0, 0xAE, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "ssca", 0, 0xAE, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "stos", 0, 0xAA, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "ssto", 0, 0xAA, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "xlat", 0, 0xD7, NONE, B_SUF | IS_STRING, { 0, 0, 0 }, 0 },
    
    /* Bit manipulation instructions. */
    { "bsf", 2, 0x0FBC, NONE, WL_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, 3 },
    { "bsr", 2, 0x0FBD, NONE, WL_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, 3 },
    { "bt", 2, 0x0FA3, NONE, WL_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, 3 },
    { "bt", 2, 0x0FBA, 4, WL_SUF | MODRM, { IMM8, WORD_REG | ANY_MEM, 0 }, 3 },
    { "btc", 2, 0x0FBB, NONE, WL_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, 3 },
    { "btc", 2, 0x0FBA, 7, WL_SUF | MODRM, { IMM8, WORD_REG | ANY_MEM, 0 }, 3 },
    { "btr", 2, 0x0FB3, NONE, WL_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, 3 },
    { "btr", 2, 0x0FBA, 6, WL_SUF | MODRM, { IMM8, WORD_REG | ANY_MEM, 0 }, 3 },
    { "bts", 2, 0x0FAB, NONE, WL_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, 3 },
    { "bts", 2, 0x0FBA, 5, WL_SUF | MODRM, { IMM8, WORD_REG | ANY_MEM, 0 }, 3 },
    
    /* Interrupts. */
#define     INT_OPCODE            0xCD
#define     INT3_OPCODE           0xCC
    { "int", 1, 0xCD, NONE, 0, { IMM8, 0, 0 }, 0 },
    { "int3", 0, 0xCC, NONE, 0, { 0, 0, 0 }, 0 },
    { "into", 0, 0xCE, NONE, 0, { 0, 0, 0 }, 0 },
    { "iret", 0, 0xCF, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    
    { "rsm", 0, 0x0FAA, NONE, 0, { 0, 0, 0 }, 3 },
    { "bound", 2, 0x62, NONE, 0, { WORD_REG, ANY_MEM, 0 }, 1 },
    
    { "hlt", 0, 0xF4, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "nop", 0, 0x90, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Protection control. */
    { "arpl", 2, 0x63, NONE, W_SUF | MODRM | IGNORE_SIZE, { REG16, REG16 | ANY_MEM, 0 }, 2 },
    { "lar", 2, 0x0F02, NONE, WL_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, 2 },
    { "lgdt", 1, 0x0F01, 2, WL_SUF | MODRM, { ANY_MEM, 0, 0 }, 2 },
    { "lidt", 1, 0x0F01, 3, WL_SUF | MODRM, { ANY_MEM, 0, 0 }, 2 },
    { "lldt", 1, 0x0F00, 2, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, 2 },
    { "lmsw", 1, 0x0F01, 6, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, 2 },
    { "lsl", 2, 0x0F03, NONE, WL_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, 2 },
    { "ltr", 1, 0x0F00, 3, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, 2 },
    
    { "sgdt", 1, 0x0F01, 0, WL_SUF | MODRM, { ANY_MEM, 0, 0 }, 2 },
    { "sidt", 1, 0x0F01, 1, WL_SUF | MODRM, { ANY_MEM, 0, 0 }, 2 },
    { "sldt", 1, 0x0F00, 0, WL_SUF | MODRM, { WORD_REG | INV_MEM, 0, 0 }, 2 },
    { "sldt", 1, 0x0F00, 0, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, 0, 0 }, 2 },
    { "smsw", 1, 0x0F01, 4, WL_SUF | MODRM, { WORD_REG | INV_MEM, 0, 0 }, 2 },
    { "smsw", 1, 0x0F01, 4, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, 0, 0 }, 2 },
    { "str", 1, 0x0F00, 1, WL_SUF | MODRM, { WORD_REG | INV_MEM, 0, 0 }, 2 },
    { "str", 1, 0x0F00, 1, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, 0, 0 }, 2 },
    
    { "verr", 1, 0x0F00, 4, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, 2 },
    { "verw", 1, 0x0F00, 5, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, 2 },
    
    /* Floating point instructions. */
    /* Load. */
    { "fld", 1, 0xD9C0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fld", 1, 0xD9, 0, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fld", 1, 0xDB, 5, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fild", 1, 0xDF, 0, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fild", 1, 0xDF, 5, Q_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fildll", 1, 0xDF, 5, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fldt", 1, 0xDB, 5, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fbld", 1, 0xDF, 4, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    /* Store without pop. */
    { "fst", 1, 0xDDD0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fst", 1, 0xD9, 2, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fist", 1, 0xDF, 2, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    /* Store with pop. */
    { "fstp", 1, 0xDDD8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fstp", 1, 0xD9, 3, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fstp", 1, 0xDB, 7, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fistp", 1, 0xDF, 3, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fistp", 1, 0xDF, 7, Q_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fistpll", 1, 0xDF, 7, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fstpt", 1, 0xDB, 7, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fbstp", 1, 0xDF, 6, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    /* Exchange of %st(0) with %st(x). */
    { "fxch", 1, 0xD9C8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    /* Exchange of %st(0) with %st(1). */
    { "fxch", 0, 0xD9C9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Comparison without pop. */
    { "fcom", 1, 0xD8D0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fcom", 0, 0xD8D1, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fcom", 1, 0xD8, 2, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "ficom", 1, 0xDE, 2, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    /* Comparison with pop. */
    { "fcomp", 1, 0xD8D8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fcomp", 0, 0xD8D9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fcomp", 1, 0xD8, 3, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "ficomp", 1, 0xDE, 3, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fcompp", 0, 0xDED9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Unordered comparison. */
    { "fucom", 1, 0xDDE0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 3 },
    { "fucom", 0, 0xDDE1, NONE, NO_SUF, { 0, 0, 0 }, 3 },
    { "fucomp", 1, 0xDDE8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 3 },
    { "fucomp", 0, 0xDDE9, NONE, NO_SUF, { 0, 0, 0 }, 3 },
    { "fucompp", 0, 0xDAE9, NONE, NO_SUF, { 0, 0, 0 }, 3 },
    
    { "ftst", 0, 0xD9E4, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fxam", 0, 0xD9E5, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Load constant. */
    { "fld1", 0, 0xD9E8, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fldl2t", 0, 0xD9E9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fldl2e", 0, 0xD9EA, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fldpi", 0, 0xD9EB, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fldlg2", 0, 0xD9EC, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fldln2", 0, 0xD9ED, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fldz", 0, 0xD9EE, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Arithmetic floating point instructions. */
    /* Add. */
    { "fadd", 2, 0xD8C0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, 0 },
    { "fadd", 1, 0xD8C0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fadd", 1, 0xD8, 0, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fiadd", 1, 0xDE, 0, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    { "faddp", 2, 0xDEC0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, 0 },
    { "faddp", 1, 0xDEC0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "faddp", 0, 0xDEC1, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Subtract. */
    { "fsub", 2, 0xD8E0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, 0 },
    { "fsub", 1, 0xD8E0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fsub", 1, 0xD8, 4, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fisub", 1, 0xDE, 4, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    { "fsubp", 2, 0xDEE0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, 0 },
    { "fsubp", 1, 0xDEE0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fsubp", 0, 0xDEE1, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Subtract reverse. */
    { "fsubr", 2, 0xD8E8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, 0 },
    { "fsubr", 1, 0xD8E8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fsubr", 1, 0xD8, 5, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fisubr", 1, 0xDE, 5, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    { "fsubrp", 2, 0xDEE8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, 0 },
    { "fsubrp", 1, 0xDEE8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fsubrp", 0, 0xDEE9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Multiply. */
    { "fmul", 2, 0xD8C8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, 0 },
    { "fmul", 1, 0xD8C8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fmul", 1, 0xD8, 1, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fimul", 1, 0xDE, 1, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    { "fmulp", 2, 0xDEC8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, 0 },
    { "fmulp", 1, 0xDEC8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fmulp", 0, 0xDEC9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Divide. */
    { "fdiv", 2, 0xD8F0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, 0 },
    { "fdiv", 1, 0xD8F0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fdiv", 1, 0xD8, 6, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fidiv", 1, 0xDE, 6, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    { "fdivp", 2, 0xDEF0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, 0 },
    { "fdivp", 1, 0xDEF0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fdivp", 0, 0xDEF1, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Divide reverse. */
    { "fdivr", 2, 0xD8F8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, 0 },
    { "fdivr", 1, 0xD8F8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fdivr", 1, 0xD8, 7, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fidivr", 1, 0xDE, 7, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    { "fdivrp", 2, 0xDEF8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, 0 },
    { "fdivrp", 1, 0xDEF8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "fdivrp", 0, 0xDEF9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Other operations. */
    { "f2xm1", 0, 0xD9F0, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fyl2x", 0, 0xD9F1, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fptan", 0, 0xD9F2, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fpatan", 0, 0xD9F3, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fxtract", 0, 0xD9F4, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fprem1", 0, 0xD9F5, NONE, NO_SUF, { 0, 0, 0 }, 3 },
    { "fdecstp", 0, 0xD9F6, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fincstp", 0, 0xD9F7, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fprem", 0, 0xD9F8, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fyl2xp1", 0, 0xD9F9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fsqrt", 0, 0xD9FA, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fsincos", 0, 0xD9FB, NONE, NO_SUF, { 0, 0, 0 }, 3 },
    { "frndint", 0, 0xD9FC, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fscale", 0, 0xD9FD, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fsin", 0, 0xD9FE, NONE, NO_SUF, { 0, 0, 0 }, 3 },
    { "fcos", 0, 0xD9FF, NONE, NO_SUF, { 0, 0, 0 }, 3 },
    { "fchs", 0, 0xD9E0, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fabs", 0, 0xD9E1, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Processor control instructions. */
    { "fninit", 0, 0xDBE3, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "finit", 0, 0xDBE3, NONE, NO_SUF | ADD_FWAIT, { 0, 0, 0 }, 0 },
    { "fldcw", 1, 0xD9, 5, W_SUF | IGNORE_SIZE | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fnstcw", 1, 0xD9, 7, W_SUF | IGNORE_SIZE | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fstcw", 1, 0xD9, 7, W_SUF | IGNORE_SIZE | ADD_FWAIT | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    { "fnstsw", 1, 0xDFE0, NONE, NO_SUF | IGNORE_SIZE, { ACC, 0, 0 }, 0 },
    { "fnstsw", 1, 0xDD, 7, W_SUF | IGNORE_SIZE | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fnstsw", 0, 0xDFE0, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    { "fstsw", 1, 0xDFE0, NONE, NO_SUF | ADD_FWAIT | IGNORE_SIZE, { ACC, 0, 0 }, 0 },
    { "fstsw", 1, 0xDD, 7, W_SUF | IGNORE_SIZE | ADD_FWAIT | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fstsw", 0, 0xDFE0, NONE, NO_SUF | ADD_FWAIT, { 0, 0, 0 }, 0 },
    
    { "fnclex", 0, 0xDBE2, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "fclex", 0, 0xDBE2, NONE, NO_SUF | ADD_FWAIT, { 0, 0, 0 }, 0 },
    
    { "fnstenv", 1, 0xD9, 6, SL_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fstenv", 1, 0xD9, 6, SL_SUF | ADD_FWAIT | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fldenv", 1, 0xD9, 4, SL_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fnsave", 1, 0xDD, 6, SL_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "fsave", 1, 0xDD, 6, SL_SUF | ADD_FWAIT | MODRM, { ANY_MEM, 0, 0 }, 0 },
    { "frstor", 1, 0xDD, 4, SL_SUF | MODRM, { ANY_MEM, 0, 0 }, 0 },
    
    { "ffree", 1, 0xDDC0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 0 },
    { "ffreep", 1, 0xDFC0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, 6 },
    { "fnop", 0, 0xD9D0, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
#define     FWAIT_OPCODE                0x9B
    
    { "fwait", 0, 0x9B, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Opcode prefixes. They are allowed as separate instructions too. */
#define     ADDR_PREFIX_OPCODE          0x67
    
    { "addr16", 0, 0x67, NONE, NO_SUF | IS_PREFIX | SIZE16 | IGNORE_SIZE, { 0, 0, 0 }, 3 },
    { "addr32", 0, 0x67, NONE, NO_SUF | IS_PREFIX | SIZE32 | IGNORE_SIZE, { 0, 0, 0 }, 3 },
    { "aword", 0, 0x67, NONE, NO_SUF | IS_PREFIX | SIZE16 | IGNORE_SIZE, { 0, 0, 0 }, 3 },
    { "adword", 0, 0x67, NONE, NO_SUF | IS_PREFIX | SIZE32 | IGNORE_SIZE, { 0, 0, 0 }, 3 },
    
#define     DATA_PREFIX_OPCODE          0x66
    
    { "data16", 0, 0x66, NONE, NO_SUF | IS_PREFIX | SIZE16 | IGNORE_SIZE, { 0, 0, 0 }, 3 },
    { "data32", 0, 0x66, NONE, NO_SUF | IS_PREFIX | SIZE32 | IGNORE_SIZE, { 0, 0, 0 }, 3 },
    { "word", 0, 0x66, NONE, NO_SUF | IS_PREFIX | SIZE16 | IGNORE_SIZE, { 0, 0, 0 }, 3 },
    { "dword", 0, 0x66, NONE, NO_SUF | IS_PREFIX | SIZE32 | IGNORE_SIZE, { 0, 0, 0 }, 3 },
    
#define     CS_PREFIX_OPCODE            0x2E
    { "cs", 0, 0x2E, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
#define     DS_PREFIX_OPCODE            0x3E
    { "ds", 0, 0x3E, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
#define     ES_PREFIX_OPCODE            0x26
    { "es", 0, 0x26, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
#define     FS_PREFIX_OPCODE            0x64
    { "fs", 0, 0x64, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
#define     GS_PREFIX_OPCODE            0x65
    { "gs", 0, 0x65, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
#define     SS_PREFIX_OPCODE            0x36
    { "ss", 0, 0x36, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
    
#define     REPNE_PREFIX_OPCODE         0xF2
#define     REPE_PREFIX_OPCODE          0xF3
    
    { "repne", 0, 0xF2, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
    { "repnz", 0, 0xF2, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
    { "rep", 0, 0xF3, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
    { "repe", 0, 0xF3, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },
    { "repz", 0, 0xF3, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, 0 },

    /* i486 extensions. */
    { "bswap", 1, 0x0FC8, NONE, L_SUF | SHORT_FORM, { REG32, 0, 0 }, 4 },

    /* i686 extensions. */
#define CMOVcc(name, opcode) { name, 2, opcode, NONE, WL_SUF | MODRM, {REG16 | REG32 | ANY_MEM, REG16 | REG32, 0}, 6}
    CMOVcc ("cmovo", 0x0F40),
    CMOVcc ("cmovno", 0x0F41),
    CMOVcc ("cmovb", 0x0F42),
    CMOVcc ("cmovc", 0x0F42),
    CMOVcc ("cmovnae", 0x0F42),
    CMOVcc ("cmovnb", 0x0F43),
    CMOVcc ("cmovnc", 0x0F43),
    CMOVcc ("cmovae", 0x0F43),
    CMOVcc ("cmove", 0x0F44),
    CMOVcc ("cmovz", 0x0F44),
    CMOVcc ("cmovne", 0x0F45),
    CMOVcc ("cmovnz", 0x0F45),
    CMOVcc ("cmovbe", 0x0F46),
    CMOVcc ("cmovna", 0x0F46),
    CMOVcc ("cmova", 0x0F47),
    CMOVcc ("cmovnbe", 0x0F47),
    CMOVcc ("cmovs", 0x0F48),
    CMOVcc ("cmovns", 0x0F49),
    CMOVcc ("cmovp", 0x0F4A),
    CMOVcc ("cmovpe", 0x0F4A),
    CMOVcc ("cmovnp", 0x0F4B),
    CMOVcc ("cmovpo", 0x0F4B),
    CMOVcc ("cmovl", 0x0F4C),
    CMOVcc ("cmovnge", 0x0F4C),
    CMOVcc ("cmovge", 0x0F4D),
    CMOVcc ("cmovnl", 0x0F4D),
    CMOVcc ("cmovle", 0x0F4E),
    CMOVcc ("cmovng", 0x0F4E),
    CMOVcc ("cmovg", 0x0F4F),
    CMOVcc ("cmovnle", 0x0F4F),
#undef CMOVcc
    
    /* End of instructions. */
    { 0, 0, 0, 0, 0, { 0, 0, 0 }, 0 }

};

#define     REG_FLAT_NUMBER             ((unsigned int) ~0)

static const struct reg_entry reg_table[] = {

    /* 8 bit registers. */
    { "al", REG8 | ACC, 0 },
    { "cl", REG8 | SHIFT_COUNT, 1 },
    { "dl", REG8, 2 },
    { "bl", REG8, 3 },
    { "ah", REG8, 4 },
    { "ch", REG8, 5 },
    { "dh", REG8, 6 },
    { "bh", REG8, 7 },

    /* 16 bit registers. */
    { "ax", REG16 | ACC, 0 },
    { "cx", REG16, 1 },
    { "dx", REG16 | PORT, 2 },
    { "bx", REG16 | BASE_INDEX, 3 },
    { "sp", REG16, 4 },
    { "bp", REG16 | BASE_INDEX, 5 },
    { "si", REG16 | BASE_INDEX, 6 },
    { "di", REG16 | BASE_INDEX, 7 },
    
    /* 32 bit registers. */
    { "eax", REG32 | BASE_INDEX | ACC, 0 },
    { "ecx", REG32 | BASE_INDEX, 1 },
    { "edx", REG32 | BASE_INDEX, 2 },
    { "ebx", REG32 | BASE_INDEX, 3 },
    { "esp", REG32, 4 },
    { "ebp", REG32 | BASE_INDEX, 5 },
    { "esi", REG32 | BASE_INDEX, 6 },
    { "edi", REG32 | BASE_INDEX, 7 },
    
    /* Segment registers. */
    { "es", SEGMENT1, 0 },
    { "cs", SEGMENT1, 1 },
    { "ss", SEGMENT1, 2 },
    { "ds", SEGMENT1, 3 },
    { "fs", SEGMENT2, 4 },
    { "gs", SEGMENT2, 5 },
    
    /* Segment pseudo-register. */
    { "flat", SEGMENT1, REG_FLAT_NUMBER },
    
    /* Control registers. */
    { "cr0", CONTROL, 0 },
    { "cr1", CONTROL, 1 },
    { "cr2", CONTROL, 2 },
    { "cr3", CONTROL, 3 },
    { "cr4", CONTROL, 4 },
    { "cr5", CONTROL, 5 },
    { "cr6", CONTROL, 6 },
    { "cr7", CONTROL, 7 },
    
    /* Debug registers. */
    { "db0", DEBUG, 0 },
    { "db1", DEBUG, 1 },
    { "db2", DEBUG, 2 },
    { "db3", DEBUG, 3 },
    { "db4", DEBUG, 4 },
    { "db5", DEBUG, 5 },
    { "db6", DEBUG, 6 },
    { "db7", DEBUG, 7 },
    
    /* Other naming. */
    { "dr0", DEBUG, 0 },
    { "dr1", DEBUG, 1 },
    { "dr2", DEBUG, 2 },
    { "dr3", DEBUG, 3 },
    { "dr4", DEBUG, 4 },
    { "dr5", DEBUG, 5 },
    { "dr6", DEBUG, 6 },
    { "dr7", DEBUG, 7 },
    
    /* Test registers. */
    { "tr0", TEST, 0 },
    { "tr1", TEST, 1 },
    { "tr2", TEST, 2 },
    { "tr3", TEST, 3 },
    { "tr4", TEST, 4 },
    { "tr5", TEST, 5 },
    { "tr6", TEST, 6 },
    { "tr7", TEST, 7 },
    
    /* Floating point registers. Explicit "st(0)" is not needed. */
    { "st", FLOAT_REG | FLOAT_ACC, 0},
    { "st(1)", FLOAT_REG, 1 },
    { "st(2)", FLOAT_REG, 2 },
    { "st(3)", FLOAT_REG, 3 },
    { "st(4)", FLOAT_REG, 4 },
    { "st(5)", FLOAT_REG, 5 },
    { "st(6)", FLOAT_REG, 6 },
    { "st(7)", FLOAT_REG, 7 },
    
    /* End of registers. */
    { 0, 0, 0 }

};

static const unsigned char segment_prefixes[] = {

    ES_PREFIX_OPCODE,
    CS_PREFIX_OPCODE,
    SS_PREFIX_OPCODE,
    DS_PREFIX_OPCODE,
    FS_PREFIX_OPCODE,
    GS_PREFIX_OPCODE

};
