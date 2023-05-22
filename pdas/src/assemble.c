/******************************************************************************
 * @file            assemble.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include    <ctype.h>
#include    <stddef.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "as.h"
#include    "assemble.h"
#include    "hashtab.h"

#include    "options.h"

#define DEFAULT_CPU_ARCH_FLAGS (~0LU)
#define DEFAULT_CPU_ARCH_NAME "ALL"

static flag_int cpu_arch_flags = DEFAULT_CPU_ARCH_FLAGS;
static char *cpu_arch_name = NULL;
static char *cpu_extensions_name = NULL;

static int bits = 0;

/**
 * 0 for att syntax.
 * 1 for intel syntax.
 */
static int intel_syntax = 0;

/** 1 if the % prefix for register is not required. */
static int allow_no_prefix_reg = 0;

/** Table for lexical analysis. */
static char register_chars_table[256] = { 0 };

#define     RELAX_SUBTYPE_SHORT_JUMP                        0x00
#define     RELAX_SUBTYPE_CODE16_JUMP                       0x01
#define     RELAX_SUBTYPE_LONG_JUMP                         0x02

#define     RELAX_SUBTYPE_SHORT16_JUMP                      (RELAX_SUBTYPE_SHORT_JUMP | RELAX_SUBTYPE_CODE16_JUMP)
#define     RELAX_SUBTYPE_LONG16_JUMP                       (RELAX_SUBTYPE_LONG_JUMP | RELAX_SUBTYPE_CODE16_JUMP)

#define     RELAX_SUBTYPE_UNCONDITIONAL_JUMP                0x00
#define     RELAX_SUBTYPE_CONDITIONAL_JUMP                  0x01
#define     RELAX_SUBTYPE_CONDITIONAL_JUMP86                0x02
#define     RELAX_SUBTYPE_FORCED_SHORT_JUMP                 0x03

#define     ENCODE_RELAX_SUBTYPE(type, size)                (((type) << 2) | (size))
#define     TYPE_FROM_RELAX_SUBTYPE(subtype)                ((subtype) >> 2)

#define     DISPLACEMENT_SIZE_FROM_RELAX_SUBSTATE(s)        \
    (((s) & 3) == RELAX_SUBTYPE_LONG_JUMP ? 4 : (((s) & 3) == RELAX_SUBTYPE_LONG16_JUMP ? 2 : 1))

struct relax_table_entry {

    long forward_reach;
    long backward_reach;
    long size_of_variable_part;
    
    relax_subtype_t next_subtype;

};

struct relax_table_entry relax_table[] = {

    /* Unconditional jumps. */
    { 127 + 1,  -128 + 1,   1,      ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_UNCONDITIONAL_JUMP, RELAX_SUBTYPE_LONG_JUMP)    },
    { 127 + 1,  -128 + 1,   1,      ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_UNCONDITIONAL_JUMP, RELAX_SUBTYPE_LONG16_JUMP)  },
    { 0,        0,          4,      0                                                                                   },
    { 0,        0,          2,      0                                                                                   },
    
    /* Conditional jumps. */
    { 127 + 1,  -128 + 1,   1,      ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP, RELAX_SUBTYPE_LONG_JUMP)      },
    { 127 + 1,  -128 + 1,   1,      ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP, RELAX_SUBTYPE_LONG16_JUMP)    },
    { 0,        0,          5,      0                                                                                   },
    { 0,        0,          3,      0                                                                                   },
    
    /* Conditional jumps 86. */
    { 127 + 1,  -128 + 1,   1,      ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP86, RELAX_SUBTYPE_LONG_JUMP)    },
    { 127 + 1,  -128 + 1,   1,      ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP86, RELAX_SUBTYPE_LONG16_JUMP)  },
    { 0,        0,          5,      0                                                                                   },
    { 0,        0,          4,      0                                                                                   },
    
    /* Forced short jump that cannot be relaxed. */
    { 127 + 1,  -128 + 1,   1,      0  },

};

#define     TWOBYTE_OPCODE              0x0F

static const struct reg_entry *reg_esp;
static const struct reg_entry *reg_st0;

static const struct reg_entry bad_register = { "<bad>", 0, 0 };

struct instruction {

    struct template template;
    char suffix;
    
    int operands;
    int reg_operands;
    int disp_operands;
    int mem_operands;
    
    unsigned int types[MAX_OPERANDS];
    
    const struct reg_entry *regs[MAX_OPERANDS];
    struct expr *imms[MAX_OPERANDS];
    struct expr *disps[MAX_OPERANDS];
    
    struct modrm_byte modrm;
    struct sib_byte sib;
    
    const struct reg_entry *base_reg;
    const struct reg_entry *index_reg;
    
    unsigned int log2_scale_factor;

    unsigned char rex;
    
    unsigned int prefixes[MAX_PREFIXES];
    int prefix_count;
    
    const struct reg_entry *segments[MAX_OPERANDS];
    
    int force_short_jump;

};

/**
 * Groups together instruction templates with the same name for hash table search.
 * Templates start at start (included) and end at end (not included).
 */
struct templates {

    const char *name;
    const struct template *start;
    const struct template *end;

};

static struct instruction instruction;
static struct expr operand_exprs[MAX_OPERANDS];
static int operand_exprs_count;

static const struct templates *current_templates;

static struct hashtab *templates_hashtab;
static struct hashtab *reg_entry_hashtab;

static void set_bits (int new_bits, int cause_fatal_error);

/* Prototypes for functions from assemble_intel_support.c. */
static int intel_parse_name (struct expr *expr, char *name);
static int intel_parse_operand (char *operand_string);

static hash_value_t hash_templates (const void *p) {

    const struct templates *templates = (const struct templates *) p;
    return hashtab_help_default_hash_string (templates->name);

}

static int equal_templates (const void *p1, const void *p2) {

    const struct templates *templates1 = (const struct templates *) p1;
    const struct templates *templates2 = (const struct templates *) p2;
    
    return strcmp (templates1->name, templates2->name) == 0;

}

static const struct templates *find_templates (const char *name) {

    struct templates fake = { NULL, NULL, NULL };
    fake.name = name;
    
    return (const struct templates *) hashtab_find (templates_hashtab, &fake);

}

static hash_value_t hash_reg_entry (const void *p) {

    const struct reg_entry *reg_entry = (const struct reg_entry *) p;
    return hashtab_help_default_hash_string (reg_entry->name);

}

static int equal_reg_entry (const void *p1, const void *p2) {

    const struct reg_entry *reg_entry1 = (const struct reg_entry *) p1;
    const struct reg_entry *reg_entry2 = (const struct reg_entry *) p2;
       
    return strcmp (reg_entry1->name, reg_entry2->name) == 0;

}

static const struct reg_entry *find_reg_entry (const char *name) {

    struct reg_entry fake = { NULL, 0, 0 };
    fake.name = name;
    
    return (const struct reg_entry *) hashtab_find (reg_entry_hashtab, &fake);

}

void machine_dependent_init (void) {

    const struct template *template;
    const struct reg_entry *reg_entry;
    
    struct templates *templates;
    int c;
    
    templates_hashtab = hashtab_create_hashtab (0, hash_templates, equal_templates, &xmalloc, &free);
    
    if (templates_hashtab == NULL) {
        as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "error creating templates_hashtab");
    }
    
    template = template_table;
    templates = xmalloc (sizeof (*templates));
    templates->name = template->name;
    templates->start = template;
    
    while (1) {
    
        template++;
        
        if (template->name == NULL || strcmp (template->name, (template - 1)->name)) {
        
            templates->end = template;
            
            if ((templates = (struct templates *) hashtab_insert (templates_hashtab, templates))) {
                as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "error inserting '%s' into templates_hashtab", templates->name);
            }
            
            if (template->name == NULL) { break; }
            
            templates = xmalloc (sizeof (*templates));
            templates->name = template->name;
            templates->start = template;
        
        }
    
    }
    
    reg_entry_hashtab = hashtab_create_hashtab (0, hash_reg_entry, equal_reg_entry, &xmalloc, &free);
    
    if (reg_entry_hashtab == NULL) {
        as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "error creating reg_entry_hashtab");
    }
    
    for (reg_entry = reg_table; reg_entry->name; reg_entry++) {
    
        if (reg_entry->type & FLOAT_REG) {
        
            if (!(reg_entry->type & FLOAT_ACC)) {
                continue;
            }
            
            reg_st0 = reg_entry;
        
        }
        
        if ((reg_entry->type & REG32) && reg_entry->number == 4) {
            reg_esp = reg_entry;
        }
        
        if (hashtab_insert (reg_entry_hashtab, reg_entry)) {
            as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "error inserting into reg_entry_hashtab");
        }
    
    }
    
    /* Fills lexical table. */
    for (c = 0; c < 256; c++) {
    
        if (islower (c) || isdigit (c)) {
            register_chars_table[c] = c;
        } else if (isupper (c)) {
            register_chars_table[c] = tolower (c);
        }
    
    }

    if (bits == 0) {
        if (!(cpu_arch_flags & CPU_386)) bits = 16;
        else bits = 32;
    }

    set_bits (bits, 1);

    if (state->format == AS_FORMAT_COFF) coff_x86_set_bits (bits);

}

static void templates_destroy (void *p)
{
    free (p);
}

void machine_dependent_destroy (void)
{
    hashtab_destroy_hashtab (reg_entry_hashtab);
    hashtab_for_each_element (templates_hashtab, &templates_destroy); 
    hashtab_destroy_hashtab (templates_hashtab);
}

void machine_dependent_number_to_chars (unsigned char *p, unsigned long number, unsigned long size) {
    
    unsigned long i;
    
    for (i = 0; i < size; i++) {
        p[i] = (number >> (8 * i)) & 0xff;
    }

}

#define CPU_FLAGS_ARCH_MATCH (1U << 0)
#define CPU_FLAGS_64BIT_MATCH (1U << 1)
#define CPU_FLAGS_PERFECT_MATCH (CPU_FLAGS_ARCH_MATCH | CPU_FLAGS_64BIT_MATCH)

static int cpu_flags_match (const struct template *template)
{
    flag_int cpu_flags = template->cpu_flags;
    int match = CPU_FLAGS_64BIT_MATCH;

    if ((bits == 64 && (template->cpu_flags & CPU_NO64))
        || (bits != 64 && (template->cpu_flags & CPU_64))) match = 0;

    cpu_flags &= ~(CPU_NO64 | CPU_64);

    if (!cpu_flags || (cpu_flags & cpu_arch_flags)) match |= CPU_FLAGS_ARCH_MATCH;

    return match;
}

/**
 * Returns 0 when the new prefix is of the same type as already present prefixes,
 * 2 when REPE or REPNE prefix is added and 1 when other prefix is added.
 */
static int add_prefix (unsigned char prefix) {

    int ret = 1;
    unsigned int prefix_type;

    if (bits == 64
        && prefix >= REX_PREFIX_OPCODE
        && prefix < REX_PREFIX_OPCODE + 16) {
        
        prefix_type = REX_PREFIX;

        if (instruction.prefixes[prefix_type] & prefix & ~REX_PREFIX_OPCODE) ret = 0;
        
    } else {

        switch (prefix) {
        
            case CS_PREFIX_OPCODE:
            case DS_PREFIX_OPCODE:
            case ES_PREFIX_OPCODE:
            case FS_PREFIX_OPCODE:
            case GS_PREFIX_OPCODE:
            case SS_PREFIX_OPCODE:
            
                prefix_type = SEGMENT_PREFIX;
                break;
            
            case REPNE_PREFIX_OPCODE:
            case REPE_PREFIX_OPCODE:
            
                prefix_type = REP_PREFIX;
                
                ret = 2;
                break;
            
            case FWAIT_OPCODE:
            
                prefix_type = FWAIT_PREFIX;
                break;
            
            case ADDR_PREFIX_OPCODE:
            
                prefix_type = ADDR_PREFIX;
                break;
            
            case DATA_PREFIX_OPCODE:
            
                prefix_type = DATA_PREFIX;
                break;

            default:
            
                as_internal_error_at_source (__FILE__, __LINE__, "add_prefix invalid case %i", prefix);
                break;
        
        }
    
        if (instruction.prefixes[prefix_type]) ret = 0;

    }
    
    if (ret) {
    
        instruction.prefix_count++;
        instruction.prefixes[prefix_type] |= prefix;
    
    } else {
        as_error ("same type of prefix used twice");
    }
    
    return (ret);

}

