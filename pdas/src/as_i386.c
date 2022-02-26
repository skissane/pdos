/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "as.h"

#include "asi_i386.h"

/* Relaxation. */
#define RELAX_SUBTYPE_SHORT_JUMP 0x0
#define RELAX_SUBTYPE_LONG_JUMP 0x1
#define RELAX_SUBTYPE_UNCONDITIONAL_JUMP 0x0
#define RELAX_SUBTYPE_CONDITIONAL_JUMP 0x2

#define DISPLACEMENT_SIZE_FROM_RELAX_SUBTYPE(subtype) \
 ((subtype & RELAX_SUBTYPE_LONG_JUMP) ? 4 : 1)

typedef struct {
    long forward_reach;
    long backward_reach;
    long size_of_variable_part;
    relax_subtypeT next_subtype; /* 0 means no next entry. */
} relax_table_entryT;

relax_table_entryT relax_table[] = {
    /* Unconditional jumps. */
    {127 + 1, -128 + 1, 1, (RELAX_SUBTYPE_UNCONDITIONAL_JUMP
                            |RELAX_SUBTYPE_LONG_JUMP)},
    {0, 0, 4, 0},

    /* Conditional jumps. */
    {127 + 1, -128 + 1, 1, (RELAX_SUBTYPE_CONDITIONAL_JUMP
                            |RELAX_SUBTYPE_LONG_JUMP)},
    {0, 0, 5, 0}
};

typedef struct {
    template tm;

    char suffix;

    /* Number of given operands. */
    unsigned int operands;

    /* Number of given operands of specific type. */
    unsigned int reg_operands;
    unsigned int disp_operands;
    unsigned int mem_operands;

    unsigned int types[MAX_OPERANDS]; /* of operands. */

    /* Here are the actual operands stored. */
    /* regs for register operands. */ 
    const reg_entry *regs[MAX_OPERANDS];
    /* imms for immediate operands. */
    exprS *imms[MAX_OPERANDS];
    /* disps for displacement operands. */
    exprS *disps[MAX_OPERANDS];

    modrm_byte modrm;
    sib_byte sib;

    const reg_entry *base_reg;
    const reg_entry *index_reg;
    unsigned int log2_scale_factor;

    unsigned char prefix;
} i386_instruction;

static i386_instruction instruction;
static exprS operand_exprs[MAX_OPERANDS];

const template *find_template(const char *name)
{
    const template *t;

    for (t = template_table;
         t->name && (strcmp(t->name, name) != 0);
         t++) ;

    if (t->name == NULL) return (NULL);

    return (t);
}

const reg_entry *find_reg(const char *name)
{
    const reg_entry *reg;
    
    for (reg = reg_table;
         reg->name && (strcmp(reg->name, name) != 0);
         reg++) ;

    if (reg->name == NULL) return (NULL);

    return (reg);
}

