/******************************************************************************
 * @file            expr.c
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
#include    <string.h>

#include    "as.h"

/**
 * Expression symbols are mapped to file positions to provide
 * better error messages.
 */
struct expr_symbol_line {

    struct symbol *symbol;
    
    const char *filename;
    unsigned long line_number;
    
    struct expr_symbol_line *next;

};

static struct expr_symbol_line *expr_symbol_lines = NULL;

static char *read_character (char *p, uint_fast64_t *c) {

    if (*p == '\\') {
    
        p++;
        
        if (*p == '\'') {
        
            *c = '\'';
            p++;
        
        } else if (*p == '\"') {
        
            *c = '"';
            p++;
        
        } else if (*p == '\\') {
        
            *c = '\\';
            p++;
        
        } else if (*p == 'a') {
        
            *c = 0x07;
            p++;
        
        } else if (*p == 'b') {
        
            *c = 0x08;
            p++;
        
        } else if (*p == 't') {
        
            *c = 0x09;
            p++;
        
        } else if (*p == 'n') {
        
            *c = 0x0a;
            p++;
        
        } else if (*p == 'v') {
        
            *c = 0x0b;
            p++;
        
        } else if (*p == 'f') {
        
            *c = 0x0c;
            p++;
        
        } else if (*p == 'r') {
        
            *c = 0x0d;
            p++;
        
        } else if (*p == 'e') {
        
            *c = 0x1b;
            p++;
        
        } else if (*p >= '0' && *p <= '7') {
        
            *c = 0;
            
            while (*p >= '0' && *p <= '7') {
            
                *c = *c * 8 + (*p - '0');
                p++;
            
            }
        
        } else {
            p--;
        }
    
    } else {
        *c = *p++;
    }
    
    return p;

}

section_t current_location (struct expr *expr) {

    expr->type = EXPR_TYPE_SYMBOL;
    expr->add_symbol = symbol_temp_new_now ();
    expr->op_symbol = NULL;
    expr->add_number = 0;
    
    return symbol_get_section (expr->add_symbol);

}

static unsigned int hex_value (int z) {

    if ((z >= '0') && (z <= '9')) {
        return (z - '0');
    }
    
    if ((z >= 'a') && (z <= 'f')) {
        return (z + 10 - 'a');
    }
    
    if ((z >= 'A') && (z <= 'F')) {
        return (z + 10 - 'A');
    }
    
    return (100);

}

static void integer_constant (char **pp, struct expr *expr, unsigned int radix) {

    value_t number;
    unsigned int digit;
        
    for (number = 0; (digit = hex_value ((int) **pp)) < radix; (*pp)++) {
        number = number * radix + digit;
    }
    
    expr->type = EXPR_TYPE_CONSTANT;
    expr->add_number = number;

}

