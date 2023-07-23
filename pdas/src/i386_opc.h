/******************************************************************************
 * @file            i386_opc.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define MAX_OPERANDS 3

#define GENERATOR_MACRO(a) unsigned int a : 1;

#define GENERATOR_CPU_FLAGS_LIST \
 GENERATOR_MACRO(cpu_186) \
 GENERATOR_MACRO(cpu_286) \
 GENERATOR_MACRO(cpu_386) \
 GENERATOR_MACRO(cpu_486) \
 GENERATOR_MACRO(cpu_586) \
 GENERATOR_MACRO(cpu_686) \
 GENERATOR_MACRO(cpu_8087) \
 GENERATOR_MACRO(cpu_287) \
 GENERATOR_MACRO(cpu_387) \
 GENERATOR_MACRO(cpu_687) \
 GENERATOR_MACRO(cpu_cmov) \
 GENERATOR_MACRO(cpu_sse) \
 GENERATOR_MACRO(cpu_sse2) \
\
 GENERATOR_MACRO(cpu_long_mode) \
 GENERATOR_MACRO(cpu_no64) \
 GENERATOR_MACRO(cpu_64)

struct cpu_flags {
    
    GENERATOR_CPU_FLAGS_LIST
    
};

#define GENERATOR_OPCODE_MODIFIER_LIST \
 GENERATOR_MACRO(w) \
 GENERATOR_MACRO(d) \
 GENERATOR_MACRO(modrm) \
 GENERATOR_MACRO(short_form) \
 GENERATOR_MACRO(segshortform) \
 GENERATOR_MACRO(reg_duplication) \
\
 GENERATOR_MACRO(jump) \
 GENERATOR_MACRO(call) \
 GENERATOR_MACRO(jumpintersegment) \
 GENERATOR_MACRO(jumpbyte) \
\
 GENERATOR_MACRO(float_d) \
 GENERATOR_MACRO(float_mf) \
 GENERATOR_MACRO(add_fwait) \
\
 GENERATOR_MACRO(ignore_size) \
 GENERATOR_MACRO(default_size) \
 GENERATOR_MACRO(size16) \
 GENERATOR_MACRO(size32) \
 GENERATOR_MACRO(size64) \
\
 GENERATOR_MACRO(is_prefix) \
 GENERATOR_MACRO(is_string) \
\
 GENERATOR_MACRO(no_bsuf) \
 GENERATOR_MACRO(no_wsuf) \
 GENERATOR_MACRO(no_lsuf) \
 GENERATOR_MACRO(no_ssuf) \
 GENERATOR_MACRO(no_qsuf) \
 GENERATOR_MACRO(no_intelsuf) \
\
 GENERATOR_MACRO(no_rex_w) \
 GENERATOR_MACRO(immext)

struct opcode_modifier {

    GENERATOR_OPCODE_MODIFIER_LIST
    
};

#define GENERATOR_OPERAND_TYPE_LIST \
 GENERATOR_MACRO(reg8) \
 GENERATOR_MACRO(reg16) \
 GENERATOR_MACRO(reg32) \
 GENERATOR_MACRO(reg64)  \
 GENERATOR_MACRO(segment1) \
 GENERATOR_MACRO(segment2) \
 GENERATOR_MACRO(control) \
 GENERATOR_MACRO(test) \
 GENERATOR_MACRO(debug) \
 GENERATOR_MACRO(float_reg) \
 GENERATOR_MACRO(float_acc) \
\
 GENERATOR_MACRO(acc) \
 GENERATOR_MACRO(port) \
 GENERATOR_MACRO(shift_count) \
 GENERATOR_MACRO(jump_absolute) \
\
 GENERATOR_MACRO(reg_rex) \
 GENERATOR_MACRO(reg_rex64) \
 GENERATOR_MACRO(reg_xmm) \
\
 GENERATOR_MACRO(imm8) \
 GENERATOR_MACRO(imm8s) \
 GENERATOR_MACRO(imm16) \
 GENERATOR_MACRO(imm32) \
 GENERATOR_MACRO(imm32s) \
 GENERATOR_MACRO(imm64) \
\
 GENERATOR_MACRO(disp8) \
 GENERATOR_MACRO(disp16) \
 GENERATOR_MACRO(disp32) \
 GENERATOR_MACRO(disp32s) \
 GENERATOR_MACRO(disp64) \
\
 GENERATOR_MACRO(base_index) \
 GENERATOR_MACRO(inv_mem)

struct operand_type {
    
    GENERATOR_OPERAND_TYPE_LIST
    
};

#undef GENERATOR_MACRO

struct template {

    const char *name;
    unsigned long base_opcode;
    unsigned int extension_opcode;
#define NONE 0x0000FFFF
    
    unsigned int operands;

    struct cpu_flags cpu_flags;
    struct opcode_modifier opcode_modifier;
    struct operand_type operand_types[MAX_OPERANDS];

};

struct reg_entry {

    const char *name;
    struct operand_type type;
    unsigned int number;

};

#define REG_FLAT_NUMBER ((unsigned int) ~0)
#define REG_IP_NUMBER ((unsigned int) ~0)