int parse_operand(char *operand_string)
{
    if (*operand_string == REGISTER_PREFIX)
    {
        const reg_entry *reg;

        operand_string++;
        reg = find_reg(operand_string);

        if (reg == NULL)
        {
            as_error("Unknown register: `%s'\n", operand_string);
            return (1);
        }

        instruction.types[instruction.operands] = reg->type;
        instruction.regs[instruction.operands] = reg;
        instruction.operands++;
        instruction.reg_operands++;
    }
    else if (*operand_string == IMMEDIATE_PREFIX)
    {
        exprS *expr;

        operand_string++;

        /* Starts with the largest possible immediate and optimizes later. */
        instruction.types[instruction.operands] = Imm32;
        expr = &operand_exprs[instruction.operands];
        instruction.imms[instruction.operands] = expr;
        instruction.operands++;
        
        {
            char *saved_input_line_pointer = input_line_pointer;
            input_line_pointer = operand_string;

            expr_read_into(expr);

            input_line_pointer = saved_input_line_pointer;
        }
    }
    else
    {
        char *base_string;
        char *displacement_string_end = (operand_string
                                         + strlen(operand_string));

        /* Checks for a base index form (like -12(%ebp)). */
        base_string = operand_string + strlen(operand_string) - 1;
        
        if (*base_string == ')')
        {
            while (*base_string != '(')
            {
                base_string--;
                if (base_string < operand_string)
                {
                    as_error("+++parse_operand\n");
                    return (1);
                }
            }
            
            displacement_string_end = base_string;

            /* Moves past the '('. */
            base_string++;

            if (*base_string == REGISTER_PREFIX)
            {
                char *p;
                char saved_c;
            
                base_string++;

                for (p = base_string;
                     (*p != ',') && (*p != ')');
                     p++) ;

                saved_c = *p;
                *p = '\0';
                
                instruction.base_reg = find_reg(base_string);
                if (instruction.base_reg == NULL)
                {
                    as_error("Bad register name `%s'!\n", base_string);
                    *p = saved_c;
                    return (1);
                }

                *p = saved_c;

                instruction.types[instruction.operands] = BaseIndex;

                if (*p == ',')
                {
                    base_string = p;
                    base_string++;

                    while ((*base_string == ' ')
                           || (*base_string == '\t')) base_string++;

                    if (*base_string == REGISTER_PREFIX)
                    {
                        base_string++;
                        
                        for (p = base_string;
                             (*p != ',') && (*p != ')');
                             p++) ;
                        
                        saved_c = *p;
                        *p = '\0';
                        
                        instruction.index_reg = find_reg(base_string);
                        if (instruction.index_reg == NULL)
                        {
                            as_error("Bad register name `%s'!\n", base_string);
                            *p = saved_c;
                            return (1);
                        }

                        *p = saved_c;

                        base_string = p;
                    }

                    if (*base_string != ')')
                    {
                        exprS expr;
                        char *saved_input_line_pointer = input_line_pointer;

                        base_string++;

                        for (p = base_string; *p != ')'; p++) ;

                        input_line_pointer = base_string;
                        
                        saved_c = *p;
                        *p = '\0';

                        expr_read_into(&expr);

                        input_line_pointer = saved_input_line_pointer;

                        if (expr.type != Expr_type_constant)
                        {
                            as_error("+++parse operand\n");
                            return (1);
                        }

                        switch (expr.add_number)
                        {
                            case 0:
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
                                as_error("Expecting scale factor"
                                         " of 1, 2, 4 or 8: got `%s'.\n",
                                         base_string);
                                *p = saved_c;
                                return (1);
                        }

                        *p = saved_c;  
                    }
                }
            }
        }

        if (operand_string != displacement_string_end)
        {
            exprS *expr;
            unsigned int disp_type = Disp32;

            instruction.types[instruction.operands] |= disp_type;
            expr = &operand_exprs[instruction.operands];
            instruction.disps[instruction.operands] = expr;
            
            {
                char saved_c;
                char *saved_input_line_pointer = input_line_pointer;
                input_line_pointer = operand_string;

                saved_c = *displacement_string_end;
                *displacement_string_end = '\0';

                expr_read_into(expr);

                *displacement_string_end = saved_c;

                input_line_pointer = saved_input_line_pointer;
            }

            instruction.disp_operands++;
        }

        instruction.operands++;
        /* Displacement references memory. */
        instruction.mem_operands++;
    }

    return (0);
}

int fits_in_signed_byte(long number)
{
    return ((number >= -128) && (number <= 127));
}

int fits_in_unsigned_byte(long number)
{
    return ((number & 0xff) == number);
}

int fits_in_signed_word(long number)
{
    return ((number >= -32768) && (number <= 32767));
}

int fits_in_unsigned_word(long number)
{
    return ((number & 0xffff) == number);
}

unsigned int smallest_imm_type(long number)
{
    return (fits_in_signed_byte(number)
            ? (Imm8S | Imm8 | Imm16 | Imm32)
            : fits_in_unsigned_byte(number)
            ? (Imm8 | Imm16 | Imm32)
            : (fits_in_signed_word(number) || fits_in_unsigned_word(number))
            ? (Imm16 | Imm32)
            : Imm32);
}

