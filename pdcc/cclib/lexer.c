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
#include <ctype.h>
#include <assert.h>
#include "lexer.h"
#include "diag.h"
#include "cclib.h"
#include "support.h"
#include "xmalloc.h"

/* Lookup table for parsing tokens */
cc_token_info g_token_info[CC_NUM_TOKENS] = {
    { CC_TOKEN_ASSIGN, "=" },
    { CC_TOKEN_PLUS, "+" },
    { CC_TOKEN_ASPLUS, "+=" },
    { CC_TOKEN_MINUS, "-" },
    { CC_TOKEN_ASMINUS, "-=" },
    { CC_TOKEN_DIV, "/" },
    { CC_TOKEN_ASDIV, "/=" },
    { CC_TOKEN_REM, "%" },
    { CC_TOKEN_ASREM, "%=" },
    { CC_TOKEN_MUL, "*" },
    { CC_TOKEN_ASMUL, "*=" },
    { CC_TOKEN_LSHIFT, "<<" },
    { CC_TOKEN_ASLSHIFT, "<<=" },
    { CC_TOKEN_RSHIFT, ">>" },
    { CC_TOKEN_ASRSHIFT, ">>=" },
    { CC_TOKEN_BIT_ASAND, "&=" },
    { CC_TOKEN_BIT_OR, "|" },
    { CC_TOKEN_BIT_ASOR, "|=" },
    { CC_TOKEN_BIT_XOR, "^" },
    { CC_TOKEN_BIT_ASXOR, "^=" },
    { CC_TOKEN_BIT_NOT, "~" },

    { CC_TOKEN_LPAREN, "(" },
    { CC_TOKEN_RPAREN, ")" },
    { CC_TOKEN_LBRACKET, "[" },
    { CC_TOKEN_RBRACKET, "]" },
    { CC_TOKEN_LBRACE, "{" },
    { CC_TOKEN_RBRACE, "}" },

    { CC_TOKEN_LTE, "<=" },
    { CC_TOKEN_GTE, ">=" },
    { CC_TOKEN_LT, "<" },
    { CC_TOKEN_GT, ">" },
    { CC_TOKEN_EQ, "==" },
    { CC_TOKEN_NEQ, "!=" },
    { CC_TOKEN_NOT, "^" },
    { CC_TOKEN_OR, "||" },
    { CC_TOKEN_AND, "&&" },

    { CC_TOKEN_DOT, "." },
    { CC_TOKEN_ARROW, "->" },
    { CC_TOKEN_COMMA, "," },
    { CC_TOKEN_ELLIPSIS, "..." },
    { CC_TOKEN_TERNARY, "?" },
    { CC_TOKEN_SEMICOLON, ";" },
    { CC_TOKEN_COLON, ":" },
    { CC_TOKEN_INCREMENT, "++" },
    { CC_TOKEN_DECREMENT, "--" },

    { CC_TOKEN_AMPERSAND, "&" },
    { CC_TOKEN_ASTERISK, "*" },

    { CC_TOKEN_LITERAL, "<literal>" },
    { CC_TOKEN_STRING, "<string>" },
    { CC_TOKEN_NUMBER, "<number>" },
    { CC_TOKEN_IDENT, "<ident>" },
    { CC_TOKEN_VARIABLE, "<variable>" },
    { CC_TOKEN_TYPE, "<type>" },

    { CC_TOKEN_KW, "<keyword>" },
    { CC_TOKEN_KW_STRUCT, "struct" },
    { CC_TOKEN_KW_UNION, "union" },
    { CC_TOKEN_KW_TYPEDEF, "typedef" },
    { CC_TOKEN_KW_ENUM, "enum" },
    { CC_TOKEN_KW_VOID, "void" },
    { CC_TOKEN_KW_CHAR, "char" },
    { CC_TOKEN_KW_SHORT, "short" },
    { CC_TOKEN_KW_INT, "int" },
    { CC_TOKEN_KW_LONG, "long" },
    { CC_TOKEN_KW_FLOAT, "float" },
    { CC_TOKEN_KW_DOUBLE, "double" },
    { CC_TOKEN_KW_RETURN, "return" },
    { CC_TOKEN_KW_WHILE, "while" },
    { CC_TOKEN_KW_DO, "do" },
    { CC_TOKEN_KW_FOR, "for" },
    { CC_TOKEN_KW_IF, "if" },
    { CC_TOKEN_KW_ELSE, "else" },
    { CC_TOKEN_KW_SWITCH, "switch" },
    { CC_TOKEN_KW_CASE, "case" },
    { CC_TOKEN_KW_BREAK, "break" },
    { CC_TOKEN_KW_CONTINUE, "continue" },
    { CC_TOKEN_KW_DEFAULT, "default" },
    { CC_TOKEN_KW_GOTO, "goto" },
    { CC_TOKEN_KW_SIGNED, "signed" },
    { CC_TOKEN_KW_UNSIGNED, "unsigned" },
    { CC_TOKEN_KW_CONST, "const" },
    { CC_TOKEN_KW_VOLATILE, "volatile" },
    { CC_TOKEN_KW_RESTRICT, "restrict" },
    { CC_TOKEN_KW_FAR, "far" },
    { CC_TOKEN_KW_NEAR, "near" },
    { CC_TOKEN_KW_EXTERN, "extern" },
    { CC_TOKEN_KW_STATIC, "static" },
    { CC_TOKEN_KW_AUTO, "auto" },
    { CC_TOKEN_KW_INLINE, "inline" },

    { CC_TOKEN_EOF, "<eof>" },
};