static section_t operand (char **pp, struct expr *expr, enum expr_mode expr_mode) {

    char c;
    section_t ret_section;
    
    expr->type = EXPR_TYPE_INVALID;
    expr->add_symbol = NULL;
    expr->op_symbol = NULL;
    expr->add_number = 0;
    
    *pp = skip_whitespace (*pp);
    
    if (is_end_of_line[(int) **pp]) {
        goto end_of_line;
    }
    
    switch (**pp) {
    
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        
            integer_constant (pp, expr, 10);
            break;
        
        case '0':
        
            /* Non-decimal radix. */
            (*pp)++;
            
            switch (**pp) {
            
                case 'x':
                case 'X':
                
                    (*pp)++;
                    
                    integer_constant (pp, expr, 16);
                    break;
                
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                
                    integer_constant (pp, expr, 8);
                    break;
                
                case 'b':
                case 'B':
                
                    if ((*pp)[1] == '0' || (*pp)[1] == '1') {
                    
                        (*pp)++;
                        
                        integer_constant (pp, expr, 2);
                        break;
                    
                    }
                    
                    /* fall through. */
                
                default:
                
                    /* Just a single 0. */
                    expr->type = EXPR_TYPE_CONSTANT;
                    expr->add_number = 0;
                    break;
            
            }
            
            break;
        
        case '[':
        
            if (machine_dependent_need_index_operator ()) {
                goto default_;
            }
            
            /* fall through. */
        
        case '(':
        
            c = (*pp)++[0];
            
            ret_section = read_into (pp, expr, 0, expr_mode);
            
            if ((c == '(' && (*pp)[0] != ')') || (c == '[' && (*pp)[0] != ']')) {
            
                if ((*pp)[0]) {
                    as_error ("found '%c' but expected '%c'", (*pp)[0], c == '(' ? ')' : ']');
                } else {
                    as_error ("missing '%c'", c == '(' ? ')' : ']');
                }
            
            } else {
                ++*pp;
            }
            
            (*pp) = skip_whitespace (*pp);
            return ret_section;
        
        case '+':
        case '-':
        case '~':
        case '!':
        
            c = (*pp)[0];
            (*pp)++;
            
        unary:
            
            operand (pp, expr, expr_mode);
            
            if (expr->type == EXPR_TYPE_CONSTANT) {
            
                switch (c) {
                
                    case '-':
                    
                        expr->add_number = -expr->add_number;
                        break;
                    
                    case '~':
                    
                        expr->add_number = ~expr->add_number;
                        break;
                    
                    case '!':
                    
                        expr->add_number = !expr->add_number;
                        break;
                
                }
            
            } else if (expr->type != EXPR_TYPE_INVALID && expr->type != EXPR_TYPE_ABSENT) {
            
                if (c != '+') {
                
                    expr->add_symbol = make_expr_symbol (expr);
                    expr->op_symbol = NULL;
                    expr->add_number = 0;
                    
                    switch (c) {
                    
                        case '-':
                        
                            expr->type = EXPR_TYPE_UNARY_MINUS;
                            break;
                        
                        case '~':
                        
                            expr->type = EXPR_TYPE_BIT_NOT;
                            break;
                        
                        case '!':
                        
                            expr->type = EXPR_TYPE_LOGICAL_NOT;
                            break;
                    
                    }
                
                }
            
            } else {
                as_warn ("unary operator %c ignored because bad operand follows", c);
            }
            
            break;
        
        case '\'':
        
            (*pp) = read_character (++(*pp), &expr->add_number);
            
            if (*(*pp) == '\'') {
                (*pp)++;
            }
            
            expr->type = EXPR_TYPE_CONSTANT;
            break;
        
        case '.':
        
            if (!is_name_part ((int) (*pp)[1])) {
            
                current_location (expr);
                
                ++*pp;
                break;
            
            } else {
                goto is_name;
            }
        
        default_:
        default:
        
            if (is_name_beginner ((int) (*pp)[0])) {
            
                struct symbol *symbol;
                char *name;
            
            is_name:
            
                name = *pp;
                
                c = get_symbol_name_end (pp);
                
                /* Checks in machine dependent way whether the name is an unary operator. */
                {
                
                    enum expr_type ret;
                    
                    ret = machine_dependent_parse_operator (pp, name, &c, 1);
                    
                    switch (ret) {
                    
                        case EXPR_TYPE_UNARY_MINUS:
                        
                            **pp = c;
                            
                            c = '-';
                            goto unary;
                        
                        case EXPR_TYPE_BIT_NOT:
                        
                            **pp = c;
                            
                            c = '~';
                            goto unary;
                        
                        case EXPR_TYPE_LOGICAL_NOT:
                        
                            **pp = c;
                            
                            c = '!';
                            goto unary;
                        
                        case EXPR_TYPE_INVALID:
                        
                            as_error ("invalid use of operator \"%s\"", name);
                            break;
                        
                        default:
                        
                            break;
                    
                    }
                    
                    if (ret != EXPR_TYPE_INVALID && ret != EXPR_TYPE_ABSENT) {
                    
                        /* It is a machine dependent unary operator. */
                        **pp = c;
                        
                        read_into (pp, expr, 9, expr_mode);
                        
                        expr->add_symbol = make_expr_symbol (expr);
                        expr->op_symbol = NULL;
                        expr->add_number = 0;
                        expr->type = ret;
                        
                        break;
                    
                    }
                
                }
                
                if (machine_dependent_parse_name (pp, expr, name, &c)) {

                    **pp = c;
                    break;
                }
                
                symbol = symbol_find_or_make (name);
                
                **pp = c;
                
                if (symbol_get_section (symbol) == absolute_section && !symbol_force_reloc (symbol)) {
                
                    expr->type = EXPR_TYPE_CONSTANT;
                    expr->add_number = symbol_get_value (symbol);
                
                } else {
                
                    expr->type = EXPR_TYPE_SYMBOL;
                    expr->add_symbol = symbol;
                    expr->add_number = 0;
                
                }
            
            } else {

                /**
                 * Target machine dependent parsing.
                 * If successful, expr->type is set
                 * to something different than EXPR_TYPE_ABSENT
                 * and *pp is modified.
                 */
                expr->type = EXPR_TYPE_ABSENT;
                machine_dependent_parse_operand (pp, expr);
                
                if (expr->type == EXPR_TYPE_ABSENT) {
                
                    as_error ("bad expression");
                    ++*pp;
                    
                    expr->type = EXPR_TYPE_CONSTANT;
                    expr->add_number = 0;
                
                }
            
            }
            
            break;
        
        end_of_line:
        case ',':
        
            expr->type = EXPR_TYPE_ABSENT;
            break;
    
    }
    
    (*pp) = skip_whitespace ((*pp));
    
    switch (expr->type) {
    
        case EXPR_TYPE_SYMBOL:
        
            return symbol_get_section (expr->add_symbol);
        
        case EXPR_TYPE_REGISTER:
        
            return reg_section;
        
        default:
        
            return absolute_section;
    
    }

}

