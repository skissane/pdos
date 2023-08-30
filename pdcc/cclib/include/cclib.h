/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#ifndef CCLIB_H
#define CCLIB_H 1

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "cpplib.h"
#include "lexer.h"

#ifndef CC_READER_DEFINED
#   define CC_READER_DEFINED 1
typedef struct cc_reader cc_reader;
#endif
typedef struct cc_token_info cc_token_info;
typedef struct cc_ptrinfo cc_ptrinfo;
typedef struct cc_member cc_member;
typedef struct cc_enum_member cc_enum_member;
typedef struct cc_param cc_param;
typedef struct cc_expr cc_expr;

typedef enum cc_type_mode {
    CC_TYPE_VOID,
    CC_TYPE_CHAR,
    CC_TYPE_INT,
    CC_TYPE_SHORT,
    CC_TYPE_LONG,
    CC_TYPE_LLONG,
    CC_TYPE_FLOAT,
    CC_TYPE_DOUBLE,
    CC_TYPE_LDOUBLE,
    CC_TYPE_STRUCT,
    CC_TYPE_UNION,
    CC_TYPE_ENUM,
    CC_TYPE_FUNCTION
} cc_type_mode;

#define MAX_PTR_DEPTH 3
struct cc_type {
    cc_type_mode mode;
    struct cc_ptrinfo {
        enum cc_ptrinfo_flags {
            CC_PTRINFO_NONE, /* None */
            CC_PTRINFO_VOLATILE = 0x01,
            CC_PTRINFO_CONST = 0x02
        } flags;
    } ptr[MAX_PTR_DEPTH];
    size_t ptr_depth;
    char *name;
    union {
        char sign;
        struct {
            cc_member *members;
            size_t n_members;
        } s_or_u; /* struct or union */
        struct {
            cc_enum_member *enum_members;
            size_t n_enum_members;
        } e; /* enums */
        struct {
            cc_type *return_type;
            cc_param *params;
            size_t n_params;
            char variadic;
        } f; /* Functions */
    } data;
};

struct cc_member {
    char *name;
    cc_type type;
};

struct cc_param {
    char *name;
    cc_type type;
};

struct cc_enum_member {
    char *name;
    char sign;
    union {
        long int s_value;
        unsigned long int u_value;
    } data;
};

typedef enum cc_linkage {
    CC_LINKAGE_AUTO,
    CC_LINKAGE_STATIC,
    CC_LINKAGE_EXTERN,
    /* ... */
    CC_LINKAGE_INLINE,
    CC_LINKAGE_COROUTINE,
    CC_LINKAGE_THREAD_LOCAL
} cc_linkage;

struct cc_variable {
    char *name;
    cc_linkage linkage;
    cc_type type;
    cc_expr *block_expr; /* Expression for the block {} */
};

typedef enum cc_expr_type {
    CC_EXPR_NONE, /* Required -- do not remove! */
    CC_EXPR_ASSIGN,
    CC_EXPR_PLUS,
    CC_EXPR_MINUS,
    CC_EXPR_DIV,
    CC_EXPR_REM,
    CC_EXPR_MUL,
    CC_EXPR_BIT_AND,
    CC_EXPR_BIT_OR,
    CC_EXPR_BIT_NOT,
    CC_EXPR_LSHIFT,
    CC_EXPR_RSHIFT,

    CC_EXPR_AND,
    CC_EXPR_OR,
    CC_EXPR_NOT,
    CC_EXPR_LT,
    CC_EXPR_LTE,
    CC_EXPR_GT,
    CC_EXPR_GTE,

    CC_EXPR_CAST,
    CC_EXPR_CONSTANT,
    CC_EXPR_STRING,
    CC_EXPR_CALL,
    CC_EXPR_BLOCK,
    CC_EXPR_DECL,
    CC_EXPR_VARREF,

    CC_EXPR_IF,
    CC_EXPR_ELSE,
    CC_EXPR_ELIF,
    CC_EXPR_RETURN,
    CC_EXPR_GOTO
} cc_expr_type;

struct cc_expr {
    cc_expr_type type;
    union {
        struct {
            cc_expr *callee; /* Function to call expression */
            cc_expr *params; /* Expression for params */
            size_t n_params;
            cc_type callee_type; /* Type of function to call */
            cc_variable *callee_func; /* Can be NULL if expression is computed
                                         at runtime */
        } call;
        struct {
            char *data; /* String data (escaped) */
        } string;
        struct {
            cc_expr *ret_expr; /* Return expression */
        } ret;
        struct {
            cc_expr *exprs; /* Body of the block */
            size_t n_exprs;
            cc_type *types;
            size_t n_types;
            cc_type *union_types;
            size_t n_union_types;
            cc_type *struct_types;
            size_t n_struct_types;
            cc_type *enum_types;
            size_t n_enum_types;
            cc_variable *vars;
            size_t n_vars;
            size_t parent_id;
        } block;
        struct {
            unsigned long numval; /* Numeric value for constant */
        } _const;
        struct {
            cc_expr *cond_expr; /* Condition expression */
            cc_expr *body_expr; /* Body of the if-expression */
        } if_else;
        struct {
            cc_type type;
        } cast;
        struct {
            cc_expr *expr;
        } unary_op;
        struct {
            cc_expr *left;
            cc_expr *right;
        } binary_op;
        struct {
            cc_variable var;
        } decl;
        struct {
            cc_variable *var;
        } var_ref;
    } data;
    size_t id; /* Unique Id for this expression */
};

struct cc_reader {
    FILE *input;
    FILE *output;
    char *file; /* Input filename */
    
    cc_token *tokens;
    size_t n_tokens;
    cc_token *curr_token;
    cc_expr *curr_block;
    cc_expr *root_expr;
    unsigned char ptr_bits; /* Bits in a pointer */
    unsigned char char_bits; /* Bits in a char */
    unsigned char short_bits;
    unsigned char int_bits;
    unsigned char long_bits;
    unsigned char llong_bits;
    unsigned long id; /* Current unique id assigneer */
    unsigned long line; /* Current line number */
};

#define cc_parse_variable ccparvar
#define cc_parse_statement ccparstat
#define cc_parse_member ccparmem
#define cc_parse_type ccpartype
#define cc_parse_file ccparfile

cc_variable cc_parse_variable(cc_reader *reader);
cc_expr cc_parse_statement(cc_reader *reader);
cc_member cc_parse_member(cc_reader *reader);
cc_type cc_parse_type(cc_reader *reader);
int cc_parse_file(cc_reader *reader);

size_t cc_get_variable_size(cc_reader *reader, const cc_variable *var);

/**
 * @brief Grabs the name/string field from a token and ensures to take
 * ownership from it, this to avoid extra XSTRDUPs
 * 
 */
#define TOKEN_GETSTR(x, str)    \
    (x) = xstrdup(str);         \
    (str) = NULL;

#endif
