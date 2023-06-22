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

#define     MAX_OPERANDS                3
#define     MAX_REG_NAME_SIZE           8

/* Prefixes are emitted in the following order. */
#define     FWAIT_PREFIX                0x00
#define     REP_PREFIX                  0x01
#define     ADDR_PREFIX                 0x02
#define     DATA_PREFIX                 0x03
#define     SEGMENT_PREFIX              0x04
#define     REX_PREFIX                  0x05
#define     MAX_PREFIXES                0x06

#define     MODRM_REGMEM_TWO_BYTE_ADDRESSING                0x04
#define     SIB_BASE_NO_BASE_REGISTER                       0x05
#define     SIB_BASE_NO_BASE_REGISTER_16                    0x06
#define     SIB_INDEX_NO_INDEX_REGISTER                     0x04

struct template {

    const char *name;
    
#define     NO_BSUF                     (1LU << 25)
#define     NO_WSUF                     (1LU << 26)
#define     NO_SSUF                     (1LU << 27)
#define     NO_LSUF                     (1LU << 28)
#define     NO_QSUF                     (1LU << 29)
#define     NO_INTELSUF                 (1LU << 30)
    
    int operands;
    unsigned long base_opcode;
    unsigned int extension_opcode;
    
#define     NONE                        0x0000FFFF
    
    flag_int opcode_modifier;

#define     W                           (1LU << 0)
#define     D                           (1LU << 1)
    
#define     MODRM                       (1LU << 2)
#define     SHORT_FORM                  (1LU << 3)

#define     JUMP                        (1LU << 4)
#define     CALL                        (1LU << 5)
    
#define     IGNORE_SIZE                 (1LU << 6)
#define     DEFAULT_SIZE                (1LU << 7)
#define     SEGSHORTFORM                (1LU << 8)

#define     FLOAT_D                     (1LU << 10)
    
#define     JUMPINTERSEGMENT            (1LU << 11)
#define     JUMPBYTE                    (1LU << 12)
    
#define     SIZE16                      (1LU << 13)
#define     SIZE32                      (1LU << 14)
#define     SIZE64                      (1LU << 15)
    
#define     IS_PREFIX                   (1LU << 16)
#define     IS_STRING                   (1LU << 17)
    
#define     REG_DUPLICATION             (1LU << 18)
    
#define     FLOAT_MF                    (1LU << 19)
#define     ADD_FWAIT                   (1LU << 20)

#define     NO_REX_W                    (1LU << 21)

#define     IMMEXT                      (1LU << 22)
    
    flag_int operand_types[MAX_OPERANDS];
    
#define     REG8                        (1LU << 0)
#define     REG16                       (1LU << 1)
#define     REG32                       (1LU << 2)
#define     REG64                       (1LU << 3)
    
#define     REG                         (REG8 | REG16 | REG32 | REG64)
#define     WORD_REG                    (REG16 | REG32 | REG64)
    
#define     SEGMENT1                    (1LU << 4)
#define     SEGMENT2                    (1LU << 5)
#define     CONTROL                     (1LU << 6)
#define     DEBUG                       (1LU << 7)
#define     TEST                        (1LU << 8)
#define     FLOAT_REG                   (1LU << 9)
#define     FLOAT_ACC                   (1LU << 10)
    
#define     IMM8                        (1LU << 11)
#define     IMM8S                       (1LU << 12)
#define     IMM16                       (1LU << 13)
#define     IMM32                       (1LU << 14)
#define     IMM32S                      (1LU << 15)
#define     IMM64                       (1LU << 16)
    
#define     IMM                         (IMM8 | IMM8S | IMM16 | IMM32 | IMM32S | IMM64)
    
#define     DISP8                       (1LU << 17)
#define     DISP16                      (1LU << 18)
#define     DISP32                      (1LU << 19)
    
#define     DISP                        (DISP8 | DISP16 | DISP32)
#define     BASE_INDEX                  (1LU << 22)
    
/*
 * INV_MEM is for instruction with modrm where general register
 * encoding is allowed only in modrm.regmem (control register move).
 * */
#define     INV_MEM                     (1LU << 23)
#define     ANY_MEM                     (DISP8 | DISP16 | DISP32 | BASE_INDEX | INV_MEM)
    
#define     ACC                         (1LU << 25)
#define     PORT                        (1LU << 26)
#define     SHIFT_COUNT                 (1LU << 27)
#define     JUMP_ABSOLUTE               (1LU << 28)

#define     REG_REX                     (1LU << 29)
#define     REG_REX64                   (1LU << 30)
    
#define     IMPLICIT_REGISTER           (SHIFT_COUNT | ACC)

#define     REG_XMM                     (1LU << 31)
    
    flag_int cpu_flags;

#define     CPU_186                     (1LU << 0)
#define     CPU_286                     (1LU << 1)
#define     CPU_386                     (1LU << 2)
#define     CPU_486                     (1LU << 3)
#define     CPU_686                     (1LU << 4)

#define     CPU_8087                    (1LU << 7)
#define     CPU_287                     (1LU << 8)
#define     CPU_387                     (1LU << 9)
#define     CPU_687                     (1LU << 10)

#define     CPU_CMOV                    (1LU << 12)

#define     CPU_SSE                     (1LU << 13)
/* Preparation for future when those 2 will be separate. */
#define     CPU_SSE2                    CPU_SSE

#define     CPU_LONG_MODE               (1LU << 29)

#define     CPU_NO64                    (1LU << 30)
#define     CPU_64                      (1LU << 31)

};

struct reg_entry {

    const char *name;
    flag_int type;
    unsigned int number;

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
#define     WQ_SUF                      (NO_BSUF | NO_SSUF | NO_LSUF | NO_INTELSUF)
#define     SL_SUF                      (NO_BSUF | NO_WSUF | NO_QSUF | NO_INTELSUF)
#define     LQ_SUF                      (NO_BSUF | NO_WSUF | NO_SSUF | NO_INTELSUF)
#define     BWL_SUF                     (NO_SSUF | NO_QSUF | NO_INTELSUF)
#define     WLQ_SUF                     (NO_BSUF | NO_SSUF | NO_INTELSUF)
#define     BWLQ_SUF                    (NO_SSUF | NO_INTELSUF)

#define     CPU_FP                      (CPU_8087 | CPU_287 | CPU_387)  

/*
 * All instructions with the exact same name must be together without any other instructions mixed in
 * for the code searching the template table to work properly.
 * */
static const struct template template_table[] = {
    
#define OPCODE_D            0x2
#define OPCODE_FLOAT_D      0x400
#define OPCODE_SIMD_FLOAT_D 0x1
#define OPCODE_SIMD_INT_D   0x10

    /* Move instructions. */
    { "mov", 2, 0xA0, NONE, BWL_SUF | D | W, { DISP16 | DISP32, ACC, 0 }, 0 },
    { "mov", 2, 0x88, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "mov", 2, 0xB0, NONE, BWL_SUF | W | SHORT_FORM, { IMM8 | IMM16 | IMM32 | IMM32S, REG8 | REG16 | REG32, 0 }, 0 },
    { "mov", 2, 0xC6, NONE, BWLQ_SUF | D | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    /* Move instructions for segment registers. */
    { "mov", 2, 0x8C, NONE, WL_SUF | MODRM, { SEGMENT1, WORD_REG | INV_MEM, 0 }, 0 },
    { "mov", 2, 0x8C, NONE, W_SUF | MODRM | IGNORE_SIZE, { SEGMENT1, ANY_MEM, 0 }, 0 },
    { "mov", 2, 0x8C, NONE, WL_SUF | MODRM, { SEGMENT2, WORD_REG | INV_MEM, 0 }, CPU_386 },
    { "mov", 2, 0x8C, NONE, W_SUF | MODRM | IGNORE_SIZE, { SEGMENT2, ANY_MEM, 0 }, CPU_386 },
    { "mov", 2, 0x8E, NONE, WL_SUF | MODRM | IGNORE_SIZE, { WORD_REG | INV_MEM, SEGMENT1, 0 }, 0 },
    { "mov", 2, 0x8E, NONE, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, SEGMENT1, 0 }, 0 },
    { "mov", 2, 0x8E, NONE, WL_SUF | MODRM | IGNORE_SIZE, { WORD_REG | INV_MEM, SEGMENT2, 0 }, CPU_386 },
    { "mov", 2, 0x8E, NONE, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, SEGMENT2, 0 }, CPU_386 },
    