void optimize_size_of_disps(void)
{
    int operand;

    for (operand = 0; operand < instruction.operands; operand++)
    {
        if ((instruction.types[operand] & Disp)
            && (instruction.disps[operand]->type == Expr_type_constant))
        {
            unsigned long disp = instruction.disps[operand]->add_number;
            
            if (instruction.types[operand] & Disp32)
            {
                /* This operand has maximally 32 bits,
                 * so it is converted to signed 32 bit number
                 * before checking whether it can be even smaller. */
                disp &= 0xFFFFFFFF;
                disp = (disp ^ (1UL << 31)) - (1UL << 31);
            }
            
            if ((instruction.types[operand] & (Disp16 | Disp32))
                && fits_in_signed_byte(disp))
            {
                instruction.types[operand] |= Disp8;
            }
        }
    }
}

void optimize_size_of_imms(void)
{
    int operand;

    for (operand = 0; operand < instruction.operands; operand++)
    {
        if ((instruction.types[operand] & Imm)
            && (instruction.imms[operand]->type == Expr_type_constant))
        {
            if (instruction.types[operand] & Imm32)
            {
                /* This operand has maximally 32 bits,
                 * so it is converted to signed 32 bit number
                 * before checking whether it can be even smaller. */
                instruction.imms[operand]->add_number &= 0xFFFFFFFF;
                (instruction.imms[operand]->add_number
                 = ((instruction.imms[operand]->add_number ^ (1UL << 31))
                    - (1UL << 31)));
            }
            
            (instruction.types[operand]
             |= smallest_imm_type(instruction.imms[operand]->add_number));
        }
    }
}

/* Finds the suitable template for the provided operands. */
int match_template(const template *t)
{
    const char *name = t->name;
    unsigned int found_reverse_match = 0;
    unsigned int suffix_check = ((instruction.suffix == BYTE_SUFFIX)
                                 ? No_bSuf
                                 : ((instruction.suffix == WORD_SUFFIX)
                                    ? No_wSuf
                                    : ((instruction.suffix == DWORD_SUFFIX)
                                       ? No_lSuf
                                       : 0)));

    for ( ; t->name && (strcmp(t->name, name) == 0); t++)
    {
        unsigned int operand_type_overlap0, operand_type_overlap1;
        
        if (instruction.operands != t->operands) continue;
        /* Checks if the suffix is valid for the template. */
        if (t->opcode_modifier & suffix_check) continue;

        if (instruction.operands == 0) break;

        operand_type_overlap0 = instruction.types[0] & t->operand_types[0];
        switch (t->operands)
        {
            case 1:
                if ((instruction.types[0] & BaseIndex)
                    != (operand_type_overlap0 & BaseIndex)) continue;
                if (operand_type_overlap0 == 0) continue;
                break;

            case 2:
                operand_type_overlap1 = (instruction.types[1]
                                         & t->operand_types[1]);

                if ((operand_type_overlap0 == 0)
                    || ((instruction.types[0] & BaseIndex)
                        != (operand_type_overlap0 & BaseIndex))
                    || (operand_type_overlap1 == 0)
                    || ((instruction.types[1] & BaseIndex)
                        != (operand_type_overlap1 & BaseIndex)))
                {
                    /* Checks if reverse direction is valid. */
                    if ((t->opcode_modifier & D) == 0) continue;

                    /* Tries reversing the direction. */
                    operand_type_overlap0 = (instruction.types[0]
                                             & t->operand_types[1]);
                    operand_type_overlap1 = (instruction.types[1]
                                             & t->operand_types[0]);
                    if ((operand_type_overlap0 == 0)
                        || ((instruction.types[0] & BaseIndex)
                            != (operand_type_overlap0 & BaseIndex))
                        || (operand_type_overlap1 == 0)
                        || ((instruction.types[1] & BaseIndex)
                            != (operand_type_overlap1 & BaseIndex))) continue;

                    /* Reverse match found. */
                    found_reverse_match = t->opcode_modifier & D;
                }
                break;
        }

        break;
    }

    if ((t->name == NULL) || strcmp(t->name, name))
    {
        as_error("Operands invalid for `%s'.\n", name);
        return (1);
    }

    /* Copies the found template. */
    instruction.tm = *t;
    if (found_reverse_match)
    {
        instruction.tm.base_opcode |= found_reverse_match;
        instruction.tm.operand_types[0] = t->operand_types[1];
        instruction.tm.operand_types[1] = t->operand_types[0];
    }

    return (0);
}