static int check_reg (const struct reg_entry *reg) {

    if (!(cpu_arch_flags & CPU_386) && (reg->type & (REG32 | SEGMENT2 | CONTROL | DEBUG))) {
        return 0;
    }

    if ((reg->type & (REG64 | REG_REX)) && bits != 64) {
        return 0;
    }
    
    if ((reg->type & SEGMENT1) && reg->number == REG_FLAT_NUMBER && !intel_syntax) {
        return 0;
    }
    
    return 1;

}

/** reg_string points to the REGISTER_PREFIX. */
static const struct reg_entry *parse_register (const char *reg_string, char **end_pp) {

    const struct reg_entry *reg;
    char *p, *p_into_reg_name_cleaned;
    char reg_name_cleaned[MAX_REG_NAME_SIZE + 1];
    
    if (!(*reg_string == REGISTER_PREFIX || allow_no_prefix_reg)) {
        return NULL;
    }

    if (*reg_string == REGISTER_PREFIX) {
        reg_string++;
    }

    p = skip_whitespace ((char *) reg_string);

    for (p_into_reg_name_cleaned = reg_name_cleaned; (*(p_into_reg_name_cleaned++) = register_chars_table[(unsigned char) *p]) != '\0'; p++) {
    
        if (p_into_reg_name_cleaned >= reg_name_cleaned + MAX_REG_NAME_SIZE) {
            return NULL;
        }
    
    }
    
    /* If registers without prefix are allowed, it is necessary to check if the string
     * is really a register and not an identifier here
     * ("ax_var" must not be interpreted as register "ax" in this case).
     * This does not apply if prefixes are mandatory. */
    if (allow_no_prefix_reg && is_name_part ((int) *p)) {
        return NULL;
    }
    
    reg = find_reg_entry (reg_name_cleaned);
    
    *end_pp = p;
    
    if (reg == NULL) {
        return NULL;
    }
    
    if (reg == reg_st0) {
    
        if ((cpu_arch_flags & (CPU_8087 | CPU_287 | CPU_387)) == 0) {
            return NULL;
        }
        
        p = skip_whitespace (p);
        
        if (*p == '(') {
        
            p = skip_whitespace (++p);
            
            if  (*p >= '0' && *p <= '7') {
            
                reg = &(reg_st0[*p - '0']);
                
                p = skip_whitespace (++p);
                
                if (*p == ')') {
                
                    *end_pp = p + 1;
                    return reg;
                
                }
                
            }
            
            return NULL;
        
        }
    
    }       

    if (reg == NULL) {
        return NULL;
    } else if (check_reg (reg)) {
        return reg;
    }

    if (intel_syntax) {
        as_error ("register %s cannot be used here", reg->name);
    } else {
        as_error ("register %%%s cannot be used here", reg->name);
    }
    
    return &bad_register;

}

static int fits_in_signed_byte (long number) {
    return ((number >= -128) && (number <= 127));
}

static int fits_in_unsigned_byte (long number) {
    return ((number & 0xff) == number);
}

static int fits_in_signed_word (long number) {
    return ((number >= -32768) && (number <= 32767));
}

static int fits_in_unsigned_word (long number) {
    return ((number & 0xffff) == number);
}

static int base_index_check (char *operand_string) {

    if (bits == 64) {

        if ((instruction.base_reg && !(instruction.base_reg->type & REG64))
            || (instruction.index_reg && (!(instruction.index_reg->type & BASE_INDEX) || !(instruction.index_reg->type & REG64)))) {
            goto bad;
        }

    } else if (bits == 32) {
    
        if ((instruction.base_reg && !(instruction.base_reg->type & REG32))
            || (instruction.index_reg && (!(instruction.index_reg->type & BASE_INDEX) || !(instruction.index_reg->type & REG32)))) {
        bad:
        
            as_error ("'%s' is not a valid base/index expression", operand_string);
            return 1;
        
        }
    
    } else {
    
        if ((instruction.base_reg && (!(instruction.base_reg->type & BASE_INDEX) || !(instruction.base_reg->type & REG16)))
            || (instruction.index_reg && (!(instruction.index_reg->type & BASE_INDEX) || !(instruction.index_reg->type & REG16)
                || !(instruction.base_reg && instruction.base_reg->number < 6 && instruction.index_reg->number >= 6
                && instruction.log2_scale_factor == 0)))) {
            goto bad;
        }
    
    }
    
    return 0;

}

static int finalize_immediate (struct expr *expr, const char *imm_start) {

    if (expr->type == EXPR_TYPE_INVALID || expr->type == EXPR_TYPE_ABSENT) {
    
        if (imm_start) {
            as_error ("missing or invalid immediate expression '%s'", imm_start);
        }
        
        return 1;
    
    } else if (expr->type == EXPR_TYPE_CONSTANT) {
    
        /* Size will be determined later. */
        instruction.types[instruction.operands] |= IMM32;
    
    } else {
    
        /* It is an address and size will determined later. */
        instruction.types[instruction.operands] = IMM8 | IMM16 | IMM32;
    
    }
    
    return 0;

}

static int finalize_displacement (struct expr *expr, const char *disp_start) {

    if (expr->type == EXPR_TYPE_INVALID || expr->type == EXPR_TYPE_ABSENT) {
    
        if (disp_start) {
            as_error ("missing or invalid displacement expression '%s'", disp_start);
        }
        
        return 1;
    
    }
    
    return 0;

}

static int att_parse_operand (char *operand_string) {

    const struct reg_entry *reg;
    char *end_p;
    
    if (*operand_string == ABSOLUTE_PREFIX) {
    
        operand_string = skip_whitespace (operand_string + 1);
        instruction.types[instruction.operands] |= JUMP_ABSOLUTE;
    
    }
    
    if ((reg = parse_register (operand_string, &end_p)) != NULL) {

        if (reg == &bad_register) {
            return 1;
        }
        
        operand_string = skip_whitespace (end_p);
        
        if (*operand_string == ':' && (reg->type & (SEGMENT1 | SEGMENT2))) {
        
            instruction.segments[instruction.operands] = reg;
            
            operand_string = skip_whitespace (++operand_string);
            
            if (*operand_string == ABSOLUTE_PREFIX) {
            
                operand_string = skip_whitespace (++operand_string);
                instruction.types[instruction.operands] |= JUMP_ABSOLUTE;
            
            }
            
            goto do_memory_reference;
        
        }
        
        if (*operand_string) {

            as_error ("junk '%s' after register", operand_string);
            return 1;

        }
        
        instruction.types[instruction.operands] |= reg->type & ~BASE_INDEX;
        instruction.regs[instruction.operands] = reg;
        
        instruction.operands++;
        instruction.reg_operands++;
    
    } else if (*operand_string == REGISTER_PREFIX) {
    
        as_error ("bad register name '%s'", operand_string);
        return 1;
    
    } else if (*operand_string == IMMEDIATE_PREFIX) {
    
        struct expr *expr;
        section_t section_of_expr;
        
        char *imm_start;
        int ret;
        
        operand_string++;
        
        if (instruction.types[instruction.operands] & JUMP_ABSOLUTE) {
        
            as_error ("immediate operand is not allowed with absolute jump");
            return 1;
        
        }
        
        expr = &operand_exprs[operand_exprs_count++];
        instruction.imms[instruction.operands] = expr;
        
        imm_start = operand_string;
        section_of_expr = expression_read_into (&operand_string, expr);
        
        operand_string = skip_whitespace (operand_string);
        
        if (*operand_string) {
        
            as_error ("junk '%s' after immediate expression", operand_string);
            return 1;
        
        }
        
        if (section_of_expr == reg_section) {
        
            as_error ("illegal immediate register operand %s", imm_start);
            return 1;
        
        }
        
        ret = finalize_immediate (expr, imm_start);
        instruction.operands++;
        
        return ret;
    
    } else {
    
        char *base_string, *displacement_string_end, *p2;
        int ret;
        
    do_memory_reference:
        
        ret = 0;
        p2 = operand_string + strlen (operand_string);

        if (p2 - operand_string > 1 && p2[-1] == ' ') {
            p2--;
        }
        
        base_string = p2 - 1;
        displacement_string_end = p2;
        
        if (*base_string == ')') {
        
            unsigned int paren_not_balanced = 1;
            char *temp_string;
            
            do {
            
                base_string--;
                
                if (*base_string == ')') {
                    paren_not_balanced++;
                } else if (*base_string == '(') {
                    paren_not_balanced--;
                }
            
            } while (paren_not_balanced);
            
            temp_string = base_string;
            base_string = skip_whitespace (base_string + 1);
            
            if (*base_string == ',' || (instruction.base_reg = parse_register (base_string, &end_p)) != NULL) {
            
                displacement_string_end = temp_string;
                instruction.types[instruction.operands] = BASE_INDEX;
                
                if (instruction.base_reg) {

                    if (reg == &bad_register) {
                        return 1;
                    }
                    
                    base_string = skip_whitespace (end_p);
                }
                
                /* Possible index or scale factor.*/
                if (*base_string == ',') {
                
                    base_string = skip_whitespace (base_string + 1);
                    
                    if ((instruction.index_reg = parse_register (base_string, &end_p)) != NULL) {

                        if (reg == &bad_register) {
                            return 1;
                        }
                        
                        base_string = skip_whitespace (end_p);
                        
                        if (*base_string == ',') {
                            base_string = skip_whitespace (base_string + 1);
                        } else if (*base_string != ')') {
                        
                            as_error ("expecting ',' or ')' after index register in '%s'", operand_string);
                            return 1;
                        
                        }
                    
                    } else if (*base_string == REGISTER_PREFIX) {
                    
                        end_p = strchr (base_string, ',');
                        
                        if (end_p) {
                            *end_p = '\0';
                        }
                        
                        as_error ("bad register name '%s'", base_string);
                        return 1;
                    
                    }
                    
                    if (*base_string != ')') {
                    
                        char *saved = base_string;
                        offset_t value = get_result_of_absolute_expression (&base_string);
                        
                        switch (value) {
                        
                            case 1:
                            
                                instruction.log2_scale_factor = 0;
                                break;
                            
                            case 2:
                            
                                instruction.log2_scale_factor = 1;
                                break;
                            
                            case 4:
                            
                                instruction.log2_scale_factor = 2;
                                break;
                            
                            case 8:
                            
                                instruction.log2_scale_factor = 3;
                                break;
                            
                            default:
                            
                                *base_string = '\0';
                                
                                as_error ("expecting scale factor of 1, 2, 4 or 8 but got '%s'", saved);
                                return 1;
                        
                        }
                        
                        if (instruction.log2_scale_factor && instruction.index_reg == NULL) {
                        
                            as_warn ("scale factor of %i without an index register", 1 << instruction.log2_scale_factor);
                            instruction.log2_scale_factor = 0;
                        
                        }
                        
                        base_string = skip_whitespace (base_string);
                        
                        if (*base_string != ')') {
                        
                            as_error ("expecting ')' after scale factor in '%s'", operand_string);
                            return 1;
                        
                        }
                    
                    } else if (instruction.index_reg == NULL) {
                    
                        as_error ("expecting index register or scale factor after ',' but got '%c'", *base_string);
                        return 1;
                    
                    }
                
                } else if (*base_string != ')') {
                
                    as_error ("expecting ',' or ')' after base register in '%s'", operand_string);
                    return 1;
                
                }
            
            } else if (*base_string == REGISTER_PREFIX) {
            
                end_p = strchr (base_string, ',');
                
                if (end_p) {
                    *end_p = '\0';
                }
                
                as_error ("bad register name '%s'", base_string);
                return 1;
            
            }
        
        }
        
        if (base_index_check (operand_string)) {
            return 1;
        }
        
        if (operand_string != displacement_string_end) {
        
            struct expr *expr;
            unsigned int disp_type;
            
            if (bits == 32) {
                disp_type = DISP32;
            } else {
                disp_type = DISP16;
            }
            
            expr = &operand_exprs[operand_exprs_count++];
            
            {
            
                char saved_ch;
                char *disp_start;
                
                disp_start = operand_string;
                
                saved_ch = *displacement_string_end;
                *displacement_string_end = '\0';
                
                expression_read_into (&operand_string, expr);
                
                if (*skip_whitespace (operand_string)) {
                    as_error ("junk '%s' after displacement expression", operand_string);
                }
                
                ret = finalize_displacement (expr, disp_start);
                
                *displacement_string_end = saved_ch;
            
            }
            
            instruction.types[instruction.operands] |= disp_type;
            instruction.disps[instruction.operands] = expr;
            
            instruction.disp_operands++;
        
        }
        
        instruction.operands++;
        instruction.mem_operands++;
        
        return ret;
    
    }
    
    return 0;

}

