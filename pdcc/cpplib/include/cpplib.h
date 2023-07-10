/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#ifndef CPPLIB_H
#define CPPLIB_H

#include "symtab.h"

#define cpp_get_help cppgh
#define cpp_get_help2 cppgh2
#define cpp_set_help2 cppsh2
#define cpp_error_with_line cppewl
#define cpp_interpret_integer cppii
#define cpp_get_token cppgett
#define cpp_get_token_with_location cppgtwl
#define cpp_interpret_int_suffix cppiis
#define cpp_token_as_text cpptat
#define cpp_output_token cppot
#define cpp_init_builtins cppib
#define cpp_interpret_string cppis
#define cpp_set_lang cppsl
#define cpp_set_include_chains cppsic

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct cpp_reader cpp_reader;
typedef struct cpp_options cpp_options;
typedef struct cpp_token cpp_token;
typedef struct cpp_string cpp_string;
typedef struct cpp_unknown cpp_unknown;
typedef struct cpp_macro cpp_macro;
typedef struct cpp_dir cpp_dir;

#include "diag.h"

#define TYPE_TABLE \
IDONOTKNOW(EQ, "=") \
IDONOTKNOW(NOT, "!") \
IDONOTKNOW(GREATER, ">") \
IDONOTKNOW(LESS, "<") \
IDONOTKNOW(PLUS, "+") \
IDONOTKNOW(MINUS, "-") \
IDONOTKNOW(STAR, "*") \
IDONOTKNOW(DIV, "/") \
IDONOTKNOW(MODULO, "%") \
IDONOTKNOW(AND, "&") \
IDONOTKNOW(OR, "|") \
IDONOTKNOW(XOR, "^") \
IDONOTKNOW(LSHIFT, "<<") \
IDONOTKNOW(RSHIFT, ">>") \
\
IDONOTKNOW(COMPLEMENT, "~") \
IDONOTKNOW(AND_AND, "&&") \
IDONOTKNOW(OR_OR, "||") \
IDONOTKNOW(QUERY, "?") \
IDONOTKNOW(COLON, ":") \
IDONOTKNOW(COMMA, ",") \
IDONOTKNOW(LPAREN, "(") \
IDONOTKNOW(RPAREN, ")") \
IDONOTKNOW2(END, NONE) \
IDONOTKNOW(EQ_EQ, "==") \
IDONOTKNOW(NOT_EQ, "!=") \
IDONOTKNOW(GREATER_EQ, ">=") \
IDONOTKNOW(LESS_EQ, "<=") \
\
/* These two tokens are CPP_UPLUS and CPP_UMINUS \
* in expr.c. */ \
IDONOTKNOW(PLUS_EQ, "+=") \
IDONOTKNOW(MINUS_EQ, "-=") \
\
IDONOTKNOW(STAR_EQ, "*=") \
IDONOTKNOW(DIV_EQ, "/=") \
IDONOTKNOW(MODULO_EQ, "%=") \
IDONOTKNOW(AND_EQ, "&=") \
IDONOTKNOW(OR_EQ, "|=") \
IDONOTKNOW(XOR_EQ, "^=") \
IDONOTKNOW(LSHIFT_EQ, "<<=") \
IDONOTKNOW(RSHIFT_EQ, ">>=") \
/* Digrafy. */ \
IDONOTKNOW(HASH, "#") \
IDONOTKNOW(PASTE, "##") \
IDONOTKNOW(LSQUARE, "[") \
IDONOTKNOW(RSQUARE, "]") \
IDONOTKNOW(LBRACE, "{") \
IDONOTKNOW(RBRACE, "}") \
\
IDONOTKNOW(SEMICOLON, ";") \
IDONOTKNOW(ELLIPSIS, "...") \
IDONOTKNOW(DOT, ".") \
IDONOTKNOW(DEREF, "->") \
IDONOTKNOW(PLUS_PLUS, "++") \
IDONOTKNOW(MINUS_MINUS, "--") \
IDONOTKNOW(ATSIGN, "@") \
\
IDONOTKNOW2(IDENT, IDENT) \
IDONOTKNOW2(NUMBER, LITERAL) \
\
IDONOTKNOW2(CHAR, LITERAL) \
IDONOTKNOW2(LCHAR, LITERAL) \
IDONOTKNOW2(uCHAR, LITERAL) \
IDONOTKNOW2(UCHAR, LITERAL) \
IDONOTKNOW2(OTHER, LITERAL) /* Stray punctuation. */ \
\
IDONOTKNOW2(STRING, LITERAL) \
IDONOTKNOW2(LSTRING, LITERAL) \
IDONOTKNOW2(uSTRING, LITERAL) \
IDONOTKNOW2(USTRING, LITERAL) \
IDONOTKNOW2(u8STRING, LITERAL) \
IDONOTKNOW2(HEADER_NAME, LITERAL) \
\
IDONOTKNOW2(MACRO_ARGUMENT, NONE) \
IDONOTKNOW2(PADDING, NONE) 