unsigned int modrm_mode_from_disp_size(unsigned int type)
{
    return ((type & Disp8) ? 1 : ((type & (Disp16 | Disp32)) ? 2 : 0));
}

unsigned long convert_number_to_size(unsigned long value, int size)
{
    unsigned long mask;

    switch (size)
    {
        case 1: mask = 0xff; break;
        case 2: mask = 0xffff; break;
        case 4: mask = 0xffffffff; break;
    }

    if ((value & ~mask) && ((value & ~mask) != ~mask))
    {
        as_error("%ld shortened to %ld.\n",
                 value, value & mask);
    }

    value &= mask;

    return (value);
}

void output_disps(void)
{
    int operand;

    for (operand = 0; operand < instruction.operands; operand++)
    {
        if (instruction.types[operand] & Disp)
        {
            if (instruction.disps[operand]->type == Expr_type_constant)
            {
                int size = 4;
                int i;
                unsigned long value;

                if (instruction.types[operand] & Disp8)
                {
                    size = 1;
                }
                if (instruction.types[operand] & Disp16)
                {
                    size = 2;
                }

                (value =
                 convert_number_to_size(instruction.disps[operand]
                                        ->add_number, size));
                
                for (i = 0; i < size; i++)
                {
                    frag_append_1_char((value >> (8 * i)) & 0xFF);
                }
            }
            else
            {
                int size = 4;

                if (instruction.types[operand] & Disp8)
                {
                    size = 1;
                }
                if (instruction.types[operand] & Disp16)
                {
                    size = 2;
                }
                
                fixup_new_expr(current_frag,
                               current_frag->fixed_size,
                               size,
                               instruction.disps[operand],
                               0 /* pcrel */);

                frag_increase_fixed_size(size);
            }
        }
    }
}

void output_imms(void)
{
    int operand;
    
    for (operand = 0; operand < instruction.operands; operand++)
    {
        if (instruction.types[operand] & Imm)
        {
            if (instruction.imms[operand]->type == Expr_type_constant)
            {
                int size = 4;
                int i;
                unsigned long value;

                if (instruction.types[operand] & (Imm8 | Imm8S))
                {
                    size = 1;
                }
                if (instruction.types[operand] & (Imm16))
                {
                    size = 2;
                }

                (value =
                 convert_number_to_size(instruction.imms[operand]
                                        ->add_number, size));
                
                for (i = 0; i < size; i++)
                {
                    frag_append_1_char((value >> (8 * i)) & 0xFF);
                }
            }
            else
            {
                int size = 4;

                if (instruction.types[operand] & (Imm8 | Imm8S))
                {
                    size = 1;
                }
                if (instruction.types[operand] & Imm16)
                {
                    size = 2;
                }
    
                fixup_new_expr(current_frag,
                               current_frag->fixed_size,
                               size,
                               instruction.imms[operand],
                               0 /* pcrel */);

                frag_increase_fixed_size(size);
            }
        }
    }
}