unsigned int smallest_imm_type (long number) {

    if (fits_in_signed_byte (number)) {
        return (IMM8S | IMM8 | IMM16 | IMM32);
    }
    
    if (fits_in_unsigned_byte (number)) {
        return (IMM8 | IMM16 | IMM32);
    }
    
    if (fits_in_signed_word (number) || fits_in_unsigned_word (number)) {
        return (IMM16 | IMM32);
    }
    
    return IMM32;

}

static void optimize_size_of_disps (void) {

    int operand;
    
    for (operand = 0; operand < instruction.operands; operand++) {
    
        if (instruction.types[operand] & DISP) {
        
            if (instruction.disps[operand]->type == EXPR_TYPE_CONSTANT) {
            
                unsigned long disp = instruction.disps[operand]->add_number;
                
                if (instruction.types[operand] & DISP32) {
                
                    disp &= 0xffffffff;
                    disp = (disp ^ (1UL << 31)) - (1UL << 31);
                
                }
                
                if ((instruction.types[operand] & (DISP16 | DISP32)) && fits_in_signed_byte (disp)) {
                    instruction.types[operand] |= DISP8;
                }
            
            }
        
        }
    
    }

}

static void optimize_size_of_imms (void) {

    char guessed_suffix = 0;
    int operand;
    
    if (instruction.suffix) {
        guessed_suffix = instruction.suffix;
    } else if (instruction.reg_operands) {
    
        /**
         * Guesses a suffix from the last register operand
         * what is good enough for shortening immediates
         * but the real suffix cannot be set yet.
         * Example: mov $1234, %al
         */
        for (operand = instruction.operands; --operand >= 0; ) {
        
            if (instruction.types[operand] & REG) {
            
                guessed_suffix = ((instruction.types[operand] & REG8) ? BYTE_SUFFIX : (instruction.types[operand] & REG16) ? WORD_SUFFIX : DWORD_SUFFIX);
                break;
            
            }
        
        }
    
    } else if ((bits == 16) ^ (instruction.prefixes[DATA_PREFIX] != 0)) {
    
        /**
         * Immediate shortening for 16 bit code.
         * Example: .code16\n push $12341234
         */
        guessed_suffix = WORD_SUFFIX;
    
    }
    
    for (operand = 0; operand < instruction.operands; operand++) {
    
        if (instruction.types[operand] & IMM) {
        
            if (instruction.imms[operand]->type == EXPR_TYPE_CONSTANT) {
            
                /* If a suffix is given, it is allowed to shorten the immediate. */
                switch (guessed_suffix) {
                
                    case BYTE_SUFFIX:
                    
                        instruction.types[operand] |= IMM8 | IMM8S | IMM16 | IMM32;
                        break;
                    
                    case WORD_SUFFIX:
                    
                        instruction.types[operand] |= IMM16 | IMM32;
                        break;
                    
                    case DWORD_SUFFIX:
                    
                        instruction.types[operand] |= IMM32;
                        break;
                
                }
                
                if (instruction.types[0] & IMM32) {
                
                    instruction.imms[operand]->add_number &= 0xffffffff;
                    instruction.imms[operand]->add_number = ((instruction.imms[operand]->add_number ^ (1UL << 31)) - (1UL << 31));
                
                }
                
                instruction.types[operand] |= smallest_imm_type (instruction.imms[operand]->add_number);
            
            }
        
        }
    
    }

}

static unsigned int modrm_mode_from_disp_size (unsigned int type) {
    return ((type & DISP8) ? 1 : ((type & (DISP16 | DISP32)) ? 2 : 0));
}

static long convert_number_to_size (unsigned long value, int size) {

    unsigned long mask;
    
    switch (size) {
    
        case 1:
        
            mask = 0xff;
            break;
        
        case 2:
        
            mask = 0xffff;
            break;
        
        case 4:
        
            mask = 0xffffffff;
            break;

        default:
        
            as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "convert_number_to_size invalid case %i", size);
            break;
    
    }
    
    if ((value & ~mask) && ((value & ~mask) != ~mask)) {
        as_warn ("%ld shortened to %ld", value, value & mask);
    }
    
    value &= mask;
    return value;

}

static void output_disps (void) {

    int operand;
    
    for (operand = 0; operand < instruction.operands; operand++) {
    
        if (instruction.types[operand] & DISP) {
        
            if (instruction.disps[operand]->type == EXPR_TYPE_CONSTANT) {
            
                int size = 4;
                unsigned long value;
                
                if (instruction.types[operand] & DISP8) {
                    size = 1;
                } else if (instruction.types[operand] & DISP16) {
                    size = 2;
                }
                
                value = convert_number_to_size (instruction.disps[operand]->add_number, size);
                machine_dependent_number_to_chars (frag_increase_fixed_size (size), value, size);
            
            } else {
            
                int size = 4;
                
                if (instruction.types[operand] & DISP8) {
                    size = 1;
                } else if (instruction.types[operand] & DISP16) {
                    size = 2;
                }
                
                fixup_new_expr (current_frag, current_frag->fixed_size, size, instruction.disps[operand], 0, RELOC_TYPE_DEFAULT);
                frag_increase_fixed_size (size);
            
            }
        
        }
    
    }

}

static void output_imm (unsigned int operand) {

    if (instruction.types[operand] & IMM) {
    
        if (instruction.imms[operand]->type == EXPR_TYPE_CONSTANT) {
        
            int size = 4;
            unsigned long value;
            
            if (instruction.types[operand] & (IMM8 | IMM8S)) {
                size = 1;
            } else if (instruction.types[operand] & IMM16) {
                size = 2;
            }
            
            value = convert_number_to_size (instruction.imms[operand]->add_number, size);
            machine_dependent_number_to_chars (frag_increase_fixed_size (size), value, size);
        
        } else {
        
            int size = 4;
            
            if (instruction.types[operand] & (IMM8 | IMM8S)) {
                size = 1;
            } else if (instruction.types[operand] & IMM16) {
                size = 2;
            }
            
            fixup_new_expr (current_frag, current_frag->fixed_size, size, instruction.imms[operand], 0, RELOC_TYPE_DEFAULT);
            frag_increase_fixed_size (size);
        
        }
    
    }

}

static void output_imms (void) {

    int operand;
    
    for (operand = 0; operand < instruction.operands; operand++) {
        output_imm (operand);
    }

}

static int check_byte_reg (void) {

    int op;
    
    for (op = instruction.operands; --op >= 0; ) {
    
        if (!(instruction.types[op] & REG)) continue;
        
        if (instruction.types[op] & REG8) continue;
        
        if ((instruction.template.operand_types[op] & PORT) && (instruction.types[op] & WORD_REG)) continue;
        
        as_error ("'%%%s' not allowed with '%s%c'", instruction.regs[op]->name, instruction.template.name, instruction.suffix);
        return 1;
    
    }
    
    return 0;

}

static int check_word_reg (void) {

    int op;
    
    for (op = instruction.operands; --op >= 0; ) {

        if (!(instruction.types[op] & REG)) continue;
    
        if ((instruction.types[op] & REG8) && (instruction.template.operand_types[op] & (REG16 | REG32 | ACC))) {
        
            as_error ("'%%%s' not allowed with '%s%c'", instruction.regs[op]->name, instruction.template.name, instruction.suffix);
            return 1;
        
        }
        
        if ((instruction.types[op] & (REG32 | REG64)) && (instruction.template.operand_types[op] & (REG16 | ACC))) {
            as_error ("incorrect register '%%%s' used with `%c' suffix", instruction.regs[op]->name, instruction.suffix);
            return 1;
        }
    
    }
    
    return 0;

}

static int check_dword_reg (void) {

    int op;
    
    for (op = instruction.operands; --op >= 0; ) {

        if (!(instruction.types[op] & REG)) continue;
    
        if ((instruction.types[op] & REG8) && (instruction.template.operand_types[op] & (REG16 | REG32 | ACC))) {
        
            as_error ("'%%%s' not allowed with '%s%c'", instruction.regs[op]->name, instruction.template.name, instruction.suffix);
            return 1;
        
        }
        
        if ((instruction.types[op] & (REG16 | REG64)) && (instruction.template.operand_types[op] & (REG32 | ACC))) {
            as_error ("incorrect register '%%%s' used with `%c' suffix", instruction.regs[op]->name, instruction.suffix);
            return 1;
        }
    
    }
    
    return 0;

}

static int check_qword_reg (void) {

    int op;
    
    for (op = instruction.operands; --op >= 0; ) {

        if (!(instruction.types[op] & REG)) continue;
    
        if ((instruction.types[op] & REG8) && (instruction.template.operand_types[op] & (REG16 | REG32 | ACC))) {
        
            as_error ("'%%%s' not allowed with '%s%c'", instruction.regs[op]->name, instruction.template.name, instruction.suffix);
            return 1;
        
        }
        
        if ((instruction.types[op] & (REG16 | REG32)) && (instruction.template.operand_types[op] & (REG64 | ACC))) {
            as_error ("incorrect register '%%%s' used with `%c' suffix", instruction.regs[op]->name, instruction.suffix);
            return 1;
        }
    
    }
    
    return 0;

}