static enum expr_type operator (char *p, unsigned int *operator_size) {

    char *start;
    enum expr_type ret;
    
    if (is_name_beginner ((int) *p)) {
    
        char *name;
        char c;
        
        name = p;
        c = get_symbol_name_end (&p);
        
        ret = machine_dependent_parse_operator (&p, name, &c, 2);
        
        switch (ret) {
        
            case EXPR_TYPE_ABSENT:
            
                *p = c;
                
                p = name;
                break;
            
            default:
            
                *p = c;
                *operator_size = p - name;
                
                return ret;
        
        }
    
    }
    
    switch (*p) {
    
        case '+':
        
            *operator_size = 1;
            return EXPR_TYPE_ADD;
        
        case '-':
        
            *operator_size = 1;
            return EXPR_TYPE_SUBTRACT;
        
        case '<':
        
            switch (p[1]) {
            
                case '<':
                
                    *operator_size = 2;
                    return EXPR_TYPE_LEFT_SHIFT;
                
                case '>':
                
                    *operator_size = 2;
                    return EXPR_TYPE_NOT_EQUAL;
                
                case '=':
                
                    *operator_size = 2;
                    return EXPR_TYPE_LESSER_EQUAL;
            
            }
            
            *operator_size = 1;
            return EXPR_TYPE_LESSER;
        
        case '>':
        
            switch (p[1]) {
            
                case '>':
                
                    *operator_size = 2;
                    return EXPR_TYPE_RIGHT_SHIFT;
                
                case '=':
                
                    *operator_size = 2;
                    return EXPR_TYPE_GREATER_EQUAL;
            
            }
            
            *operator_size = 1;
            return EXPR_TYPE_GREATER;
        
        case '=':
        
            if (p[1] != '=') {
            
                *operator_size = 0;
                return EXPR_TYPE_INVALID;
            
            }
            
            *operator_size = 2;
            return EXPR_TYPE_EQUAL;
        
        case '!':
        
            if (p[1] != '=') {
            
                *operator_size = 0;
                return EXPR_TYPE_INVALID;
            
            }
            
            *operator_size = 2;
            return EXPR_TYPE_NOT_EQUAL;
        
        case '|':
        
            if (p[1] != '|') {
            
                *operator_size = 1;
                return EXPR_TYPE_BIT_INCLUSIVE_OR;
            
            }
            
            *operator_size = 2;
            return EXPR_TYPE_LOGICAL_OR;
        
        case '&':
        
            if (p[1] != '&') {
            
                *operator_size = 1;
                return EXPR_TYPE_BIT_AND;
            
            }
            
            *operator_size = 2;
            return EXPR_TYPE_LOGICAL_AND;
        
        case '/':
        
            *operator_size = 1;
            return EXPR_TYPE_DIVIDE;
        
        case '%':
        
            *operator_size = 1;
            return EXPR_TYPE_MODULUS;
        
        case '*':
        
            *operator_size = 1;
            return EXPR_TYPE_MULTIPLY;
        
        case '^':
        
            *operator_size = 1;
            return EXPR_TYPE_BIT_EXCLUSIVE_OR;
        
        default:
        
            start = p;
            ret = machine_dependent_parse_operator (&p, NULL, NULL, 2);
            
            *operator_size = p - start;
            return ret;
    
    }
    
}