#define IDONOTKNOW(a, b) CPP_ ## a,
#define IDONOTKNOW2(a, b) CPP_ ## a,
enum cpp_tokentype {
    TYPE_TABLE

    CPP_END_OF_TABLE,

    CPP_KEYWORD,

    CPP_LAST_CPP_OP = CPP_LESS_EQ,
    CPP_FIRST_DIGRAPH = CPP_HASH
};
#undef IDONOTKNOW
#undef IDONOTKNOW2

enum c_lang {
    STD_C89, STD_C17
};

#define AFTER_WHITE 0x1 /* After whitespace. */
#define START_OF_LINE 0x2
#define STRINGIFY_ARGUMENT 0x4
#define PASTE_LEFT 0x8 /* Token left from ##. */
#define NO_EXPAND 0x10
#define DIGRAPH 0x20
#define SP_DIGRAPH 0x40 /* # or ## was a digraph. */
#define SP_AFTER_WHITE 0x80 /* If whitespace was before ##,
                               or before this token after # operator. */

struct cpp_string {
    size_t len;
    const char *text;
};

struct cpp_macro_argument {
    unsigned int arg_number; /* Starts at 0. */
    cpp_unknown *spelling;
};

struct cpp_token {
    enum cpp_tokentype type;
    int flags;
    location_t src_loc;
    union {
        /* An identifier. */
        cpp_unknown *unknown;

        /* Inherit padding from this token. */
        const cpp_token *source;

        /* A string or a number. */
        struct cpp_string string;

        /* Argument number and original spelling. */
        struct cpp_macro_argument macro_argument;

        /* Original token number for CPP_PASTE
         * (from a sequence of consecutive CPP_PASTE tokens in macro). */
        unsigned token_number;
    } value;
};

struct cpp_macro {
    cpp_unknown **parameters;
    
    unsigned int count;
    unsigned int parameter_count;
    
    int function_like;
    int variadic;
    
    /* Indicates extra CPP_PASTE tokens at the end of macro,
     * that are stored to prevent macro redefinition. */
    int extra_tokens;
    
    cpp_token tokens[1];
};

enum cpp_unkown_type {
    UNKNOWN_VOID = 0,
    UNKNOWN_MACRO_PARAMETER,
    UNKNOWN_MACRO,
    UNKNOWN_BUILTIN_MACRO
};

enum cpp_builtin_type {
    BT_DATE, /* __DATE__ */
    BT_FILE, /* __FILE__ */
    BT_LINE, /* __LINE__ */
    BT_TIME, /* __TIME__ */
    BT_PRAGMA /* _Pragma */
};

#define UNKNOWN_DISABLED 0x1

union _cpp_unknown_value {
    cpp_macro *macro;
    enum cpp_builtin_type builtin;
    unsigned int parameter_index;
};

struct cpp_unknown {
    symtab_cell cell;
    enum cpp_unkown_type type;
    int flags;

    unsigned int is_directive;
    unsigned int directive_index;

    /* Reserved identifier code for C front end. */
    unsigned char rid_code;

    union _cpp_unknown_value value;
};

#define CPP_UNKNOWN(cell) ((cpp_unknown *)(cell))
#define UNKNOWN_NAME(unknown) ((unknown)->cell.name)
#define UNKNOWN_LEN(unknown) ((unknown)->cell.len)

struct cpp_options {
    enum c_lang lang;
    int trigraphs;
    int c99;
    /* Wheter p+ a p- is allowed in numbers. */
    int extended_numbers;

    size_t precision, char_precision, int_precision;

    int unsigned_char;
};

struct cpp_dir {
    /* NULL terminated chain. */
    struct cpp_dir *next;

    /* '\0' terminated name. */
    char *name;
};

enum cpp_diagnostic_level {
    CPP_DL_WARNING,
    CPP_DL_ERROR,
    CPP_DL_CRITICAL,
    CPP_DL_INTERNAL_ERROR
};

enum cpp_reasons {
    CPP_REASON_ENTER,
    CPP_REASON_LEAVE,
    CPP_REASON_RENAME
};

struct cpp_callbacks {
    void (*line_change)(cpp_reader *,
                        const cpp_token *,
                        int collecting_args);
    void (*file_change)(cpp_reader *, location_t, int odchod);
    void (*diagnostics)(cpp_reader *,
                        enum cpp_diagnostic_level,
                        int,
                        location_t,
                        const char *,
                        va_list *);
};