    /* Move instructions for control, debug and test registers. */
    { "mov", 2, 0x0F20, NONE, L_SUF | D | MODRM | IGNORE_SIZE, { CONTROL, REG32 | INV_MEM, 0 }, CPU_386 | CPU_NO64 },
    { "mov", 2, 0x0F20, NONE, Q_SUF | D | MODRM | NO_REX_W, { CONTROL, REG64 | INV_MEM, 0 }, CPU_64 },
    { "mov", 2, 0x0F21, NONE, L_SUF | D | MODRM | IGNORE_SIZE, { DEBUG, REG32 | INV_MEM, 0 }, CPU_386 | CPU_NO64 },
    { "mov", 2, 0x0F21, NONE, Q_SUF | D | MODRM | NO_REX_W, { DEBUG, REG64 | INV_MEM, 0 }, CPU_64 },
    { "mov", 2, 0x0F24, NONE, L_SUF | D | MODRM | IGNORE_SIZE, { TEST, REG32 | INV_MEM, 0 }, CPU_386 | CPU_NO64 },

    /* 64-bit only moves. */
    { "movabs", 2, 0xB8, NONE, Q_SUF | SHORT_FORM, { IMM64, REG64, 0 }, CPU_64 },
    
    /* Move with sign extend. */
    /* "movsbl" and "movsbw" are not unified into "movsb" to prevent conflict with "movs". */
    { "movsbl", 2, 0x0FBE, NONE, NO_SUF | MODRM, { REG8 | ANY_MEM, REG32, 0 }, CPU_386 },
    { "movsbw", 2, 0x0FBE, NONE, NO_SUF | MODRM, { REG8 | ANY_MEM, REG16, 0 }, CPU_386 },
    { "movswl", 2, 0x0FBF, NONE, NO_SUF | MODRM, { REG16 | ANY_MEM, REG32, 0 }, CPU_386 },
    { "movsbq", 2, 0x0FBE, NONE, NO_SUF | MODRM | SIZE64, { REG8 | ANY_MEM, REG64, 0 }, CPU_64 },
    { "movswq", 2, 0x0FBF, NONE, NO_SUF | MODRM | SIZE64, { REG16 | ANY_MEM, REG64, 0 }, CPU_64 },
    { "movslq", 2, 0x63, NONE, NO_SUF | MODRM | SIZE64, { REG32 | ANY_MEM, REG64, 0 }, CPU_64 },
    
    /* Alternative syntax. */
    { "movsx", 2, 0x0FBE, NONE, BW_SUF | W | MODRM, { REG8 | REG16 | ANY_MEM, WORD_REG, 0 }, CPU_386 },
    { "movsx", 2, 0x63, NONE, L_SUF | MODRM, { REG32 | ANY_MEM, REG32 | REG64, 0 }, CPU_64 },
    
    /* Move with zero extend. */
    { "movzb", 2, 0x0FB6, NONE, WLQ_SUF | MODRM, { REG8 | ANY_MEM, WORD_REG, 0 }, CPU_386 },
    { "movzw", 2, 0x0FB7, NONE, LQ_SUF | MODRM, { REG16 | ANY_MEM, REG32 | REG64, 0 }, CPU_386 },
    
    /* Alternative syntax. */
    { "movzx", 2, 0x0FB6, NONE, BW_SUF | W | MODRM, { REG8 | REG16 | ANY_MEM, WORD_REG, 0 }, CPU_386 },
    
    /* Push instructions. */
    { "push", 1, 0x50, NONE, WL_SUF | SHORT_FORM, { REG16 | REG32, 0, 0 }, CPU_NO64 },
    { "push", 1, 0xFF, 6, WL_SUF | DEFAULT_SIZE | MODRM, { REG16 | REG32 | ANY_MEM, 0, 0 }, CPU_NO64 },
    { "push", 1, 0x6A, NONE, WL_SUF | DEFAULT_SIZE, { IMM8S, 0, 0 }, CPU_186 | CPU_NO64 },
    { "push", 1, 0x68, NONE, WL_SUF | DEFAULT_SIZE, { IMM16 | IMM32, 0, 0 }, CPU_186 | CPU_NO64 },
    
    { "push", 1, 0x06, NONE, WL_SUF | DEFAULT_SIZE | SEGSHORTFORM, { SEGMENT1, 0, 0 }, CPU_NO64 },
    { "push", 1, 0x0FA0, NONE, WL_SUF | DEFAULT_SIZE | SEGSHORTFORM, { SEGMENT2, 0, 0 }, CPU_386 | CPU_NO64 },

    { "push", 1, 0x50, NONE, WQ_SUF | SHORT_FORM | NO_REX_W, { REG16 | REG64, 0, 0 }, CPU_64 },
    { "push", 1, 0xFF, 6, WQ_SUF | DEFAULT_SIZE | MODRM | NO_REX_W, { REG16 | REG64 | ANY_MEM, 0, 0 }, CPU_64 },
    { "push", 1, 0x6A, NONE, WQ_SUF | DEFAULT_SIZE | NO_REX_W, { IMM8S, 0, 0 }, CPU_64 },
    { "push", 1, 0x68, NONE, WQ_SUF | DEFAULT_SIZE | NO_REX_W, { IMM16 | IMM32, 0, 0 }, CPU_64 },

    { "push", 1, 0x0FA0, NONE, WQ_SUF | DEFAULT_SIZE | SEGSHORTFORM | NO_REX_W, { SEGMENT2, 0, 0 }, CPU_64 },
    
    { "pusha", 0, 0x60, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, CPU_186 | CPU_NO64 },
    
    /* Pop instructions. */
    { "pop", 1, 0x58, NONE, WL_SUF | SHORT_FORM, { REG16 | REG32, 0, 0 }, CPU_NO64 },
    { "pop", 1, 0x8F, 0, WL_SUF | DEFAULT_SIZE | MODRM, { REG16 | REG32 | ANY_MEM, 0, 0 }, CPU_NO64 },
    
#define     POP_SEGMENT_SHORT           0x07
    