static int process_suffix (void) {

    int is_movsx_or_movzx = 0;
    
    if (instruction.template.opcode_modifier & (SIZE16 | SIZE32)) {
    
        if (instruction.template.opcode_modifier & SIZE16) {
            instruction.suffix = WORD_SUFFIX;
        } else {
            instruction.suffix = DWORD_SUFFIX;
        }
    
    } else if (instruction.reg_operands
               && (instruction.operands > 1 || (instruction.types[0] & REG))) {
    
        int saved_operands = instruction.operands;
        
        is_movsx_or_movzx = ((instruction.template.base_opcode & 0xFF00) == 0x0F00
            && ((instruction.template.base_opcode & 0xFF) | 8) == 0xBE);
        
        /* For movsx/movzx only the source operand is considered for the ambiguity checking.
         * The suffix is replaced to represent the destination later. */
        if (is_movsx_or_movzx && (instruction.template.opcode_modifier & W)) {
            instruction.operands--;
        }
        
        if (!instruction.suffix) {
        
            int op;

            for (op = instruction.operands; --op >= 0; ) {
            
                if ((instruction.types[op] & REG)
                    && !(instruction.template.operand_types[op] & SHIFT_COUNT)
                    && !(instruction.template.operand_types[op] & PORT)) {
                
                    instruction.suffix = ((instruction.types[op] & REG8) ? BYTE_SUFFIX
                                          : (instruction.types[op] & REG16) ? WORD_SUFFIX
                                          : (instruction.types[op] & REG32) ? DWORD_SUFFIX
                                          : QWORD_SUFFIX);
                    break;
                
                }
            
            }
            
            /* When .att_syntax is used, movsx and movzx silently default to byte memory source. */
            if (is_movsx_or_movzx && (instruction.template.opcode_modifier & W) && !instruction.suffix && !intel_syntax) {
                instruction.suffix = BYTE_SUFFIX;
            }
        
        } else {
        
            int ret;
            
            switch (instruction.suffix) {
            
                case BYTE_SUFFIX:
                
                    ret = check_byte_reg ();
                    break;
                
                case WORD_SUFFIX:
                
                    ret = check_word_reg ();
                    break;
                
                case DWORD_SUFFIX:
                
                    ret = check_dword_reg ();
                    break;

                case QWORD_SUFFIX:
                
                    ret = check_qword_reg ();
                    break;

                default:
                
                    as_internal_error_at_source (__FILE__, __LINE__, "process_suffix invalid case %i", instruction.suffix);
                    break;
            
            }
            
            if (ret) { return 1; }
        
        }
        
        /* Undoes the movsx/movzx change done above. */
        instruction.operands = saved_operands;
    
    } else if ((instruction.template.opcode_modifier & DEFAULT_SIZE) && !instruction.suffix) {
        instruction.suffix = (bits == 64) ? QWORD_SUFFIX : (bits == 32) ? DWORD_SUFFIX : WORD_SUFFIX;
    } else if (!instruction.suffix
               && ((instruction.template.operand_types[0] & JUMP_ABSOLUTE)
                   || (instruction.template.opcode_modifier & JUMPBYTE)
                   || (instruction.template.opcode_modifier & JUMPINTERSEGMENT)
                   /* lgdt, lidt, sgdt, sidt */
                   || ((instruction.template.base_opcode == 0x0F01
                        && instruction.template.extension_opcode <= 3)))) {

        if (bits == 32) {
            
            if (!(instruction.template.opcode_modifier & NO_LSUF)) {
                instruction.suffix = DWORD_SUFFIX;
            }
            
        } else {

            if (!(instruction.template.opcode_modifier & NO_WSUF)) {
                instruction.suffix = WORD_SUFFIX;
            }

        }

    }
                                          
    if (!instruction.suffix
        && !(instruction.template.opcode_modifier & IGNORE_SIZE)
        && !(instruction.template.opcode_modifier & DEFAULT_SIZE)
        /* Explicit data size prefix allows determining the size. */
        && !instruction.prefixes[DATA_PREFIX]
        /* fldenv and similar instructions do not require a suffix. */
        && ((instruction.template.opcode_modifier & NO_SSUF)
            || (instruction.template.opcode_modifier & FLOAT_MF))) {

        int suffixes;

        suffixes = !(instruction.template.opcode_modifier & NO_BSUF);
        if (!(instruction.template.opcode_modifier & NO_WSUF)) {
            suffixes |= 1 << 1;
        }
        if (!(instruction.template.opcode_modifier & NO_SSUF)) {
            suffixes |= 1 << 2;
        }
        if (!(instruction.template.opcode_modifier & NO_LSUF)) {
            suffixes |= 1 << 3;
        }
        if (!(instruction.template.opcode_modifier & NO_INTELSUF)) {
            suffixes |= 1 << 4;
        }

        if (suffixes & (suffixes - 1)) {
            
            if (intel_syntax) {
                
                as_error ("ambiguous operand size for '%s'", instruction.template.name);
                return 1;
                
            }

            as_warn ("%s, using default for '%s'",
                     intel_syntax
                     ? "ambiguous operand size"
                     : "no instruction mnemonic suffix given and no register operands",
                     instruction.template.name);

            if (instruction.template.opcode_modifier & FLOAT_MF) {
                instruction.suffix = SHORT_SUFFIX;
            } else if (is_movsx_or_movzx) {
                /* Handled below. */
            } else if (bits == 16) {
                instruction.suffix = WORD_SUFFIX;
            } else if (!(instruction.template.opcode_modifier & NO_LSUF)) {
                instruction.suffix = DWORD_SUFFIX;
            } else {
                instruction.suffix = QWORD_SUFFIX;
            }
            
        }
    
    }
    
    if (is_movsx_or_movzx) {
    
        /* The W modifier applies to the source memory or register, not to the destination register. */
        if ((instruction.template.opcode_modifier & W) && instruction.suffix && instruction.suffix != BYTE_SUFFIX) {
            instruction.template.base_opcode |= 1;
        }
        
        /* Changes the suffix to represent the destination and turns off the W modifier as it was already used above. */
        if ((instruction.template.opcode_modifier & W) || !instruction.suffix) {
        
            if (instruction.types[1] & REG16) {
                instruction.suffix = WORD_SUFFIX;
            } else {
                instruction.suffix = DWORD_SUFFIX;
            }
            
            instruction.template.opcode_modifier &= ~W;
        
        }
    
    }
    
    switch (instruction.suffix) {
    
        case DWORD_SUFFIX:
        
            if (instruction.template.opcode_modifier & FLOAT_MF) {
                instruction.template.base_opcode ^= 4;
                break;
            }
            
            /* fall through. */
        
        case WORD_SUFFIX:
        case QWORD_SUFFIX:
        
            /* Selects word/dword operation. */
            if (instruction.template.opcode_modifier & W) {
            
                if (instruction.template.opcode_modifier & SHORT_FORM) {
                    instruction.template.base_opcode |= 8;
                } else {
                    instruction.template.base_opcode |= 1;
                }
            
            }
            
            /* fall through. */
        
        case SHORT_SUFFIX:
        
            if (instruction.suffix != QWORD_SUFFIX
                && !(instruction.template.opcode_modifier & (IGNORE_SIZE | FLOAT_MF))
                && ((instruction.suffix == DWORD_SUFFIX) == (bits == 16))) {
            
                unsigned int prefix = DATA_PREFIX_OPCODE;
                
                if (instruction.template.opcode_modifier & JUMPBYTE) {
                    prefix = ADDR_PREFIX_OPCODE;
                }
                
                if (!add_prefix (prefix)) {
                    return 1;
                }
            
            }

            /* 64 bit operands are wanted, so REX_W is used. */
            if (instruction.suffix == QWORD_SUFFIX
                && bits == 64
                && (instruction.template.opcode_modifier & NO_REX_W) == 0) {
                instruction.rex |= REX_W;
            }
            
            break;

        case 0:

            /* Selects word/dword operation based on explicit data size prefix
             * if no suitable register are present. */
            if ((instruction.template.opcode_modifier & W)
                && instruction.prefixes[DATA_PREFIX]
                && (!instruction.reg_operands
                    || (instruction.reg_operands == 1
                        && !(instruction.template.operand_types[0] & SHIFT_COUNT)
                        && !(instruction.template.operand_types[0] & PORT)
                        && !(instruction.template.operand_types[1] & PORT)))) {

                instruction.template.base_opcode |= 1;

            }

            break;
    
    }
    
    return 0;

}

static int finalize_imms (void) {

    int operand;
    
    for (operand = 0; operand < instruction.operands; operand++) {
    
        unsigned int overlap = instruction.types[operand] & instruction.template.operand_types[operand];
        
        if ((overlap & IMM) && (overlap != IMM8) && (overlap != IMM8S) && (overlap != IMM16) && (overlap != IMM32)) {
        
            if (instruction.suffix) {
            
                switch (instruction.suffix) {
                
                    case BYTE_SUFFIX:
                    
                        overlap &= IMM8 | IMM8S;
                        break;
                    
                    case WORD_SUFFIX:
                    
                        overlap &= IMM16;
                        break;
                    
                    case DWORD_SUFFIX:
                    
                        overlap &= IMM32;
                        break;
                
                }
            
            } else if (overlap == (IMM16 | IMM32)) {
            
                if ((bits == 16) ^ (instruction.prefixes[DATA_PREFIX] != 0)) {
                    overlap = IMM16;
                } else {
                    overlap = IMM32;
                }
            
            } else if (instruction.prefixes[DATA_PREFIX]) {
                overlap &= (bits != 16) ? IMM16 : IMM32;
            }

            if ((overlap != IMM8) && (overlap != IMM8S) && (overlap != IMM16) && (overlap != IMM32)) {
            
                as_error ("no instruction suffix given; cannot determine immediate size");
                return 1;
            
            }
        
        }
        
        instruction.types[operand] = overlap;
    
    }
    
    return 0;

}

static void output_jump (void) {
    
    struct symbol *symbol;
    
    unsigned long offset;
    unsigned long opcode_offset_in_buf;
    
    unsigned int code16 = 0;
    
    if (bits == 16) {
        code16 = RELAX_SUBTYPE_CODE16_JUMP;
    }
    
    if (instruction.prefixes[DATA_PREFIX]) {
    
        frag_append_1_char (instruction.prefixes[DATA_PREFIX]);
        instruction.prefix_count--;
        
        code16 ^= RELAX_SUBTYPE_CODE16_JUMP;
    
    }
    
    if ((instruction.prefixes[SEGMENT_PREFIX] == CS_PREFIX_OPCODE) || (instruction.prefixes[SEGMENT_PREFIX] == DS_PREFIX_OPCODE)) {
    
        frag_append_1_char (instruction.prefixes[SEGMENT_PREFIX]);
        instruction.prefix_count--;
    
    }
    
    if (instruction.prefix_count) {
        as_warn ("skipping prefixes on this instruction");
    }
    
    frag_alloc_space (2 + 4);
    
    opcode_offset_in_buf = current_frag->fixed_size;
    frag_append_1_char (instruction.template.base_opcode);
    
    if (instruction.disps[0]->type == EXPR_TYPE_CONSTANT) {
    
        /* "jmp 5" is converted to "temp_label: jmp 1 + temp_label + 5".
         * The "1" is the size of the opcode
         * and it is included by calling symbol_temp_new_now ()
         * after the opcode is written above.
         */
        instruction.disps[0]->type = EXPR_TYPE_SYMBOL;
        instruction.disps[0]->add_symbol = symbol_temp_new_now ();
    
    }
    
    symbol = instruction.disps[0]->add_symbol;
    offset = instruction.disps[0]->add_number;
    
    if (!instruction.force_short_jump) {
        relax_subtype_t relax_subtype;
    
        if (instruction.template.base_opcode == PC_RELATIVE_JUMP) {
            relax_subtype = ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_UNCONDITIONAL_JUMP, RELAX_SUBTYPE_SHORT_JUMP);
        } else if (cpu_arch_flags & CPU_386) {
            relax_subtype = ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP, RELAX_SUBTYPE_SHORT_JUMP);
        } else {
            relax_subtype = ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP86, RELAX_SUBTYPE_SHORT_JUMP);
        }
        
        relax_subtype |= code16;
        frag_set_as_variant (RELAX_TYPE_MACHINE_DEPENDENT, relax_subtype, symbol, offset, opcode_offset_in_buf);
    
    } else {
    
        frag_set_as_variant (RELAX_TYPE_MACHINE_DEPENDENT,
                             ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_FORCED_SHORT_JUMP, RELAX_SUBTYPE_SHORT_JUMP),
                             symbol,
                             offset,
                             opcode_offset_in_buf);
    
    }

}

static void output_call_or_jumpbyte (void) {

    int size;
    
    if (instruction.template.opcode_modifier & JUMPBYTE) {
    
        size = 1;
        
        if (instruction.prefixes[ADDR_PREFIX]) {
        
            frag_append_1_char (instruction.prefixes[ADDR_PREFIX]);
            instruction.prefix_count--;
        
        }
        
        if ((instruction.prefixes[SEGMENT_PREFIX] == CS_PREFIX_OPCODE) || (instruction.prefixes[SEGMENT_PREFIX] == DS_PREFIX_OPCODE)) {
        
            frag_append_1_char (instruction.prefixes[SEGMENT_PREFIX]);
            instruction.prefix_count--;
        
        }
    
    } else {
    
        unsigned int code16 = 0;
        
        if (bits == 16) {
            code16 = RELAX_SUBTYPE_CODE16_JUMP;
        }
        
        if (instruction.prefixes[DATA_PREFIX]) {
        
            frag_append_1_char (instruction.prefixes[DATA_PREFIX]);
            instruction.prefix_count--;
            
            code16 ^= RELAX_SUBTYPE_CODE16_JUMP;
        
        }
        
        size = code16 ? 2 : 4;
    
    }
    
    if (instruction.prefix_count) {
        as_warn ("skipping prefixes on this instruction");
    }
    
    frag_append_1_char (instruction.template.base_opcode);
    
    if (instruction.disps[0]->type == EXPR_TYPE_CONSTANT) {
    
        /* "call 5" is converted to "temp_label: call 1 + temp_label + 5".
         * The "1" is the size of the opcode
         * and it is included by calling symbol_temp_new_now ()
         * after the opcode is written above.
         */
        instruction.disps[0]->type = EXPR_TYPE_SYMBOL;
        instruction.disps[0]->add_symbol = symbol_temp_new_now ();
    
    }
    
    fixup_new_expr (current_frag, current_frag->fixed_size, size, instruction.disps[0], 1, RELOC_TYPE_DEFAULT);
    frag_increase_fixed_size (size);

}

