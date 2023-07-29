/******************************************************************************
 * @file            i386_as_intel_support.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
static struct {

    enum expr_type operand_modifier;

    int is_mem;
    int is_indirect;
    int has_offset;

    int in_offset;
    int in_bracket;
    int in_scale;

    const struct reg_entry *base_reg;
    const struct reg_entry *index_reg;
    offset_t scale_factor;

    struct symbol *segment;

} intel_state;

#define EXPR_TYPE_SHORT     EXPR_TYPE_MACHINE_DEPENDENT_0
#define EXPR_TYPE_OFFSET    EXPR_TYPE_MACHINE_DEPENDENT_1
#define EXPR_TYPE_FULL_PTR  EXPR_TYPE_MACHINE_DEPENDENT_2

#define EXPR_TYPE_NEAR_PTR  EXPR_TYPE_MACHINE_DEPENDENT_3
#define EXPR_TYPE_FAR_PTR   EXPR_TYPE_MACHINE_DEPENDENT_4

#define EXPR_TYPE_BYTE_PTR  EXPR_TYPE_MACHINE_DEPENDENT_5
#define EXPR_TYPE_WORD_PTR  EXPR_TYPE_MACHINE_DEPENDENT_6
#define EXPR_TYPE_DWORD_PTR EXPR_TYPE_MACHINE_DEPENDENT_7
#define EXPR_TYPE_FWORD_PTR EXPR_TYPE_MACHINE_DEPENDENT_8
#define EXPR_TYPE_QWORD_PTR EXPR_TYPE_MACHINE_DEPENDENT_9
#define EXPR_TYPE_TBYTE_PTR EXPR_TYPE_MACHINE_DEPENDENT_10

#define EXPR_TYPE_XMMWORD_PTR EXPR_TYPE_MACHINE_DEPENDENT_11

struct intel_type {

    const char *name;
    enum expr_type expr_type;
    unsigned int size[2];

};

static const struct intel_type intel_types[] = {

#define INTEL_TYPE(name, size) { #name, EXPR_TYPE_##name##_PTR, {size, size}}

    INTEL_TYPE (BYTE, 1),
    INTEL_TYPE (WORD, 2),
    INTEL_TYPE (DWORD, 4),
    INTEL_TYPE (FWORD, 6),
    INTEL_TYPE (QWORD, 8),
    INTEL_TYPE (TBYTE, 8),
    INTEL_TYPE (XMMWORD, 16),

#undef INTEL_TYPE

    {"near", EXPR_TYPE_NEAR_PTR, {0xFF02, 0xFF04}},
    {"far", EXPR_TYPE_FAR_PTR, {0xFF05, 0xFF06}},
    {NULL, EXPR_TYPE_INVALID, {0, 0}}

};

struct intel_operator {

    const char *name;
    enum expr_type expr_type;
    unsigned int operands;

};

static const struct intel_operator intel_operators[] = {

    { "and", EXPR_TYPE_BIT_AND, 2},
    { "eq", EXPR_TYPE_EQUAL, 2},
    { "ge", EXPR_TYPE_GREATER_EQUAL, 2},
    { "gt", EXPR_TYPE_GREATER, 2},
    { "le", EXPR_TYPE_LESSER_EQUAL, 2},
    { "lt", EXPR_TYPE_LESSER, 2},
    { "mod", EXPR_TYPE_MODULUS, 2},
    { "ne", EXPR_TYPE_NOT_EQUAL, 2},
    { "not", EXPR_TYPE_BIT_NOT, 1},
    { "offset", EXPR_TYPE_OFFSET, 1},
    { "or", EXPR_TYPE_BIT_INCLUSIVE_OR, 2},
    { "shl", EXPR_TYPE_LEFT_SHIFT, 2},
    { "shr", EXPR_TYPE_RIGHT_SHIFT, 2},
    { "short", EXPR_TYPE_SHORT, 1 },
    { "xor", EXPR_TYPE_BIT_EXCLUSIVE_OR, 2},

    { NULL, EXPR_TYPE_INVALID, 0 }

};

static int intel_parse_name (struct expr *expr, char *name)
{
    int i;

    if (strcmp (name, "$") == 0) {
        current_location (expr);
        return 1;
    }

    for (i = 0; intel_types[i].name; i++) {
        if (xstrcasecmp (name, intel_types[i].name) == 0) {
            expr->type = EXPR_TYPE_CONSTANT;
            expr->add_symbol = NULL;
            expr->op_symbol = NULL;
            expr->add_number = intel_types[i].size[(bits == 16) ? 0 : 1];

            return 1;
        }
    }

    return 0;
}

enum expr_type machine_dependent_parse_operator (char **pp, char *name, char *original_saved_c, unsigned int operands)
{
    size_t i;

    if (!intel_syntax) return EXPR_TYPE_ABSENT;

    if (name == NULL) {
        if (operands != 2) return EXPR_TYPE_INVALID;

        switch (**pp) {

            case ':':
                ++*pp;
                return EXPR_TYPE_FULL_PTR;

            case '[':
                ++*pp;
                return EXPR_TYPE_INDEX;

        }

        return EXPR_TYPE_INVALID;
    }

    for (i = 0; intel_types[i].name; i++) {
        if (xstrcasecmp (name, intel_types[i].name) == 0) {
            break;
        }
    }

    if (intel_types[i].name && *original_saved_c == ' ') {
        char *second_name;
        char c;

        ++*pp;
        second_name = *pp;
        c = get_symbol_name_end (pp);

        if (xstrcasecmp (second_name, "ptr") == 0) {
            second_name[-1] = *original_saved_c;
            *original_saved_c = c;

            return intel_types[i].expr_type;
        }

        **pp = c;
        *pp = second_name - 1;

        return EXPR_TYPE_ABSENT;
    }

    for (i = 0; intel_operators[i].name; i++) {
        if (xstrcasecmp (name, intel_operators[i].name) == 0) {
            if (operands != intel_operators[i].operands) {
                return EXPR_TYPE_INVALID;
            }

            return intel_operators[i].expr_type;
        }
    }
    
    return EXPR_TYPE_ABSENT;
}

static int intel_simplify_expr (struct expr *expr);

static int intel_simplify_symbol (struct symbol *symbol)
{
    int ret;

    ret = intel_simplify_expr (symbol_get_value_expression (symbol));

    if (ret == 2) {
        symbol_set_section (symbol, absolute_section);
        ret = 1;
    }

    return ret;
}

static void intel_fold_symbol_into_expr (struct expr *expr, struct symbol *symbol)
{
    struct expr *symbol_expr;

    symbol_expr = symbol_get_value_expression (symbol);

    if (symbol_get_section (symbol) == absolute_section) {
        offset_t add_number;

        add_number = expr->add_number;

        *expr = *symbol_expr;
        expr->add_number += add_number;
    } else {

        expr->type = EXPR_TYPE_SYMBOL;
        expr->add_symbol = symbol;
        expr->op_symbol = NULL;

    }
}
    
static int intel_process_register_expr (struct expr *expr)
{
    int reg_num;

    reg_num = expr->add_number;

    if (intel_state.in_offset || instruction.operands < 0) {
        as_error ("invalid use of register");
        return 0;
    }

    if (!intel_state.in_bracket) {
        if (instruction.regs[instruction.operands]) {
            as_error ("invalid use of register");
            return 0;
        }

        if (reg_table[reg_num].type.segment1
            && reg_table[reg_num].number == REG_FLAT_NUMBER) {
            as_error ("invalid use of pseudo-register");
            return 0;
        }

        instruction.regs[instruction.operands] = reg_table + reg_num;
    } else if (!intel_state.base_reg && !intel_state.in_scale) {
        intel_state.base_reg = reg_table + reg_num;
    } else if (!intel_state.index_reg) {
        intel_state.index_reg = reg_table + reg_num;
    } else{
        intel_state.index_reg = reg_esp;
    }

    return 2;
}

static int intel_simplify_expr (struct expr *expr)
{
    int ret;

    switch (expr->type) {

        case EXPR_TYPE_INDEX:
            if (expr->add_symbol) {
                if (!intel_simplify_symbol (expr->add_symbol)) {
                    return 0;
                }
            }

            if (!intel_state.in_offset) {
                intel_state.in_bracket++;
            }
            ret = intel_simplify_symbol (expr->op_symbol);
            if (!intel_state.in_offset) {
                intel_state.in_bracket--;
            }
            
            if (!ret) return 0;

            if (expr->add_symbol) {
                expr->type = EXPR_TYPE_ADD;
            } else {
                intel_fold_symbol_into_expr (expr, expr->op_symbol);
            }
            break;

        case EXPR_TYPE_OFFSET:
            intel_state.has_offset = 1;
            
            intel_state.in_offset++;
            ret = intel_simplify_symbol (expr->add_symbol);
            intel_state.in_offset--;

            if (!ret) return 0;

            intel_fold_symbol_into_expr (expr, expr->add_symbol);
            return ret;

        case EXPR_TYPE_MULTIPLY:
            if (intel_state.in_bracket) {
                struct expr *scale_expr = NULL;

                if (!intel_state.in_scale++) {
                    intel_state.scale_factor = 1;
                }

                ret = intel_simplify_symbol (expr->add_symbol);
                if (ret && intel_state.index_reg) {
                    scale_expr = symbol_get_value_expression (expr->op_symbol);
                }

                if (ret) {
                    ret = intel_simplify_symbol (expr->op_symbol);
                }

                if (ret && !scale_expr && intel_state.index_reg) {
                    scale_expr = symbol_get_value_expression (expr->add_symbol);
                }

                if (ret && scale_expr) {
                    resolve_expression (scale_expr);
                    if (scale_expr->type != EXPR_TYPE_CONSTANT) {
                        scale_expr->add_number = 0;
                    }
                    intel_state.scale_factor *= scale_expr->add_number;
                }

                intel_state.in_scale--;
                if (!ret) return 0;

                if (!intel_state.in_scale) {
                    switch (intel_state.scale_factor) {

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
                            /* %esp is invalid as index register. */
                            intel_state.index_reg = reg_esp;
                            break;

                    }
                }

                break;
            }

            goto default_;

        case EXPR_TYPE_SHORT:
            instruction.force_short_jump = 1;
            goto ptr_after_setting_operand_modifier;

        case EXPR_TYPE_BYTE_PTR:
        case EXPR_TYPE_WORD_PTR:
        case EXPR_TYPE_DWORD_PTR:
        case EXPR_TYPE_FWORD_PTR:
        case EXPR_TYPE_QWORD_PTR:
        case EXPR_TYPE_TBYTE_PTR:
        case EXPR_TYPE_XMMWORD_PTR:
        case EXPR_TYPE_NEAR_PTR:
        case EXPR_TYPE_FAR_PTR:
            if (intel_state.operand_modifier == EXPR_TYPE_ABSENT) {
                intel_state.operand_modifier = expr->type;
            }

        ptr_after_setting_operand_modifier:
            if (symbol_get_value_expression (expr->add_symbol)->type == EXPR_TYPE_REGISTER) {
                as_error ("invalid use of register");
                return 0;
            }
            
            if (!intel_simplify_symbol (expr->add_symbol)) {
                return 0;
            }

            intel_fold_symbol_into_expr (expr, expr->add_symbol);
            break;

        case EXPR_TYPE_FULL_PTR:
            if (symbol_get_value_expression (expr->op_symbol)->type == EXPR_TYPE_REGISTER) {
                as_error ("invalid use of register");
                return 0;
            }

            if (!intel_simplify_symbol (expr->op_symbol)) {
                return 0;
            }

            if (!intel_state.in_offset) {
                if (!intel_state.segment) {
                    intel_state.segment = expr->add_symbol;
                } else {
                    struct expr temp_expr = {0};

                    temp_expr.type = EXPR_TYPE_FULL_PTR;
                    temp_expr.add_symbol = expr->add_symbol;
                    temp_expr.op_symbol = intel_state.segment;
                    
                    intel_state.segment = make_expr_symbol (&temp_expr);
                }
            }

            intel_fold_symbol_into_expr (expr, expr->op_symbol);
            break;

        case EXPR_TYPE_REGISTER:
            ret = intel_process_register_expr (expr);
            if (ret == 2) {
                expr->type = EXPR_TYPE_CONSTANT;
                expr->add_number = 0;
            }
            return ret;

        default_:
        default:
            if (expr->add_symbol && !intel_simplify_symbol (expr->add_symbol)) {
                return 0;
            }
            if (expr->op_symbol && !intel_simplify_symbol (expr->op_symbol)) {
                return 0;
            }
            break;

    }

    if (expr->type == EXPR_TYPE_SYMBOL
        && !intel_state.in_offset) {
        section_t section;

        section = symbol_get_section (expr->add_symbol);

        if (section != absolute_section && section != expr_section && section != reg_section) {
            intel_state.is_mem |= 2 - !intel_state.in_bracket;
        }
    }   

    return 1;
}