void machine_dependent_assemble_line(char *line)
{
    char *token_start;
    char saved_c;
    const template *t;
    
    memset(&instruction, '\0', sizeof(instruction));
    memset(operand_exprs, '\0', sizeof(operand_exprs));

    while ((*line == ' ') || (*line == '\t')) line++;
    
    token_start = line;
    while ((*line != ' ') && (*line != '\t') && (*line != '\0')) line++;

    /* Finds the base template for the instruction. */
    saved_c = *line; *line = '\0';
    t = find_template(token_start);
    *line = saved_c;
    
    if (t == NULL)
    {
        switch (line[-1])
        {
            case BYTE_SUFFIX:
            case WORD_SUFFIX:
            case DWORD_SUFFIX:
                instruction.suffix = line[-1];
                line[-1] = '\0';
                t = find_template(token_start);
                line[-1] = instruction.suffix;
                break;
        }

        if (t == NULL)
        {
            as_error("Unknown instruction: `%s'\n", token_start);
            return;
        }
    }

    /* Parses operands. */
    while (*line != '\0')
    {
        unsigned int paren_not_balanced;
        
        while ((*line == ' ') || (*line == '\t')) line++;

        token_start = line;
        paren_not_balanced = 0;
        while (paren_not_balanced || (*line != ','))
        {
            if (*line == '\0')
            {
                if (paren_not_balanced)
                {
                    as_error("Unbalanced parentheses in operand %u.\n",
                             instruction.operands);
                    return;
                }
                else break;
            }

            if (*line == '(') paren_not_balanced++;
            if (*line == ')') paren_not_balanced--;

            line++;
        }

        if (token_start != line)
        {
            int ret;
            
            saved_c = *line; *line = '\0';
            ret = parse_operand(token_start);
            *line = saved_c;

            /* Problem occured while processing the operand. */
            if (ret) return;
        }

        if (*line == ',') line++;
    }
    
    optimize_size_of_disps();
    optimize_size_of_imms();

    if (match_template(t)) return;

    /* +++suffix logic */
    if (instruction.suffix && (instruction.suffix != BYTE_SUFFIX))
    {
        /* Not a byte, selects word/dword operation. */
        if (instruction.tm.opcode_modifier & W)
        {
            if (instruction.tm.opcode_modifier & ShortForm)
            {
                instruction.tm.base_opcode |= 8;
            }
            else instruction.tm.base_opcode |= 1;
        }

        if (instruction.suffix == WORD_SUFFIX)
        {
            instruction.prefix = DATA_PREFIX_OPCODE;
        }
    }

    /* Selects the size of immediates. */
    {
        unsigned int overlap;

        overlap = instruction.types[0] & instruction.tm.operand_types[0];
        if ((overlap & Imm)
            && (overlap != Imm8) && (overlap != Imm8S)
            && (overlap != Imm16) && (overlap != Imm32))
        {
            if (instruction.suffix)
            {
                switch (instruction.suffix)
                {
                    case BYTE_SUFFIX: overlap &= Imm8 | Imm8S; break;
                    case WORD_SUFFIX: overlap &= Imm16; break;
                    case DWORD_SUFFIX: overlap &= Imm32; break;
                }
            }
        }
        instruction.types[0] = overlap;

        overlap = instruction.types[1] & instruction.tm.operand_types[1];
        if ((overlap & Imm)
            && (overlap != Imm8) && (overlap != Imm8S)
            && (overlap != Imm16) && (overlap != Imm32))
        {
            as_error("+++immediate\n");
        }
        instruction.types[1] = overlap;
    }
    
    if (instruction.tm.operand_types[0] & ImplicitRegister)
    {
        instruction.reg_operands--;
    }
    if (instruction.tm.operand_types[1] & ImplicitRegister)
    {
        instruction.reg_operands--;
    }
    
    /* Finalizes the opcode. */
    if (instruction.operands)
    {
        if (instruction.tm.opcode_modifier & ShortForm)
        {
            int operand = (instruction.types[0] & Reg) ? 0 : 1;

            instruction.tm.base_opcode |= instruction.regs[operand]->number;
        }

        if (instruction.tm.opcode_modifier & Modrm)
        {
            if (instruction.reg_operands == 2)
            {
                instruction.modrm.mode = 3;

                if (instruction.tm.operand_types[0] & AnyMem)
                {
                    instruction.modrm.regmem = instruction.regs[0]->number;
                    instruction.modrm.reg = instruction.regs[1]->number;
                }
                else
                {
                    instruction.modrm.regmem = instruction.regs[1]->number;
                    instruction.modrm.reg = instruction.regs[0]->number;
                }
            }
            else
            {
                if (instruction.mem_operands)
                {
                    int fake_zero_displacement = 0;
                    int operand = ((instruction.types[0] & AnyMem) ? 0
                                   : ((instruction.types[1] & AnyMem) ? 1
                                      : (as_error("+++modrm\n"), 0)));

                    if (instruction.base_reg == NULL)
                    {
                        instruction.modrm.mode = 0;
                        instruction.modrm.regmem = 5;
                        instruction.types[operand] &= ~Disp;
                        instruction.types[operand] |= Disp32;
                    }
                    else if (instruction.base_reg->type & Reg16)
                    {
                        as_error("+++machine_dependent_assemble_line modrm\n");
                    }
                    else /* 32 bit base register. */
                    {
                        instruction.modrm.regmem = instruction.base_reg->number;
                        instruction.sib.base = instruction.base_reg->number;
                        instruction.sib.scale = instruction.log2_scale_factor;
                        
                        if (instruction.base_reg->number == EBP_REGISTER_NUMBER)
                        {
                            if (instruction.disp_operands == 0)
                            {
                                fake_zero_displacement = 1;
                                instruction.types[operand] |= Disp8;
                            }
                        }

                        if (instruction.index_reg)
                        {
                            (instruction.sib.index
                             = instruction.index_reg->number);
                            (instruction.modrm.regmem
                             = MODRM_REGMEM_TWO_BYTE_ADDRESSING);
                        }
                        else
                        {
                            (instruction.sib.index
                             = SIB_INDEX_NO_INDEX_REGISTER);
                            if (instruction.log2_scale_factor)
                            {
                                (instruction.modrm.regmem
                                 = MODRM_REGMEM_TWO_BYTE_ADDRESSING);
                            }
                        }

                        (instruction.modrm.mode
                         = modrm_mode_from_disp_size(instruction.types[operand]));
                    }

                    if (fake_zero_displacement)
                    {
                        /* Fakes a zero displacement.
                         * instruction.types[operand] must be holding
                         * the size of displacement. */
                        exprS *expr;

                        expr = &operand_exprs[operand];
                        instruction.disps[operand] = expr;

                        expr->type = Expr_type_constant;
                        expr->add_number = 0;
                        expr->add_symbol = NULL;
                    }
                }

                if (instruction.reg_operands)
                {
                    int operand = ((instruction.types[0] & Reg) ? 0
                                   : ((instruction.types[1] & Reg) ? 1
                                      : (as_error("+++modrm\n"), 0)));
                    
                    /* If extension opcode is used,
                     * the register goes into regmem field. */
                    if (instruction.tm.extension_opcode != None)
                    {
                        (instruction.modrm.regmem
                         = instruction.regs[operand]->number);
                    }
                    else
                    {
                        (instruction.modrm.reg
                         = instruction.regs[operand]->number);
                    }

                    /* If mode field has not been set by memory operand,
                     * it must be set to 3 to indicate register operand. */
                    if (instruction.mem_operands == 0)
                    {
                        instruction.modrm.mode = 3;
                    }
                }

                /* Extension opcode goes into reg field. */
                if (instruction.tm.extension_opcode != None)
                {
                    instruction.modrm.reg = instruction.tm.extension_opcode;
                }
            }
        }
    }
    
    /* Outputs the instruction. */
    {
        if (instruction.tm.opcode_modifier & Jump)
        {
            relax_subtypeT relax_subtype;
            symbolS *symbol;
            unsigned long offset;
            unsigned long opcode_offset_in_buf;
            
            /* Reserves space for the largest instruction
             * that might be generated
             * (2 opcode bytes and 4 byte displacement). */
            frag_alloc_space(2 + 4);

            opcode_offset_in_buf = current_frag->fixed_size;
            frag_append_1_char(instruction.tm.base_opcode);

            if (instruction.tm.base_opcode == PC_RELATIVE_JUMP)
            {
                relax_subtype = (RELAX_SUBTYPE_UNCONDITIONAL_JUMP
                                 | RELAX_SUBTYPE_SHORT_JUMP);
            }
            else
            {
                relax_subtype = (RELAX_SUBTYPE_CONDITIONAL_JUMP
                                 | RELAX_SUBTYPE_SHORT_JUMP);
            }

            symbol = instruction.disps[0]->add_symbol;
            offset = instruction.disps[0]->add_number;

            if ((instruction.disps[0]->type != Expr_type_constant)
                && (instruction.disps[0]->type != Expr_type_symbol))
            {
                as_error("+++assemble output jump\n");
            }
            
            frag_set_as_variant(relax_type_machine_dependent,
                                relax_subtype,
                                symbol,
                                offset,
                                opcode_offset_in_buf);
        }
        else if (instruction.tm.opcode_modifier & Call)
        {
            int size = 4;
            
            frag_append_1_char(instruction.tm.base_opcode);
            
            fixup_new_expr(current_frag,
                           current_frag->fixed_size,
                           size,
                           instruction.disps[0],
                           1 /* pcrel */);

            frag_increase_fixed_size(size);
        }
        else
        {
            int operand;

            if (instruction.prefix) frag_append_1_char(instruction.prefix);

            /* High byte of the opcode goes first. */
            if (instruction.tm.base_opcode & 0xFF00)
            {
                frag_append_1_char((instruction.tm.base_opcode >> 8) & 0xFF);
            }
            
            frag_append_1_char(instruction.tm.base_opcode & 0xFF);

            if (instruction.tm.opcode_modifier & Modrm)
            {
                frag_append_1_char(((instruction.modrm.regmem << 0)
                                    | (instruction.modrm.reg << 3)
                                    | (instruction.modrm.mode << 6)));
                if ((instruction.modrm.regmem
                     == MODRM_REGMEM_TWO_BYTE_ADDRESSING)
                    && (instruction.modrm.mode != 3))
                {
                    frag_append_1_char(((instruction.sib.base << 0)
                                        | (instruction.sib.index << 3)
                                        | (instruction.sib.scale << 6)));
                }
            }

            /* Displacements go first. */
            output_disps();
            output_imms();
        }
    }
}

