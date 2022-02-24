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


static unsigned int hex_value(unsigned int z)
{
    if ((z >= '0') && (z <= '9')) return (z - '0');
    if ((z >= 'a') && (z <= 'f')) return (z + 10 - 'a');
    if ((z >= 'A') && (z <= 'F')) return (z + 10 - 'A');

    return (~0);
}

static void integer_constant(unsigned int radix, exprS *expr)
{
    unsigned long number;
    unsigned int digit_value;
    char c;

    c = *input_line_pointer++;
    for (number = 0;
         (digit_value = hex_value(c)) < radix;
         c = *input_line_pointer++)
    {
        number = number * radix + digit_value;
    }

    expr->type = Expr_type_constant;
    expr->add_number = number;
    --input_line_pointer;
}

static void operand(exprS *expr)
{
    char c;
    
    SKIP_WHITESPACE();

    c = *input_line_pointer++;

    switch (c)
    {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            input_line_pointer--;
            integer_constant(10, expr);
            break;

        case '0':
            c = *input_line_pointer;
            
            switch (c)
            {
                case 'x':
                case 'X':
                    input_line_pointer++;
                    integer_constant(16, expr);
                    break;

                case 'b':
                case 'B':
                    integer_constant(2, expr);
                    break;

                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    integer_constant(8, expr);
                    break;

                default:
                    /* The string is only a zero. */
                    expr->type = Expr_type_constant;
                    expr->add_number = 0;
                    break;
            }
            break;

        case '-':
            operand(expr);
            if (expr->type == Expr_type_constant)
            {
                expr->add_number = - expr->add_number;
            }
            else
            {
                as_error("+++operand\n");
            }
            break;

        case '\'':
            expr->type = Expr_type_constant;
            expr->add_number = *input_line_pointer++;
            if (*input_line_pointer++ != '\'')
            {
                as_error("+++operand\n");
            }
            break;

        default:
            if (is_name_beginner(c))
            {
                symbolS *symbol;
                char *name = --input_line_pointer;
                
                c = get_symbol_end();

                symbol = symbol_find_or_make(name);

                expr->type = Expr_type_symbol;
                expr->add_symbol = symbol;
                expr->add_number = 0;

                *input_line_pointer = c;
            }
            else
            {
                as_error("+++operand\n");
            }
            break;
    }

    SKIP_WHITESPACE();
}

void expr_read_into(exprS *expr)
{
    operand(expr);

    if ((*input_line_pointer == '\n')
        || (*input_line_pointer == '\0')) return;
}
