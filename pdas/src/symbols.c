/******************************************************************************
 * @file            symbols.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include    <stddef.h>
#include    <string.h>

#include    "as.h"

static struct symbol **pointer_to_pointer_to_next_symbol = &symbols;

struct symbol *symbols = NULL;
int finalize_symbols = 0;

struct expr *symbol_get_value_expression (struct symbol *symbol) {
    return &(symbol->value);
}

struct frag *symbol_get_frag (struct symbol *symbol) {
    return symbol->frag;
}

struct symbol *symbol_create (const char *name, section_t section, unsigned long value, struct frag *frag) {

    struct symbol *symbol = xmalloc (sizeof (*symbol));
    
    memset (symbol, 0, sizeof (*symbol));
    
    symbol->name    = xstrdup (name);
    symbol->section = section;
    symbol->frag    = frag;
    
    symbol_set_value (symbol, value);
    return symbol;

}

struct symbol *symbol_find (const char *name) {

    struct symbol *symbol;
    
    for (symbol = symbols; symbol && strcmp (symbol->name, name); symbol = symbol->next);
    return symbol;

}

struct symbol *symbol_find_or_make (const char *name) {

    struct symbol *symbol = symbol_find (name);
    
    if (symbol == NULL) {
    
        symbol = symbol_make (name);
        symbol_add_to_chain (symbol);
    
    }
    
    return symbol;

}

struct symbol *symbol_label (const char *name) {

    struct symbol *symbol;
    
    if ((symbol = symbol_find (name))) {
    
        if (symbol->section == undefined_section) {
        
            symbol->section = current_section;
            symbol->frag = current_frag;
            
            symbol_set_value (symbol, current_frag->fixed_size);
        
        } else {
            as_error ("symbol '%s' is already defined", name);
        }
    
    } else {
    
        symbol = symbol_create (name, current_section, current_frag->fixed_size, current_frag);
        symbol_add_to_chain (symbol);
    
    }
    
    return symbol;

}

struct symbol *symbol_make (const char *name) {
    return symbol_create (name, undefined_section, 0, &zero_address_frag);
}

struct symbol *symbol_temp_new_now (void) {
    return symbol_create (FAKE_LABEL_NAME, current_section, current_frag->fixed_size, current_frag);
}

struct symbol *symbol_temp_make (void) {
    return symbol_make (FAKE_LABEL_NAME);
}

section_t symbol_get_section (struct symbol *symbol) {
    return symbol->section;
}

char *symbol_get_name (struct symbol *symbol) {
    return symbol->name;
}

/** Obtains the value of the symbol without changing any sub-expressions. */
int get_symbol_snapshot (struct symbol **symbol_p, value_t *value_p, section_t *section_p, struct frag **frag_p) {

    struct symbol *symbol = *symbol_p;
    struct expr *expr = symbol_get_value_expression (symbol);
    
    if (!symbol_is_resolved (symbol) && expr->type != EXPR_TYPE_INVALID) {
    
        int resolved;
        
        if (symbol->resolving) {
            return 1;
        }
        
        symbol->resolving = 1;
        resolved = resolve_expression (expr);
        symbol->resolving = 0;
        
        if (resolved == 0) {
            return 1;
        }
        
        switch (expr->type) {
        
            case EXPR_TYPE_CONSTANT:
            case EXPR_TYPE_REGISTER:
            
                if (!symbol_uses_other_symbol (symbol)) break;
                /* fall through. */
            
            case EXPR_TYPE_SYMBOL:
            case EXPR_TYPE_SYMBOL_RVA:
            
                symbol = expr->add_symbol;
                break;
            
            default:
            
                return 1;
        
        }
    
    }
    
    *symbol_p = symbol;
    *value_p = expr->add_number;
    *section_p = symbol_get_section (symbol);
    *frag_p = symbol_get_frag (symbol);
    
    if (*section_p == expr_section) {
    
        switch (expr->type) {
        
            case EXPR_TYPE_CONSTANT:
            
                *section_p = absolute_section;
                break;
            
            case EXPR_TYPE_REGISTER:
            
                *section_p = reg_section;
                break;
            
            default:
            
                break;
        
        }
    
    }
    
    return 0;

}

int symbol_force_reloc (struct symbol *symbol) {
    return symbol->section == undefined_section;
}