long machine_dependent_pcrel_from(fixupS *fixup)
{
    return (fixup->size + fixup->where + fixup->frag->address);
}

void machine_dependent_apply_fixup(fixupS *fixup,
                                   unsigned long value)
{
    unsigned char *p = fixup->where + fixup->frag->buf;

    if (fixup->add_symbol == NULL)
    {
        /* We are finished with this fixup and no relocation is needed. */
        fixup->done = 1;
    }

#ifdef TARGET_OBJECT_FILE_COFF
    /* +++Explain why is this necessary. */
    if (fixup->pcrel
        && fixup->add_symbol && symbol_is_undefined(fixup->add_symbol))
    {
        value += machine_dependent_pcrel_from(fixup);
    }
#endif

    {
        int i;
        
        for (i = 0; i < fixup->size; i++)
        {
            p[i] = (value >> (8 * i)) & 0xFF;
        }
    }
}

long machine_dependent_estimate_size_before_relax(fragS *frag,
                                                  sectionT section)
{
    if (symbol_get_section(frag->symbol) != section)
    {
        /* Jump cannot be relaxed because the target is undefined
         * or in other section. Largest possible jump is used. */
        int size = 4;
        unsigned char *opcode_pos;
        unsigned long old_frag_fixed_size;
        
        opcode_pos = frag->buf + frag->opcode_offset_in_buf;
        old_frag_fixed_size = frag->fixed_size;

        switch (frag->relax_subtype
                & ~(RELAX_SUBTYPE_LONG_JUMP | RELAX_SUBTYPE_SHORT_JUMP))
        {
            case RELAX_SUBTYPE_UNCONDITIONAL_JUMP:
                *opcode_pos = 0xE9;
                fixup_new(frag, frag->fixed_size, size,
                          frag->symbol, frag->offset, 1 /* pcrel */);
                frag->fixed_size += size;
                break;

            case RELAX_SUBTYPE_CONDITIONAL_JUMP:
                opcode_pos[1] = opcode_pos[0] + 0x10;
                opcode_pos[0] = TWOBYTE_OPCODE;
                fixup_new(frag, frag->fixed_size + 1, size,
                          frag->symbol, frag->offset, 1 /* pcrel */);
                frag->fixed_size += size + 1;
                break;

            default:
                as_error("+++machine_dependent_estimate_size_before_relax\n");
                return;
        }

        frag->relax_type = relax_type_none_needed;

        return (frag->fixed_size - old_frag_fixed_size);
    }

    return (relax_table[frag->relax_subtype].size_of_variable_part);
}

