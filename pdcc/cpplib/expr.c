/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include "cpplib.h"
#include "internal.h"
#include "support.h"
#include "xmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define PART_PRECISION (sizeof(cpp_number_part) * CHAR_BIT)
#define HALF_MASK ((~(cpp_number_part)0) >> (PART_PRECISION / 2))
#define LOW_PART(number_part) ((number_part) & HALF_MASK)
#define HIGH_PART(number_part) ((number_part) >> (PART_PRECISION / 2))

struct op {
    const cpp_token *token;
    cpp_number value; /* The value right from the operator. */
    enum cpp_tokentype op;
    location_t loc;
};

#define CPP_UPLUS ((enum cpp_tokentype) CPP_LAST_CPP_OP + 1)
#define CPP_UMINUS ((enum cpp_tokentype) CPP_LAST_CPP_OP + 2)

#define SYNTAX_ERROR_AT(loc, message) \
    do { \
        cpp_error_with_line(reader, CPP_DL_ERROR, (loc), \
                            0, (message)); \
        goto syntax_error;} while (0)
#define SYNTAX_ERROR2_AT(loc, message, arg) \
    do { \
        cpp_error_with_line(reader, CPP_DL_ERROR, (loc), \
                            0, (message), (arg)); \
        goto syntax_error;} while (0)

#define number_zerop(a) (((a).high == 0) && ((a).low == 0))
#define numbers_eq(a, b) (((a).high == (b).high) \
                          && ((a).low == (b).low))

unsigned int cpp_interpret_float_suffix(cpp_reader *reader,
                                        const char *str,
                                        size_t len)
{
    size_t f, l;

    f = l = 0;

    while (len--)
    {
        switch(str[0])
        {
            case 'f':
            case 'F':
                f++;
                break;

            case 'l':
            case 'L':
                l++;
                break;

            default: return (0);
        }
        str++;
    }

    if (f + l > 1) return (0);
    
    return (f ? CPP_NUMBER_SMALL :
            l ? CPP_NUMBER_LARGE :
            CPP_NUMBER_DEFAULT);
}

unsigned int cpp_interpret_int_suffix(cpp_reader *reader,
                                      const char *str,
                                      size_t len)
{
    size_t l = 0, u = 0;
    while (len--)
    {
        switch(str[len])
        {
            case 'u':
            case 'U':
                u++;
                break;
            
            case 'l':
            case 'L':
                l++;
                /* If 2 Ls are present,
                 * they must be the same case and adjacent. */
                if ((l == 2) && (str[len] != str[len + 1])) return (0);
                break;

            default: return (0);
        }
    }

    if ((l > 2) || (u > 1)) return (0);
    
    return ((u ? CPP_NUMBER_UNSIGNED : 0)
            | ((l == 0) ? CPP_NUMBER_SMALL :
               (l == 1) ? CPP_NUMBER_MEDIUM :
               CPP_NUMBER_LARGE));
}

static unsigned int hex_value(unsigned int z)
{
    if ((z >= '0') && (z <= '9')) return (z - '0');
    if ((z >= 'a') && (z <= 'f')) return (z + 10 - 'a');
    if ((z >= 'A') && (z <= 'F')) return (z + 10 - 'A');

    fprintf(stderr, "CPPLIB Internal error %s:%u\n", __FILE__, __LINE__);
    return 0;
}