#define is_ident(x) (isalnum(x) || (x) == '$' || (x) == '@' || (x) == '_')

/**
 * @brief Adds a token to the current reader
 * @param reader Reader object
 * @param tok Token to add
 */
static void cc_add_token(cc_reader *reader, cc_token tok)
{
    reader->tokens = xrealloc(reader->tokens,
        (reader->n_tokens + 1) * sizeof(cc_token));
    reader->tokens[reader->n_tokens++] = tok;
}

/**
 * @brief 
 * @param reader Reader object
 * @return cc_token Token just consumed
 */
cc_token cc_consume_token(cc_reader *reader)
{
#if 0
    cc_token tok = reader->tokens[0];
    reader->n_tokens--;
    memmove(&reader->tokens[0], &reader->tokens[1], (reader->n_tokens + 1)
        * sizeof(cc_token));
    reader->tokens = realloc(reader->tokens,
        reader->n_tokens * sizeof(cc_token));
    reader->curr_token = &reader->tokens[0];
    return tok;
    printf("CONSUMING=%s\n", g_token_info[reader->curr_token->type].name);
#endif
    return *reader->curr_token++;
}

static unsigned int hex_value(unsigned int c)
{
    if ((c >= '0') && (c <= '9')) return (c - '0');
    if ((c >= 'a') && (c <= 'f')) return (c + 10 - 'a');
    if ((c >= 'A') && (c <= 'F')) return (c + 10 - 'A');
    abort();
}

static char *cc_interpret_string(cc_reader *reader, const char *src)
{
    const char *p = src;
    char *p2, *dest;
    dest = p2 = xmalloc(strlen(src) + 1);
    for ( ; p < src + strlen(src); p++, p2++)
    {
        if (*p == '"') p++;
        if (*p == '\\')
        {
            p++;
            switch (*p)
            {
                case 'a': *p2 = '\a'; continue;
                case 'b': *p2 = '\b'; continue;
                case 'f': *p2 = '\f'; continue;
                case 'n': *p2 = '\n'; continue;
                case 'r': *p2 = '\r'; continue;
                case 't': *p2 = '\t'; continue;
                case 'v': *p2 = '\v'; continue;
                case '0': *p2 = '\0'; continue;
                case 'x': {
                    const char *orig = p;
                    unsigned int h;
                    p++;
                    if (!ISXDIGIT(*p))
                    {
                        cc_report(reader, CC_DL_ERROR,
                                 "\\x used"
                                 " with no following hexadecimal digits");
                    }
                    h = 0;
                    while (ISXDIGIT(*p))
                    {
                        h *= 16;
                        h += hex_value(*p);
                        p++;
                    }
                    p--;
                    if (h > 0xff)
                    {
                        cc_report(reader, CC_DL_WARNING,
                                 "Hexadecimal escape sequence out of range");
                        *(p2++) = '\\';
                        p = orig;
                    }
                    else
                    {
                        *p2 = h;
                        continue;
                    }
                    break;
                }
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                case '7': {
                    const char *orig = p;
                    unsigned int h;

                    h = 0;
                    while ((*p >= '0') && (*p <= '7'))
                    {
                        h *= 8;
                        h += *p - '0';
                        p++;
                    }
                    p--;
                    if (h > 0xff)
                    {
                        cc_report(reader, CC_DL_WARNING,
                                 "Octal escape sequence out of range");
                        *(p2++) = '\\';
                        p = orig;
                    }
                    else
                    {
                        *p2 = h;
                        continue;
                    }
                    break;
                }
            }
        }
        *p2 = *p;
    }
    
    *p2 = '\0';
    return dest;
}