static void output_intersegment_jump (void) {

    unsigned int code16;
    unsigned int size;
    unsigned char *p;
    
    code16 = 0;
    
    if (bits == 16) {
        code16 = RELAX_SUBTYPE_CODE16_JUMP;
    }
    
    if (instruction.prefixes[DATA_PREFIX]) {
    
        frag_append_1_char (instruction.prefixes[DATA_PREFIX]);
        instruction.prefix_count--;
        
        code16 ^= RELAX_SUBTYPE_CODE16_JUMP;
    
    }
    
    if (instruction.prefix_count) {
        as_warn ("skipping prefixes on this instruction");
    }
    
    size = code16 ? 2 : 4;
    frag_append_1_char (instruction.template.base_opcode);
    
    /* size for the offset, 2 for the segment. */
    p = frag_increase_fixed_size (size + 2);
    
    if (instruction.imms[1]->type == EXPR_TYPE_CONSTANT) {
    
        if ((size == 2) && !fits_in_unsigned_word (instruction.imms[1]->add_number) && !fits_in_signed_word (instruction.imms[1]->add_number)) {
        
            as_error ("16-bit jump out of range.");
            return;
        
        }
        
        machine_dependent_number_to_chars (p, instruction.imms[1]->add_number, size);
    
    } else {
        fixup_new_expr (current_frag, p - current_frag->buf, size, instruction.imms[1], 0, RELOC_TYPE_DEFAULT);
    }
    
    if (instruction.imms[0]->type != EXPR_TYPE_CONSTANT) {
        as_error ("cannot handle non absolute segment in '%s'", instruction.template.name);
    }
    
    machine_dependent_number_to_chars (p + size, instruction.imms[0]->add_number, size);

}

/**
 * Return value meaning:
 *  1 - translate 'd' suffix to SHORT_SUFFIX (instead of DWORD_SUFFIX)
 *  2 - translate WORD_SUFFIX to SHORT_SUFFIX
 *  3 - translate WORD_SUFFIX to SHORT_SUFFIX but do no translation for WORD PTR
 * This applies only when intel_syntax is 1.
 */
static int intel_float_suffix_translation (const char *mnemonic) {

    if (mnemonic[0] != 'f') {
        return 0;                       /* Not a floating point operation. */
    }
    
    switch (mnemonic[1]) {
    
        case 'i':
        
            return 2;
        
        case 'l':
        
            if (mnemonic[2] == 'd' && (mnemonic[3] == 'c' || mnemonic[3] == 'e')) {
                return 3;               /* fldcw/fldenv */
            }
            
            break;
        
        case 'n':
        
            if (mnemonic[2] != 'o') {
                return 3;               /* Not fnop. */
            }
            
            break;
        
        case 'r':
        
            if (mnemonic[2] == 's') {
                return 3;               /* frstor */
            }
            
            break;
        
        case 's':
        
            if (mnemonic[2] == 'a') {
                return 3;               /* fsave */
            }
            
            if (mnemonic[2] == 't') {
            
                switch (mnemonic[3]) {
                
                    case 'c':           /* fstcw */
                    case 'e':           /* fstenv */
                    case 's':           /* fstsw */
                    
                        return 3;
                
                }
            
            }
            
            break;
    
    }
    
    return 1;

}

static char *parse_instruction (char *line) {

    const struct template *template;
    const char *expecting_string_instruction = NULL;
    
    char *p2;
    char saved_ch;

    int match;
    
    current_templates = NULL;
    
    while (1) {
    
        p2 = line = skip_whitespace (line);
        
        while ((*p2 != ' ') && (*p2 != '\t') && (*p2 != '\0')) {
        
            *p2 = tolower (*p2);
            p2++;
        
        }
        
        saved_ch = *p2;
        *p2 = '\0';
        
        if (line == p2) {
        
            as_error ("expecting mnemonic; got nothing");
            goto end;
        
        }
        
        current_templates = find_templates (line);
        
        if (saved_ch && (*skip_whitespace (p2 + 1)) && current_templates && (current_templates->start->opcode_modifier & IS_PREFIX)) {
        
            if ((current_templates->start->opcode_modifier & (SIZE16 | SIZE32)) && (((current_templates->start->opcode_modifier & SIZE32) != 0) ^ (bits == 16))) {
            
                as_error ("redundant %s prefix", current_templates->name);
                current_templates = NULL;
                goto end;
            
            }
            
            switch (add_prefix (current_templates->start->base_opcode)) {
            
                case 0:
                
                    current_templates = NULL;
                    goto end;
                
                case 2:
                
                    expecting_string_instruction = current_templates->name;
                    break;
            
            }
            
            *p2 = saved_ch;
            line = p2 + 1;
        
        } else {
            break;
        }
    
    }
    
    if (current_templates == NULL) {
    
        switch (p2[-1]) {
        
            case WORD_SUFFIX:
            
                if (intel_syntax && (intel_float_suffix_translation (line) & 2)) {
                    p2[-1] = SHORT_SUFFIX;
                }
                
                /* fall through. */
            
            case BYTE_SUFFIX:
            case QWORD_SUFFIX:
            
                instruction.suffix = p2[-1];
                p2[-1] = '\0';
                
                break;
            
            case SHORT_SUFFIX:
            case DWORD_SUFFIX:
            
                if (!intel_syntax) {
                
                    instruction.suffix = p2[-1];
                    p2[-1] = '\0';
                
                }
                
                break;
            
            /* Intel syntax only. */
            case 'd':
            
                if (intel_syntax) {
                
                    if (intel_float_suffix_translation (line) == 1) {
                        instruction.suffix = SHORT_SUFFIX;
                    } else {
                        instruction.suffix = DWORD_SUFFIX;
                    }
                    
                    p2[-1] = '\0';

                }
                
                break;
            
            default:
            
                as_error ("no such instruction '%s'", line);
                goto end;
        
        }
        
        current_templates = find_templates (line);
        
        if (current_templates == NULL) {
        
            as_error ("no such instruction '%s'", line);
            goto end;
        
        }
    
    }
    
    if (expecting_string_instruction) {
    
        if (!(current_templates->start->opcode_modifier & IS_STRING)) {
        
            as_error ("expecting string instruction after '%s'", expecting_string_instruction);
            goto end;
        
        }
    
    }

    match = 0;
    for (template = current_templates->start; template < current_templates->end; template++) {

        match |= cpu_flags_match (template);
        if (match == CPU_FLAGS_PERFECT_MATCH) goto end;

    }

    if (!(match & CPU_FLAGS_64BIT_MATCH)) {
        as_error ((bits == 64)
                  ? "'%s' is not supported in 64-bit mode"
                  : "'%s' is supported only in 64-bit mode",
                  current_templates->name);
    } else {
        as_error ("'%s' is not supported on '%s%s'",
                  current_templates->name,
                  cpu_arch_name ? cpu_arch_name : DEFAULT_CPU_ARCH_NAME,
                  cpu_extensions_name ? cpu_extensions_name : "");
    }
    current_templates = NULL;

end:
    *p2 = saved_ch;
    line = p2;
    
    return (line);

}

static int parse_operands (char **p_line) {

    char *line = *p_line;

    while (*line != '\0') {
    
        char *token_start;
        unsigned int paren_not_balanced;
        paren_not_balanced = 0;
        
        line = skip_whitespace (line);
        token_start = line;
        
        while (paren_not_balanced || (*line != ',')) {
        
            if (*line == '\0') {
            
                if (paren_not_balanced) {
                
                    as_error ("unbalanced parentheses in operand %u", instruction.operands);
                    *p_line = line;
                    return 1;
                
                } else {
                    break;
                }
            
            } else if (line[0] == '#' || (line[0] == '/' && (line[1] == '/' || line[1] == '*'))) {
            
                if (!paren_not_balanced) {
                    break;
                }
            
            }
            
            if (!intel_syntax) {
            
                if (*line == '(') { paren_not_balanced++; }
                if (*line == ')') { paren_not_balanced--; }
            
            }
            
            line++;
        
        }
        
        if (token_start != line) {
        
            int ret;
            char saved_ch;
            
            saved_ch = *line;
            *line = '\0';
            
            if (intel_syntax) {
                ret = intel_parse_operand (token_start);
            } else {
                ret = att_parse_operand (token_start);
            }
            
            *line = saved_ch;
            
            if (ret) {
            
                *p_line = line;
                return 1;
            
            }
        
        }
        
        if (line[0] == '#' || (line[0] == '/' && line[1] == '/')) {
            break;
        }
        
        if (line[0] == '/' && line[1] == '*') {
        
            while (*line) {
            
                if (line[0] == '*' && line[1] == '/') {
                
                    line += 2;
                    break;
                
                }
                
                line++;
            
            }
        
        }
        
        if (*line == ',') { line++; }
    
    }
    
    *p_line = line;
    return 0;
    
}

enum x86_error {
    x86_error_number_of_operands_mismatch,
    x86_error_unsupported,
    x86_error_operand_type_mismatch,
    x86_error_invalid_instruction_suffix
};    

#define MATCH(overlap, operand_type)    (((overlap) & ~JUMP_ABSOLUTE) && (((operand_type) & (BASE_INDEX | JUMP_ABSOLUTE)) == ((overlap) & (BASE_INDEX | JUMP_ABSOLUTE))))