unsigned int cpp_classify_number(cpp_reader *reader,
                                 const cpp_token *token,
                                 location_t virtual_loc)
{
    const char *str = token->value.string.text;
    const char *end;
    unsigned int result, radix, max_digit;
    enum {NOT_FLOAT, AFTER_POINT, AFTER_EXPONENT} float_flag;
    int seen_digit;

    /* If len is 1, the number is just one digit. */
    if (token->value.string.len == 1)
        return (CPP_NUMBER_INTEGER | CPP_NUMBER_SMALL | CPP_NUMBER_DECIMAL);

    end = str + (token->value.string.len);
    float_flag = NOT_FLOAT;
    radix = 10;
    max_digit = 0;
    seen_digit = 0;

    /* Interprets the radix first. */
    if (*str == '0')
    {
        radix = 8;
        str++;

        /* At least one correct digit after binary or hexadecimal prefix
         * is required for the prefix to be valid. */
        switch (*str)
        {
            case 'x':
            case 'X':
                if ((str[1] == '.') || ISXDIGIT(str[1]))
                {
                    radix = 16;
                    str++;
                }
                break;
            
            case 'b':
            case 'B':
                if ((str[1] == '0') || (str[1] == '1'))
                {
                    radix = 2;
                    str++;
                }
                break;
        }
    }

    for (;;)
    {
        unsigned int c = *(str++);

        if (ISDIGIT(c) || (ISXDIGIT(c) && (radix == 16)))
        {
            seen_digit = 1;
            c = hex_value(c);
            if (c > max_digit) max_digit = c;
        }
        else if (c == '.')
        {
            if (float_flag == NOT_FLOAT)
                float_flag = AFTER_POINT;
            else
            {
                SYNTAX_ERROR_AT(virtual_loc,
                                "too many decimal points in number");
            }
        }
        else if (((radix <= 10) && ((c == 'e') || (c == 'E')))
                 || ((radix == 16) && ((c == 'p') || (c == 'P'))))
        {
            float_flag = AFTER_EXPONENT;
            break;
        }
        else
        {
            /* Start of suffix. */
            str--;
            break;
        }
    }

    if ((float_flag != NOT_FLOAT) && (radix == 8)) radix = 10;

    /* Can happen only in binary and octal constants. */
    if (max_digit > radix)
    {
        if (radix == 2)
            SYNTAX_ERROR2_AT(virtual_loc,
                             "invalid digit \"%c\" in binary constant",
                             '0' + max_digit);
        else
            SYNTAX_ERROR2_AT(virtual_loc,
                             "invalid digit \"%c\" in octal constant",
                             '0' + max_digit);
    }

    if (float_flag != NOT_FLOAT)
    {
        if (radix == 2)
        {
            cpp_error_with_line(reader,
                                CPP_DL_ERROR,
                                virtual_loc,
                                0,
                                "invalid prefix \"0b\" for floating constant");
            return (CPP_NUMBER_INVALID);
        }

        if ((radix == 16) && !seen_digit)
        {
            SYNTAX_ERROR_AT(virtual_loc,
                            "no digits in hexadecimal floating constant");
        }

        if ((radix == 16) && !CPP_OPTION(reader, extended_numbers))
        {
            cpp_error_with_line(reader,
                                CPP_DL_WARNING,
                                virtual_loc,
                                0,
                                "use of hexadecimal floating constant");
        }

        if (float_flag == AFTER_EXPONENT)
        {
            if ((*str == '+') || (*str == '-')) str++;

            /* Exponent is always decimal. */
            if (!ISDIGIT(*str))
            {
                SYNTAX_ERROR_AT(virtual_loc,
                                "exponent has no digits");
            }

            do {
                str++;
            } while (ISDIGIT(*str));
        }
        else if (radix == 16)
        {
            SYNTAX_ERROR_AT(virtual_loc,
                            "hexadecimal floating constants"
                            " require an exponent");
        }

        result = cpp_interpret_float_suffix(reader, str, end - str);
        if (result == 0)
        {
            cpp_error_with_line(reader,
                                CPP_DL_ERROR,
                                virtual_loc,
                                0,
                                "invalid suffix \"%s\" on floating constant",
                                str);
            return (CPP_NUMBER_INVALID);
        }

        result |= CPP_NUMBER_FLOAT;
    }
    else
    {
        result = cpp_interpret_int_suffix(reader, str, end - str);
        if (result == 0)
        {
            cpp_error_with_line(reader,
                                CPP_DL_ERROR,
                                virtual_loc,
                                0,
                                "invalid suffix \"%s\" on integer constant",
                                str);
            return (CPP_NUMBER_INVALID);
        }

        result |= CPP_NUMBER_INTEGER;
    }

    switch (radix)
    {
        case 10: result |= CPP_NUMBER_DECIMAL; break;
        case 16: result |= CPP_NUMBER_HEX; break;
        case 8: result |= CPP_NUMBER_OCTAL; break;
        case 2: result |= CPP_NUMBER_BINARY; break;
    }

    return (result);

syntax_error:
    return (CPP_NUMBER_INVALID);
}

static cpp_number number_trim(cpp_number result, size_t precision)
{
    if (precision > PART_PRECISION)
    {
        precision -= PART_PRECISION;
        if (precision < PART_PRECISION)
            result.high &= (((cpp_number_part)1) << precision) - 1;
    }
    else
    {
        if (precision < PART_PRECISION)
            result.low &= (((cpp_number_part)1) << precision) - 1;
        result.high = 0;
    }
    
    return (result);
}

static cpp_number append_digit(cpp_number num,
                               unsigned int digit,
                               unsigned int base,
                               size_t precision)
{
    cpp_number result;
    unsigned int shift;
    cpp_number_part add_high, add_low;

    switch (base)
    {
        case 2: shift = 1; break;
        case 16: shift = 4; break;
        default: shift = 3; break;
    }

    result.overflow = !!(num.high >> (PART_PRECISION - shift));
    result.high = num.high << shift;
    result.low = num.low << shift;
    result.high |= num.low >> (PART_PRECISION - shift);
    result.unsignedp = num.unsignedp;

    if (base == 10)
    {
        add_low = num.low << 1;
        add_high = (num.high << 1) + (num.low >> (PART_PRECISION - 1));
    }
    else add_low = add_high = 0;

    if (add_low + digit < add_low) add_high++;
    add_low += digit;

    if (result.low + add_low < result.low)
        add_high++;
    if (result.high + add_high < result.high)
        result.overflow = 1;

    result.low += add_low;
    result.high += add_high;

    num.low = result.low;
    num.high = result.high;
    result = number_trim(result, precision);
    if (!numbers_eq(result, num))
        result.overflow = 1;
    
    return (result);
}

static int number_positive(cpp_number num, size_t precision)
{
    if (precision > PART_PRECISION)
    {
        precision -= PART_PRECISION;
        return ((num.high & (((cpp_number_part)1) << (precision - 1))) == 0);
    }
    return (num.low & (((cpp_number_part)1) << (precision - 1))) == 0;
}