/**
 * @brief Parse a line
 * 
 * @param reader Reader object
 * @param line Line string
 * @return int 0 on success, 1 on error
 */
int cc_lex_line(cc_reader *reader, const char *line)
{
    const char *p = line;
    while (*p)
    {
        cc_token tok = {0};
        while (isspace(*p))
            p++;
        
        if (*p == '\0') break;

        tok.loc.column = (size_t)(p - line);
        tok.loc.line = reader->line;
        tok.loc.file = reader->file;
        switch (*p++)
        {
        case '{':
            tok.type = CC_TOKEN_LBRACE;
            break;
        case '}':
            tok.type = CC_TOKEN_RBRACE;
            break;
        case '(':
            tok.type = CC_TOKEN_LPAREN;
            break;
        case ')':
            tok.type = CC_TOKEN_RPAREN;
            break;
        case '[':
            tok.type = CC_TOKEN_LBRACKET;
            break;
        case ']':
            tok.type = CC_TOKEN_RBRACKET;
            break;
        case '=':
            tok.type = CC_TOKEN_ASSIGN;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_EQ;
                p++;
            }
            break;
        case '+':
            tok.type = CC_TOKEN_PLUS;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASPLUS;
                p++;
            }
            else if (*p == '+')
            {
                tok.type = CC_TOKEN_INCREMENT;
                p++;
            }
            break;
        case '-':
            tok.type = CC_TOKEN_MINUS;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASMINUS;
                p++;
            }
            else if (*p == '-')
            {
                tok.type = CC_TOKEN_DECREMENT;
                p++;
            }
            else if (*p == '>')
            {
                tok.type = CC_TOKEN_ARROW;
                p++;
            }
            break;
        case '/':
            tok.type = CC_TOKEN_DIV;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASDIV;
                p++;
            }
            break;
        case '%':
            tok.type = CC_TOKEN_REM;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASREM;
                p++;
            }
            break;
        case '*':
            tok.type = CC_TOKEN_ASTERISK;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASMUL;
                p++;
            }
            break;
        case '?':
            tok.type = CC_TOKEN_TERNARY;
            break;
        case ':':
            tok.type = CC_TOKEN_COLON;
            break;
        case ';':
            tok.type = CC_TOKEN_SEMICOLON;
            break;
        case ',':
            tok.type = CC_TOKEN_COMMA;
            break;
        case '.':
            tok.type = CC_TOKEN_DOT;
            if (p[0] == '.' && p[1] == '.')
            {
                tok.type = CC_TOKEN_ELLIPSIS;
                p += 2;
            }
            break;
        case '>':
            tok.type = CC_TOKEN_GT;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_GTE;
                p++;
            }
            break;
        case '<':
            tok.type = CC_TOKEN_LT;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_LTE;
                p++;
            }
            break;
        case '!':
            tok.type = CC_TOKEN_NOT;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_NEQ;
                p++;
            }
            break;
        case '|':
            tok.type = CC_TOKEN_BIT_OR;
            if (*p == '|')
            {
                tok.type = CC_TOKEN_OR;
                p++;
            }
            else if (*p == '=')
            {
                tok.type = CC_TOKEN_BIT_ASOR;
                p++;
            }
            break;
        case '^':
            tok.type = CC_TOKEN_BIT_XOR;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_BIT_ASXOR;
                p++;
            }
            break;
        case '&':
            tok.type = CC_TOKEN_AMPERSAND;
            if (*p == '&')
            {
                tok.type = CC_TOKEN_AND;
                p++;
            }
            else if (*p == '=')
            {
                tok.type = CC_TOKEN_BIT_ASAND;
                p++;
            }
            break;
        case '\'':
        case '\"':
            {
                const char *start = p;
                char term = *(p - 1);
                tok.type = term == '\'' ? CC_TOKEN_LITERAL : CC_TOKEN_STRING;
                for ( ; *p != term && *p != '\0'; p++)
                    if (*p == '\\')
                        p++;

                if (*p == term)
                {
                    size_t len = (size_t)(p - start);
                    char *non_escaped, *escaped;
                    non_escaped = xstrndup(start, len);
                    /* Escape the string */
                    escaped = cc_interpret_string(reader, non_escaped);
                    tok.data.string = escaped;
                    free(non_escaped);
                }
                else
                    cc_report(reader, CC_DL_ERROR, "Unterminated string");
                p++;
            }
            break;
        default:
            p--;
            if (isalpha(*p))
            {
                const char *start = p;
                char is_keyword = 0;
                size_t len, i;
                while (is_ident(*p))
                    p++;
                len = (size_t)(p - start);
                
                tok.type = CC_TOKEN_KW;
                for (i = CC_TOKEN_KW; i < CC_NUM_TOKENS; i++)
                {
                    if (strlen(g_token_info[i].name) == len
                     && !strncmp(g_token_info[i].name, start, len))
                    {
                        tok.type = g_token_info[i].type;
                        is_keyword = 1;
                        break;
                    }
                }

                /* Identifier */
                if (!is_keyword)
                {
                    tok.type = CC_TOKEN_IDENT;
                    tok.data.name = xstrndup(start, len);
                }
            }
            else if (isdigit(*p))
            {
                const char *start = p;
                char *numstr;
                size_t len;
                while (isalnum(*p) || *p == '.') /* Lex first, parse later */
                    p++;
                
                len = (size_t)(p - start);
                numstr = xstrndup(start, len);
                /* TODO: more advanced number parsing */
                tok.type = CC_TOKEN_NUMBER;
                tok.data.numval = atoi(numstr);
                free(numstr);
            }
            else
                p++;
            break;
        }
        cc_add_token(reader, tok);
    }
    reader->line++;
    return 0;
}