static int intel_parse_operand (char *operand_string)
{
    int ret;
    struct expr expr_buf, *expr;
    char *operand_start;

    memset (&intel_state, 0, sizeof (intel_state));
    intel_state.operand_modifier = EXPR_TYPE_ABSENT;

    expr = &expr_buf;
    operand_start = operand_string;

    intel_syntax = -1;
    expression_read_into (&operand_string, expr);
    ret = intel_simplify_expr (expr);
    intel_syntax = 1;

    operand_string = skip_whitespace (operand_string);
    if (*operand_string) {
        as_error ("junk '%s' after expression", operand_string);
        return 1;
    } else if (!intel_state.has_offset
               && operand_string > operand_start
               && strrchr (operand_start, ']')
               && skip_whitespace (strrchr (operand_start, ']') + 1) == operand_string) {
        intel_state.is_mem |= 1;
        intel_state.is_indirect = 1;
    }

    if (!ret) return 1;

    ret = 0;

    if (intel_state.operand_modifier != EXPR_TYPE_ABSENT
        && current_templates->start->base_opcode != 0x8D /* lea */) {
        char suffix = 0;

        switch (intel_state.operand_modifier) {

            case EXPR_TYPE_BYTE_PTR:
                instruction.types[instruction.operands].byte = 1;
                suffix = BYTE_SUFFIX;
                break;

            case EXPR_TYPE_WORD_PTR:
                instruction.types[instruction.operands].word = 1;
                if (intel_float_suffix_translation (current_templates->name) == 2) {
                    suffix = SHORT_SUFFIX;
                } else {
                    suffix = WORD_SUFFIX;
                }
               break;

            case EXPR_TYPE_DWORD_PTR:
                instruction.types[instruction.operands].dword = 1;
                if ((current_templates->name[0] == 'l'
                     && current_templates->name[1]
                     && current_templates->name[2] == 's'
                     && current_templates->name[3] == '\0')
                    || current_templates->start->base_opcode == 0x62 /* bound */) {
                    suffix = WORD_SUFFIX;
                } else if (bits != 32
                    && (current_templates->start->opcode_modifier.jump
                        || current_templates->start->opcode_modifier.call)) {
                    suffix = bits == 16 ? INTEL_SUFFIX : WORD_SUFFIX;
                } else if (intel_float_suffix_translation (current_templates->name) == 1) {
                    suffix = SHORT_SUFFIX;
                } else {
                    suffix = DWORD_SUFFIX;
                }
                break;

            case EXPR_TYPE_FWORD_PTR:
                instruction.types[instruction.operands].fword = 1;
                /* lgdt, lidt, sgdt, sidt accept fword ptr but ignore it. */
                if ((current_templates->name[0] == 'l' || current_templates->name[0] == 's')
                    && (current_templates->name[1] == 'g' || current_templates->name[1] == 'i')
                    && current_templates->name[2] == 'd'
                    && current_templates->name[3] == 't'
                    && current_templates->name[4] == '\0') {
                    break;
                }

                if (current_templates->name[0] == 'l'
                    && current_templates->name[1]
                    && current_templates->name[2] == 's'
                    && current_templates->name[3] == '\0') {
                    suffix = DWORD_SUFFIX;
                } else if (!intel_float_suffix_translation (current_templates->name)) {
                    if (bits == 16) {
                        add_prefix (DATA_PREFIX_OPCODE);
                    }
                    
                    suffix = INTEL_SUFFIX;
                }
                break;

            case EXPR_TYPE_QWORD_PTR:
                instruction.types[instruction.operands].qword = 1;
                if (current_templates->start->base_opcode == 0x62 /* bound */
                    || intel_float_suffix_translation (current_templates->name) == 1) {
                    suffix = DWORD_SUFFIX;
                } else {
                    suffix = QWORD_SUFFIX;
                }
                break;

            case EXPR_TYPE_TBYTE_PTR:
                instruction.types[instruction.operands].tbyte = 1;
                if (intel_float_suffix_translation (current_templates->name) == 1) {
                    suffix = INTEL_SUFFIX;
                }
                break;

            case EXPR_TYPE_XMMWORD_PTR:
                instruction.types[instruction.operands].xmmword = 1;
                break;

            case EXPR_TYPE_FAR_PTR:
                suffix = INTEL_SUFFIX;
                break;
            
            default:
                break;

        }

        if (!instruction.suffix) {
            instruction.suffix = suffix;
        } else if (instruction.suffix != suffix) {
            as_error ("conficting operand size modifiers");
            return 1;
        }
    }

    if (current_templates->start->opcode_modifier.jump
        || current_templates->start->opcode_modifier.call
        || current_templates->start->opcode_modifier.jumpintersegment) {
        int is_absolute_jump = 0;

        if (instruction.regs[instruction.operands]
            || intel_state.base_reg
            || intel_state.index_reg
            || intel_state.is_mem > 1) {
            is_absolute_jump = 1;
        } else {
            switch (intel_state.operand_modifier) {

                case EXPR_TYPE_NEAR_PTR:
                    if (intel_state.segment) {
                        is_absolute_jump = 1;
                    } else {
                        intel_state.is_mem = 1;
                    }
                    break;

                case EXPR_TYPE_FAR_PTR:
                case EXPR_TYPE_ABSENT:
                    if (!intel_state.segment) {
                        intel_state.is_mem = 1;
                        if (intel_state.operand_modifier == EXPR_TYPE_ABSENT) {
                            if (intel_state.is_indirect) {
                                is_absolute_jump = 1;
                            }
                            break; 
                        }

                        as_error ("cannot infer the segment part of the operand");
                        return 1;
                    } else if (symbol_get_section (intel_state.segment) == reg_section) {
                        is_absolute_jump = 1;
                    } else {
                        /* Something like "jmp 12:34" must be converted into "jmp 12, 34". */
                        instruction.imms[instruction.operands] = &operand_exprs[operand_exprs_count++];
                        memset (instruction.imms[instruction.operands], 0, sizeof (*instruction.imms[instruction.operands]));
                        instruction.imms[instruction.operands]->type = EXPR_TYPE_SYMBOL;
                        instruction.imms[instruction.operands]->add_symbol = intel_state.segment;
                        resolve_expression (instruction.imms[instruction.operands]);
                        
                        if (finalize_immediate (instruction.imms[instruction.operands], operand_start)) {
                            return 1;
                        }

                        instruction.operands++;

                        if (instruction.suffix == INTEL_SUFFIX) {
                            instruction.suffix = 0;
                        }
                        intel_state.segment = NULL;
                        intel_state.is_mem = 0;

                    }
                    break;

                default:
                    is_absolute_jump = 1;
                    break;

            }
        }

        if (is_absolute_jump) {
            instruction.types[instruction.operands].jump_absolute = 1;
            intel_state.is_mem |= 1;
        }
    }

    if (instruction.regs[instruction.operands]) {
        instruction.types[instruction.operands] = instruction.regs[instruction.operands]->type;
        instruction.types[instruction.operands].base_index = 0;
        instruction.reg_operands++;
    } else if (intel_state.base_reg
               || intel_state.index_reg
               || intel_state.segment
               || intel_state.is_mem) {

        if (instruction.mem_operands >= 1) {
            /* Handles "call 0x9090, 0x9090", "lcall 0x9090, 0x9090",
             * "jmp 0x9090, 0x9090", "ljmp 0x9090, 0x9090". */

            if ((current_templates->start->opcode_modifier.jump
                 || current_templates->start->opcode_modifier.call
                 || current_templates->start->opcode_modifier.jumpintersegment)
                && instruction.operands == 1
                && instruction.mem_operands == 1
                && instruction.disp_operands == 1
                && intel_state.segment == NULL
                && intel_state.operand_modifier == EXPR_TYPE_ABSENT) {
                    
                instruction.operands = 0;
                if (!finalize_immediate (instruction.disps[instruction.operands], NULL)) {
                    instruction.imms[instruction.operands] = instruction.disps[instruction.operands];
                    
                    instruction.operands = 1;
                    operand_exprs[operand_exprs_count] = *expr;
                    instruction.imms[instruction.operands] = &operand_exprs[operand_exprs_count++];
                    resolve_expression (instruction.imms[instruction.operands]);

                    if (!finalize_immediate (instruction.imms[instruction.operands], operand_start)) {
                        instruction.mem_operands = 0;
                        instruction.disp_operands = 0;
                        instruction.operands = 2;

                        instruction.types[0] = operand_type_and_not_disp (instruction.types[0]);

                        return 0;
                    }
                }
            }

            as_error ("too many memory references for '%s'", current_templates->name);
            return 1;
        }

        if (intel_state.base_reg
            && intel_state.index_reg
            && intel_state.base_reg->type.word
            && intel_state.index_reg->type.word
            && intel_state.base_reg->number >= 6
            && intel_state.index_reg->number < 6) {
            /* Converts [si + bp] to [bp + si] as addition is commutative
             * but other code accepts only (%bp,%si), not (%si,%bp). */
            instruction.base_reg = intel_state.index_reg;
            instruction.index_reg = intel_state.base_reg;
        } else {
            instruction.base_reg = intel_state.base_reg;
            instruction.index_reg = intel_state.index_reg;
        }

        if (instruction.base_reg || instruction.index_reg) {
            instruction.types[instruction.operands].base_index = 1;
        }

        operand_exprs[operand_exprs_count] = *expr;
        expr = &operand_exprs[operand_exprs_count++];
        
        resolve_expression (expr);

        if (expr->type != EXPR_TYPE_CONSTANT
            || expr->add_number
            || !instruction.types[instruction.operands].base_index) {
            instruction.disps[instruction.operands] = expr;
            instruction.disp_operands++;

            if (bits == 64) {
                instruction.types[instruction.operands].disp64 = 1;
                instruction.types[instruction.operands].disp32s = 1;
            } else if ((bits == 16) ^ instruction.prefixes[ADDR_PREFIX]) {
                instruction.types[instruction.operands].disp16 = 1;
            } else {
                instruction.types[instruction.operands].disp32 = 1;
            }

            if (finalize_displacement (instruction.disps[instruction.operands], operand_start)) {
                return 1;
            }
        }

        if (intel_state.segment) {
            int more_than_1_segment = 0;

            while (1) {
                expr = symbol_get_value_expression (intel_state.segment);

                if (expr->type != EXPR_TYPE_FULL_PTR
                    || symbol_get_value_expression (expr->op_symbol)->type != EXPR_TYPE_REGISTER) {
                    break;
                }

                intel_state.segment = expr->add_symbol;
                more_than_1_segment = 1;
            }

            if (expr->type != EXPR_TYPE_REGISTER) {
                as_error ("segment register name expected");
                return 1;
            }

            if (!reg_table[expr->add_number].type.segment1
                && !reg_table[expr->add_number].type.segment2) {
                as_error ("invalid use of register");
                return 1;
            }

            if (more_than_1_segment) {
                as_warn ("redundant segment overrides");
            }

            if (reg_table[expr->add_number].number == REG_FLAT_NUMBER) {
                instruction.segments[instruction.mem_operands] = NULL;
            } else {
                instruction.segments[instruction.mem_operands] = &reg_table[expr->add_number];
            }
        }

        if (base_index_check (operand_start)) return 1;

        instruction.mem_operands++;
    } else {
        operand_exprs[operand_exprs_count] = *expr;
        instruction.imms[instruction.operands] = &operand_exprs[operand_exprs_count++];

        resolve_expression (instruction.imms[instruction.operands]);
        
        ret = finalize_immediate (instruction.imms[instruction.operands], operand_start);
    }
    
    instruction.operands++;

    return ret;
}

int machine_dependent_need_index_operator (void) {

    return intel_syntax < 0;

}

section_t machine_dependent_simplified_expression_read_into (char **pp, struct expr *expr)
{
    int ret;
    section_t ret_section;

    if (!intel_syntax) {
        return expression_read_into (pp, expr);
    }

    memset (&intel_state, 0, sizeof (intel_state));
    intel_state.operand_modifier = EXPR_TYPE_ABSENT;

    instruction.operands = -1;

    intel_syntax = -1;
    ret_section = expression_read_into (pp, expr);
    ret = intel_simplify_expr (expr);
    intel_syntax = 1;

    if (!ret) {
        as_error ("bad machine-dependent expression");
        expr->type = EXPR_TYPE_INVALID;
    }

    return ret_section;
}