/**
 * Creates an internal symbol for holding expressions in the
 * fake section expr_section.
 */
struct symbol *make_expr_symbol (struct expr *expr) {

    struct symbol *symbol;
    struct expr_symbol_line *es_line;
    
    if (expr->type == EXPR_TYPE_SYMBOL && expr->add_number == 0) {
        return (expr->add_symbol);
    }
    
    symbol = symbol_create (FAKE_LABEL_NAME, (expr->type == EXPR_TYPE_CONSTANT ?
                                                absolute_section :
                                               	    (expr->type == EXPR_TYPE_REGISTER ?
                                                  	    reg_section :
                                                            expr_section)), 0, &zero_address_frag);
    symbol_set_value_expression (symbol, expr);
    
    es_line = xmalloc (sizeof (*es_line));
    es_line->symbol = symbol;
    
    get_filename_and_line_number (&(es_line->filename), &(es_line->line_number));
    
    es_line->next = expr_symbol_lines;
    expr_symbol_lines = es_line;
    
    return symbol;

}

static unsigned int op_rank_table[EXPR_TYPE_MAX] = {

    0,                                  /* EXPR_TYPE_INVALID */
    0,                                  /* EXPR_TYPE_ABSENT */
    0,                                  /* EXPR_TYPE_CONSTANT */
    0,                                  /* EXPR_TYPE_SYMBOL */
    0,                                  /* EXPR_TYPE_SYMBOL_RVA */
    0,                                  /* EXPR_TYPE_REGISTER */
    1,                                  /* EXPR_TYPE_INDEX */
    2,                                  /* EXPR_TYPE_LOGICAL_OR */
    3,                                  /* EXPR_TYPE_LOGICAL_AND */
    4,                                  /* EXPR_TYPE_EQUAL */
    4,                                  /* EXPR_TYPE_NOT_EQUAL */
    4,                                  /* EXPR_TYPE_LESSER */
    4,                                  /* EXPR_TYPE_LESSER_EQUAL */
    4,                                  /* EXPR_TYPE_GREATER */
    4,                                  /* EXPR_TYPE_GREATER_EQUAL */
    5,                                  /* EXPR_TYPE_ADD */
    5,                                  /* EXPR_TYPE_SUBTRACT */
    7,                                  /* EXPR_TYPE_BIT_INCLUSIVE_OR */
    7,                                  /* EXPR_TYPE_BIT_EXCLUSIVE_OR */
    7,                                  /* EXPR_TYPE_BIT_AND */
    8,                                  /* EXPR_TYPE_MULTIPLY */
    8,                                  /* EXPR_TYPE_DIVIDE */
    8,                                  /* EXPR_TYPE_MODULUS */
    8,                                  /* EXPR_TYPE_LEFT_SHIFT */
    8,                                  /* EXPR_TYPE_RIGHT_SHIFT */
    9,                                  /* EXPR_TYPE_LOGICAL_NOT */
    9,                                  /* EXPR_TYPE_BIT_NOT */
    9                                   /* EXPR_TYPE_UNARY_MINUS */
    
    /* Machine dependent operators default to rank 0 but expr_type_set_rank() can be used to change the rank. */

};

void expr_type_set_rank (enum expr_type expr_type, unsigned int rank) {
    op_rank_table[expr_type] = rank;
}