long machine_dependent_relax_frag(fragS *frag, sectionT section, long change)
{
    unsigned long target;
    long aim;
    relax_subtypeT new_subtype;
    long growth;

    target = frag->offset;
    if (frag->symbol)
    {
        target += symbol_get_value(frag->symbol);

        /* The symbol targeted by this jump might have moved
         * during the relaxation. */
        if (section == symbol_get_section(frag->symbol))
        {
            target += change;
        }
    }
    
    aim = target - frag->address - frag->fixed_size;
    /* +++Not sure if this is correct,
     * but the same logic is used in machine_dependent_finish_frag. */
    aim -= relax_table[frag->relax_subtype].size_of_variable_part;

    if (aim > 0)
    {
        for (new_subtype = frag->relax_subtype;
             relax_table[new_subtype].next_subtype;
             new_subtype = relax_table[new_subtype].next_subtype)
        {
            if (aim <= relax_table[new_subtype].forward_reach) break;
        }
    }
    else if (aim < 0)
    {
        for (new_subtype = frag->relax_subtype;
             relax_table[new_subtype].next_subtype;
             new_subtype = relax_table[new_subtype].next_subtype)
        {          
            if (aim >= relax_table[new_subtype].backward_reach) break;
        }
    }
    else return (0);

    growth = (relax_table[new_subtype].size_of_variable_part
              - relax_table[frag->relax_subtype].size_of_variable_part);
    if (growth) frag->relax_subtype = new_subtype;

    return (growth);
}