cpp_number cpp_interpret_integer(cpp_reader *reader,
                                 const cpp_token *token,
                                 unsigned int type)
{
    cpp_number result;
    const char *p;

    result.unsignedp = !!(type & CPP_NUMBER_UNSIGNED);
    result.overflow = 0;
    result.high = result.low = 0;

    p = token->value.string.text;
    
    if (token->value.string.len == 1)
    {
        result.low = p[0] - '0';
    }
    else
    {
        cpp_number_part max;
        size_t precision = CPP_OPTION(reader, precision);
        const char *end = p + (token->value.string.len);
        unsigned int base = 10;
        int overflow = 0;

        switch (type & CPP_NUMBER_RADIX)
        {
            case CPP_NUMBER_OCTAL: base = 8; p++; break;
            case CPP_NUMBER_HEX: base = 16; p += 2; break;
            case CPP_NUMBER_BINARY: base = 2; p += 2; break;
        }

        max = ~(cpp_number_part)0;
        if (precision < PART_PRECISION)
        {
            max >>= PART_PRECISION - precision;
        }
        max = (max - base + 1) / base + 1;

        for (; p < end; p++)
        {
            unsigned int c = *p;

            if (ISDIGIT(c) || (ISXDIGIT(c) && (base == 16)))
            {
                c = hex_value(c);
            }
            else break;

            if (result.low < max)
            {
                result.low = result.low * base + c;
            }
            else
            {
                result = append_digit(result, c, base, precision);
                overflow |= result.overflow;
                max = 0;
            }
        }

        if (overflow)
        {
            cpp_error(reader, CPP_DL_WARNING,
                      "integer constant is too large for its type");
        }
        else if (!(result.unsignedp)
                 && !number_positive(result, precision))
        {
            if (base == 10)
                cpp_error(reader, CPP_DL_WARNING,
                          "integer constant is so large that it is unsigned");
            result.unsignedp = 1;
        }
    }
    
    return (result);
}

cpp_number handle_defined(cpp_reader *reader)
{
    cpp_number result;
    const cpp_token *token;
    int paren = 0;
    cpp_unknown *unknown = NULL;
    cpp_unknown2 *initial_unknown2 = reader->unknown2;

    reader->state.prevent_expansion++;

    token = cpp_get_token(reader);
    if (token->type == CPP_LPAREN)
    {
        paren = 1;
        token = cpp_get_token(reader);
    }

    if (token->type == CPP_IDENT)
    {
        unknown = token->value.unknown;
        if (paren && (cpp_get_token(reader)->type != CPP_RPAREN))
        {
            cpp_error(reader, CPP_DL_ERROR,
                      "missing ')' after \"defined\"");
            unknown = NULL;
        }
    }
    else
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "operat \"defined\" requires an identifier");
    }

    if (unknown)
    {
        if ((reader->unknown2 != initial_unknown2)
            || (initial_unknown2 != NULL))
        {
            cpp_error(reader, CPP_DL_WARNING,
                      "this use of \"defined\" may not be portable"
                      " (expansion to \"defined\")");
        }
    }

    reader->state.prevent_expansion--;

    result.unsignedp = 0;
    result.overflow = 0;
    result.high = 0;
    result.low = (unknown && cpp_macro_s(unknown));
    
    return (result);
}

static cpp_number evaluate_token(cpp_reader *reader,
                                 const cpp_token *token,
                                 location_t loc)
{
    cpp_number result;

    result.unsignedp = 0;
    result.overflow = 0;
    
    switch (token->type)
    {
        case CPP_NUMBER: {
            unsigned int type = cpp_classify_number(reader, token, loc);

            switch (type & CPP_NUMBER_CATEGORY)
            {
                case CPP_NUMBER_FLOAT:
                    cpp_error_with_line(reader,
                                        CPP_DL_ERROR,
                                        loc,
                                        0,
                                        "floating constant"
                                        " in preprocessor expression");
                    break;

                case CPP_NUMBER_INTEGER:
#ifdef __CC64__
                    result = cpp_interpret_integer(reader, token, type);
                    goto myend;
#else
                    return (cpp_interpret_integer(reader, token, type));
#endif

                case CPP_NUMBER_INVALID:
                    break;
            }

            result.high = result.low = 0;
            break;
        }

        case CPP_CHAR:
        case CPP_LCHAR:
        case CPP_uCHAR:
        case CPP_UCHAR: {
            unsigned int temp;
            cppchar_t cc = cpp_interpret_charconst(reader, token,
                                                   &temp, &(result.unsignedp));
            result.high = 0;
            result.low = cc;

            if (!(result.unsignedp) && (((cppchar_signed_t)cc) < 0))
            {
                if (PART_PRECISION > 32)
                {
                    result.low |= ~((~((cpp_number_part)0))
                                    >> (PART_PRECISION - 32));
                }
                result.high = ~(cpp_number_part)0;
                result = number_trim(result, CPP_OPTION(reader, precision));
#ifdef __CC64__
                goto myend;
#endif
            }
            break;
        }
        
        case CPP_IDENT:
            if (token->value.unknown == (reader->spec_unknowns.n_defined))
#ifdef __CC64__
                result = handle_defined(reader);
                goto myend;
#else
                return (handle_defined(reader));
#endif
            result.high = result.low = 0;
            break;

        default:
            fprintf(stderr,
                    "CPPLIB Internal error %s:%u\n", __FILE__, __LINE__);
            return result;
    }

#ifdef __CC64__
myend:
#endif
    return (result);
}

