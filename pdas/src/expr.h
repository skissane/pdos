/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

typedef enum {
    Expr_type_invalid,

    /* The value is add_number. */
    Expr_type_constant,

    /* The value is the symbol in add_symbol. */
    Expr_type_symbol
} expr_type_T;

typedef struct {
    expr_type_T type;
    symbolS *add_symbol;
    unsigned long add_number;
} exprS;

void expr_read_into(exprS *result);