static int match_template (void) {

    const struct template *template;
    enum x86_error matching_error;
    
    unsigned int found_reverse_match = 0;
    flag_int suffix_check = 0;
    
    switch (instruction.suffix) {
    
        case BYTE_SUFFIX:
        
            suffix_check = NO_BSUF;
            break;
        
        case WORD_SUFFIX:
        
            suffix_check = NO_WSUF;
            break;
        
        case SHORT_SUFFIX:
        
            suffix_check = NO_SSUF;
            break;
        
        case DWORD_SUFFIX:
        
            suffix_check = NO_LSUF;
            break;
        
        case QWORD_SUFFIX:
        
            suffix_check = NO_QSUF;
            break;
        
        case INTEL_SUFFIX:
        
            suffix_check = NO_INTELSUF;
            break;
    
    }
    
    matching_error = x86_error_number_of_operands_mismatch;
    
    for (template = current_templates->start; template < current_templates->end; template++) {
    
        flag_int operand_type_overlap0, operand_type_overlap1;
        
        if (instruction.operands != template->operands) {
            continue;
        }
        
        matching_error = x86_error_unsupported;
        if (cpu_flags_match (template) != CPU_FLAGS_PERFECT_MATCH) {
            continue;
        }

        matching_error = x86_error_invalid_instruction_suffix;
        if (template->opcode_modifier & suffix_check) {
            continue;
        }

        if (((instruction.suffix == QWORD_SUFFIX
             && bits != 64)
             ||(instruction.suffix == DWORD_SUFFIX
                && !(cpu_arch_flags & CPU_386)))
            && !(template->opcode_modifier & IGNORE_SIZE)
            && !intel_float_suffix_translation (template->name)) {
            continue;
        }
        
        if (instruction.operands == 0) {
            break;
        }

        matching_error = x86_error_operand_type_mismatch;
        
        operand_type_overlap0 = instruction.types[0] & template->operand_types[0];
        
        switch (template->operands) {
        
            case 1:
            
                if (!MATCH (operand_type_overlap0, instruction.types[0])) {
                    continue;
                }
                
                if (operand_type_overlap0 == 0) {
                    continue;
                }
                
                break;
            
            case 2:
            case 3:
            
                operand_type_overlap1 = instruction.types[1] & template->operand_types[1];
                
                if (!MATCH (operand_type_overlap0, instruction.types[0]) || !MATCH (operand_type_overlap1, instruction.types[1])) {
                
                    if ((template->opcode_modifier & (D | FLOAT_D)) == 0) {
                        continue;
                    }
                    
                    operand_type_overlap0 = instruction.types[0] & template->operand_types[1];
                    operand_type_overlap1 = instruction.types[1] & template->operand_types[0];
                    
                    if (!MATCH (operand_type_overlap0, instruction.types[0]) || !MATCH (operand_type_overlap1, instruction.types[1])) {
                        continue;
                    }
                    
                    found_reverse_match = template->opcode_modifier & (D | FLOAT_D);
                
                } else if (instruction.operands == 3) {
                
                    flag_int operand_type_overlap2 = instruction.types[2] & template->operand_types[2];
                    
                    if (!MATCH (operand_type_overlap2, instruction.types[2])) {
                        continue;
                    }
                
                }
                
                break;
        
        }
        
        break;
    
    }
    
    if (template == current_templates->end) {

        /* No match was found. */

        const char *error_msg;

        switch (matching_error) {

            case x86_error_number_of_operands_mismatch:
                error_msg = "number of operands mismatch";
                break;

            case x86_error_unsupported:
                as_error ("unsupported instruction '%s' on '%s%s'",
                          current_templates->name,
                          cpu_arch_name ? cpu_arch_name : DEFAULT_CPU_ARCH_NAME,
                          cpu_extensions_name ? cpu_extensions_name : "");
                return 1;

            case x86_error_invalid_instruction_suffix:
                error_msg = "invalid instruction suffix";
                break;

            case x86_error_operand_type_mismatch:
                error_msg = "operands invalid";
                break;

            default:
                as_internal_error_at_source (__FILE__, __LINE__, "match_template invalid case %i", matching_error);
                break;

        }

        as_error ("%s for '%s'", error_msg, current_templates->name);
        return 1;
    
    }
    
    instruction.template = *template;
    
    if (found_reverse_match) {
    
        instruction.template.base_opcode |= found_reverse_match;
        
        instruction.template.operand_types[0] = template->operand_types[1];
        instruction.template.operand_types[1] = template->operand_types[0];
    
    }
    
    return 0;

}

static int process_operands (void) {

    if (instruction.template.opcode_modifier & REG_DUPLICATION) {
    
        unsigned int first_reg_operand = (instruction.types[0] & REG) ? 0 : 1;
        
        instruction.regs[first_reg_operand + 1] = instruction.regs[first_reg_operand];
        instruction.types[first_reg_operand + 1] = instruction.types[first_reg_operand];
        instruction.reg_operands = 2;
    
    }
    
    if (instruction.template.opcode_modifier & SHORT_FORM) {
    
        int operand = (instruction.types[0] & (REG | FLOAT_REG)) ? 0 : 1;
        instruction.template.base_opcode |= instruction.regs[operand]->number;
        if (instruction.regs[operand]->type & REG_REX) instruction.rex |= REX_B;
    
    }
    
    if (instruction.template.opcode_modifier & MODRM) {
    
        if (instruction.reg_operands == 2) {
        
            unsigned int source, dest;
            
            source = (instruction.types[0] & (REG | SEGMENT1 | SEGMENT2 | CONTROL | DEBUG | TEST)) ? 0 : 1;
            dest = source + 1;
            
            instruction.modrm.mode = 3;
            
            if ((instruction.template.operand_types[dest] & ANY_MEM) == 0) {
            
                instruction.modrm.regmem = instruction.regs[source]->number;
                instruction.modrm.reg = instruction.regs[dest]->number;
                if (instruction.regs[dest]->type & REG_REX) instruction.rex |= REX_R;
                if (instruction.regs[source]->type & REG_REX) instruction.rex |= REX_B;
            
            } else {
            
                instruction.modrm.regmem = instruction.regs[dest]->number;
                instruction.modrm.reg = instruction.regs[source]->number;
                if (instruction.regs[dest]->type & REG_REX) instruction.rex |= REX_B;
                if (instruction.regs[source]->type & REG_REX) instruction.rex |= REX_R;
            
            }
        
        } else {
        
            if (instruction.mem_operands) {
            
                int fake_zero_displacement = 0;
                int operand = 0;
                
                if (instruction.types[0] & ANY_MEM) {
                    ;
                } else if (instruction.types[1] & ANY_MEM) {
                    operand = 1;
                } else {
                    operand = 2;
                }
                
                if (instruction.base_reg == NULL) {
                
                    instruction.modrm.mode = 0;
                    
                    if (instruction.disp_operands == 0) {
                        fake_zero_displacement = 1;
                    }
                    
                    if (instruction.index_reg == NULL) {
                    
                        if (bits == 64) {
                            
                            /* In 64 bit mode default addressing is %rip relative addressing
                             * instead of absolute addresing. */
                            instruction.modrm.regmem = MODRM_REGMEM_TWO_BYTE_ADDRESSING;
                            instruction.sib.base = SIB_BASE_NO_BASE_REGISTER;
                            instruction.sib.index = SIB_INDEX_NO_INDEX_REGISTER;
                            instruction.types[operand] = DISP32;
                            
                        } else if ((bits == 16) ^ (instruction.prefixes[ADDR_PREFIX] != 0)) {
                        
                            instruction.modrm.regmem = SIB_BASE_NO_BASE_REGISTER_16;
                            instruction.types[operand] = DISP16;
                        
                        } else {
                        
                            instruction.modrm.regmem = SIB_BASE_NO_BASE_REGISTER;
                            instruction.types[operand] = DISP32;
                        
                        }
                    
                    } else {
                    
                        instruction.sib.base = SIB_BASE_NO_BASE_REGISTER;
                        instruction.sib.index = instruction.index_reg->number;
                        instruction.sib.scale = instruction.log2_scale_factor;
                        
                        instruction.modrm.regmem = MODRM_REGMEM_TWO_BYTE_ADDRESSING;
                        
                        instruction.types[operand] &= ~DISP;
                        instruction.types[operand] |= DISP32;

                        if (instruction.index_reg->type & REG_REX) instruction.rex |= REX_X;
                    
                    }
                
                } else if (instruction.base_reg->type & REG16) {
                
                    switch (instruction.base_reg->number) {
                    
                        case 3:
                        
                            if (instruction.index_reg == NULL) {
                                instruction.modrm.regmem = 7;
                            } else {
                                instruction.modrm.regmem = (instruction.index_reg->number - 6);
                            }
                            
                            break;
                        
                        case 5:
                        
                            if (instruction.index_reg == NULL) {
                            
                                instruction.modrm.regmem = 6;
                                
                                if ((instruction.types[operand] & DISP) == 0) {
                                
                                    fake_zero_displacement = 1;
                                    instruction.types[operand] |= DISP8;
                                
                                }
                            
                            } else {
                                instruction.modrm.regmem = (instruction.index_reg->number - 6 + 2);
                            }
                            
                            break;
                        
                        default:
                        
                            instruction.modrm.regmem = (instruction.base_reg->number - 6 + 4);
                            break;
                    
                    }
                    
                    instruction.modrm.mode = modrm_mode_from_disp_size (instruction.types[operand]);
                
                } else {
                
                    if (bits == 16 && (instruction.types[operand] & BASE_INDEX)) {
                        add_prefix (ADDR_PREFIX_OPCODE);
                    }
                    
                    instruction.modrm.regmem = instruction.base_reg->number;
                    if (instruction.base_reg->type & REG_REX) instruction.rex |= REX_B;
                    
                    instruction.sib.base = instruction.base_reg->number;
                    instruction.sib.scale = instruction.log2_scale_factor;
                    
                    if (instruction.base_reg->number == 5 && instruction.disp_operands == 0) {
                    
                        fake_zero_displacement = 1;
                        instruction.types[operand] |= DISP8;
                    
                    }
                    
                    if (instruction.index_reg) {
                    
                        instruction.sib.index = instruction.index_reg->number;
                        instruction.modrm.regmem = MODRM_REGMEM_TWO_BYTE_ADDRESSING;
                        if (instruction.index_reg->type & REG_REX) instruction.rex |= REX_X;
                    
                    } else {
                        instruction.sib.index = SIB_INDEX_NO_INDEX_REGISTER;
                    }
                    
                    instruction.modrm.mode = modrm_mode_from_disp_size (instruction.types[operand]);
                
                }
                
                if (fake_zero_displacement) {
                
                    struct expr *expr = &operand_exprs[operand_exprs_count++];
                    instruction.disps[operand] = expr;
                    
                    expr->type = EXPR_TYPE_CONSTANT;
                    expr->add_number = 0;
                    expr->add_symbol = NULL;
                    expr->op_symbol = NULL;
                
                }
            
            }
            
            if (instruction.reg_operands) {
            
                int operand = 0;
                
                if (instruction.types[0] & (REG | SEGMENT1 | SEGMENT2 | CONTROL | DEBUG | TEST)) {
                    ;
                } else if (instruction.types[1] & (REG | SEGMENT1 | SEGMENT2 | CONTROL | DEBUG | TEST)) {
                    operand = 1;
                } else {
                    operand = 2;
                }
                
                if (instruction.template.extension_opcode != NONE) {
                    instruction.modrm.regmem = instruction.regs[operand]->number;
                    if (instruction.regs[operand]->type & REG_REX) instruction.rex |= REX_B;
                } else {
                    instruction.modrm.reg = instruction.regs[operand]->number;
                    if (instruction.regs[operand]->type & REG_REX) instruction.rex |= REX_R;
                }
                
                if (instruction.mem_operands == 0) {
                    instruction.modrm.mode = 3;
                }
            
            }
            
            if (instruction.template.extension_opcode != NONE) {
                instruction.modrm.reg = instruction.template.extension_opcode;
            }
        
        }
    
    }
    
    if (instruction.template.opcode_modifier & SEGSHORTFORM) {
    
        if ((instruction.template.base_opcode == POP_SEGMENT_SHORT) && (instruction.regs[0]->number == 1)) {
        
            as_error ("'pop %%cs' is not valid");
            return 1;
        
        }
        
        instruction.template.base_opcode |= instruction.regs[0]->number << 3;
    
    }
    
    {
    
        int operand;
        
        for (operand = 0; operand < instruction.operands; operand++) {
        
            if (instruction.segments[operand]) {
            
                add_prefix (segment_prefixes[instruction.segments[operand]->number]);
                break;
            
            }
        
        }
    
    }
    
    return 0;

}

static void swap_2_operands (unsigned int op1, unsigned int op2) {

    unsigned int temp_type;
    const struct reg_entry *temp_reg;
    struct expr *temp_expr;
    
    temp_type = instruction.types[op1];
    instruction.types[op1] = instruction.types[op2];
    instruction.types[op2] = temp_type;
    
    temp_reg = instruction.regs[op1];
    instruction.regs[op1] = instruction.regs[op2];
    instruction.regs[op2] = temp_reg;
    
    temp_expr = instruction.disps[op1];
    instruction.disps[op1] = instruction.disps[op2];
    instruction.disps[op2] = temp_expr;
    
    temp_expr = instruction.imms[op1];
    instruction.imms[op1] = instruction.imms[op2];
    instruction.imms[op2] = temp_expr;

}

static void swap_operands (void) {

    swap_2_operands (0, instruction.operands - 1);
    
    if (instruction.mem_operands == 2) {
    
        const struct reg_entry *seg;
        
        seg = instruction.segments[0];
        instruction.segments[0] = instruction.segments[1];
        instruction.segments[1] = seg;
    
    }

}