section_t read_into (char **pp, struct expr *expr, unsigned int rank, enum expr_mode expr_mode) {

    enum expr_type left_op;
    struct expr right_expr;
    
    section_t ret_section;
    unsigned int operator_size;
    
    ret_section = operand (pp, expr, expr_mode);
    left_op = operator (*pp, &operator_size);
    
    while (left_op != EXPR_TYPE_INVALID && op_rank_table[left_op] > rank) {
    
        enum expr_type right_op;
        offset_t offset;
        
        section_t right_section;
        
        *pp += operator_size;
        
        right_section = read_into (pp, &right_expr, op_rank_table[left_op], expr_mode);
        
        if (right_expr.type == EXPR_TYPE_ABSENT) {
        
            as_warn ("missing operand; zero assumed");
            
            right_expr.type = EXPR_TYPE_CONSTANT;
            right_expr.add_symbol = NULL;
            right_expr.op_symbol = NULL;
            right_expr.add_number = 0;
        
        }
        
        if (left_op == EXPR_TYPE_INDEX) {
        
            if (**pp != ']') {
                as_error ("missing ']'");
            } else {
                *pp = skip_whitespace (*pp + 1);
            }
        
        }
        
        right_op = operator (*pp, &operator_size);
        
        if (left_op == EXPR_TYPE_ADD &&
          right_expr.type == EXPR_TYPE_CONSTANT &&
          expr->type != EXPR_TYPE_REGISTER) {
            expr->add_number += right_expr.add_number;
        } else if (left_op == EXPR_TYPE_SUBTRACT &&
          right_expr.type == EXPR_TYPE_SYMBOL &&
          expr->type == EXPR_TYPE_SYMBOL &&
          ret_section == right_section &&
          ((SECTION_IS_NORMAL (ret_section) &&
            !symbol_force_reloc (expr->add_symbol) &&
            !symbol_force_reloc (right_expr.add_symbol)) ||
           expr->add_symbol == right_expr.add_symbol) &&
          frags_offset_is_fixed (symbol_get_frag (expr->add_symbol),
                                    symbol_get_frag (right_expr.add_symbol),
                                        &offset)) {
            
            expr->add_number += symbol_get_value (expr->add_symbol) - symbol_get_value (right_expr.add_symbol);
            expr->add_number -= right_expr.add_number;
            expr->add_number -= offset;
            expr->type = EXPR_TYPE_CONSTANT;
            expr->add_symbol = NULL;
        
        } else if (left_op == EXPR_TYPE_SUBTRACT &&
          right_expr.type == EXPR_TYPE_CONSTANT &&
          expr->type != EXPR_TYPE_REGISTER) {
            expr->add_number -= right_expr.add_number;
        } else if (left_op == EXPR_TYPE_ADD &&
          expr->type == EXPR_TYPE_CONSTANT &&
          right_expr.type != EXPR_TYPE_REGISTER) {
        
            expr->type = right_expr.type;
            expr->add_symbol = right_expr.add_symbol;
            expr->op_symbol = right_expr.op_symbol;
            expr->add_number += right_expr.add_number;
            ret_section = right_section;
        
        } else if (expr->type == EXPR_TYPE_CONSTANT && right_expr.type == EXPR_TYPE_CONSTANT) {
        
            /* Checks for division by zero. */
            if ((left_op == EXPR_TYPE_DIVIDE || left_op == EXPR_TYPE_MODULUS) && right_expr.add_number == 0) {
            
                as_warn ("division by zero");
                right_expr.add_number = 1;
            
            }
            
            switch (left_op) {
            
                case EXPR_TYPE_LOGICAL_OR:
                
                    expr->add_number = expr->add_number || right_expr.add_number;
                    break;
                
                case EXPR_TYPE_LOGICAL_AND:
                
                    expr->add_number = expr->add_number && right_expr.add_number;
                    break;
                
                case EXPR_TYPE_EQUAL:
                case EXPR_TYPE_NOT_EQUAL:
                
                    expr->add_number = (expr->add_number == right_expr.add_number) ? ~ (offset_t) 0 : 0;
                    
                    if (left_op == EXPR_TYPE_NOT_EQUAL) {
                        expr->add_number = ~expr->add_number;
                    }
                    
                    break;
                
                case EXPR_TYPE_LESSER:
                
                    expr->add_number = (offset_t) expr->add_number < (offset_t) right_expr.add_number ? ~(offset_t) 0 : 0;
                    break;
                
                case EXPR_TYPE_LESSER_EQUAL:
                
                    expr->add_number = (offset_t) expr->add_number <= (offset_t) right_expr.add_number ? ~(offset_t) 0 : 0;
                    break;
                
                case EXPR_TYPE_GREATER:
                
                    expr->add_number = (offset_t) expr->add_number > (offset_t) right_expr.add_number ? ~(offset_t) 0 : 0;
                    break;
                
                case EXPR_TYPE_GREATER_EQUAL:
                
                    expr->add_number = (offset_t) expr->add_number >= (offset_t) right_expr.add_number ? ~(offset_t) 0 : 0;
                    break;
                
                case EXPR_TYPE_ADD:
                
                    expr->add_number += right_expr.add_number;
                    break;
                
                case EXPR_TYPE_SUBTRACT:
                
                    expr->add_number -= right_expr.add_number;
                    break;
                
                case EXPR_TYPE_BIT_INCLUSIVE_OR:
                
                    expr->add_number |= right_expr.add_number;
                    break;
                
                case EXPR_TYPE_BIT_EXCLUSIVE_OR:
                
                    expr->add_number ^= right_expr.add_number;
                    break;
                
                case EXPR_TYPE_BIT_AND:
                
                    expr->add_number &= right_expr.add_number;
                    break;
                
                case EXPR_TYPE_MULTIPLY:
                
                    expr->add_number *= right_expr.add_number;
                    break;
                
                case EXPR_TYPE_DIVIDE:
                
                    expr->add_number /= right_expr.add_number;
                    break;
                
                case EXPR_TYPE_MODULUS:
                
                    expr->add_number %= right_expr.add_number;
                    break;
                
                case EXPR_TYPE_LEFT_SHIFT:
                
                    expr->add_number = (value_t) (expr->add_number) << (value_t) (right_expr.add_number);
                    break;
                
                case EXPR_TYPE_RIGHT_SHIFT:
                
                    expr->add_number = (value_t) (expr->add_number) >> (value_t) (right_expr.add_number);
                    break;
                
                default:
                
                    goto general_case;
            
            }
        
        } else if (expr->type == EXPR_TYPE_SYMBOL
                   && right_expr.type == EXPR_TYPE_SYMBOL
                   && (left_op == EXPR_TYPE_ADD
                       || left_op == EXPR_TYPE_SUBTRACT
                       || (expr->add_number == 0 && right_expr.add_number == 0))) {
            
            expr->type = left_op;
            expr->op_symbol = right_expr.add_symbol;
            
            if (left_op == EXPR_TYPE_ADD) {
                expr->add_number += right_expr.add_number;
            } else if (left_op == EXPR_TYPE_SUBTRACT) {
            
                expr->add_number -= right_expr.add_number;
                
                if (ret_section == right_section &&
                  SECTION_IS_NORMAL (ret_section) &&
                  !symbol_force_reloc (expr->add_symbol) &&
                  !symbol_force_reloc (right_expr.add_symbol)) {
                    ret_section = right_section = absolute_section;
                }
            
            }
        
        } else {
        
        general_case:
        
            expr->add_symbol = make_expr_symbol (expr);
            expr->op_symbol = make_expr_symbol (&right_expr);
            expr->add_number = 0;
            expr->type = left_op;
        
        }
        
        if (ret_section != right_section) {
        
            if (ret_section == undefined_section) {
                /* Nothing is done. */
            } else if (right_section == undefined_section) {
                ret_section = right_section;
            } else if (ret_section == expr_section) {
                /* Nothing is done. */
            } else if (right_section == expr_section) {
                ret_section = right_section;
            } else if (ret_section == reg_section) {
                /* Nothing is done. */
            } else if (right_section == reg_section) {
                ret_section = right_section;
            } else if (right_section == absolute_section) {
                /* Nothing is done. */
            } else if (ret_section == absolute_section) {
                ret_section = right_section;
            } else {
                as_error ("operation combines symbols in different sections");
            }
        
        }
        
        left_op = right_op;
        
    }
    
    if (rank == 0 && expr_mode == expr_mode_evaluate) {
        resolve_expression (expr);
    }
    
    (*pp) = skip_whitespace ((*pp));
    return (expr->type == EXPR_TYPE_CONSTANT) ? absolute_section : ret_section;

}