int symbol_is_external (struct symbol *symbol) {
    return symbol->flags & SYMBOL_FLAG_EXTERNAL;
}

int symbol_is_resolved (struct symbol *symbol) {
    return symbol->resolved;
}

int symbol_is_section_symbol (struct symbol *symbol) {
    return symbol->flags & SYMBOL_FLAG_SECTION_SYMBOL;
}

int symbol_is_undefined (struct symbol *symbol) {
    return symbol->section == undefined_section;
}

int symbol_uses_other_symbol (struct symbol *symbol) {
    return (symbol->value.type == EXPR_TYPE_SYMBOL);
}

int symbol_uses_reloc_symbol (struct symbol *symbol) {
    return (symbol->value.type == EXPR_TYPE_SYMBOL && ((symbol_is_resolved (symbol) && symbol->value.op_symbol) || symbol_is_undefined (symbol)));
}

unsigned long symbol_get_symbol_table_index (struct symbol *symbol) {
    return symbol->symbol_table_index;
}

value_t symbol_get_value (struct symbol *symbol) {
    return symbol_resolve_value (symbol);
}

static void report_op_error (struct symbol *symbol, struct symbol *left, enum expr_type op, struct symbol *right) {

    const char *op_name;
    
    section_t left_section = left ? symbol_get_section (left) : NULL;
    section_t right_section = symbol_get_section (right);
    
    const char *filename;
    unsigned long line_number;
    
    switch (op) {
    
        case EXPR_TYPE_LOGICAL_OR:
        
            op_name = "||";
            break;
        
        case EXPR_TYPE_LOGICAL_AND:
        
            op_name = "&&";
            break;
        
        case EXPR_TYPE_EQUAL:
        
            op_name = "==";
            break;
        
        case EXPR_TYPE_NOT_EQUAL:
        
            op_name = "!=";
            break;
        
        case EXPR_TYPE_LESSER:
        
            op_name = "<";
            break;
        
        case EXPR_TYPE_LESSER_EQUAL:
        
            op_name = "<=";
            break;
        
        case EXPR_TYPE_GREATER:
        
            op_name = ">";
            break;
        
        case EXPR_TYPE_GREATER_EQUAL:
        
            op_name = ">=";
            break;
        
        case EXPR_TYPE_ADD:
        
            op_name = "+";
            break;
        
        case EXPR_TYPE_SUBTRACT:
        
            op_name = "-";
            break;
        
        case EXPR_TYPE_BIT_INCLUSIVE_OR:
        
            op_name = "|";
            break;
        
        case EXPR_TYPE_BIT_EXCLUSIVE_OR:
        
            op_name = "^";
            break;
        
        case EXPR_TYPE_BIT_AND:
        
            op_name = "&";
            break;
        
        case EXPR_TYPE_MULTIPLY:
        
            op_name = "*";
            break;
        
        case EXPR_TYPE_DIVIDE:
        
            op_name = "/";
            break;
        
        case EXPR_TYPE_MODULUS:
        
            op_name = "%";
            break;
        
        case EXPR_TYPE_LEFT_SHIFT:
        
            op_name = "<<";
            break;
        
        case EXPR_TYPE_RIGHT_SHIFT:
        
            op_name = ">>";
            break;
        
        case EXPR_TYPE_LOGICAL_NOT:
        
            op_name = "!";
            break;
        
        case EXPR_TYPE_BIT_NOT:
        
            op_name = "~";
            break;
        
        case EXPR_TYPE_UNARY_MINUS:
        
            op_name = "-";
            break;
        
        default:
        
            as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "report_op_error invalid case %i", op);
            break;
    
    }
    
    if (expr_symbol_get_filename_and_line_number (symbol, &filename, &line_number) == 0) {
    
        if (left) {
            as_error_at (filename, line_number, "invalid operands (%s and %s sections) for `%s'", section_get_name (left_section), section_get_name (right_section), op_name);
        } else {
            as_error_at (filename, line_number, "invalid operand (%s section) for `%s'", section_get_name (right_section), op_name);
        }
    
    } else {
        
        if (left) {
            as_error_at (NULL, 0, "invalid operands (%s and %s sections) for `%s' when setting `%s'", section_get_name (left_section), section_get_name (right_section), op_name, symbol_get_name (symbol));
        } else {
            as_error_at (NULL, 0, "invalid operand (%s section) for `%s' when setting `%s'", section_get_name (right_section), op_name, symbol_get_name (symbol));
        }
    
    }

}