int cc_lex_with_preprocess (cc_reader *reader)
{
    while (1)
    {        
        location_t loc;
        const cpp_token *token = cpp_get_token_with_location (reader->cpp_reader, &loc);

        cc_token tok = {0};
        char spelled_token[100];
        const char *p;
        
        if (token == NULL) break;
        if (token->type == CPP_END) break;
        if (token->type == CPP_PADDING) continue;
        
        tok.loc = loc;
        /* The original file name is freed when preprocessor exits the source file,
         * so it needs to be preserved. Linked list with filenames
         * and checking the most recent filename using original pointer
         * would be probably better.
         * Memory leak currently.
         */
        tok.loc.file = xstrdup (loc.file);
        cpp_spell_token (reader->cpp_reader, token, spelled_token, 1)[0] = '\0';
        p = spelled_token;
        
        switch (*p++)
        {
        case '{':
            tok.type = CC_TOKEN_LBRACE;
            break;
        case '}':
            tok.type = CC_TOKEN_RBRACE;
            break;
        case '(':
            tok.type = CC_TOKEN_LPAREN;
            break;
        case ')':
            tok.type = CC_TOKEN_RPAREN;
            break;
        case '[':
            tok.type = CC_TOKEN_LBRACKET;
            break;
        case ']':
            tok.type = CC_TOKEN_RBRACKET;
            break;
        case '=':
            tok.type = CC_TOKEN_ASSIGN;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_EQ;
            }
            break;
        case '+':
            tok.type = CC_TOKEN_PLUS;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASPLUS;
            }
            else if (*p == '+')
            {
                tok.type = CC_TOKEN_INCREMENT;
            }
            break;
        case '-':
            tok.type = CC_TOKEN_MINUS;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASMINUS;
            }
            else if (*p == '-')
            {
                tok.type = CC_TOKEN_DECREMENT;
            }
            else if (*p == '>')
            {
                tok.type = CC_TOKEN_ARROW;
            }
            break;
        case '/':
            tok.type = CC_TOKEN_DIV;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASDIV;
            }
            break;
        case '%':
            tok.type = CC_TOKEN_REM;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASREM;
            }
            break;
        case '*':
            tok.type = CC_TOKEN_ASTERISK;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_ASMUL;
            }
            break;
        case '?':
            tok.type = CC_TOKEN_TERNARY;
            break;
        case ':':
            tok.type = CC_TOKEN_COLON;
            break;
        case ';':
            tok.type = CC_TOKEN_SEMICOLON;
            break;
        case ',':
            tok.type = CC_TOKEN_COMMA;
            break;
        case '.':
            tok.type = CC_TOKEN_DOT;
            if (p[0] == '.' && p[1] == '.')
            {
                tok.type = CC_TOKEN_ELLIPSIS;
            }
            break;
        case '>':
            tok.type = CC_TOKEN_GT;
            if (*p == '>') {
                tok.type = CC_TOKEN_RSHIFT;
            } else if (*p == '=') {
                tok.type = CC_TOKEN_GTE;
            }
            break;
        case '<':
            tok.type = CC_TOKEN_LT;
            if (*p == '<') {
                tok.type = CC_TOKEN_LSHIFT;
            } else if (*p == '=') {
                tok.type = CC_TOKEN_LTE;
            }
            break;
        case '!':
            tok.type = CC_TOKEN_NOT;
            if (*p == '=')
            {
                tok.type = CC_TOKEN_NEQ;
            }
            break;
        case '|':
            tok.type = CC_TOKEN_BIT_OR;
            if (*p == '|') {
                tok.type = CC_TOKEN_OR;
            } else if (*p == '=') {
                tok.type = CC_TOKEN_BIT_ASOR;
            }
            break;
        case '^':
            tok.type = CC_TOKEN_BIT_XOR;
            if (*p == '=') {
                tok.type = CC_TOKEN_BIT_ASXOR;
            }
            break;
        case '&':
            tok.type = CC_TOKEN_AMPERSAND;
            if (*p == '&')
            {
                tok.type = CC_TOKEN_AND;
            }
            break;
        case '\'':
        case '\"':
            {
                const char *start = p;
                char term = *(p - 1);
                tok.type = term == '\'' ? CC_TOKEN_LITERAL : CC_TOKEN_STRING;
                for ( ; *p != term && *p != '\0'; p++)
                    if (*p == '\\')
                        p++;

                if (*p == term)
                {
#if 0
                    size_t len = (size_t)(p - start);
                    char *non_escaped, *escaped;
                    non_escaped = xstrndup(start, len);
                    /* Escape the string */
                    escaped = cc_interpret_string(reader, non_escaped);
                    tok.data.string = escaped;
                    free(non_escaped);
#else
                    /* The assembler can handle escape sequences on its own. */
                    tok.data.string = xstrndup (start, p - start);
#endif
                }
                else
                    cc_report(reader, CC_DL_ERROR, "Unterminated string");
            }
            break;
        default:
            p--;
            if (!isdigit(*p))
            {
                const char *start = p;
                char is_keyword = 0;
                size_t len, i;
                while (is_ident(*p))
                    p++;
                len = (size_t)(p - start);
                
                tok.type = CC_TOKEN_KW;
                for (i = CC_TOKEN_KW; i < CC_NUM_TOKENS; i++)
                {
                    if (strlen(g_token_info[i].name) == len
                     && !strncmp(g_token_info[i].name, start, len))
                    {
                        tok.type = g_token_info[i].type;
                        is_keyword = 1;
                        break;
                    }
                }

                /* Identifier */
                if (!is_keyword)
                {
                    tok.type = CC_TOKEN_IDENT;
                    tok.data.name = xstrndup(start, len);
                }
            }
            else
            {
                const char *start = p;
                char *numstr;
                size_t len;
                while (isalnum(*p) || *p == '.') /* Lex first, parse later */
                    p++;
                
                len = (size_t)(p - start);
                numstr = xstrndup(start, len);
                /* TODO: more advanced number parsing */
                tok.type = CC_TOKEN_NUMBER;
                tok.data.numval = atoi(numstr);
                free(numstr);
            }
            break;
        }
        cc_add_token(reader, tok);
    }
    reader->line++;
    return 0;
}

void cc_dump_tokens(cc_reader *reader)
{
    size_t i;
    for (i = 0; i < reader->n_tokens; i++)
    {
        const cc_token *tok = &reader->tokens[i];
        if ((tok->type == CC_TOKEN_STRING && tok->data.string)
         || (tok->type == CC_TOKEN_LITERAL && tok->data.string))
            printf("\"%s\"", tok->data.string);
        else if(tok->type == CC_TOKEN_IDENT && tok->data.name)
            printf("\"%s\"", tok->data.name);
        else if (tok->type == CC_TOKEN_NUMBER)
            printf("(%lu)", tok->data.numval);
        printf("%s ", g_token_info[tok->type].name);

        if (i % 8 == 0 && i)
            printf("\n");
    }
    printf("\n");
}