char *machine_dependent_assemble_line (char *line) {

    memset (&instruction, 0, sizeof (instruction));
    memset (operand_exprs, 0, sizeof (operand_exprs));
    operand_exprs_count = 0;
    
    line = parse_instruction (line);
    
    if (current_templates == NULL || parse_operands (&line)) {
        goto skip;
    }
    
    /* All Intel instructions have reversed operands except "bound" and some other.
     * "ljmp" and "lcall" with 2 immediate operands also do not have operands reversed. */
    if (intel_syntax && instruction.operands > 1
        && strcmp (current_templates->name, "bound")
        && !((instruction.types[0] & IMM)
             && (instruction.types[1] & IMM))) {
        swap_operands ();
    }
    
    optimize_size_of_disps ();
    optimize_size_of_imms ();
    
    if (match_template () || process_suffix () || finalize_imms ()) {
        goto skip;
    }
    
    if (instruction.template.opcode_modifier & ADD_FWAIT) {
    
        if (!add_prefix (FWAIT_OPCODE)) {
            goto skip;
        }
    
    }
    
    if (instruction.template.operand_types[0] & IMPLICIT_REGISTER) {
        instruction.reg_operands--;
    }
    
    if (instruction.template.operand_types[1] & IMPLICIT_REGISTER) {
        instruction.reg_operands--;
    }
    
    if (instruction.operands) {
    
        if (process_operands ()) {
            goto skip;
        }
    
    }

    /* int $3 should be converted to the one byte INT3. */
    if (instruction.template.base_opcode == INT_OPCODE
        && instruction.imms[0]->add_number == 3) {

        instruction.template.base_opcode = INT3_OPCODE;
        instruction.operands = 0;

    }
    
    if (instruction.rex) add_prefix (REX_PREFIX_OPCODE | instruction.rex);        
    
    if (instruction.template.opcode_modifier & JUMP) {
        output_jump ();
    } else if (instruction.template.opcode_modifier & (CALL | JUMPBYTE)) {
        output_call_or_jumpbyte ();
    } else if (instruction.template.opcode_modifier & JUMPINTERSEGMENT) {
        output_intersegment_jump ();
    } else {
    
        unsigned int i;

        for (i = 0; i < ARRAY_SIZE (instruction.prefixes); i++) {
        
            if (instruction.prefixes[i]) {
                frag_append_1_char (instruction.prefixes[i]);
            }
        
        }
        
        if (instruction.template.base_opcode & 0xff00) {
            frag_append_1_char ((instruction.template.base_opcode >> 8) & 0xff);
        }
        
        frag_append_1_char (instruction.template.base_opcode & 0xff);
        
        if (instruction.template.opcode_modifier & MODRM) {
        
            frag_append_1_char (((instruction.modrm.regmem << 0) | (instruction.modrm.reg << 3) | (instruction.modrm.mode << 6)));
            
            if ((instruction.modrm.regmem == MODRM_REGMEM_TWO_BYTE_ADDRESSING) && (instruction.modrm.mode != 3) && !(instruction.base_reg && (instruction.base_reg->type & REG16))) {
                frag_append_1_char (((instruction.sib.base << 0) | (instruction.sib.index << 3) | (instruction.sib.scale << 6)));
            }
        
        }
        
        output_disps ();
        output_imms ();
    
    }
    
    return line;
    
skip:

    while (*line != '\0') {
    
        if (line[0] == '#') {
            break;
        }
        
        if (line[0] == '/') {
        
            if (line[1] == '/' || line[1] == '*') {
                break;
            }
        
        }
        
        line++;
    
    }
    
    return line;

}

int machine_dependent_force_relocation_local (struct fixup *fixup) {
    return fixup->pcrel == 0;
}

long machine_dependent_estimate_size_before_relax (struct frag *frag, section_t section) {

    if (symbol_get_section (frag->symbol) != section) {
    
        int size = (frag->relax_subtype & RELAX_SUBTYPE_CODE16_JUMP) ? 2 : 4;
        
        unsigned char *opcode_pos = frag->buf + frag->opcode_offset_in_buf;
        unsigned long old_frag_fixed_size = frag->fixed_size;
        
        switch (TYPE_FROM_RELAX_SUBTYPE (frag->relax_subtype)) {
        
            case RELAX_SUBTYPE_UNCONDITIONAL_JUMP:
            
                *opcode_pos = 0xE9;
                
                fixup_new (frag, frag->fixed_size, size, frag->symbol, frag->offset, 1, RELOC_TYPE_DEFAULT);
                frag->fixed_size += size;
                
                break;
            
            case RELAX_SUBTYPE_CONDITIONAL_JUMP86:
            
                if (size == 2) {

                    /* Negates the condition and jumps past unconditional jump. */
                    opcode_pos[0] ^= 1;
                    opcode_pos[1] = 3;
                    
                    /* Inserts the unconditional jump. */
                    opcode_pos[2] = 0xE9;
                    
                    frag->fixed_size += 4;
                    fixup_new (frag, old_frag_fixed_size + 2, size, frag->symbol, frag->offset, 1, RELOC_TYPE_DEFAULT);
                    
                    break;
                
                }
                
                /* fall through. */
            
            case RELAX_SUBTYPE_CONDITIONAL_JUMP:
                
                opcode_pos[1] = opcode_pos[0] + 0x10;
                opcode_pos[0] = TWOBYTE_OPCODE;
                
                fixup_new (frag, frag->fixed_size + 1, size, frag->symbol, frag->offset, 1, RELOC_TYPE_DEFAULT);
                frag->fixed_size += size + 1;
                
                break;
            
            case RELAX_SUBTYPE_FORCED_SHORT_JUMP:
            
                if (state->format == AS_FORMAT_COFF) {
                    as_fatal_error_at (frag->filename, frag->line_number,
                                       "forced short jump with target in different section (origin section %s and target section %s) is not supported for COFF",
                                       section_get_name (section), section_get_name (symbol_get_section (frag->symbol)));
                }
                
                size = 1;
                
                fixup_new (frag, frag->fixed_size, size, frag->symbol, frag->offset, 1, RELOC_TYPE_DEFAULT);
                frag->fixed_size += size;
                
                break;
            
            default:
            
                as_internal_error_at_source_at (__FILE__, __LINE__, frag->filename, frag->line_number,
                                                "machine_dependent_estimate_size_before_relax invalid case");
                return 0;
        
        }
        
        frag->relax_type = RELAX_TYPE_NONE_NEEDED;
        return frag->fixed_size - old_frag_fixed_size;
    
    }
    
    return relax_table[frag->relax_subtype].size_of_variable_part;

}

long machine_dependent_pcrel_from (struct fixup *fixup) {
    return (fixup->size + fixup->where + fixup->frag->address);
}

long machine_dependent_relax_frag (struct frag *frag, section_t section, long change) {

    unsigned long target;
    
    long aim, growth;
    relax_subtype_t new_subtype;
    
    target = frag->offset;
    
    if (frag->symbol) {
    
        target += symbol_get_value (frag->symbol);
        
        if ((section == symbol_get_section (frag->symbol)) && (frag->relax_marker != frag->symbol->frag->relax_marker)) {
            target += change;
        }
    
    }
    
    aim = target - frag->address - frag->fixed_size;
    
    if (aim > 0) {
        
        for (new_subtype = frag->relax_subtype; relax_table[new_subtype].next_subtype; new_subtype = relax_table[new_subtype].next_subtype) {
        
            if (aim <= relax_table[new_subtype].forward_reach) {
                break;
            }
        
        }
    
    } else if (aim < 0) {
    
        for (new_subtype = frag->relax_subtype; relax_table[new_subtype].next_subtype; new_subtype = relax_table[new_subtype].next_subtype) {
        
            if (aim >= relax_table[new_subtype].backward_reach) {
                break;
            }
        
        }
    
    } else {
        return 0;
    }
    
    growth  = relax_table[new_subtype].size_of_variable_part;
    growth -= relax_table[frag->relax_subtype].size_of_variable_part;
    
    if (growth) { frag->relax_subtype = new_subtype; }
    return growth;

}

void machine_dependent_apply_fixup (struct fixup *fixup, unsigned long value) {

    unsigned char *p = fixup->where + fixup->frag->buf;
    
    if (fixup->add_symbol == NULL) {
        fixup->done = 1;
    }
    
    /* Not sure why COFF requires this adjustment. */
    if (state->format == AS_FORMAT_COFF
        && fixup->pcrel
        && fixup->add_symbol
        && symbol_get_section (fixup->add_symbol) != current_section) {
        value += machine_dependent_pcrel_from (fixup);
    }
    
    machine_dependent_number_to_chars (p, value, fixup->size);

}

void machine_dependent_finish_frag (struct frag *frag) {

    unsigned char *opcode_pos;
    
    unsigned char *displacement_pos;
    long displacement;
    
    int size;
    unsigned long extension = 0;
    
    opcode_pos = frag->buf + frag->opcode_offset_in_buf;
    
    displacement_pos = opcode_pos + 1;
    displacement = (symbol_get_value (frag->symbol) + frag->offset - frag->address - frag->fixed_size);
    
    if ((frag->relax_subtype & RELAX_SUBTYPE_LONG_JUMP) == 0) {
    
        displacement_pos = opcode_pos + 1;
        extension = relax_table[frag->relax_subtype].size_of_variable_part;
        
        if (RELAX_SUBTYPE_FORCED_SHORT_JUMP) {
        
            if (displacement > relax_table[frag->relax_subtype].forward_reach
                || displacement < relax_table[frag->relax_subtype].backward_reach) {
            
                as_error_at (frag->filename, frag->line_number, "forced short jump out of range");
            
            }
        
        }
    
    } else {
    
        switch (frag->relax_subtype) {
        
            case ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_UNCONDITIONAL_JUMP, RELAX_SUBTYPE_LONG_JUMP):
            case ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_UNCONDITIONAL_JUMP, RELAX_SUBTYPE_LONG16_JUMP):
            
                extension = relax_table[frag->relax_subtype].size_of_variable_part;
                opcode_pos[0] = 0xE9;
                
                displacement_pos = opcode_pos + 1;
                break;

            case ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP, RELAX_SUBTYPE_LONG_JUMP):
            case ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP86, RELAX_SUBTYPE_LONG_JUMP):
            case ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP, RELAX_SUBTYPE_LONG16_JUMP):
            
                extension = relax_table[frag->relax_subtype].size_of_variable_part;
                
                opcode_pos[1] = opcode_pos[0] + 0x10;
                opcode_pos[0] = TWOBYTE_OPCODE;
                
                displacement_pos = opcode_pos + 2;
                break;
            
            case ENCODE_RELAX_SUBTYPE (RELAX_SUBTYPE_CONDITIONAL_JUMP86, RELAX_SUBTYPE_LONG16_JUMP):
            
                extension = relax_table[frag->relax_subtype].size_of_variable_part;
            
                /* Negates the condition and jumps past unconditional jump. */
                opcode_pos[0] ^= 1;
                opcode_pos[1] = 3;
            
                /* Inserts the unconditional jump. */
                opcode_pos[2] = 0xE9;
                
                displacement_pos = opcode_pos + 3;
                break;
            
        } 
    
    }
    
    size = DISPLACEMENT_SIZE_FROM_RELAX_SUBSTATE (frag->relax_subtype);
    displacement -= extension;

    machine_dependent_number_to_chars (displacement_pos, displacement, size);
    frag->fixed_size += extension;

}

