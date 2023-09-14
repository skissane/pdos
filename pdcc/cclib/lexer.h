/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#ifndef CC_LEXER_H
#define CC_LEXER_H 1

#include "diag.h"

#ifndef CC_READER_DEFINED
#   define CC_READER_DEFINED 1
typedef struct cc_reader cc_reader;
#endif
typedef struct cc_type cc_type;
typedef struct cc_variable cc_variable;
typedef struct cc_token cc_token;

typedef enum cc_token_type {
    CC_TOKEN_PLUS,
    CC_TOKEN_MINUS,
    CC_TOKEN_DIV,
    CC_TOKEN_REM,
    CC_TOKEN_MUL,
    CC_TOKEN_LSHIFT,
    CC_TOKEN_RSHIFT,
    CC_TOKEN_BIT_AND,
    CC_TOKEN_BIT_OR,
    CC_TOKEN_BIT_NOT,
    CC_TOKEN_LPAREN,
    CC_TOKEN_RPAREN,
    CC_TOKEN_LBRACKET,
    CC_TOKEN_RBRACKET,
    CC_TOKEN_LBRACE,
    CC_TOKEN_RBRACE,
    CC_TOKEN_LTE,
    CC_TOKEN_GTE,
    CC_TOKEN_LT,
    CC_TOKEN_GT,
    CC_TOKEN_EQ,
    CC_TOKEN_NEQ,
    CC_TOKEN_NOT,
    CC_TOKEN_OR,
    CC_TOKEN_AND,
    CC_TOKEN_DOT,
    CC_TOKEN_ARROW,
    CC_TOKEN_COMMA,
    CC_TOKEN_ELLIPSIS,
    CC_TOKEN_TERNARY,
    CC_TOKEN_SEMICOLON,
    CC_TOKEN_COLON,
    CC_TOKEN_INCREMENT,
    CC_TOKEN_DECREMENT,
    CC_TOKEN_AMPERSAND,
    CC_TOKEN_ASTERISK,
    CC_TOKEN_LITERAL,
    CC_TOKEN_STRING,
    CC_TOKEN_NUMBER,
    CC_TOKEN_IDENT,
    CC_TOKEN_VARIABLE,
    CC_TOKEN_TYPE,
    CC_TOKEN_KW,
    CC_TOKEN_KW_STRUCT,
    CC_TOKEN_KW_UNION,
    CC_TOKEN_KW_TYPEDEF,
    CC_TOKEN_KW_ENUM,
    CC_TOKEN_KW_VOID,
    CC_TOKEN_KW_CHAR,
    CC_TOKEN_KW_SHORT,
    CC_TOKEN_KW_INT,
    CC_TOKEN_KW_LONG,
    CC_TOKEN_KW_FLOAT,
    CC_TOKEN_KW_DOUBLE,
    CC_TOKEN_KW_RETURN,
    CC_TOKEN_KW_WHILE,
    CC_TOKEN_KW_DO,
    CC_TOKEN_KW_FOR,
    CC_TOKEN_KW_IF,
    CC_TOKEN_KW_ELSE,
    CC_TOKEN_KW_SWITCH,
    CC_TOKEN_KW_CASE,
    CC_TOKEN_KW_BREAK,
    CC_TOKEN_KW_CONTINUE,
    CC_TOKEN_KW_DEFAULT,
    CC_TOKEN_KW_GOTO,
    CC_TOKEN_KW_SIGNED,
    CC_TOKEN_KW_UNSIGNED,
    CC_TOKEN_KW_CONST,
    CC_TOKEN_KW_VOLATILE,
    CC_TOKEN_KW_RESTRICT,
    CC_TOKEN_KW_FAR,
    CC_TOKEN_KW_NEAR,
    CC_TOKEN_KW_EXTERN,
    CC_TOKEN_KW_STATIC,
    CC_TOKEN_KW_AUTO,
    CC_TOKEN_KW_INLINE,
    CC_TOKEN_EOF,
    CC_NUM_TOKENS,
    CC_TOKEN_ASSIGN = 0x20
} cc_token_type;

struct cc_token {
    cc_token_type type;
    union {
        char *name; /* Name */
        char *string; /* String */
        unsigned long numval; /* Numeric value */
        cc_variable *var;
        cc_type *type;
    } data;
    location_t loc;
};

extern struct cc_token_info {
    cc_token_type type;
    const char *name;
} g_token_info[CC_NUM_TOKENS];

cc_token cc_consume_token(cc_reader *reader);
int cc_lex_line(cc_reader *reader, const char *line);
int cc_lex_with_preprocess (cc_reader *reader);
void cc_dump_tokens(cc_reader *reader);

#endif