#define NO_L_OPERAND     0x1
#define LEFT_ASSOCIATIVE 0x2
#define CHECK_PROMOTION  0x4

static const struct cpp_operator {
    unsigned int priority;
    unsigned int flags;
} optab[] = {
    /* EQ */ {0, 0}, /* Should not happen. */
    /* NOT */ {16, NO_L_OPERAND},
    /* GREATER */ {12, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* LESS */ {12, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* PLUS */ {13+1, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* MINUS */ {13+1, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* STAR */ {15, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* DIV */ {15, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* MODULO */ {15, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* AND */ {9, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* OR */ {7, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* XOR */ {8, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* LSHIFT */ {13, LEFT_ASSOCIATIVE},
    /* RSHIFT */ {13, LEFT_ASSOCIATIVE},

    /* COMPLEMENT */ {16, NO_L_OPERAND},
    /* AND_AND */ {6, LEFT_ASSOCIATIVE},
    /* OR_OR */ {5, LEFT_ASSOCIATIVE},
    /* QUERY */ {4, 0},
    /* COLON */ {4, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* COMMA */ {4, LEFT_ASSOCIATIVE},
    /* LPAREN */ {1, NO_L_OPERAND},
    /* RPAREN */ {0, 0},
    /* END */ {0, 0},
    /* EQ_EQ */ {11, LEFT_ASSOCIATIVE},
    /* NOT_EQ */ {11, LEFT_ASSOCIATIVE},
    /* GREATER_EQ */ {12, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* LESS_EQ */ {12, LEFT_ASSOCIATIVE | CHECK_PROMOTION},
    /* UPLUS */ {16, NO_L_OPERAND},
    /* UMINUS */ {16, NO_L_OPERAND}
};

static void check_promotion(cpp_reader *reader, struct op *op)
{
    if (op->value.unsignedp == (op[-1].value.unsignedp))
        return;

    if (op->value.unsignedp)
    {
        if (!number_positive(op[-1].value, CPP_OPTION(reader, precision)))
        {
            cpp_error_with_line(reader,
                                CPP_DL_WARNING,
                                op[-1].loc,
                                0,
                                "the left operand of \"%s\""
                                " changes sign when promoted",
                                cpp_token_as_text(op->token));
        }
    }
    else if (!number_positive(op->value, CPP_OPTION(reader, precision)))
    {
        cpp_error_with_line(reader,
                            CPP_DL_WARNING,
                            op[-1].loc,
                            0,
                            "the right operand of \"%s\""
                            " changes sign when promoted",
                            cpp_token_as_text(op->token));
    }
}

static cpp_number number_negate(cpp_number num, size_t precision)
{
    cpp_number copy = num;

    num.low = ~(num.low);
    num.high = ~(num.high);
    if (++(num.low) == 0) num.high++;
    num = number_trim(num, precision);
    num.overflow = (!(num.unsignedp)
                    && numbers_eq(num, copy)
                    && !number_zerop(num));

    return (num);
}

static cpp_number number_unary_op(cpp_reader *reader, cpp_number num,
                                        enum cpp_tokentype op)
{
    switch (op)
    {
        case CPP_UPLUS:
            num.overflow = 0;
            break;

        case CPP_UMINUS:
            num = number_negate(num, CPP_OPTION(reader, precision));
            break;

        case CPP_NOT:
            num.low = number_zerop(num);
            num.high = 0;
            num.unsignedp = 0;
            num.overflow = 0;
            break;

        case CPP_COMPLEMENT:
            num.low = ~(num.low);
            num.high = ~(num.high);
            num = number_trim(num, CPP_OPTION(reader, precision));
            num.overflow = 0;
            break;

        default:
            cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                      "invalid operator in number_unary_op");
            break;
    }

    return (num);
}

static cpp_number number_rshift(cpp_number num,
                                size_t precision,
                                size_t n)
{
    cpp_number_part sign_mask;

    if (num.unsignedp || number_positive(num, precision))
        sign_mask = 0;
    else sign_mask = ~(cpp_number_part)0;

    if (n >= precision) num.high = num.low = sign_mask;
    else
    {
        if (precision < PART_PRECISION)
        {
            num.high = sign_mask;
            num.low |= sign_mask << precision;
        }
        else if (precision < 2 * PART_PRECISION)
            num.high |= sign_mask << (precision - PART_PRECISION);

        if (n >= PART_PRECISION)
        {
            n -= PART_PRECISION;
            num.low = num.high;
            num.high = sign_mask;
        }

        if (n)
        {
            num.low = (num.low >> n) | (num.high << (PART_PRECISION - n));
            num.high = (num.high >> n) | (sign_mask << (PART_PRECISION - n));
        }
    }

    num = number_trim(num, precision);
    num.overflow = 0;
    
    return (num);
}

static cpp_number number_lshift(cpp_number num,
                                size_t precision,
                                size_t n)
{
    if (n >= precision)
    {
        num.overflow = !(num.unsignedp) && !number_zerop(num);
        num.high = num.low = 0;
    }
    else
    {
        cpp_number orig, back;
        size_t n2 = n;

        orig = num;
        if (n2 >= PART_PRECISION)
        {
            n2 -= PART_PRECISION;
            num.high = num.low;
            num.low = 0;
        }
        if (n2)
        {
            num.high = (num.high << n2) | (num.low >> (PART_PRECISION - n2));
            num.low <<= n2;
        }
        num = number_trim(num, precision);
        if (num.unsignedp) num.overflow = 0;
        else
        {
            back = number_rshift(num, precision, n);
            num.overflow = !numbers_eq(orig, back);
        }
    }

    return (num);
}

static cpp_number number_binary_op(cpp_reader *reader,
                                   cpp_number num1,
                                   cpp_number num2,
                                   enum cpp_tokentype op)
{
    cpp_number result;
    size_t precision = CPP_OPTION(reader, precision);

    switch (op)
    {
        case CPP_PLUS:
            result.low = num1.low + num2.low;
            result.high = num1.high + num2.high;
            if (result.low < num1.low)
                result.high++;
            result.unsignedp = num1.unsignedp || (num2.unsignedp);
            result.overflow = 0;

            result = number_trim(result, precision);
            if (!(result.unsignedp))
            {
                result.overflow = ((number_positive(num1, precision)
                                    == number_positive(num2, precision))
                                   &&
                                   (number_positive(num1, precision)
                                    != number_positive(result, precision)));
            }
            return (result);

        case CPP_MINUS:
            result.low = num1.low - num2.low;
            result.high = num1.high - num2.high;
            if (result.low > num1.low)
                result.high--;
            result.unsignedp = num1.unsignedp || (num2.unsignedp);
            result.overflow = 0;

#ifdef __CC64__
#else
            result = number_trim(result, precision);
#endif
            if (!(result.unsignedp))
            {
                result.overflow = ((number_positive(num1, precision)
                                    != number_positive(num2, precision))
                                   &&
                                   (number_positive(num1, precision)
                                    != number_positive(result, precision)));
            }
            return (result);
        
        case CPP_LSHIFT:
        case CPP_RSHIFT: {
            size_t n;
            if (!(num2.unsignedp) && !number_positive(num2, precision))
            {
                /* Negative shift one way is positive shift the other way. */
                op = op == CPP_LSHIFT ? CPP_RSHIFT : CPP_LSHIFT;
#ifdef __CC64__
#else
                num2 = number_negate(num2, precision);
#endif
            }
            if (num2.high) n = ~0;
            else n = num2.low;
            if (op == CPP_LSHIFT)
#ifdef __CC64__
;
#else
                num1 = number_lshift(num1, precision, n);
#endif
            else
#ifdef __CC64__
;
#else
                num1 = number_rshift(num1, precision, n);
#endif
            break;
        }

        case CPP_COMMA:
            if (!CPP_OPTION(reader, c99) && !(reader->state.skip_eval))
                cpp_error(reader, CPP_DL_WARNING,
                          "comma operator in preprocessor expression");
            num1 = num2;
            break;

        default:
            cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                      "invalid operator in number_binary_op");
            break;
    }

    return (num1);
}

/* Returns 1, if num1 >= num2. */
static int number_greater_eq(cpp_number num1,
                             cpp_number num2,
                             size_t precision)
{
    int unsignedp = num1.unsignedp || (num2.unsignedp);

    if (!unsignedp)
    {
        /* Both are signed, so if the signs are different,
         * the answer is sign of the first one. */
        unsignedp = number_positive(num1, precision);

        if (unsignedp != number_positive(num2, precision))
            return (unsignedp);

        /* Otherwise unsigned comparison is done. */
    }

    return ((num1.high > num2.high)
            || ((num1.high == num2.high)
                && (num1.low >= num2.low)));
}

static cpp_number number_inequality_op(cpp_reader *reader,
                                       cpp_number num1,
                                       cpp_number num2,
                                       enum cpp_tokentype op)
{
    int gte = number_greater_eq(num1, num2,
                                CPP_OPTION(reader, precision));

    switch (op)
    {
        case CPP_GREATER:
            num1.low = gte && !numbers_eq(num1, num2);
            break;
        
        case CPP_LESS:
            num1.low = !gte;
            break;
        
        case CPP_GREATER_EQ:
            num1.low = gte;
            break;
        
        case CPP_LESS_EQ:
            num1.low = !gte || numbers_eq(num1, num2);
            break;

        default:
            cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                      "invalid operator in number_inequality_op");
            return (num1);
    }
    
    num1.high = 0;
    num1.unsignedp = 0;
    num1.overflow = 0;
    return (num1);
}

static cpp_number number_equality_op(cpp_reader *reader,
                                     cpp_number num1,
                                     cpp_number num2,
                                     enum cpp_tokentype op)
{
    switch (op)
    {
        case CPP_EQ_EQ:
            num1.low = numbers_eq(num1, num2);
            break;
        
        case CPP_NOT_EQ:
            num1.low = !numbers_eq(num1, num2);
            break;

        default:
            cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                      "invalid operator in number_equality_op");
            return (num1);
    }

    num1.high = 0;
    num1.unsignedp = 0;
    num1.overflow = 0;
    return (num1);
}

static cpp_number number_bitwise_op(cpp_reader *reader,
                                    cpp_number num1,
                                    cpp_number num2,
                                    enum cpp_tokentype op)
{
    num1.overflow = 0;
    num1.unsignedp = num1.unsignedp || (num2.unsignedp);

    switch (op)
    {
        case CPP_AND:
            num1.high &= num2.high;
            num1.low &= num2.low;
            break;
        
        case CPP_OR:
            num1.high |= num2.high;
            num1.low |= num2.low;
            break;
        
        case CPP_XOR:
            num1.high ^= num2.high;
            num1.low ^= num2.low;
            break;

        default:
            cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                      "invalid operator in number_bitwise_op");
            break;
    }

    return (num1);
}

static cpp_number number_part_multiply(cpp_number_part num1,
                                       cpp_number_part num2)
{
    cpp_number result;
    cpp_number_part middle[2], temp;

    result.low = LOW_PART(num1) * LOW_PART(num2);
    result.high = HIGH_PART(num1) * HIGH_PART(num2);

    middle[0] = LOW_PART(num1) * HIGH_PART(num2);
    middle[1] = HIGH_PART(num1) * LOW_PART(num2);

    temp = result.low;
    result.low += LOW_PART(middle[0]) << (PART_PRECISION / 2);
    if (result.low < temp)
        result.high++;

    temp = result.low;
    result.low += LOW_PART(middle[1]) << (PART_PRECISION / 2);
    if (result.low < temp)
        result.high++;

    result.high += HIGH_PART(middle[0]);
    result.high += HIGH_PART(middle[1]);
    result.unsignedp = 1;
    result.overflow = 0;

    return (result);
}

static cpp_number number_multiply(cpp_reader *reader,
                                  cpp_number num1,
                                  cpp_number num2)
{
    int negate = 0;
    int unsignedp = num1.unsignedp || (num2.unsignedp);
    int overflow;
    size_t precision = CPP_OPTION(reader, precision);
    cpp_number result, temp;

    /* Prepares for unsigned multiplication. */
    if (!unsignedp)
    {
        if (!number_positive(num1, precision))
        {
            negate = !negate;
            num1 = number_negate(num1, precision);
        }
        if (!number_positive(num2, precision))
        {
            negate = !negate;
            num2 = number_negate(num2, precision);
        }
    }

    overflow = num1.high && num2.high;
    result = number_part_multiply(num1.low, num2.low);

    temp = number_part_multiply(num1.high, num2.low);
    result.high += temp.low;
    if (temp.high) overflow = 1;

    temp = number_part_multiply(num1.low, num2.high);
    result.high += temp.low;
    if (temp.high) overflow = 1;

    temp.low = result.low;
    temp.high = result.high;
    result = number_trim(result, precision);
    if (!numbers_eq(result, temp)) overflow = 1;

    if (negate) result = number_negate(result, precision);

    if (unsignedp) result.overflow = 0;
    else
    {
        result.overflow = (overflow
                           || ((number_positive(result, precision) ^ !negate)
                               && !number_zerop(result)));
    }

    result.unsignedp = unsignedp;

    return (result);
}

static cpp_number number_div_op(cpp_reader *reader,
                                cpp_number num1,
                                cpp_number num2,
                                enum cpp_tokentype op,
                                location_t location)
{
    int negate = 0;
    int negate_num1 = 0;
    int unsignedp = num1.unsignedp || (num2.unsignedp);
    size_t i, precision = CPP_OPTION(reader, precision);
    cpp_number result, sub;
    cpp_number_part mask;

    /* Prepares for unsigned division. */
    if (!unsignedp)
    {
        if (!number_positive(num1, precision))
        {
            negate = !negate;
            negate_num1 = 1;
            num1 = number_negate(num1, precision);
        }
        if (!number_positive(num2, precision))
        {
            negate = !negate;
            num2 = number_negate(num2, precision);
        }
    }

    if (num2.high)
    {
        i = precision - 1;
        mask = ((cpp_number_part)1) << (i - PART_PRECISION);
        for (;; i--, mask >>= 1)
        {
            if (num2.high & mask) break;
        }
    }
    else if (num2.low)
    {
        if (precision > PART_PRECISION)
        {
            i = precision - PART_PRECISION - 1;
        }
        else i = precision - 1;
        mask = ((cpp_number_part)1) << i;
        for (;; i--, mask >>= 1)
        {
            if (num2.low & mask) break;
        }
    }
    else
    {
        if (!(reader->state.skip_eval))
        {
            cpp_error_with_line(reader,
                                CPP_DL_ERROR,
                                location,
                                0,
                                "division by zero in preprocessor expression");
        }
        return (num1);
    }

    num1.unsignedp = 1;
    num2.unsignedp = 1;
    i = precision - i - 1;
#ifdef __CC64__
#else
    sub = number_lshift(num2, precision, i);
#endif

    result.high = result.low = 0;
    for (;;)
    {
        if (number_greater_eq(num1, sub, precision))
        {
#ifdef __CC64__
#else
            num1 = number_binary_op(reader, num1, sub, CPP_MINUS);
#endif
            if (i >= PART_PRECISION)
                result.high |= ((cpp_number_part)1) << (i - PART_PRECISION);
            else
                result.low |= ((cpp_number_part)1) << i;
        }
        if (i-- == 0) break;
        sub.low = (sub.low >> 1) || (sub.high << (PART_PRECISION - 1));
        sub.high >>= 1;
    }

    switch (op)
    {
        case CPP_DIV:
            result.unsignedp = unsignedp;
            result.overflow = 0;
            if (!unsignedp)
            {
#ifdef __CC64__
#else
                if (negate) result = number_negate(result, precision);
#endif
                result.overflow = ((number_positive(result, precision)
                                    ^ !negate)
                                   && !number_zerop(result));
            }

            return (result);

        case CPP_MODULO:
            num1.unsignedp = unsignedp;
            num1.overflow = 0;
#ifdef __CC64__
#else
            if (negate_num1) num1 = number_negate(num1, precision);
#endif
            break;
                
        default:
            cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                      "invalid operator in number_div_op");
            break;
    }

    return (num1);
}

static struct op *reduce(cpp_reader *reader,
                         struct op *top,
                         enum cpp_tokentype op)
{
    unsigned int priority;

    if ((top->op <= CPP_EQ) || (top->op > CPP_LAST_CPP_OP + 2))
    {
        cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                  "impossible operator '%u'", top->op);
        return (NULL);
    }

    if (op == CPP_LPAREN) return (top);

    priority = (optab[op].priority
                - ((optab[op].flags & LEFT_ASSOCIATIVE) != 0));
    while (priority < optab[top->op].priority)
    {
        if (optab[top->op].flags & CHECK_PROMOTION)
            check_promotion(reader, top);

        switch (top->op)
        {
            case CPP_UPLUS:
            case CPP_UMINUS:
            case CPP_NOT:
            case CPP_COMPLEMENT:
#ifdef __CC64__
#else
                top[-1].value = number_unary_op(reader,
                                                top->value,
                                                top->op);
#endif
                top[-1].loc = top->loc;
                break;

            case CPP_PLUS:
            case CPP_MINUS:
            case CPP_LSHIFT:
            case CPP_RSHIFT:
            case CPP_COMMA:
#ifdef __CC64__
#else
                top[-1].value = number_binary_op(reader,
                                                 top[-1].value,
                                                 top->value,
                                                 top->op);
#endif
                top[-1].loc = top->loc;
                break;

            case CPP_GREATER:
            case CPP_LESS:
            case CPP_GREATER_EQ:
            case CPP_LESS_EQ:
#ifdef __CC64__
#else
                top[-1].value = number_inequality_op(reader,
                                                     top[-1].value,
                                                     top->value,
                                                     top->op);
#endif
                top[-1].loc = top->loc;
                break;

            case CPP_EQ_EQ:
            case CPP_NOT_EQ:
#ifdef __CC64__
#else
                top[-1].value = number_equality_op(reader,
                                                   top[-1].value,
                                                   top->value,
                                                   top->op);
#endif
                top[-1].loc = top->loc;
                break;

            case CPP_AND:
            case CPP_OR:
            case CPP_XOR:
#ifdef __CC64__
#else
                top[-1].value = number_bitwise_op(reader,
                                                  top[-1].value,
                                                  top->value,
                                                  top->op);
#endif
                top[-1].loc = top->loc;
                break;

            case CPP_STAR:
#ifdef __CC64__
#else
                top[-1].value = number_multiply(reader,
                                                top[-1].value,
                                                top->value);
#endif
                top[-1].loc = top->loc;
                break;

            case CPP_DIV:
            case CPP_MODULO:
#ifdef __CC64__
#else
                top[-1].value = number_div_op(reader,
                                              top[-1].value,
                                              top->value,
                                              top->op,
                                              top->loc);
#endif
                top[-1].loc = top->loc;
                break;

            case CPP_AND_AND:
                top--;
                if (number_zerop(top->value))
                    reader->state.skip_eval--;
                top->value.low = (!number_zerop(top->value)
                                  && !number_zerop(top[1].value));
                top->value.high = 0;
                top->value.unsignedp = 0;
                top->value.overflow = 0;
                top->loc = top[1].loc;
                continue;

            case CPP_OR_OR:
                top--;
                if (!number_zerop(top->value))
                    reader->state.skip_eval--;
                top->value.low = (!number_zerop(top->value)
                                  || !number_zerop(top[1].value));
                top->value.high = 0;
                top->value.unsignedp = 0;
                top->value.overflow = 0;
                top->loc = top[1].loc;
                continue;

            case CPP_LPAREN:
                if (op != CPP_RPAREN)
                {
                    cpp_error_with_line(reader, CPP_DL_ERROR,
                                        top->token->src_loc,
                                        0,
                                        "missing ')'"
                                        " in preprocessor expression");
                    return (NULL);
                }
                top--;
                top->value = top[1].value;
                top->loc = top[1].loc;
                return (top);

            case CPP_QUERY:
                if ((op == CPP_COMMA) || (op == CPP_COLON))
                    return (top);
                cpp_error(reader, CPP_DL_ERROR,
                          "'?' without following ':'");
                return (NULL);

            case CPP_COLON:
                top -= 2;
                if (!number_zerop(top->value))
                {
                    reader->state.skip_eval--;
                    top->value = top[1].value;
                    top->loc = top[1].loc;
                }
                else
                {
                    top->value = top[2].value;
                    top->loc = top[2].loc;
                }
                top->value.unsignedp = ((top[1].value.unsignedp)
                                        || (top[2].value.unsignedp));
                continue;

            default:
                cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                          "impossible operator '%u'", top->op);
                return (NULL);
        }

        top--;
        if (top->value.overflow
            && !(reader->state.skip_eval))
            cpp_error(reader, CPP_DL_WARNING,
                      "integer overflow in preprocessor expression");
    }

    if (op == CPP_RPAREN)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "missing '(' in expression");
        return (NULL);
    }
    
    return (top);
}

/* Parses and evaluates expression from reader
 * and returns the truth value of the expression. */
int _cpp_process_expr(cpp_reader *reader, int is_if)
{
    struct op *top = reader->op_stack;
    int wants_value = 1;
    location_t virtual_location;
    
    reader->state.skip_eval = 0;

    top->op = CPP_END;
    
    for (;;)
    {
        struct op op;

        op.token = cpp_get_token_with_location(reader, &virtual_location);
        op.op = op.token->type;
        op.loc = virtual_location;

        switch (op.op)
        {
            case CPP_PADDING: continue;
            
            /* These tokens convert into values. */
            case CPP_NUMBER:
            case CPP_CHAR:
            case CPP_LCHAR:
            case CPP_uCHAR:
            case CPP_UCHAR:
            case CPP_IDENT:
                if (!wants_value)
                {
                    SYNTAX_ERROR2_AT(op.loc,
                                     "missing binary operator "
                                     "before token \"%s\"",
                                     cpp_token_as_text(op.token));
                }
                wants_value = 0;
                top->value = evaluate_token(reader, op.token, op.loc);
                continue;

            case CPP_PLUS:
                if (wants_value) op.op = CPP_UPLUS;
                break;

            case CPP_MINUS:
                if (wants_value) op.op = CPP_UMINUS;
                break;

            default:
                if ((op.op <= CPP_EQ) || (op.op >= CPP_PLUS_EQ))
                {
                    SYNTAX_ERROR2_AT(op.loc,
                                     "token \"%s\" is not valid"
                                     " in preprocessor expression",
                                     cpp_token_as_text(op.token));
                    return (0);
                }
        }

        if (optab[op.op].flags & NO_L_OPERAND)
        {
            if (!wants_value)
            {
                SYNTAX_ERROR2_AT(op.loc,
                                 "missing binary operator before token \"%s\"",
                                 cpp_token_as_text(op.token));
            }
        }
        else if (wants_value)
        {
            if ((top->op == CPP_LPAREN) && (op.op == CPP_RPAREN))
            {
                SYNTAX_ERROR_AT(op.loc,
                                "missing expression between '(' and ')'");
            }
            else if ((top->op == CPP_END) && (op.op == CPP_END))
            {
                SYNTAX_ERROR2_AT(op.loc,
                                 "%s without expression\n",
                                 is_if ? "#if" : "#elif");
            }
            else if ((top->op != CPP_END) && (top->op != CPP_LPAREN))
            {
                SYNTAX_ERROR2_AT(op.loc,
                                 "operator '%s' has no right operand",
                                 cpp_token_as_text(top->token));
            }
            else if ((op.op == CPP_RPAREN) || (op.op == CPP_END))
                /* Later. */;
            else
            {
                SYNTAX_ERROR2_AT(op.loc,
                                 "operator '%s' has no left operand",
                                 cpp_token_as_text(op.token));
            }
        }
        
        top = reduce(reader, top, op.op);
        if (!top) goto syntax_error;

        if (op.op == CPP_END) break;

        switch (op.op)
        {
            case CPP_RPAREN:
                continue;

            case CPP_OR_OR:
                if (!number_zerop(top->value))
                    reader->state.skip_eval++;
                break;

            case CPP_AND_AND:
            case CPP_QUERY:
                if (number_zerop(top->value))
                    reader->state.skip_eval++;
                break;

            case CPP_COLON:
                if (top->op != CPP_QUERY)
                {
                    SYNTAX_ERROR_AT(op.loc,
                                    "':' without preceding '?'");
                    return (0);
                }
                if (!number_zerop(top[-1].value))
                    reader->state.skip_eval++;
                else
                    reader->state.skip_eval--;
                break;

            default: break;
        }

        wants_value = 1;

        if (++top == (reader->op_end))
        {
            top = _cpp_expand_op_stack(reader);
        }

        top->op = op.op;
        top->token = op.token;
        top->loc = op.loc;
    }

    if (top != (reader->op_stack))
    {
        cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                  "unbalanced stack in %s",
                  is_if ? "#if" : "#elif");
syntax_error:
        return (0);
    }
    
    return (!number_zerop(top->value));
}

struct op *_cpp_expand_op_stack(cpp_reader *reader)
{
    size_t old_size = reader->op_end - (reader->op_stack);
    size_t new_size = old_size * 2 + 20;

    reader->op_stack = xrealloc(reader->op_stack,
                                sizeof(*(reader->op_stack)) * new_size);

    return (reader->op_stack + old_size);
}