static void set_bits (int new_bits, int cause_fatal_error)
{
    if (new_bits == 64 && !(cpu_arch_flags & CPU_LONG_MODE)) {
        if (cause_fatal_error) {
            as_fatal_error_at (NULL, 0,
                               "64bit mode not supported on '%s'",
                               cpu_arch_name ? cpu_arch_name : DEFAULT_CPU_ARCH_NAME);
        } else {
            as_error ("64bit mode not supported on '%s'",
                      cpu_arch_name ? cpu_arch_name : DEFAULT_CPU_ARCH_NAME);
        }
        return;
    }
    if (new_bits == 32 && !(cpu_arch_flags & CPU_386)) {
        if (cause_fatal_error) {
            as_fatal_error_at (NULL, 0,
                               "32bit mode not supported on '%s'",
                               cpu_arch_name ? cpu_arch_name : DEFAULT_CPU_ARCH_NAME);
        } else {
            as_error ("32bit mode not supported on '%s'",
                      cpu_arch_name ? cpu_arch_name : DEFAULT_CPU_ARCH_NAME);
        }
        return;
    }

    bits = new_bits;

    if (bits == 64) {
        as_warn ("64 bit support is unfinished and for testing only");
        cpu_arch_flags &= ~CPU_NO64;
        cpu_arch_flags |= CPU_64;
    } else {
        cpu_arch_flags &= ~CPU_64;
        cpu_arch_flags |= CPU_NO64;
    }
}

static void handler_code16 (char **pp)
{
    (void) pp;
    set_bits (16, 0);
}

static void handler_code32 (char **pp)
{
    (void) pp;
    set_bits (32, 0);
}

static void handler_code64 (char **pp)
{
    (void) pp;
    set_bits (64, 0);
}

static void set_syntax (char **pp, int set_intel_syntax);

static void handler_att_syntax (char **pp) {
    set_syntax (pp, 0);
}

static void handler_intel_syntax (char **pp) {
    set_syntax (pp, 1);
}

static struct pseudo_op_entry pseudo_op_table[] = {

    { "att_syntax",     &handler_att_syntax     },
    { "intel_syntax",   &handler_intel_syntax   },
    { "code16",         &handler_code16         },
    { "code32",         &handler_code32         },
    { "code64",         &handler_code64         },
    { 0,                0                       }

};

struct pseudo_op_entry *machine_dependent_get_pseudo_op_table (void) {
    return pseudo_op_table;
}

static const char *line_separators = ";";

const char *machine_dependent_get_line_separators (void) {
    return line_separators;
}

static const char *comment_at_the_start_of_line_beginners = "#/";

const char *machine_dependent_get_comment_at_the_start_of_line_beginners (void) {
    return comment_at_the_start_of_line_beginners;
}

int machine_dependent_parse_name (char **pp, struct expr *expr, char *name, char *original_saved_c) {

    const struct reg_entry *reg;
    char *orig_end;
    
    orig_end = *pp;
    **pp = *original_saved_c;
    
    reg = parse_register (name, pp);
    
    if (reg && orig_end <= *pp) {
    
        *original_saved_c = **pp;
        **pp = '\0';

        if (reg != &bad_register) {
            
            expr->type = EXPR_TYPE_REGISTER;
            expr->add_number = reg - reg_table;

        } else {
            expr->type = EXPR_TYPE_INVALID;
        }
        
        return 1;
    
    }
    
    *pp = orig_end;
    **pp = '\0';
    
    return intel_syntax ? intel_parse_name (expr, name) : 0;

}

void machine_dependent_parse_operand (char **pp, struct expr *expr) {

    const struct reg_entry *reg;
    char *end;
    
    switch (**pp) {
    
        case REGISTER_PREFIX:
        
            reg = parse_register (*pp, &end);
            
            if (reg) {
            
                expr->type = EXPR_TYPE_REGISTER;
                expr->add_number = reg - reg_table;
                
                *pp = end;
            
            }
            
            break;
        
        case '[':
        
            end = (*pp)++;
            expression_read_into (pp, expr);
            
            if (**pp == ']' && expr->type != EXPR_TYPE_INVALID) {
            
                ++*pp;
                
                if (expr->type == EXPR_TYPE_SYMBOL && expr->add_number == 0) {
                
                    /* Forces make_expr_symbol to create an expression symbol instead of returning the used symbol. */
                    expr->add_number = 1;
                    expr->op_symbol = make_expr_symbol (expr);
                    symbol_get_value_expression (expr->op_symbol)->add_number = 0;
                
                } else {
                    expr->op_symbol = make_expr_symbol (expr);
                }
                
                expr->add_symbol = NULL;
                expr->add_number = 0;
                expr->type = EXPR_TYPE_INDEX;
            
            } else {
            
                expr->type = EXPR_TYPE_ABSENT;
                *pp = end;
            
            }
            
            break;
    
    }

}

#include    "assemble_intel_support.c"

static void set_syntax (char **pp, int set_intel_syntax) {

    /* -1 forces the prefix use, 0 does not force anything, 1 forces allowing the no prefix. */
    int force_allow_no_prefix_reg = 0;
    
    *pp = skip_whitespace (*pp);
    
    if (!is_end_of_line[(int) **pp]) {
    
        char *s = *pp;
        char saved_c = get_symbol_name_end (pp);
        
        if (xstrcasecmp (s, "prefix") == 0) {
            force_allow_no_prefix_reg = -1;
        } else if (xstrcasecmp (s, "noprefix") == 0) {
            force_allow_no_prefix_reg = 1;
        } else {
            as_error ("bad argument to syntax directive");
        }
        
        **pp = saved_c;
    
    }
    
    if (set_intel_syntax) {
        allow_no_prefix_reg = force_allow_no_prefix_reg > -1;
    } else {
        allow_no_prefix_reg = force_allow_no_prefix_reg > 0;
    }
    
    demand_empty_rest_of_line (pp);
    intel_syntax = set_intel_syntax;
    
    expr_type_set_rank (EXPR_TYPE_FULL_PTR, intel_syntax ? 10 : 0);

}

/* Command-line options handling. */

enum option_index {

    AS_OPTION_IGNORED = 0,
    AS_OPTION_BITS16,
    AS_OPTION_BITS32,
    AS_OPTION_BITS64,
    AS_OPTION_MARCH,
    AS_OPTION_SYNTAX

};

static const struct as_option options[] = {

    { "-16",        AS_OPTION_BITS16,       AS_OPTION_NO_ARG            },
    { "-32",        AS_OPTION_BITS32,       AS_OPTION_NO_ARG            },
    { "-64",        AS_OPTION_BITS64,       AS_OPTION_NO_ARG            },
    { "march",      AS_OPTION_MARCH,        AS_OPTION_HAS_ARG           },
    { "msyntax",    AS_OPTION_SYNTAX,       AS_OPTION_HAS_ARG           },
    { NULL,         0,                      0                           }

};

const struct as_option *machine_dependent_get_options (void) {
    return options;
}

void machine_dependent_print_help (void) {

    int i, cols;

    fprintf (stderr, "    -march=CPU[+EXTENSION]\n");
    fprintf (stderr, "                          Generate code for CPU and EXTENSION. CPU is one of:\n");
    fprintf (stderr, "                          ");
    for (i = 0, cols = 0; i < ARRAY_SIZE (cpu_archs); i++) {

        if (cols + strlen (cpu_archs[i].name) > strlen ("Generate code for CPU and EXTENSION. CPU is one of:")) {
            
            fprintf (stderr, "\n");
            fprintf (stderr, "                          ");
            cols = 0;
            
        }

        cols += fprintf (stderr, " %s%s", cpu_archs[i].name, (i + 1 != ARRAY_SIZE (cpu_archs)) ? "," : "");
        
    }
    fprintf (stderr, "\n");

    fprintf (stderr, "                          EXTENSION is combination of:\n");
    fprintf (stderr, "                          ");
    for (i = 0, cols = 0; i < ARRAY_SIZE (cpu_extensions); i++) {

        if (cols + strlen (cpu_extensions[i].name) > strlen ("Generate code for CPU and EXTENSION. CPU is one of:")) {
            
            fprintf (stderr, "\n");
            fprintf (stderr, "                          ");
            cols = 0;
            
        }

        cols += fprintf (stderr, " %s,", cpu_extensions[i].name);
        
    }

    for (i = 0; i < ARRAY_SIZE (cpu_no_extensions); i++) {

        if (cols + strlen (cpu_no_extensions[i].name) > strlen ("Generate code for CPU and EXTENSION. CPU is one of:")) {
            
            fprintf (stderr, "\n");
            fprintf (stderr, "                          ");
            cols = 0;
            
        }

        cols += fprintf (stderr, " %s%s",
                         cpu_no_extensions[i].name,
                         (i + 1 != ARRAY_SIZE (cpu_no_extensions)) ? "," : "");
        
    }
    fprintf (stderr, "\n");
    
    fprintf (stderr, "    -msyntax=[att|intel]  Use AT&T/Intel syntax (default: att)\n");
    fprintf (stderr, "    --16/--32/--64        Generate 16-bit/32-bit/64-bit object\n");

}

void machine_dependent_handle_option (const struct as_option *popt, const char *optarg) {

    char dummy[] = "prefix";
    char *dummy_p = dummy;

    switch (popt->index) {

        case AS_OPTION_BITS16:

            bits = 16;
            break;

        case AS_OPTION_BITS32:

            bits = 32;
            break;

        case AS_OPTION_BITS64:

            bits = 64;
            as_warn_at (NULL, 0, "64 bit support is unfinished and for testing only");
            break;

        case AS_OPTION_MARCH: {
        
            char *arg, *orig_arg;
            char *next;
            
            if (*optarg == '=') {
                ++optarg;
            }

            arg = orig_arg = xstrdup (optarg);
            if (cpu_extensions_name == NULL) {
                cpu_extensions_name = xstrdup ("");
            }

            if (*arg == '+') {
                ++arg;
            }

            while (1) {

                int i;
                char saved_c;

                next = strchr (arg, '+');
                if (next == NULL) {
                    next = arg + strlen (arg);
                }

                saved_c = *next;
                *next = '\0';

                for (i = 0; i < ARRAY_SIZE (cpu_archs); i++) {

                    if (strcmp (arg, cpu_archs[i].name) == 0) {

                        cpu_arch_flags = cpu_archs[i].cpu_flags;
                        free (cpu_arch_name);
                        cpu_arch_name = xstrdup (arg);
                        free (cpu_extensions_name);
                        cpu_extensions_name = xstrdup ("");
                        break;

                    }

                }

                if (i == ARRAY_SIZE (cpu_archs)) {

                    for (i = 0; i < ARRAY_SIZE (cpu_extensions); i++) {

                        if (strcmp (arg, cpu_extensions[i].name) == 0) {

                            cpu_arch_flags |= cpu_extensions[i].cpu_flags;
                            cpu_extensions_name = xrealloc (cpu_extensions_name,
                                                            strlen (cpu_extensions_name) + 1
                                                            + 1 + strlen (cpu_extensions[i].name));
                            strcat (cpu_extensions_name, ".");
                            strcat (cpu_extensions_name, cpu_extensions[i].name);
                            break;

                        }

                    }

                    if (i == ARRAY_SIZE (cpu_extensions)) {

                        for (i = 0; i < ARRAY_SIZE (cpu_no_extensions); i++) {

                            if (strcmp (arg, cpu_no_extensions[i].name) == 0) {

                                cpu_arch_flags &= ~cpu_no_extensions[i].cpu_flags;
                                cpu_extensions_name = xrealloc (cpu_extensions_name,
                                                            strlen (cpu_extensions_name) + 1
                                                            + 1 + strlen (cpu_no_extensions[i].name));
                                strcat (cpu_extensions_name, ".");
                                strcat (cpu_extensions_name, cpu_no_extensions[i].name);
                                break;

                            }

                        }

                        if (i == ARRAY_SIZE (cpu_no_extensions)) {
                            as_fatal_error_at (NULL, 0, "invalid -march= option: '%s'", optarg);
                        }

                    }

                }
                
                if (saved_c == '\0') {
                    break;
                }

                *next = saved_c;
                arg = next + 1;

            }

            free (orig_arg);

            break;
        
        }

        case AS_OPTION_SYNTAX:

            if (*optarg == '=') {
                optarg++;
            }

            if (xstrcasecmp (optarg, "att") == 0) {
                set_syntax (&dummy_p, 0);
            } else if (xstrcasecmp (optarg, "intel") == 0) {
                set_syntax (&dummy_p, 1);
            } else {
                _error ("invalid -msyntax= option '%s'", optarg);
            }

            break;

    }

}