#define cpp_builtin_macro_s(unknown) ((unknown)->type == UNKNOWN_BUILTIN_MACRO)
#define cpp_macro_s(unknown) ((unknown)->type \
                             & (UNKNOWN_BUILTIN_MACRO | UNKNOWN_MACRO))

cpp_reader *cpp_create_reader(enum c_lang lang, symtab *tab);
void cpp_set_lang(cpp_reader *reader, enum c_lang lang);
void cpp_init_builtins(cpp_reader *reader, int hosted);

void cpp_set_include_chains(cpp_reader *reader,
                            cpp_dir *quote,
                            cpp_dir *angled,
                            int quote_ignores_source_dir);

struct cpp_callbacks *cpp_get_callbacks(cpp_reader *);

const char *cpp_read_main_file(cpp_reader *, const char *name);

const cpp_token *cpp_get_token_with_location(cpp_reader *reader,
                                             location_t *vloc);

void cpp_destroy_reader(cpp_reader *);


cpp_unknown *cpp_find(cpp_reader *reader, const char *str, size_t index);

const cpp_token *cpp_get_token(cpp_reader *reader);
const cpp_token *cpp_peek_token(cpp_reader *reader, size_t index);

int cpp_avoid_paste(cpp_reader *reader,
                    const cpp_token *token1,
                    const cpp_token *token2);

unsigned int cpp_token_len(const cpp_token *token);
char *cpp_spell_token(cpp_reader *reader, const cpp_token *token,
                      char *memory, int who_knows);
const char *cpp_token_as_text(const cpp_token *);
char *cpp_quote_string(char *dest, const char *source,
                       size_t len);
char *cpp_output_line_to_string(cpp_reader *reader,
                                const char *directive_name);
void cpp_output_token(const cpp_token *token, FILE *output);

void cpp_force_token_locations(cpp_reader *reader, location_t loc);
void cpp_stop_forcing_token_locations(cpp_reader *reader);

typedef unsigned long cppchar_t;
typedef long cppchar_signed_t;

cppchar_t cpp_interpret_charconst(cpp_reader *reader, const cpp_token *token,
                                  unsigned int *pchars_seen, int *unsignedp);

int cpp_interpret_string(cpp_reader *, const cpp_string *in,
                         cpp_string *out, enum cpp_tokentype);

/* Helper functions so that cpp_reader can be paired with other structures. */
void *cpp_get_help(cpp_reader *);
void cpp_set_help(cpp_reader *, void *);
void *cpp_get_help2(cpp_reader *);
void cpp_set_help2(cpp_reader *, void *);

/* Defines macro from command line (-Dmacro). */
void cpp_define(cpp_reader *reader, const char *reth);

/* +++It would be good, if this was 64 bit. */
typedef unsigned long cpp_number_part;

typedef struct cpp_number {
    cpp_number_part high;
    cpp_number_part low;
    int unsignedp;
    int overflow;
} cpp_number;

#define CPP_NUMBER_CATEGORY  0x000F
#define CPP_NUMBER_INVALID   0x0000
#define CPP_NUMBER_INTEGER   0x0001
#define CPP_NUMBER_FLOAT     0x0002

#define CPP_NUMBER_WIDTH     0x00F0
#define CPP_NUMBER_SMALL     0x0010 /* int, float, short */
#define CPP_NUMBER_MEDIUM    0x0020 /* long, double */
#define CPP_NUMBER_LARGE     0x0040 /* long long, long double */

#define CPP_NUMBER_RADIX     0x0F00
#define CPP_NUMBER_DECIMAL   0x0100
#define CPP_NUMBER_HEX       0x0200
#define CPP_NUMBER_OCTAL     0x0400
#define CPP_NUMBER_BINARY    0x0800

#define CPP_NUMBER_UNSIGNED  0x1000
#define CPP_NUMBER_DEFAULT   0x2000

/* Classifies a CPP_NUMBER token.
 * The return value is combination of flags above. */
unsigned int cpp_classify_number(cpp_reader *reader,
                                 const cpp_token *token,
                                 location_t virtual_loc);

unsigned int cpp_interpret_float_suffix(cpp_reader *reader,
                                        const char *str,
                                        size_t len);

unsigned int cpp_interpret_int_suffix(cpp_reader *reader,
                                      const char *str,
                                      size_t len);

/* Evaluates a token classified as CPP_NUMBER_INTEGER. */
cpp_number cpp_interpret_integer(cpp_reader *reader,
                                 const cpp_token *token,
                                 unsigned int flags);
void cpp_error(cpp_reader *reader, enum cpp_diagnostic_level level,
               const char *message, ...);
void cpp_error_with_line(cpp_reader *reader, enum cpp_diagnostic_level level,
                         location_t loc, unsigned int column,
                         const char *message, ...);

#endif /* CPPLIB_H */