void machine_dependent_finish_frag(fragS *frag)
{
    unsigned char *opcode_pos, *displacement_pos;
    int extension;
    long displacement;
    
    opcode_pos = frag->buf + frag->opcode_offset_in_buf;

    displacement = (symbol_get_value(frag->symbol)
                    + frag->offset
                    - frag->address
                    - frag->fixed_size);

    if ((frag->relax_subtype & RELAX_SUBTYPE_LONG_JUMP) == 0)
    {
        /* Original opcode does not need to be changed. */
        extension = 1;
        displacement_pos = opcode_pos + 1;
    }
    else
    {
        switch (frag->relax_subtype)
        {
            case (RELAX_SUBTYPE_UNCONDITIONAL_JUMP | RELAX_SUBTYPE_LONG_JUMP):
                *opcode_pos = 0xE9;
                extension = 4;
                displacement_pos = opcode_pos + 1;
                break;

            case (RELAX_SUBTYPE_CONDITIONAL_JUMP | RELAX_SUBTYPE_LONG_JUMP):
                opcode_pos[1] = opcode_pos[0] + 0x10;
                opcode_pos[0] = TWOBYTE_OPCODE;
                extension = 5;
                displacement_pos = opcode_pos + 2;
                break;

            default:
                as_error("+++machine_dependent_finish_frag\n");
                return;
        }
    }

    displacement -= extension;

    {
        int i;

        for (i = 0;
             i < DISPLACEMENT_SIZE_FROM_RELAX_SUBTYPE(frag->relax_subtype);
             i++)
        {
            displacement_pos[i] = (displacement >> (8 * i)) & 0xFF;
        }
    }
    frag->fixed_size += extension;
}