value_t symbol_resolve_value (struct symbol *symbol) {

    int resolved = 0;
    value_t final_value = 0;
    
    section_t final_section = symbol_get_section (symbol);
    
    if (symbol->resolved) {
        
        if (symbol->value.type == EXPR_TYPE_CONSTANT) {
            return (symbol->value.add_number);
        }
        
        return 0;
    
    }
    
    if (symbol->resolving) {
    
        as_error_at (NULL, 0, "symbol definition loop encountered at '%s'", symbol_get_name (symbol));
        
        final_value = 0;
        resolved = 1;
    
    } else {
    
        section_t left_section, right_section;
        value_t left_value, right_value;
        
        int can_move_into_absolute_section;
        
        symbol->resolving = 1;
        final_value = symbol->value.add_number;

        switch (symbol->value.type) {
            
            case EXPR_TYPE_ABSENT:
            
                final_value = 0;
                /* fall through. */
            
            case EXPR_TYPE_CONSTANT:
            
                final_value += symbol->frag->address;
                
                if (final_section == expr_section) {
                    final_section = absolute_section;
                }
                
                /* fall through */
            
            case EXPR_TYPE_REGISTER:
                
                resolved = 1;
                break;
            
            case EXPR_TYPE_SYMBOL:
            case EXPR_TYPE_SYMBOL_RVA:
            
                left_value = symbol_resolve_value (symbol->value.add_symbol);
                left_section = symbol_get_section (symbol->value.add_symbol);
                
            do_symbol:
                
                if (left_section == undefined_section || (finalize_symbols && final_section == expr_section && left_section != expr_section && left_section != absolute_section)) {
                
                    if (finalize_symbols) {
                    
                        symbol->value.type = EXPR_TYPE_SYMBOL;
                        symbol->value.op_symbol = symbol->value.add_symbol;
                        symbol->value.add_number = final_value;
                    
                    }
                    
                    final_section = left_section;
                    final_value += symbol->frag->address + left_value;
                    resolved = symbol_is_resolved (symbol->value.add_symbol);
                    symbol->resolving = 0;
                    
                    goto exit_do_not_set_value;
                
                } else {
                
                    final_value += symbol->frag->address + left_value;
                    
                    if (final_section == expr_section || final_section == undefined_section) {
                        final_section = left_section;
                    }
                
                }
                
                resolved = symbol_is_resolved (symbol->value.add_symbol);
                break;
            
            case EXPR_TYPE_LOGICAL_NOT:
            case EXPR_TYPE_BIT_NOT:
            case EXPR_TYPE_UNARY_MINUS:
            
                left_value = symbol_resolve_value (symbol->value.add_symbol);
                left_section = symbol_get_section (symbol->value.add_symbol);
                
                if (symbol->value.type != EXPR_TYPE_LOGICAL_NOT && left_section != absolute_section && finalize_symbols) {
                    report_op_error (symbol, NULL, symbol->value.type, symbol->value.add_symbol);
                }
                
                if (final_section == expr_section || final_section == undefined_section) {
                    final_section = absolute_section;
                }
                
                switch (symbol->value.type) {
                
                    case EXPR_TYPE_LOGICAL_NOT:
                    
                        left_value = !left_value;
                        break;
                    
                    case EXPR_TYPE_BIT_NOT:
                    
                        left_value = ~left_value;
                        break;
                    
                    case EXPR_TYPE_UNARY_MINUS:
                    
                        left_value = -left_value;
                        break;
                    
                    default:
                    
                        break;
                
                }
                
                final_value += left_value + symbol->frag->address;
                resolved = symbol_is_resolved (symbol->value.add_symbol);
                
                break;
            
            case EXPR_TYPE_LOGICAL_OR:
            case EXPR_TYPE_LOGICAL_AND:
            case EXPR_TYPE_EQUAL:
            case EXPR_TYPE_NOT_EQUAL:
            case EXPR_TYPE_LESSER:
            case EXPR_TYPE_LESSER_EQUAL:
            case EXPR_TYPE_GREATER:
            case EXPR_TYPE_GREATER_EQUAL:
            case EXPR_TYPE_ADD:
            case EXPR_TYPE_SUBTRACT:
            case EXPR_TYPE_BIT_INCLUSIVE_OR:
            case EXPR_TYPE_BIT_EXCLUSIVE_OR:
            case EXPR_TYPE_BIT_AND:
            case EXPR_TYPE_MULTIPLY:
            case EXPR_TYPE_DIVIDE:
            case EXPR_TYPE_MODULUS:
            case EXPR_TYPE_LEFT_SHIFT:
            case EXPR_TYPE_RIGHT_SHIFT:
            
                left_value = symbol_resolve_value (symbol->value.add_symbol);
                right_value = symbol_resolve_value (symbol->value.op_symbol);
                left_section = symbol_get_section (symbol->value.add_symbol);
                right_section = symbol_get_section (symbol->value.op_symbol);
                
                if (symbol->value.type == EXPR_TYPE_ADD) {
                
                    if (right_section == absolute_section) {
                    
                        final_value += right_value;
                        goto do_symbol;
                    
                    } else if (left_section == absolute_section) {
                    
                        final_value += left_value;
                        symbol->value.add_symbol = symbol->value.op_symbol;
                        
                        left_value = right_value;
                        left_section = right_section;
                        
                        goto do_symbol;
                    
                    }
                
                } else if (symbol->value.type == EXPR_TYPE_SUBTRACT) {
                
                    if (right_section == absolute_section) {
                    
                        final_value -= right_value;
                        goto do_symbol;
                    
                    }
                
                }
                
                can_move_into_absolute_section = 1;
                
                /**
                 * Equality and non-equality operations are allowed on everything.
                 * Subtraction and other comparison operators are allowed if both operands are in the same section.
                 * For everything else, both operands must be absolute.
                 * Addition and subtraction of constants is handled above.
                 */
                if (!(left_section == absolute_section && right_section == absolute_section)
                    && !(symbol->value.type == EXPR_TYPE_EQUAL || symbol->value.type == EXPR_TYPE_NOT_EQUAL)
                    && !((symbol->value.type == EXPR_TYPE_SUBTRACT
                            || symbol->value.type == EXPR_TYPE_LESSER || symbol->value.type == EXPR_TYPE_LESSER_EQUAL
                            || symbol->value.type == EXPR_TYPE_GREATER || symbol->value.type == EXPR_TYPE_GREATER_EQUAL)
                         && left_section == right_section
                         && (left_section != undefined_section || symbol->value.add_symbol == symbol->value.op_symbol)))
                {
                
                    if (finalize_symbols) {
                        report_op_error (symbol, symbol->value.add_symbol, symbol->value.type, symbol->value.op_symbol);
                    } else {
                        can_move_into_absolute_section = 0;
                    }
                
                }
                
                if (can_move_into_absolute_section && (final_section == expr_section || final_section == undefined_section)) {
                    final_section = absolute_section;
                }
                
                /* Checks for division by zero. */
                if ((symbol->value.type == EXPR_TYPE_DIVIDE || symbol->value.type == EXPR_TYPE_MODULUS) && right_value == 0) {
                
                    const char *filename;
                    unsigned long line_number;
                    
                    if (expr_symbol_get_filename_and_line_number (symbol, &filename, &line_number) == 0) {
                        as_error_at (filename, line_number, "division by zero");
                    } else {
                        as_error_at (NULL, 0, "division by zero when setting '%s'", symbol_get_name (symbol));
                    }
                    
                    right_value = 1;
                
                }
                
                switch (symbol->value.type) {
                
                    case EXPR_TYPE_LOGICAL_OR:
                    
                        left_value = left_value || right_value;
                        break;
                    
                    case EXPR_TYPE_LOGICAL_AND:
                    
                        left_value = left_value && right_value;
                        break;
                    
                    case EXPR_TYPE_EQUAL:
                    case EXPR_TYPE_NOT_EQUAL:
                    
                        left_value = ((left_value == right_value
                                       && left_section == right_section
                                       && (left_section != undefined_section
                                           || symbol->value.add_symbol == symbol->value.op_symbol))
                                      ? ~ (offset_t) 0 : 0);
                        
                        if (symbol->value.type == EXPR_TYPE_NOT_EQUAL) {
                            left_value = ~left_value;
                        }
                        
                        break;
                    
                    case EXPR_TYPE_LESSER:
                    
                        left_value = left_value < right_value ? ~ (offset_t) 0 : 0;
                        break;
                    
                    case EXPR_TYPE_LESSER_EQUAL:
                    
                        left_value = left_value <= right_value ? ~ (offset_t) 0 : 0;
                        break;
                    
                    case EXPR_TYPE_GREATER:
                    
                        left_value = left_value > right_value ? ~ (offset_t) 0 : 0;
                        break;
                    
                    case EXPR_TYPE_GREATER_EQUAL:
                    
                        left_value = left_value >= right_value ? ~ (offset_t) 0 : 0;
                        break;
                    
                    case EXPR_TYPE_ADD:
                    
                        left_value += right_value;
                        break;
                    
                    case EXPR_TYPE_SUBTRACT:
                    
                        left_value -= right_value;
                        break;
                    
                    case EXPR_TYPE_BIT_INCLUSIVE_OR:
                    
                        left_value |= right_value;
                        break;
                    
                    case EXPR_TYPE_BIT_EXCLUSIVE_OR:
                    
                        left_value ^= right_value;
                        break;
                    
                    case EXPR_TYPE_BIT_AND:
                    
                        left_value &= right_value;
                        break;
                    
                    case EXPR_TYPE_MULTIPLY:
                    
                        left_value *= right_value;
                        break;
                    
                    case EXPR_TYPE_DIVIDE:
                    
                        left_value /= right_value;
                        break;
                    
                    case EXPR_TYPE_MODULUS:
                    
                        left_value %= right_value;
                        break;
                    
                    case EXPR_TYPE_LEFT_SHIFT:
                    
                        left_value = (value_t) left_value << (value_t) right_value;
                        break;
                    
                    case EXPR_TYPE_RIGHT_SHIFT:
                    
                        left_value = (value_t) left_value >> (value_t) right_value;
                        break;
                    
                    default:
                    
                        break;
                
                }
                
                final_value += symbol->frag->address + left_value;
                
                if (final_section == expr_section || final_section == undefined_section) {
                
                    if (left_section == undefined_section || right_section == undefined_section) {
                        final_section = undefined_section;
                    } else if (left_section == absolute_section) {
                        final_section = right_section;
                    } else {
                        final_section = left_section;
                    }
                
                }
                
                resolved = (symbol_is_resolved (symbol->value.add_symbol) && symbol_is_resolved (symbol->value.op_symbol));
                break;
            
            default:
            
                as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                                "symbol_resolve_value invalid case %i", symbol->value.type);
                break;
        
        }
        
        symbol->resolving = 0;
    
    }

    if (finalize_symbols) {
        symbol_set_value (symbol, final_value);
    }