    { "pop", 1, 0x07, NONE, WL_SUF | DEFAULT_SIZE | SEGSHORTFORM, { SEGMENT1, 0, 0 }, CPU_NO64 },
    { "pop", 1, 0x0FA1, NONE, WL_SUF | DEFAULT_SIZE | SEGSHORTFORM, { SEGMENT2, 0, 0 }, CPU_386 | CPU_NO64 },

    { "pop", 1, 0x58, NONE, WQ_SUF | SHORT_FORM | NO_REX_W, { REG16 | REG64, 0, 0 }, CPU_64 },
    { "pop", 1, 0x8F, 0, WQ_SUF | DEFAULT_SIZE | MODRM | NO_REX_W, { REG16 | REG64 | ANY_MEM, 0, 0 }, CPU_64 },

    { "pop", 1, 0x0FA1, NONE, WQ_SUF | DEFAULT_SIZE | SEGSHORTFORM | NO_REX_W, { SEGMENT2, 0, 0 }, CPU_64 },
    
    { "popa", 0, 0x61, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, CPU_186 | CPU_NO64 },
    
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
    { "lea", 2, 0x8D, NONE, WLQ_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 0 },
    
    /* Load far pointer from memory. */
    { "lds", 2, 0xC5, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 0 },
    { "les", 2, 0xC4, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, 0 },
    { "lfs", 2, 0x0FB4, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, CPU_386 },
    { "lgs", 2, 0x0FB5, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, CPU_386 },
    { "lss", 2, 0x0FB2, NONE, WL_SUF | MODRM, { ANY_MEM, WORD_REG, 0 }, CPU_386 },
    
    /* Flags register instructions. */
    { "cmc", 0, 0xF5, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "clc", 0, 0xF8, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "stc", 0, 0xF9, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "cli", 0, 0xFA, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "sti", 0, 0xFB, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "cld", 0, 0xFC, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "std", 0, 0xFD, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "clts", 0, 0x0F06, NONE, NO_SUF, { 0, 0, 0 }, CPU_286 },
    { "lahf", 0, 0x9F, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "sahf", 0, 0x9E, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "pushf", 0, 0x9C, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    { "popf", 0, 0x9D, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    
    /* Arithmetic instructions. */
    { "add", 2, 0x00, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "add", 2, 0x83, 0, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "add", 2, 0x04, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "add", 2, 0x80, 0, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "inc", 1, 0x40, NONE, WL_SUF | SHORT_FORM, { WORD_REG, 0, 0 }, CPU_NO64 },
    { "inc", 1, 0xFE, 0, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "sub", 2, 0x28, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "sub", 2, 0x83, 5, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "sub", 2, 0x2C, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "sub", 2, 0x80, 5, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "dec", 1, 0x48, NONE, WL_SUF | SHORT_FORM, { WORD_REG, 0, 0 }, CPU_NO64 },
    { "dec", 1, 0xFE, 1, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "sbb", 2, 0x18, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "sbb", 2, 0x83, 3, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "sbb", 2, 0x1C, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "sbb", 2, 0x80, 3, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "cmp", 2, 0x38, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "cmp", 2, 0x83, 7, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "cmp", 2, 0x3C, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "cmp", 2, 0x80, 7, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "test", 2, 0x84, NONE, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, REG, 0 }, 0 },
    { "test", 2, 0x84, NONE, BWLQ_SUF | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "test", 2, 0xA8, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "test", 2, 0xF6, 0, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "and", 2, 0x20, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "and", 2, 0x83, 4, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "and", 2, 0x24, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "and", 2, 0x80, 4, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "or", 2, 0x08, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "or", 2, 0x83, 1, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "or", 2, 0x0C, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "or", 2, 0x80, 1, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "xor", 2, 0x30, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "xor", 2, 0x83, 6, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "xor", 2, 0x34, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "xor", 2, 0x80, 6, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "clr", 1, 0x30, NONE, BWLQ_SUF | W | MODRM | REG_DUPLICATION, { REG, 0, 0 }, 0 },
    
    { "adc", 2, 0x10, NONE, BWLQ_SUF | D | W | MODRM, { REG, REG | ANY_MEM, 0 }, 0 },
    { "adc", 2, 0x83, 2, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, 0 }, 0 },
    { "adc", 2, 0x14, NONE, BWLQ_SUF | W, { IMM8 | IMM16 | IMM32 | IMM32S, ACC, 0 }, 0 },
    { "adc", 2, 0x80, 2, BWLQ_SUF | W | MODRM, { IMM8 | IMM16 | IMM32 | IMM32S, REG | ANY_MEM, 0 }, 0 },
    