char *expr_read_into (char *p, struct expr *expr) {

    char *p2 = p;
    read_into (&p2, expr, 0, expr_mode_normal);
    
    return p2;

}

offset_t absolute_expression_read_into (char **pp, struct expr *expr) {

    expression_evaluate_and_read_into (pp, expr);
    
    if (expr->type != EXPR_TYPE_CONSTANT) {
    
        if (expr->type != EXPR_TYPE_ABSENT) {
            as_error ("bad or irreducible absolute expression");
        }
        
        expr->add_number = 0;
    
    }
    
    return expr->add_number;

}

offset_t get_result_of_absolute_expression (char **pp) {

    struct expr expr;
    return absolute_expression_read_into (pp, &expr);

}

int expr_symbol_get_filename_and_line_number (struct symbol *symbol, const char **filename_p, unsigned long *line_number_p) {

    struct expr_symbol_line *es_line;
    
    for (es_line = expr_symbol_lines; es_line; es_line = es_line->next) {
    
        if (symbol == es_line->symbol) {
        
            *filename_p = es_line->filename;
            *line_number_p = es_line->line_number;
            
            return 0;
        
        }
    
    }
    
    return 1;

}

int resolve_expression (struct expr *expr) {

    struct frag *left_frag, *right_frag;
    
    struct symbol *add_symbol = expr->add_symbol;
    struct symbol *original_add_symbol = add_symbol;
    struct symbol *op_symbol = expr->op_symbol;
    
    value_t final_value = expr->add_number;
    value_t left_value, right_value;
    
    offset_t frag_offset;
    section_t left_section, right_section;
    
    switch (expr->type) {
    
        case EXPR_TYPE_CONSTANT:
        case EXPR_TYPE_REGISTER:
        
            left_value = 0;
            break;
        
        case EXPR_TYPE_SYMBOL:
        case EXPR_TYPE_SYMBOL_RVA:
        
            if (get_symbol_snapshot (&add_symbol, &left_value, &left_section, &left_frag)) {
                return 0;
            }
            
            break;
        
        case EXPR_TYPE_LOGICAL_NOT:
        case EXPR_TYPE_BIT_NOT:
        case EXPR_TYPE_UNARY_MINUS:
        
            if (get_symbol_snapshot (&add_symbol, &left_value, &left_section, &left_frag)) {
                return 0;
            }
            
            if (left_section != absolute_section) {
                return 0;
            }
            
            switch (expr->type) {
            
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
            
            expr->type = EXPR_TYPE_CONSTANT;
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
        
            if (get_symbol_snapshot (&add_symbol, &left_value, &left_section, &left_frag)
                || get_symbol_snapshot (&op_symbol, &right_value, &right_section, &right_frag)) {
                return 0;
            }
            
            if (expr->type == EXPR_TYPE_ADD) {
            
                if (right_section == absolute_section) {
                
                    final_value += right_value;
                    
                    expr->type = EXPR_TYPE_SYMBOL;
                    break;
                
                } else if (left_section == absolute_section) {
                
                    final_value += left_value;
                    
                    left_value = right_value;
                    left_section = right_section;
                    add_symbol = op_symbol;
                    original_add_symbol = expr->op_symbol;
                    
                    expr->type = EXPR_TYPE_SYMBOL;
                    break;
                
                }
            
            } else if (expr->type == EXPR_TYPE_SUBTRACT) {
            
                if (right_section == absolute_section) {
                
                    final_value -= right_value;
                    
                    expr->type = EXPR_TYPE_SYMBOL;
                    break;
                
                }
            
            }
            
            frag_offset = 0;
            
            /**
             * Equality and non-equality operations are allowed on everything.
             * Subtraction and other comparison operators are allowed if both operands are in the same section.
             * Bit OR, bit AND and multiplications are permitted with constant zero are permitted on anything.
             * Shifts by constant zero are permitted on anything.
             * Multiplication and division by constant one are permitted on anything.
             * Bit OR and AND are permitted on two same undefined symbols.
             * For everything else, both operands must be absolute.
             * Addition and subtraction of constants is handled above.
             */
            if (!(left_section == absolute_section &&
                  right_section == absolute_section) &&
                 !(expr->type == EXPR_TYPE_EQUAL ||
                   expr->type == EXPR_TYPE_NOT_EQUAL) &&
                 !((expr->type == EXPR_TYPE_SUBTRACT ||
                    expr->type == EXPR_TYPE_LESSER ||
                    expr->type == EXPR_TYPE_LESSER_EQUAL ||
                    expr->type == EXPR_TYPE_GREATER ||
                    expr->type == EXPR_TYPE_GREATER_EQUAL) &&
                   left_section == right_section &&
                   (finalize_symbols ||
                    frags_offset_is_fixed (left_frag, right_frag, &frag_offset) ||
                    (expr->type == EXPR_TYPE_GREATER &&
                     frags_is_greater_than_offset (left_value, left_frag, right_value, right_frag, &frag_offset))) &&
                    (left_section != undefined_section ||
                     add_symbol == op_symbol))) {
            
                if ((left_section == absolute_section && left_value == 0)
                    || (right_section == absolute_section && right_value == 0)) {
                
                    if (expr->type == EXPR_TYPE_BIT_INCLUSIVE_OR
                        || expr->type == EXPR_TYPE_BIT_EXCLUSIVE_OR) {
                    
                        if (!(right_section == absolute_section && right_value == 0)) {
                        
                            left_value = right_value;
                            left_section = right_section;
                            add_symbol = op_symbol;
                            original_add_symbol = expr->op_symbol;
                        
                        }
                        
                        expr->type = EXPR_TYPE_SYMBOL;
                        break;
                    
                    } else if (expr->type == EXPR_TYPE_LEFT_SHIFT || expr->type == EXPR_TYPE_RIGHT_SHIFT) {
                    
                        if (!(left_section == absolute_section && left_value == 0)) {
                        
                            expr->type = EXPR_TYPE_SYMBOL;
                            break;
                        
                        }
                    
                    } else if (expr->type != EXPR_TYPE_BIT_AND && expr->type != EXPR_TYPE_MULTIPLY) {
                        return 0;
                    }
                
                } else if (expr->type == EXPR_TYPE_MULTIPLY && left_section == absolute_section && left_value == 1) {
                
                    left_value = right_value;
                    left_section = right_section;
                    
                    add_symbol = op_symbol;
                    original_add_symbol = expr->op_symbol;
                    
                    expr->type = EXPR_TYPE_SYMBOL;
                    break;
                
                } else if ((expr->type == EXPR_TYPE_MULTIPLY ||
                            expr->type == EXPR_TYPE_DIVIDE) &&
                           right_section == absolute_section &&
                           right_value == 1) {
                
                    expr->type = EXPR_TYPE_SYMBOL;
                    break;
                
                } else if (!(left_value == right_value &&
                             ((left_section == reg_section &&
                               right_section == reg_section) ||
                              (left_section == undefined_section &&
                               right_section == undefined_section &&
                               add_symbol == op_symbol)))) {
                    return 0;
                } else if (expr->type == EXPR_TYPE_BIT_INCLUSIVE_OR || expr->type == EXPR_TYPE_BIT_AND) {
                
                    expr->type = EXPR_TYPE_SYMBOL;
                    break;
                
                } else if (expr->type != EXPR_TYPE_BIT_EXCLUSIVE_OR) {
                    return 0;
                }
            
            }
            
            right_value += frag_offset;
            
            switch (expr->type) {
            
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
                                   && (finalize_symbols || left_frag == right_frag)
                                   && (left_section != undefined_section
                                       || add_symbol == op_symbol))
                                  ? ~ (value_t) 0 : 0);
                    
                    if (expr->type == EXPR_TYPE_NOT_EQUAL) {
                        left_value = ~left_value;
                    }
                    
                    break;
                
                case EXPR_TYPE_LESSER:
                
                    left_value = (offset_t) left_value < (offset_t) right_value ? ~ (value_t) 0 : 0;
                    break;
                
                case EXPR_TYPE_LESSER_EQUAL:
                
                    left_value = (offset_t) left_value <= (offset_t) right_value ? ~ (value_t) 0 : 0;
                    break;
                
                case EXPR_TYPE_GREATER:
                
                    left_value = (offset_t) left_value > (offset_t) right_value ? ~ (value_t) 0 : 0;
                    break;
                
                case EXPR_TYPE_GREATER_EQUAL:
                
                    left_value = (offset_t) left_value >= (offset_t) right_value ? ~ (value_t) 0 : 0;
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
                
                    if (right_value == 0) {
                        return 0;
                    }
                    
                    left_value = (offset_t) left_value / (offset_t) right_value;
                    break;
                
                case EXPR_TYPE_MODULUS:
                
                    if (right_value == 0) {
                        return 0;
                    }
                    
                    left_value = (offset_t) left_value % (offset_t) right_value;
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
            
            expr->type = EXPR_TYPE_CONSTANT;
            break;
        
        default:
        
            return 0;
    
    }
    
    if (expr->type == EXPR_TYPE_SYMBOL) {
    
        if (left_section == absolute_section) {
            expr->type = EXPR_TYPE_CONSTANT;
        } else if (left_section == reg_section && final_value == 0) {
            expr->type = EXPR_TYPE_REGISTER;
        } else if (add_symbol != original_add_symbol) {
            final_value += left_value;
        }
        
        expr->add_symbol = add_symbol;
    
    }
    
    if (expr->type == EXPR_TYPE_CONSTANT) {
        final_value += left_value;
    }
    
    expr->add_number = final_value;
    return 1;

}