exit_do_not_set_value:             

    if (finalize_symbols) {
    
        if (resolved) {
            symbol->resolved = resolved;
        }
    
    }
    
    symbol_set_section (symbol, final_section);
    return final_value;

}

void symbol_add_to_chain (struct symbol *symbol) {

    *pointer_to_pointer_to_next_symbol = symbol;
    pointer_to_pointer_to_next_symbol = &symbol->next;

}

void symbol_set_external (struct symbol *symbol) {
    symbol->flags |= SYMBOL_FLAG_EXTERNAL;
}

void symbol_set_frag (struct symbol *symbol, struct frag *frag) {
    symbol->frag = frag;
}

void symbol_set_section (struct symbol *symbol, section_t section) {
    symbol->section = section;
}

void symbol_set_symbol_table_index (struct symbol *symbol, unsigned long index) {
    symbol->symbol_table_index = index;
}

void symbol_set_value (struct symbol *symbol, value_t value) {

    symbol->value.type = EXPR_TYPE_CONSTANT;
    symbol->value.add_number = value;

}

void symbol_set_value_expression (struct symbol *symbol, struct expr *expr) {
    symbol->value = *expr;
}

void symbol_set_value_now (struct symbol *symbol) {

    symbol_set_section (symbol, current_section);
    symbol_set_frag (symbol, current_frag);
    symbol_set_value (symbol, current_frag->fixed_size);
    
}