    { "neg", 1, 0xF6, 3, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    { "not", 1, 0xF6, 2, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
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
    { "cdqe", 0, 0x98, NONE, NO_SUF | SIZE64, { 0, 0, 0 }, CPU_64 },
    { "cwd", 0, 0x99, NONE, NO_SUF | SIZE16, { 0, 0, 0 }, 0 },
    { "cdq", 0, 0x99, NONE, NO_SUF | SIZE32, { 0, 0, 0 }, CPU_386 },
    { "cqo", 0, 0x99, NONE, NO_SUF | SIZE64, { 0, 0, 0 }, CPU_64 },
    
    /* Other naming. */
    { "cbtw", 0, 0x98, NONE, NO_SUF | SIZE16, { 0, 0, 0 }, 0 },
    { "cwtl", 0, 0x98, NONE, NO_SUF | SIZE32, { 0, 0, 0 }, 0 },
    { "cltq", 0, 0x98, NONE, NO_SUF | SIZE64, { 0, 0, 0 }, CPU_64 },
    { "cwtd", 0, 0x99, NONE, NO_SUF | SIZE16, { 0, 0, 0 }, 0 },
    { "cltd", 0, 0x99, NONE, NO_SUF | SIZE32, { 0, 0, 0 }, CPU_386 },
    { "cqto", 0, 0x99, NONE, NO_SUF | SIZE64, { 0, 0, 0 }, CPU_64 },
    
    { "mul", 1, 0xF6, 4, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "imul", 1, 0xF6, 5, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    { "imul", 2, 0x0FAF, NONE, WLQ_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, CPU_386 },
    { "imul", 3, 0x6B, NONE, WLQ_SUF | MODRM, { IMM8S, WORD_REG | ANY_MEM, WORD_REG }, CPU_186 },
    { "imul", 3, 0x69, NONE, WLQ_SUF | MODRM, { IMM16 | IMM32 | IMM32S, WORD_REG | ANY_MEM, WORD_REG }, CPU_186 },
    { "imul", 2, 0x6B, NONE, WLQ_SUF | MODRM | REG_DUPLICATION, { IMM8S, WORD_REG, 0 }, CPU_186 },
    { "imul", 2, 0x69, NONE, WLQ_SUF | MODRM | REG_DUPLICATION, { IMM16 | IMM32 | IMM32S, WORD_REG, 0 }, CPU_186 },
    
    { "div", 1, 0xF6, 6, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    { "div", 2, 0xF6, 6, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, ACC, 0 }, 0 },
    
    { "idiv", 1, 0xF6, 7, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    { "idiv", 2, 0xF6, 7, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, ACC, 0 }, 0 },
    
    { "rol", 2, 0xC0, 0, BWLQ_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, CPU_186 },
    { "rol", 2, 0xD2, 0, BWLQ_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "rol", 1, 0xD0, 0, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "ror", 2, 0xC0, 1, BWLQ_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, CPU_186 },
    { "ror", 2, 0xD2, 1, BWLQ_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "ror", 1, 0xD0, 1, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "rcl", 2, 0xC0, 2, BWLQ_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, CPU_186 },
    { "rcl", 2, 0xD2, 2, BWLQ_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "rcl", 1, 0xD0, 2, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "rcr", 2, 0xC0, 3, BWLQ_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, CPU_186 },
    { "rcr", 2, 0xD2, 3, BWLQ_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "rcr", 1, 0xD0, 3, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "sal", 2, 0xC0, 4, BWLQ_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, CPU_186 },
    { "sal", 2, 0xD2, 4, BWLQ_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "sal", 1, 0xD0, 4, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "shl", 2, 0xC0, 4, BWLQ_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, CPU_186 },
    { "shl", 2, 0xD2, 4, BWLQ_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "shl", 1, 0xD0, 4, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "shr", 2, 0xC0, 5, BWLQ_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, CPU_186 },
    { "shr", 2, 0xD2, 5, BWLQ_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "shr", 1, 0xD0, 5, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "sar", 2, 0xC0, 7, BWLQ_SUF | W | MODRM, { IMM8, REG | ANY_MEM, 0 }, CPU_186 },
    { "sar", 2, 0xD2, 7, BWLQ_SUF | W | MODRM, { SHIFT_COUNT, REG | ANY_MEM, 0 }, 0 },
    { "sar", 1, 0xD0, 7, BWLQ_SUF | W | MODRM, { REG | ANY_MEM, 0, 0 }, 0 },
    
    { "shld", 3, 0x0FA4, NONE, WLQ_SUF | MODRM, { IMM8, WORD_REG, WORD_REG | ANY_MEM }, CPU_386 },
    { "shld", 3, 0x0FA5, NONE, WLQ_SUF | MODRM, { SHIFT_COUNT, WORD_REG, WORD_REG | ANY_MEM }, CPU_386 },
    { "shld", 2, 0x0FA5, NONE, WLQ_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    
    { "shrd", 3, 0x0FAC, NONE, WLQ_SUF | MODRM, { IMM8, WORD_REG, WORD_REG | ANY_MEM }, CPU_386 },
    { "shrd", 3, 0x0FAD, NONE, WLQ_SUF | MODRM, { SHIFT_COUNT, WORD_REG, WORD_REG | ANY_MEM }, CPU_386 },
    { "shrd", 2, 0x0FAD, NONE, WLQ_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    
    /* Program control transfer instructions. */
    { "call", 1, 0xE8, NONE, WL_SUF | DEFAULT_SIZE | CALL, { DISP16 | DISP32, 0, 0 }, CPU_NO64 },
    { "call", 1, 0xE8, NONE, WQ_SUF | DEFAULT_SIZE | CALL | NO_REX_W, { DISP16 | DISP32, 0, 0 }, CPU_64 },
    { "call", 1, 0xFF, 2, WL_SUF | DEFAULT_SIZE | MODRM, { REG16 | REG32 | ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, CPU_NO64 },
    { "call", 1, 0xFF, 2, WQ_SUF | DEFAULT_SIZE | MODRM | NO_REX_W, { REG16 | REG64 | ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, CPU_64 },
    { "call", 2, 0x9A, NONE, WL_SUF | DEFAULT_SIZE | JUMPINTERSEGMENT, { IMM16, IMM16 | IMM32, 0 }, CPU_NO64 },
    { "call", 1, 0xFF, 3, INTEL_SUF | DEFAULT_SIZE | MODRM, { ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    
    /* Alternative syntax. */
    { "lcall", 2, 0x9A, NONE, WL_SUF | DEFAULT_SIZE | JUMPINTERSEGMENT, { IMM16, IMM16 | IMM32, 0 }, CPU_NO64 },
    { "lcall", 1, 0xFF, 3, WL_SUF | DEFAULT_SIZE | MODRM, { ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    
#define     PC_RELATIVE_JUMP            0xEB
    
    { "jmp", 1, 0xEB, NONE, NO_SUF | JUMP, { DISP, 0, 0 }, 0 },
    { "jmp", 1, 0xFF, 4, WL_SUF | MODRM, { REG16 | REG32 | ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, CPU_NO64 },
    { "jmp", 1, 0xFF, 4, WQ_SUF | MODRM | NO_REX_W, { REG16 | REG64 | ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, CPU_64 },
    { "jmp", 2, 0xEA, NONE, WL_SUF | JUMPINTERSEGMENT, { IMM16, IMM16 | IMM32, 0 }, CPU_NO64 },
    { "jmp", 1, 0xFF, 5, INTEL_SUF | MODRM, { ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    
    /* Alternative syntax. */
    { "ljmp", 2, 0xEA, NONE, WL_SUF | JUMPINTERSEGMENT, { IMM16, IMM16 | IMM32, 0 }, 0 },
    { "ljmp", 1, 0xFF, 5, WL_SUF | MODRM, { ANY_MEM | JUMP_ABSOLUTE, 0, 0 }, 0 },
    
    { "ret", 0, 0xC3, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, CPU_NO64 },
    { "ret", 1, 0xC2, NONE, WL_SUF | DEFAULT_SIZE, { IMM16, 0, 0 }, CPU_NO64 },
    { "ret", 0, 0xC3, NONE, WQ_SUF | DEFAULT_SIZE | NO_REX_W, { 0, 0, 0 }, CPU_64 },
    { "ret", 1, 0xC2, NONE, WQ_SUF | DEFAULT_SIZE | NO_REX_W, { IMM16, 0, 0 }, CPU_64 },
    { "retf", 0, 0xCB, NONE, WLQ_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    { "retf", 1, 0xCA, NONE, WLQ_SUF | DEFAULT_SIZE, { IMM16, 0, 0 }, 0 },
    { "lret", 0, 0xCB, NONE, WLQ_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    { "lret", 1, 0xCA, NONE, WLQ_SUF | DEFAULT_SIZE, { IMM16, 0, 0 }, 0 },
    
    { "enter", 2, 0xC8, NONE, WL_SUF | DEFAULT_SIZE, { IMM16, IMM8, 0 }, CPU_186 | CPU_NO64 },
    { "enter", 2, 0xC8, NONE, WQ_SUF | DEFAULT_SIZE | NO_REX_W, { IMM16, IMM8, 0 }, CPU_64 },
    { "leave", 0, 0xC9, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, CPU_186 | CPU_NO64 },
    { "leave", 0, 0xC9, NONE, WQ_SUF | DEFAULT_SIZE | NO_REX_W, { 0, 0, 0 }, CPU_64 },
    
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
    
    { "jcxz", 1, 0xE3, NONE, NO_SUF | JUMPBYTE | SIZE16, { DISP, 0, 0 }, CPU_NO64 },
    { "jecxz", 1, 0xE3, NONE, NO_SUF | JUMPBYTE | SIZE32, { DISP, 0, 0 }, 0 },
    { "jrcxz", 1, 0xE3, NONE, NO_SUF | JUMPBYTE | SIZE64 | NO_REX_W, { DISP, 0, 0 }, CPU_64 },
    
    /* Loop instructions. */
    { "loop", 1, 0xE2, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    { "loopz", 1, 0xE1, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    { "loope", 1, 0xE1, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    { "loopnz", 1, 0xE0, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    { "loopne", 1, 0xE0, NONE, WL_SUF | JUMPBYTE, { DISP, 0, 0 }, 0 },
    
    /* Set byte on flag instructions. */
    { "seto", 1, 0x0F90, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setno", 1, 0x0F91, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setb", 1, 0x0F92, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setc", 1, 0x0F92, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnae", 1, 0x0F92, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnb", 1, 0x0F93, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnc", 1, 0x0F93, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setae", 1, 0x0F93, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "sete", 1, 0x0F94, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setz", 1, 0x0F94, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setne", 1, 0x0F95, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnz", 1, 0x0F95, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setbe", 1, 0x0F96, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setna", 1, 0x0F96, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnbe", 1, 0x0F97, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "seta", 1, 0x0F97, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "sets", 1, 0x0F98, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setns", 1, 0x0F99, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setp", 1, 0x0F9A, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setpe", 1, 0x0F9A, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnp", 1, 0x0F9B, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setpo", 1, 0x0F9B, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setl", 1, 0x0F9C, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnge", 1, 0x0F9C, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnl", 1, 0x0F9D, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setge", 1, 0x0F9D, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setle", 1, 0x0F9E, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setng", 1, 0x0F9E, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setnle", 1, 0x0F9F, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    { "setg", 1, 0x0F9F, 0, B_SUF | MODRM, { REG8 | ANY_MEM, 0, 0 }, CPU_386 },
    
    /* String manipulation instructions. */
    { "cmps", 0, 0xA6, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "scmp", 0, 0xA6, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, 0 },
    { "ins", 0, 0x6C, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, CPU_186 },
    { "outs", 0, 0x6E, NONE, BWL_SUF | W | IS_STRING, { 0, 0, 0 }, CPU_186 },
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
    { "bsf", 2, 0x0FBC, NONE, WLQ_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, CPU_386 },
    { "bsr", 2, 0x0FBD, NONE, WLQ_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, CPU_386 },
    { "bt", 2, 0x0FA3, NONE, WLQ_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    { "bt", 2, 0x0FBA, 4, WLQ_SUF | MODRM, { IMM8, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    { "btc", 2, 0x0FBB, NONE, WLQ_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    { "btc", 2, 0x0FBA, 7, WLQ_SUF | MODRM, { IMM8, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    { "btr", 2, 0x0FB3, NONE, WLQ_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    { "btr", 2, 0x0FBA, 6, WLQ_SUF | MODRM, { IMM8, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    { "bts", 2, 0x0FAB, NONE, WLQ_SUF | MODRM, { WORD_REG, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    { "bts", 2, 0x0FBA, 5, WLQ_SUF | MODRM, { IMM8, WORD_REG | ANY_MEM, 0 }, CPU_386 },
    
    /* Interrupts. */
#define     INT_OPCODE            0xCD
#define     INT3_OPCODE           0xCC
    { "int", 1, 0xCD, NONE, 0, { IMM8, 0, 0 }, 0 },
    { "int3", 0, 0xCC, NONE, 0, { 0, 0, 0 }, 0 },
    { "into", 0, 0xCE, NONE, 0, { 0, 0, 0 }, 0 },
    { "iret", 0, 0xCF, NONE, WL_SUF | DEFAULT_SIZE, { 0, 0, 0 }, 0 },
    
    { "rsm", 0, 0x0FAA, NONE, 0, { 0, 0, 0 }, CPU_386 },
    { "bound", 2, 0x62, NONE, WL_SUF | MODRM, { REG16 | REG32 , ANY_MEM, 0 }, CPU_186 | CPU_NO64 },
    
    { "hlt", 0, 0xF4, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    { "nop", 0, 0x90, NONE, NO_SUF, { 0, 0, 0 }, 0 },
    
    /* Protection control. */
    { "arpl", 2, 0x63, NONE, W_SUF | MODRM | IGNORE_SIZE, { REG16, REG16 | ANY_MEM, 0 }, CPU_286 | CPU_NO64 },
    { "lar", 2, 0x0F02, NONE, WL_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, CPU_286 },
    { "lgdt", 1, 0x0F01, 2, WL_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_286 },
    { "lidt", 1, 0x0F01, 3, WL_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_286 },
    { "lldt", 1, 0x0F00, 2, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, CPU_286 },
    { "lmsw", 1, 0x0F01, 6, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, CPU_286 },
    { "lsl", 2, 0x0F03, NONE, WL_SUF | MODRM, { WORD_REG | ANY_MEM, WORD_REG, 0 }, CPU_286 },
    { "ltr", 1, 0x0F00, 3, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, CPU_286 },
    
    { "sgdt", 1, 0x0F01, 0, WL_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_286 },
    { "sidt", 1, 0x0F01, 1, WL_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_286 },
    { "sldt", 1, 0x0F00, 0, WL_SUF | MODRM, { WORD_REG | INV_MEM, 0, 0 }, CPU_286 },
    { "sldt", 1, 0x0F00, 0, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, 0, 0 }, CPU_286 },
    { "smsw", 1, 0x0F01, 4, WL_SUF | MODRM, { WORD_REG | INV_MEM, 0, 0 }, CPU_286 },
    { "smsw", 1, 0x0F01, 4, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, 0, 0 }, CPU_286 },
    { "str", 1, 0x0F00, 1, WL_SUF | MODRM, { WORD_REG | INV_MEM, 0, 0 }, CPU_286 },
    { "str", 1, 0x0F00, 1, W_SUF | MODRM | IGNORE_SIZE, { ANY_MEM, 0, 0 }, CPU_286 },
    
    { "verr", 1, 0x0F00, 4, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, CPU_286 },
    { "verw", 1, 0x0F00, 5, W_SUF | MODRM | IGNORE_SIZE, { REG16 | ANY_MEM, 0, 0 }, CPU_286 },
    
    /* Floating point instructions. */
    /* Load. */
    { "fld", 1, 0xD9C0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fld", 1, 0xD9, 0, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fld", 1, 0xDB, 5, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fild", 1, 0xDF, 0, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fild", 1, 0xDF, 5, Q_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fildll", 1, 0xDF, 5, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fldt", 1, 0xDB, 5, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fbld", 1, 0xDF, 4, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    /* Store without pop. */
    { "fst", 1, 0xDDD0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fst", 1, 0xD9, 2, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fist", 1, 0xDF, 2, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    /* Store with pop. */
    { "fstp", 1, 0xDDD8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fstp", 1, 0xD9, 3, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fstp", 1, 0xDB, 7, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fistp", 1, 0xDF, 3, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fistp", 1, 0xDF, 7, Q_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fistpll", 1, 0xDF, 7, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fstpt", 1, 0xDB, 7, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fbstp", 1, 0xDF, 6, NO_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    /* Exchange of %st(0) with %st(x). */
    { "fxch", 1, 0xD9C8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    /* Exchange of %st(0) with %st(1). */
    { "fxch", 0, 0xD9C9, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Comparison without pop. */
    { "fcom", 1, 0xD8D0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fcom", 0, 0xD8D1, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fcom", 1, 0xD8, 2, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "ficom", 1, 0xDE, 2, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    /* Comparison with pop. */
    { "fcomp", 1, 0xD8D8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fcomp", 0, 0xD8D9, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fcomp", 1, 0xD8, 3, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "ficomp", 1, 0xDE, 3, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fcompp", 0, 0xDED9, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Unordered comparison. */
    { "fucom", 1, 0xDDE0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_387 },
    { "fucom", 0, 0xDDE1, NONE, NO_SUF, { 0, 0, 0 }, CPU_387 },
    { "fucomp", 1, 0xDDE8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_387 },
    { "fucomp", 0, 0xDDE9, NONE, NO_SUF, { 0, 0, 0 }, CPU_387 },
    { "fucompp", 0, 0xDAE9, NONE, NO_SUF, { 0, 0, 0 }, CPU_387 },
    
    { "ftst", 0, 0xD9E4, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fxam", 0, 0xD9E5, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Load constant. */
    { "fld1", 0, 0xD9E8, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fldl2t", 0, 0xD9E9, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fldl2e", 0, 0xD9EA, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fldpi", 0, 0xD9EB, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fldlg2", 0, 0xD9EC, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fldln2", 0, 0xD9ED, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fldz", 0, 0xD9EE, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Arithmetic floating point instructions. */
    /* Add. */
    { "fadd", 2, 0xD8C0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, CPU_FP },
    { "fadd", 1, 0xD8C0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fadd", 1, 0xD8, 0, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fiadd", 1, 0xDE, 0, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    { "faddp", 2, 0xDEC0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, CPU_FP },
    { "faddp", 1, 0xDEC0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "faddp", 0, 0xDEC1, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Subtract. */
    { "fsub", 2, 0xD8E0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, CPU_FP },
    { "fsub", 1, 0xD8E0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fsub", 1, 0xD8, 4, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fisub", 1, 0xDE, 4, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    { "fsubp", 2, 0xDEE0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, CPU_FP },
    { "fsubp", 1, 0xDEE0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fsubp", 0, 0xDEE1, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Subtract reverse. */
    { "fsubr", 2, 0xD8E8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, CPU_FP },
    { "fsubr", 1, 0xD8E8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fsubr", 1, 0xD8, 5, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fisubr", 1, 0xDE, 5, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    { "fsubrp", 2, 0xDEE8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, CPU_FP },
    { "fsubrp", 1, 0xDEE8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fsubrp", 0, 0xDEE9, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Multiply. */
    { "fmul", 2, 0xD8C8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, CPU_FP },
    { "fmul", 1, 0xD8C8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fmul", 1, 0xD8, 1, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fimul", 1, 0xDE, 1, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    { "fmulp", 2, 0xDEC8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, CPU_FP },
    { "fmulp", 1, 0xDEC8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fmulp", 0, 0xDEC9, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Divide. */
    { "fdiv", 2, 0xD8F0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, CPU_FP },
    { "fdiv", 1, 0xD8F0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fdiv", 1, 0xD8, 6, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fidiv", 1, 0xDE, 6, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    { "fdivp", 2, 0xDEF0, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, CPU_FP },
    { "fdivp", 1, 0xDEF0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fdivp", 0, 0xDEF1, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Divide reverse. */
    { "fdivr", 2, 0xD8F8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_REG, FLOAT_ACC, 0 }, CPU_FP },
    { "fdivr", 1, 0xD8F8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fdivr", 1, 0xD8, 7, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fidivr", 1, 0xDE, 7, SL_SUF | FLOAT_MF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    { "fdivrp", 2, 0xDEF8, NONE, NO_SUF | SHORT_FORM | FLOAT_D, { FLOAT_ACC, FLOAT_REG, 0 }, CPU_FP },
    { "fdivrp", 1, 0xDEF8, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "fdivrp", 0, 0xDEF9, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Other operations. */
    { "f2xm1", 0, 0xD9F0, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fyl2x", 0, 0xD9F1, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fptan", 0, 0xD9F2, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fpatan", 0, 0xD9F3, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fxtract", 0, 0xD9F4, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fprem1", 0, 0xD9F5, NONE, NO_SUF, { 0, 0, 0 }, CPU_387 },
    { "fdecstp", 0, 0xD9F6, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fincstp", 0, 0xD9F7, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fprem", 0, 0xD9F8, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fyl2xp1", 0, 0xD9F9, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fsqrt", 0, 0xD9FA, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fsincos", 0, 0xD9FB, NONE, NO_SUF, { 0, 0, 0 }, CPU_387 },
    { "frndint", 0, 0xD9FC, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fscale", 0, 0xD9FD, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fsin", 0, 0xD9FE, NONE, NO_SUF, { 0, 0, 0 }, CPU_387 },
    { "fcos", 0, 0xD9FF, NONE, NO_SUF, { 0, 0, 0 }, CPU_387 },
    { "fchs", 0, 0xD9E0, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fabs", 0, 0xD9E1, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Processor control instructions. */
    { "fninit", 0, 0xDBE3, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "finit", 0, 0xDBE3, NONE, NO_SUF | ADD_FWAIT, { 0, 0, 0 }, CPU_FP },
    { "fldcw", 1, 0xD9, 5, W_SUF | IGNORE_SIZE | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fnstcw", 1, 0xD9, 7, W_SUF | IGNORE_SIZE | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fstcw", 1, 0xD9, 7, W_SUF | IGNORE_SIZE | ADD_FWAIT | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    
    { "fnstsw", 1, 0xDFE0, NONE, NO_SUF | IGNORE_SIZE, { ACC, 0, 0 }, CPU_287 | CPU_387 },
    { "fnstsw", 1, 0xDD, 7, W_SUF | IGNORE_SIZE | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fnstsw", 0, 0xDFE0, NONE, NO_SUF, { 0, 0, 0 }, CPU_287 | CPU_387 },
    
    { "fstsw", 1, 0xDFE0, NONE, NO_SUF | ADD_FWAIT | IGNORE_SIZE, { ACC, 0, 0 }, CPU_287 | CPU_387 },
    { "fstsw", 1, 0xDD, 7, W_SUF | IGNORE_SIZE | ADD_FWAIT | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fstsw", 0, 0xDFE0, NONE, NO_SUF | ADD_FWAIT, { 0, 0, 0 }, CPU_287 | CPU_387 },
    
    { "fnclex", 0, 0xDBE2, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    { "fclex", 0, 0xDBE2, NONE, NO_SUF | ADD_FWAIT, { 0, 0, 0 }, CPU_FP },
    
    { "fnstenv", 1, 0xD9, 6, SL_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fstenv", 1, 0xD9, 6, SL_SUF | ADD_FWAIT | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fldenv", 1, 0xD9, 4, SL_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fnsave", 1, 0xDD, 6, SL_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "fsave", 1, 0xDD, 6, SL_SUF | ADD_FWAIT | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },
    { "frstor", 1, 0xDD, 4, SL_SUF | MODRM, { ANY_MEM, 0, 0 }, CPU_FP },

    /* 8087 only. */
    { "fneni", 0, 0xDBE0, NONE, NO_SUF, { 0, 0, 0 }, CPU_8087 },
    { "feni", 0, 0xDBE0, NONE, NO_SUF | ADD_FWAIT, { 0, 0, 0 }, CPU_8087 },
    { "fndisi", 0, 0xDBE1, NONE, NO_SUF, { 0, 0, 0 }, CPU_8087 },
    { "fdisi", 0, 0xDBE1, NONE, NO_SUF | ADD_FWAIT, { 0, 0, 0 }, CPU_8087 },
    
    { "ffree", 1, 0xDDC0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_FP },
    { "ffreep", 1, 0xDFC0, NONE, NO_SUF | SHORT_FORM, { FLOAT_REG, 0, 0 }, CPU_687 },
    { "fnop", 0, 0xD9D0, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
#define     FWAIT_OPCODE                0x9B
    
    { "fwait", 0, 0x9B, NONE, NO_SUF, { 0, 0, 0 }, CPU_FP },
    
    /* Opcode prefixes. They are allowed as separate instructions too. */
#define     ADDR_PREFIX_OPCODE          0x67
    
    { "addr16", 0, 0x67, NONE, NO_SUF | IS_PREFIX | SIZE16 | IGNORE_SIZE, { 0, 0, 0 }, CPU_386 },
    { "addr32", 0, 0x67, NONE, NO_SUF | IS_PREFIX | SIZE32 | IGNORE_SIZE, { 0, 0, 0 }, CPU_386 },
    { "aword", 0, 0x67, NONE, NO_SUF | IS_PREFIX | SIZE16 | IGNORE_SIZE, { 0, 0, 0 }, CPU_386 },
    { "adword", 0, 0x67, NONE, NO_SUF | IS_PREFIX | SIZE32 | IGNORE_SIZE, { 0, 0, 0 }, CPU_386 },
    
#define     DATA_PREFIX_OPCODE          0x66
    
    { "data16", 0, 0x66, NONE, NO_SUF | IS_PREFIX | SIZE16 | IGNORE_SIZE, { 0, 0, 0 }, CPU_386 },
    { "data32", 0, 0x66, NONE, NO_SUF | IS_PREFIX | SIZE32 | IGNORE_SIZE, { 0, 0, 0 }, CPU_386 },
    { "word", 0, 0x66, NONE, NO_SUF | IS_PREFIX | SIZE16 | IGNORE_SIZE, { 0, 0, 0 }, CPU_386 },
    { "dword", 0, 0x66, NONE, NO_SUF | IS_PREFIX | SIZE32 | IGNORE_SIZE, { 0, 0, 0 }, CPU_386 },
    
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

#define     REX_PREFIX_OPCODE         0x40
#define     REX_B                     0x1
#define     REX_X                     0x2
#define     REX_R                     0x4
#define     REX_W                     0x8

    { "rex.w", 0, 0x48, NONE, NO_SUF | IS_PREFIX, { 0, 0, 0 }, CPU_64 },

    /* i486 extensions. */
    { "bswap", 1, 0x0FC8, NONE, L_SUF | SHORT_FORM, { REG32, 0, 0 }, CPU_486 },

    /* Official undefined instruction. */
    { "ud2", 0, 0x0F0B, NONE, NO_SUF, { 0, 0, 0 }, CPU_186 },

    /* i686 extensions. */
#define CMOVcc(name, opcode) { name, 2, opcode, NONE, WLQ_SUF | MODRM, {REG16 | REG32 | REG64 | ANY_MEM, REG16 | REG32 | REG64, 0}, CPU_CMOV}
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

    { "movups", 2, 0x0F10, NONE, NO_SUF | D | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE },

    { "movaps", 2, 0x0F28, NONE, NO_SUF | D | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE },

    { "movss", 2, 0xF30F11, NONE, NO_SUF | D | MODRM | IGNORE_SIZE, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE },

    /* SSE2 instructions. */
    { "movapd", 2, 0x660F28, NONE, NO_SUF | D | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "movsd", 2, 0xF20F10, NONE, NO_SUF | D | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "comisd", 2, 0x660F2F, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },

    { "cvtsi2sd", 2, 0xF20F2A, NONE, LQ_SUF | IGNORE_SIZE | MODRM, { REG32 | REG64 | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 | CPU_64 },
    { "cvttsd2si", 2, 0xF20F2C, NONE, LQ_SUF | IGNORE_SIZE | MODRM, { REG_XMM | ANY_MEM, REG32 | REG64, 0 }, CPU_SSE2 },
    { "cvtsd2ss", 2, 0xF20F5A, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "cvtss2sd", 2, 0xF30F5A, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },

    { "movdqa", 2, 0x660F6F, NONE, NO_SUF | D | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "movdqu", 2, 0xF30F6F, NONE, NO_SUF | D | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    
    { "andpd", 2, 0x660F54, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "andnpd", 2, 0x660F55, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "orpd", 2, 0x660F56, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "addsd", 2, 0xF20F58, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "cmpltsd", 2, 0xF20FC2, 1, NO_SUF | MODRM | IMMEXT, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "mulsd", 2, 0xF20F59, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "subsd", 2, 0xF20F5C, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "divsd", 2, 0xF20F5E, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },

    { "ucomisd", 2, 0x660F2E, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "xorpd", 2, 0x660F57, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    
    { "pxor", 2, 0x660FEF, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },

    { "movq", 2, 0xF30F7E, NONE, NO_SUF | MODRM, { REG_XMM | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 },
    { "movq", 2, 0x660F6E, NONE, NO_SUF | D | MODRM | SIZE64, { REG64 | ANY_MEM, REG_XMM, 0 }, CPU_SSE2 | CPU_64 },
    /* Above movq conflicts with non-SSE mov with 'q' suffix. */
    { "movq", 2, 0x89, NONE, NO_SUF | D | MODRM | SIZE64, { REG64, REG64 | ANY_MEM, 0 }, CPU_64 },
    { "movq", 2, 0xB8, NONE, NO_SUF | SHORT_FORM | SIZE64, { IMM64, REG64, 0 }, CPU_64 },
    { "movq", 2, 0xC7, NONE, NO_SUF | D | MODRM | SIZE64, { IMM32S, REG64 | ANY_MEM, 0 }, CPU_64 },
    
    /* End of instructions. */
    { 0, 0, 0, 0, 0, { 0, 0, 0 }, 0 }

};

#define REG_FLAT_NUMBER ((unsigned int) ~0)
#define REG_IP_NUMBER ((unsigned int) ~0)

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
    { "axl", REG8 | REG_REX64, 0 },
    { "cxl", REG8 | REG_REX64, 1 },
    { "dxl", REG8 | REG_REX64, 2 },
    { "bxl", REG8 | REG_REX64, 3 },
    { "spl", REG8 | REG_REX64, 4 },
    { "bpl", REG8 | REG_REX64, 5 },
    { "sil", REG8 | REG_REX64, 6 },
    { "dil", REG8 | REG_REX64, 7 },
    { "r8b", REG8 | REG_REX | REG_REX64, 0 },
    { "r9b", REG8 | REG_REX | REG_REX64, 1 },
    { "r10b", REG8 | REG_REX | REG_REX64, 2 },
    { "r11b", REG8 | REG_REX | REG_REX64, 3 },
    { "r12b", REG8 | REG_REX | REG_REX64, 4 },
    { "r13b", REG8 | REG_REX | REG_REX64, 5 },
    { "r14b", REG8 | REG_REX | REG_REX64, 6 },
    { "r15b", REG8 | REG_REX | REG_REX64, 7 },

    /* 16 bit registers. */
    { "ax", REG16 | ACC, 0 },
    { "cx", REG16, 1 },
    { "dx", REG16 | PORT, 2 },
    { "bx", REG16 | BASE_INDEX, 3 },
    { "sp", REG16, 4 },
    { "bp", REG16 | BASE_INDEX, 5 },
    { "si", REG16 | BASE_INDEX, 6 },
    { "di", REG16 | BASE_INDEX, 7 },
    { "r8w", REG16 | REG_REX, 0 },
    { "r9w", REG16 | REG_REX, 1 },
    { "r10w", REG16 | REG_REX, 2 },
    { "r11w", REG16 | REG_REX, 3 },
    { "r12w", REG16 | REG_REX, 4 },
    { "r13w", REG16 | REG_REX, 5 },
    { "r14w", REG16 | REG_REX, 6 },
    { "r15w", REG16 | REG_REX, 7 },
    
    /* 32 bit registers. */
    { "eax", REG32 | BASE_INDEX | ACC, 0 },
    { "ecx", REG32 | BASE_INDEX, 1 },
    { "edx", REG32 | BASE_INDEX, 2 },
    { "ebx", REG32 | BASE_INDEX, 3 },
    { "esp", REG32, 4 },
    { "ebp", REG32 | BASE_INDEX, 5 },
    { "esi", REG32 | BASE_INDEX, 6 },
    { "edi", REG32 | BASE_INDEX, 7 },
    { "r8d", REG32 | BASE_INDEX | REG_REX, 0 },
    { "r9d", REG32 | BASE_INDEX | REG_REX, 1 },
    { "r10d", REG32 | BASE_INDEX | REG_REX, 2 },
    { "r11d", REG32 | BASE_INDEX | REG_REX, 3 },
    { "r12d", REG32 | BASE_INDEX | REG_REX, 4 },
    { "r13d", REG32 | BASE_INDEX | REG_REX, 5 },
    { "r14d", REG32 | BASE_INDEX | REG_REX, 6 },
    { "r15d", REG32 | BASE_INDEX | REG_REX, 7 },

    /* 64 bit registers. */
    { "rax", REG64 | BASE_INDEX | ACC, 0 },
    { "rcx", REG64 | BASE_INDEX, 1 },
    { "rdx", REG64 | BASE_INDEX, 2 },
    { "rbx", REG64 | BASE_INDEX, 3 },
    { "rsp", REG64, 4 },
    { "rbp", REG64 | BASE_INDEX, 5 },
    { "rsi", REG64 | BASE_INDEX, 6 },
    { "rdi", REG64 | BASE_INDEX, 7 },
    { "r8", REG64 | BASE_INDEX | REG_REX, 0 },
    { "r9", REG64 | BASE_INDEX | REG_REX, 1 },
    { "r10", REG64 | BASE_INDEX | REG_REX, 2 },
    { "r11", REG64 | BASE_INDEX | REG_REX, 3 },
    { "r12", REG64 | BASE_INDEX | REG_REX, 4 },
    { "r13", REG64 | BASE_INDEX | REG_REX, 5 },
    { "r14", REG64 | BASE_INDEX | REG_REX, 6 },
    { "r15", REG64 | BASE_INDEX | REG_REX, 7 },
    
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

    /* RIP is used only for RIP relative addressing.
     * (REG_REX64 only denies RIP use in 32-bit mode.)*/
    { "rip", REG_REX64, REG_IP_NUMBER },

    { "xmm0", REG_XMM, 0 },
    { "xmm1", REG_XMM, 1 },
    { "xmm2", REG_XMM, 2 },
    { "xmm3", REG_XMM, 3 },
    { "xmm4", REG_XMM, 4 },
    { "xmm5", REG_XMM, 5 },
    { "xmm6", REG_XMM, 6 },
    { "xmm7", REG_XMM, 7 },
    { "xmm8", REG_XMM | REG_REX, 0 },
    { "xmm9", REG_XMM | REG_REX, 1 },
    { "xmm10", REG_XMM | REG_REX, 2 },
    { "xmm11", REG_XMM | REG_REX, 3 },
    { "xmm12", REG_XMM | REG_REX, 4 },
    { "xmm13", REG_XMM | REG_REX, 5 },
    { "xmm14", REG_XMM | REG_REX, 6 },
    { "xmm15", REG_XMM | REG_REX, 7 },
    
    
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

struct cpu_arch_entry {

    const char *name;
    flag_int cpu_flags;

};

#define CPU_GENERIC32_FLAGS (CPU_186 | CPU_286 | CPU_386)
#define CPU_GENERIC64_FLAGS (CPU_I686_FLAGS | CPU_LONG_MODE | CPU_SSE)

#define CPU_I186_FLAGS (CPU_186)
#define CPU_I286_FLAGS (CPU_I186_FLAGS | CPU_286)
#define CPU_I386_FLAGS (CPU_I286_FLAGS | CPU_386)
/* i486 is the first CPU with a FPU integrated. */
#define CPU_I486_FLAGS (CPU_I386_FLAGS | CPU_486 | CPU_387)
#define CPU_I686_FLAGS (CPU_I486_FLAGS | CPU_686 | CPU_687 | CPU_CMOV)

static const struct cpu_arch_entry cpu_archs[] = {

    {"generic32", CPU_GENERIC32_FLAGS},
    {"generic64", CPU_GENERIC64_FLAGS},
    {"i8086", 0},
    {"i186", CPU_I186_FLAGS},
    {"i286", CPU_I286_FLAGS},
    {"i386", CPU_I386_FLAGS},
    {"i486", CPU_I486_FLAGS},
    {"i686", CPU_I686_FLAGS}

};

static const struct cpu_arch_entry cpu_extensions[] = {

    {"8087", CPU_8087},
    {"287", CPU_287},
    {"387", CPU_387},
    {"687", CPU_387 | CPU_687},
    {"cmov", CPU_CMOV},
    {"sse", CPU_SSE}

};

static const struct cpu_arch_entry cpu_no_extensions[] = {

    {"no87", CPU_8087 | CPU_287 | CPU_387 | CPU_687},
    {"no8087", CPU_8087},
    {"no287", CPU_287},
    {"no387", CPU_387},
    {"no687", CPU_687},
    {"nocmov", CPU_CMOV},
    {"nosse", CPU_SSE}

};
