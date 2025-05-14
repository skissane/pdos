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
#include "xmalloc.h"

#define cpp_get_token_1 cppgt1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

enum {
    SPELLING_OPERATOR,
    SPELLING_IDENT,
    SPELLING_LITERAL,
    SPELLING_NONE
};

struct spelling {
    int type;
    char *text;
};

static const char *digraph_spelling[] = {
    (const char *)"%:",
    (const char *)"%:%:",
    (const char *)"<:",
    (const char *)":>",
    (const char *)"<%",
    (const char *)"%>"
};

#define IDONOTKNOW(a, b) {SPELLING_OPERATOR, (char *)b},
#define IDONOTKNOW2(a, b) {SPELLING_ ## b, (char *)#a},
static const struct spelling token_spelling[] = {TYPE_TABLE};
#undef IDONOTKNOW
#undef IDONOTKNOW2

int _cpp_equiv_tokens(const cpp_token *t1, const cpp_token *t2)
{
    if ((t1->type == t2->type) && (t1->flags == t2->flags))
    {
        switch (token_spelling[t1->type].type)
        {
            case SPELLING_OPERATOR:
                return ((t1->type != CPP_PASTE)
                        || (t1->value.token_number
                            == t2->value.token_number));
            case SPELLING_NONE:
                return ((t1->type != CPP_MACRO_ARGUMENT)
                        || ((t1->value.macro_argument.arg_number
                             == t2->value.macro_argument.arg_number)
                            && (t1->value.macro_argument.spelling
                                == t2->value.macro_argument.spelling)));
            case SPELLING_IDENT:
                return (t1->value.unknown == t2->value.unknown);
            case SPELLING_LITERAL:
                return ((t1->value.string.len
                         == t2->value.string.len)
                        && (memcmp(t1->value.string.text,
                                   t2->value.string.text,
                                   t1->value.string.len) == 0));
        }
    }
            
    return (0);
}

struct cpp_callbacks *cpp_get_callbacks(cpp_reader *reader)
{
    return (&(reader->callbacks));
}

static cpp_unknown2 *new_unknown2(cpp_reader *reader)
{
    cpp_unknown2 *new = xmalloc(sizeof(*new));

    new->next = reader->unknown2;
    reader->unknown2 = new;

    return (new);
}

static cpp_unknown *macro_unknown2(cpp_unknown2 *unknown2)
{
    if (unknown2 == NULL) return (NULL);

    return (unknown2->macro);
}

void _cpp_add_unknown2_direct(cpp_reader *reader, cpp_unknown *unknown,
                              const cpp_token *first, int count)
{
    cpp_unknown2 *unknown2 = new_unknown2(reader);

    if (unknown == NULL)
    {
        unknown = macro_unknown2(reader->unknown2->next);
    }

    unknown2->tokens_kind = TOKENS_KIND_DIRECT;
    unknown2->macro = unknown;
    unknown2->memory = NULL;
    unknown2->first.token = first;
    unknown2->last.token = first + count;
}

static void add_unknown2_indirect(cpp_reader *reader, cpp_unknown *unknown,
                                  void *memory,
                                  const cpp_token **ptokens,
                                  unsigned int count)
{
    cpp_unknown2 *unknown2 = new_unknown2(reader);

    if (unknown == NULL)
    {
        unknown = macro_unknown2(reader->unknown2->next);
    }

    unknown2->tokens_kind = TOKENS_KIND_INDIRECT;
    unknown2->macro = unknown;
    unknown2->memory = memory;
    unknown2->first.ptoken = ptokens;
    unknown2->last.ptoken = ptokens + count;
}

void _cpp_remove_unknown2(cpp_reader *reader)
{
    cpp_unknown2 *unknown2 = reader->unknown2;

    /* Is NULL, if unknown2 was created by expand_arg(). */
    if (unknown2->macro)
    {
        cpp_unknown *macro = unknown2->macro;

        if ((macro != NULL)
            && (macro_unknown2(unknown2->next) != macro))
            macro->flags &= ~UNKNOWN_DISABLED;
    }

    free(unknown2->memory);

    reader->unknown2 = unknown2->next;
    
    free(unknown2);
}

void _cpp_return_tokens_to_lexer(cpp_reader *reader, unsigned int count)
{
    reader->tokens_infront += count;
    /* _cpp_lex_token() never leaves
     * reader->cur_token == reader->cur_tokenrow->start.*/
    while (count--)
    {
        reader->cur_token--;
        if ((reader->cur_token == reader->cur_tokenrow->start)
            && (reader->cur_tokenrow->prev != NULL))
        {
            reader->cur_tokenrow = reader->cur_tokenrow->prev;
            reader->cur_token = reader->cur_tokenrow->end;
        }
    }
}

/* Returns count tokens, for macros can return only 1. */
void _cpp_return_tokens(cpp_reader *reader, unsigned int count)
{
    if (reader->unknown2 == NULL)
    {
        _cpp_return_tokens_to_lexer(reader, count);
    }
    else
    {
        if (count != 1)
        {
            printf("Internal error %s:%u\n", __FILE__, __LINE__);
            return;
        }
        switch (reader->unknown2->tokens_kind)
        {
            case TOKENS_KIND_DIRECT:
                reader->unknown2->first.token--;
                break;
            case TOKENS_KIND_INDIRECT:
                reader->unknown2->first.ptoken--;
                break;
        }
    }
}

static cpp_token *padding_token(cpp_reader *reader, const cpp_token *prev_token)
{
    cpp_token *result = _cpp_temp_token(reader);

    result->type = CPP_PADDING;
    result->value.source = prev_token;
    result->flags = 0;

    return (result);
}

enum macro_arg_token_kind {
    MACRO_ARG_TOKEN_NORMAL,
    MACRO_ARG_TOKEN_EXPANDED
};

typedef struct {
    const cpp_token **first; /* First unexpanded token. */
    const cpp_token **expanded;
    const cpp_token *stringified;
    unsigned int count;
    unsigned int expanded_count;
} macro_arg;

static const cpp_token **arg_token_pointer(macro_arg *arg,
                                           unsigned int index,
                                           enum macro_arg_token_kind kind)
{
    const cpp_token **token_p;

    switch (kind)
    {
        case MACRO_ARG_TOKEN_NORMAL: token_p = arg->first; break;
        case MACRO_ARG_TOKEN_EXPANDED: token_p = arg->expanded; break;
    }

    return (&(token_p[index]));
}

static void add_arg_token(macro_arg *arg,
                          const cpp_token *token,
                          unsigned int index,
                          enum macro_arg_token_kind kind)
{
    const cpp_token **token_p;

    token_p = arg_token_pointer(arg, index, kind);
    *token_p = token;
}

int _cpp_arguments_fine(cpp_reader *reader,
                        cpp_macro *macro,
                        cpp_unknown *unknown,
                        unsigned int arg_count)
{
    if (arg_count == macro->parameter_count)
    {
        return (1);
    }
    if (arg_count < macro->parameter_count)
    {
        if ((arg_count + 1 < macro->parameter_count)
            && macro->variadic)
        {
            cpp_error(reader, CPP_DL_WARNING,
                      "At least one argument for the \"...\""
                      " in variadic macro is required");
            return (1);
        }

        cpp_error(reader, CPP_DL_ERROR,
                  "macro \"%s\" requires %u arguments, but only %u given",
                  UNKNOWN_NAME(unknown), macro->parameter_count,
                  arg_count);
    }
    else
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "macro \"%s\" takes only %u arguments, but %u passed",
                  UNKNOWN_NAME(unknown), macro->parameter_count,
                  arg_count);
    }

    return (0);
}

static void *collect_args(cpp_reader *reader, cpp_unknown *unknown,
                          unsigned int *arg_count_p)
{
    cpp_macro *macro = unknown->value.macro;
    char *memory;
    macro_arg *args, *arg;
    const cpp_token **token_p;
    unsigned int parameter_count;
    size_t size;
    const cpp_token *token;
    unsigned int arg_count;

#define TOKENS_PER_ARGUMENT 17

    if (macro->parameter_count) parameter_count = macro->parameter_count;
    else parameter_count = 1;

    size = ((sizeof(macro_arg) + sizeof(cpp_token *) * TOKENS_PER_ARGUMENT)
                * parameter_count);
    memory = xmalloc(size);
    memset(memory, '\0', size);

    args = (macro_arg *)memory;
    token_p = (const cpp_token **)(args + parameter_count);

    arg = args;
    arg_count = 0;

    do {
        unsigned int paren_depth = 0;
        unsigned int token_count = 0;
        
        arg_count++;
        arg->first = token_p;

        for (;;)
        {
            if (&(arg->first[token_count + 2])
                > (const cpp_token **)(memory + size))
            {
                char *old = memory;

                size += 987 * sizeof(cpp_token *);
                memory = xrealloc(memory, size);

                args = (macro_arg *)memory;
                arg = args + (arg_count - 1);
                token_p = (const cpp_token **)(memory
                                               + (((char *)token_p)
                                                  - old));
                arg->first = token_p;
            }

            token = cpp_get_token(reader);

            if (token->type == CPP_PADDING)
            {
                /* Discards whitespace at the beginning. */
                if (token_count == 0) continue;
            }
            else if (token->type == CPP_LPAREN) paren_depth++;
            else if (token->type == CPP_RPAREN)
            {
                if (paren_depth) paren_depth--;
                else break;
            }
            else if (token->type == CPP_COMMA)
            {
                if (paren_depth == 0
                    && !(macro->variadic
                         && (arg_count
                             == macro->parameter_count))) break;
            }
            else if (token->type == CPP_END)
            {
                break;
            }

            add_arg_token(arg, token,
                          token_count, MACRO_ARG_TOKEN_NORMAL);
            token_count++;
        }

        /* Discards whitespace at the end. */
        while (token_count > 0
               && (arg->first[token_count - 1]->type == CPP_PADDING))
        {
            token_count--;
        }

        arg->count = token_count;
        add_arg_token(arg, &(reader->end),
                         token_count, MACRO_ARG_TOKEN_NORMAL);

        if (arg_count <= macro->parameter_count)
        {
            token_p = &(arg->first[token_count + 1]);
            if (arg_count != macro->parameter_count)
            {
                arg++;
            }
        }
    } while ((token->type != CPP_RPAREN) && (token->type != CPP_END));

    if (token->type == CPP_END)
    {
        if (reader->unknown2 || reader->state.in_directive)
        {
            _cpp_return_tokens(reader, 1);
        }
        cpp_error(reader, CPP_DL_ERROR,
                  "unterminated argument list invoking macro \"%s\"",
                  UNKNOWN_NAME(unknown));
    }
    else
    {
        /* One empty argument is zero arguments. */
        if ((arg_count == 1)
            && (macro->parameter_count == 0)
            && (args[0].count == 0))
        {
            arg_count = 0;
        }
        if (_cpp_arguments_fine(reader, macro, unknown, arg_count))
        {
            if (arg_count_p) *arg_count_p = arg_count;

            return (memory);
        }
    }

    free(memory);
    
    return (NULL);
}

static void *function_like_invocation(cpp_reader *reader, cpp_unknown *unknown,
                                   unsigned int *arg_count)
{
    const cpp_token *token, *nicsh = NULL;

    for (;;)
    {
        token = cpp_get_token(reader);
        if (token->type != CPP_PADDING) break;
        if ((nicsh == NULL)
            || (!(nicsh->flags & AFTER_WHITE)
                && (token->value.source == NULL)))
        {
            nicsh = token;
        }
    }

    if (token->type == CPP_LPAREN)
    {
        reader->state.collecting_args = 2;
        return (collect_args(reader, unknown, arg_count));
    }

    if ((token->type != CPP_END) || (token == &(reader->end)))
    {
        _cpp_return_tokens(reader, 1);
        if (nicsh)
        {
            _cpp_add_unknown2_direct(reader, NULL, nicsh, 1);
        }
    }

    return (NULL);
}

static void delete_args(void *memory, unsigned int arg_count)
{
    macro_arg *args = memory;
    unsigned int i;

    if (memory == NULL) return;

    for (i = 0; i < arg_count; i++)
    {
        free(args[i].expanded);
        args[i].expanded = NULL;
    }

    free(memory);
}

static unsigned int macro_real_token_count(cpp_macro *macro)
{
    unsigned int i;
    
    if (macro->extra_tokens == 0) return (macro->count);

    for (i = macro->count; i--;)
    {
        if (macro->tokens[i].type != CPP_PASTE)
            return (i + 1);
    }

    return (0);
}

/* Allocates and returns CPP_STRING token with text with len.
 * text must be permanently allocated. */
static const cpp_token *new_string_token(cpp_reader *reader,
                                         char *text,
                                         unsigned int len)
{
    cpp_token *token = _cpp_temp_token(reader);

    text[len] = '\0';
    token->type = CPP_STRING;
    token->value.string.text = text;
    token->value.string.len = len;
    token->flags = 0;

    return (token);
}

static const cpp_token *strigify_arg(cpp_reader *reader, macro_arg *arg)
{
    char *memory, *end, *dest;
    size_t i, escape_it, backslash_count = 0;
    const cpp_token *source = NULL;
    size_t len;

    dest = memory = xmalloc(3);
    end = memory + 3;
    *(dest++) = '"';

    for (i = 0; i < arg->count; i++)
    {
        const cpp_token *token = arg->first[i];

        if (token->type == CPP_PADDING)
        {
            if ((source == NULL)
                || (!(source->flags & AFTER_WHITE)
                    && (token->value.source == NULL)))
            {
                source = token->value.source;
            }
            continue;
        }

        escape_it = ((token->type == CPP_STRING)
                     || (token->type == CPP_LSTRING)
                     || (token->type == CPP_uSTRING)
                     || (token->type == CPP_USTRING)
                     || (token->type == CPP_u8STRING)
                     || (token->type == CPP_CHAR)
                     || (token->type == CPP_LCHAR)
                     || (token->type == CPP_uCHAR)
                     || (token->type == CPP_UCHAR));

        len = cpp_token_len(token);
        if (escape_it) len *= 4;
        len += 3;

        if (((size_t)(end - dest)) < len)
        {
            size_t len_doteraz = dest - memory;
            size_t size = end - memory + len;
            memory = xrealloc(memory, size);
            end = memory + size;
            dest = memory + len_doteraz;
        }

        if (dest - 1 != memory)
        {
            if (source == NULL) source = token;
            if (source->flags & AFTER_WHITE) *(dest++) = ' ';
        }
        source = NULL;

        if (escape_it)
        {
            char *pmemory = xmalloc(len);
            len = cpp_spell_token(reader, token, pmemory, 1) - pmemory;
            dest = cpp_quote_string(dest, pmemory, len);
            free(pmemory);
        }
        else dest = cpp_spell_token(reader, token, dest, 1);

        if (token->type == CPP_OTHER && token->value.string.text[0] == '\\')
            backslash_count++;
        else backslash_count = 0;
    }

    if (backslash_count & 1)
    {
        cpp_error(reader, CPP_DL_WARNING,
                  "invalid string literal, ignoring final '\\'");
        dest--;
    }

    *(dest++) = '"';
    len = dest - memory;
    {
        char *new = _cpp_alloc_mem (reader, len);
        memcpy (new, dest - len, len);
        free (memory);
        memory = new;
    }
    return new_string_token (reader, memory, len);
}

static void expand_arg(cpp_reader *reader, macro_arg *arg)
{
    size_t size;
    
    if ((arg->count == 0) || (arg->expanded != NULL)) return;

    size = 356;
    arg->expanded = xmalloc(sizeof(const cpp_token *) * size);

    add_unknown2_indirect(reader, NULL, NULL,
                          arg->first, arg->count + 1);

    for (;;)
    {
        const cpp_token *token;

        if (arg->expanded_count + 1 > size)
        {
            size *= 2;
            arg->expanded = xrealloc(arg->expanded, size);
        }

        token = cpp_get_token(reader);

        if (token->type == CPP_END) break;

        add_arg_token(arg, token,
                      arg->expanded_count,
                      MACRO_ARG_TOKEN_EXPANDED);
        arg->expanded_count++;
    }

    _cpp_remove_unknown2(reader);
}

static void copy_paste_flag(cpp_reader *reader, const cpp_token **paste_flag,
                            const cpp_token *src)
{
    cpp_token *token = _cpp_temp_token(reader);

    token->type = (*paste_flag)->type;
    token->value = (*paste_flag)->value;
    if (src->flags & PASTE_LEFT)
    {
        token->flags = (*paste_flag)->flags | PASTE_LEFT;
    }
    else
    {
        token->flags = (*paste_flag)->flags & ~PASTE_LEFT;
    }

    *paste_flag = token;
}

static void replace_args(cpp_reader *reader, cpp_unknown *unknown,
                         cpp_macro *macro, macro_arg *args)
{
    unsigned int tokens_total, unknown_number, i;
    const cpp_token *token, *end;
    macro_arg *arg;
    void *memory;
    const cpp_token **first, **real_first;

    unknown_number = macro_real_token_count(macro);
    tokens_total = unknown_number;
    end = macro->tokens + unknown_number;

    for (token = macro->tokens; token < end; token++)
    {
        if (token->type == CPP_MACRO_ARGUMENT)
        {
            /* CPP_PADDING before and after. */
            tokens_total += 2;

            arg = args + token->value.macro_argument.arg_number;

            if (token->flags & STRINGIFY_ARGUMENT)
            {
                if (!(arg->stringified))
                    arg->stringified = strigify_arg(reader, arg);
            }
            else if ((token->flags & PASTE_LEFT)
                     || (token != macro->tokens
                         && token[-1].flags & PASTE_LEFT))
            {
                tokens_total += arg->count - 1;
            }
            else
            {
                if (!(arg->expanded)) expand_arg(reader, arg);
                tokens_total += arg->expanded_count - 1;
            }
        }
    }

    memory = xmalloc(sizeof(cpp_token *) * tokens_total);
    first = (const cpp_token **)memory;
    real_first = first;

    i = 0;
    for (token = macro->tokens; token < end; token++)
    {
        unsigned int arg_token_count;
        enum {
            STRING, NOTEXPAND, EXPAND
            } arg_type;
        const cpp_token **paste_flag = NULL;
        const cpp_token **tmp_token_ptr;
        
        if (token->type != CPP_MACRO_ARGUMENT)
        {
            *(first++) = token;
            i++;
            continue;
        }

        paste_flag = NULL;
        arg = args + token->value.macro_argument.arg_number;

        if (token->flags & STRINGIFY_ARGUMENT)
        {
            arg_token_count = 1;
            arg_type = STRING;
        }
        else if (token->flags & PASTE_LEFT)
        {
            arg_token_count = arg->count;
            arg_type = NOTEXPAND;
        }
        else if ((token != macro->tokens) && (token[-1].flags & PASTE_LEFT))
        {
            int num_toks;
            arg_token_count = arg->count;
            arg_type = NOTEXPAND;

            num_toks = first - real_first;
            if (num_toks)
            {
                tmp_token_ptr = first - 1;
                
                if (((*tmp_token_ptr)->type == CPP_COMMA)
                    && macro->variadic
                    && (token->value.macro_argument.arg_number
                        == macro->parameter_count))
                {
                    if (arg->first[0] == NULL) first--;
                    else paste_flag = tmp_token_ptr;
                }
                else if (arg_token_count == 0) paste_flag = tmp_token_ptr;
            }   
        }
        else
        {
            arg_token_count = arg->expanded_count;
            arg_type = EXPAND;
        }

        if ((!(reader->state.in_directive)
             || reader->state.directive_wants_padding)
            && (token != macro->tokens) && (!(token[-1].flags & PASTE_LEFT)))
        {
            const cpp_token *m = padding_token(reader, token);

            *(first++) = m;
        }

        if (arg_token_count)
        {
            unsigned int j;
            for (j = 0; j < arg_token_count; j++)
            {
                const cpp_token *t;
                switch (arg_type)
                {
                    case STRING: t = arg->stringified; break;
                    case NOTEXPAND: t = arg->first[j]; break;
                    case EXPAND: t = arg->expanded[j]; break;
                }

                *(first++) = t;
            }
            
            if (token->flags & PASTE_LEFT)
            {
                paste_flag = first - 1;
            }
        }
        else
        {
            /* +++Warnings for C90 */;
        }

        if (!(reader->state.in_directive) && !(token->flags & PASTE_LEFT))
        {
            const cpp_token *m = &(reader->avoid_paste);
            
            *(first++) = m;
        }

        if (paste_flag) copy_paste_flag(reader, paste_flag, token);

        i += arg_token_count;
    }

    add_unknown2_indirect(reader, unknown, memory,
                          (const cpp_token **)memory,
                          first - (const cpp_token **)memory);
}

char *_cpp_builtin_macro_text(cpp_reader *reader,
                              cpp_unknown *unknown,
                              location_t loc)
{
    char *result = NULL;
    unsigned long number = 1;

    switch (unknown->value.builtin)
    {
        default:
            cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                      "invalid built-in macro \"%s\"",
                      UNKNOWN_NAME(unknown));
            break;

        case BT_DATE:
        case BT_TIME:
            if (reader->date == NULL)
            {
                time_t tt = time(NULL);
                struct tm *ts = NULL;

                errno = 0;
                if ((tt != (time_t)-1) || (errno == 0))
                {
                    ts = localtime(&tt);
                }

                reader->date = xmalloc(sizeof("\"Abc 12 3456\""));
                reader->Ztime = xmalloc(sizeof("\"12:34:56\""));

                if (ts)
                {
                    char *months[] = {
                        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
                    sprintf((char *)(reader->date), "\"%s %2d %4d\"",
                            months[ts->tm_mon], ts->tm_mday,
                            ts->tm_year + 1900);
                    sprintf((char *)(reader->Ztime), "\"%02d:%02d:%02d\"",
                            ts->tm_hour, ts->tm_min, ts->tm_sec);
                }
                else
                {
                    cpp_error(reader, CPP_DL_WARNING,
                              "could not determine date and time");
                    strcpy(reader->date, "\"??? ?? ????\"");
                    strcpy(reader->Ztime, "\"??:??:??\"");
                }
            }

            if (unknown->value.builtin == BT_DATE)
            {
                result = xmalloc(strlen(reader->date) + 1);
                memcpy(result, reader->date, strlen(reader->date) + 1);
            }
            else
            {
                result = xmalloc(strlen(reader->Ztime) + 1);
                memcpy(result, reader->Ztime, strlen(reader->Ztime) + 1);
            }
            break;

        case BT_FILE: {
            const char *name = loc.file;
            size_t len = strlen(name);
            char *p = xmalloc(len * 2 + 3);
            result = p;
            *p = '"';
            p = cpp_quote_string(p + 1, name, len);
            *(p++) = '"';
            *p = '\0';
            break;
        }

        case BT_LINE:
            number = loc.line;
            break;
    }

    if (result == NULL)
    {
        result = xmalloc(21);

        sprintf((char *)result, "%lu", number);
    }

    return (result);
}

static int builtin_macro(cpp_reader *reader, cpp_unknown *unknown,
                         location_t loc, location_t expansion_loc)
{
    char *p;
    size_t len;
    char *p2;
    cpp_token *token;

    if (unknown->value.builtin == BT_PRAGMA)
    {
        /* _Pragma is not interpreted in directives. */
        if (reader->state.in_directive) return (0);

        printf("+++_Pragma is not supported %s:%u\n", __FILE__, __LINE__);
        return (0);
    }

    p = _cpp_builtin_macro_text(reader, unknown, expansion_loc);
    len = strlen((const char *)p);
    p2 = xmalloc(len + 1);

    memcpy(p2, p, len);
    free(p);
    p2[len] = '\n';

    _cpp_add_mffc(reader, p2, len, /* from_stage3 */ 1);
    _cpp_clean_line(reader);

    reader->cur_token = _cpp_temp_token(reader);
    token = _cpp_lex_token_direct(reader);
    token->src_loc = loc;

    _cpp_add_unknown2_direct(reader, NULL, token, 1);

    if (reader->mffc->pos != reader->mffc->end)
    {
        cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                  "invalid built-in macro \"%s\"",
                  UNKNOWN_NAME(unknown));
    }

    _cpp_remove_mffc(reader);
    
    return (1);
}

static int expand_macro(cpp_reader *reader, cpp_unknown *unknown,
                        location_t loc)
{
    reader->about_to_expand_macro = 1;
    if (unknown->type == UNKNOWN_MACRO)
    {
        cpp_macro *macro = unknown->value.macro;

        if (macro->function_like)
        {
            unsigned int arg_count = 0;
            void *v;
            
            reader->state.prevent_expansion++;
            reader->keep_tokens++;
            reader->state.collecting_args = 1;
            v = function_like_invocation(reader, unknown, &arg_count);
            reader->state.collecting_args = 0;
            reader->keep_tokens--;
            reader->state.prevent_expansion--;

            if (v == NULL)
            {
                reader->about_to_expand_macro = 0;
                return (0);
            }

            if (macro->parameter_count > 0)
            {
                replace_args(reader, unknown, macro,
                             (macro_arg *) v);
            }

            delete_args(v, arg_count);
        }

        unknown->flags |= UNKNOWN_DISABLED;

        if (macro->parameter_count == 0)
        {
            unsigned int token_count;
            token_count = macro_real_token_count(macro);
            _cpp_add_unknown2_direct(reader, unknown,
                                     macro->tokens,
                                     token_count);
        }

        reader->about_to_expand_macro = 0;
        return (1);
    }
    
    reader->about_to_expand_macro = 0;
    {
        location_t expansion_loc = reader->invocation_location;

        return (builtin_macro(reader, unknown, loc, expansion_loc));
    }
}

static int end_unknown2(cpp_unknown2 *unknown2)
{
    switch (unknown2->tokens_kind)
    {
        case TOKENS_KIND_DIRECT:
            return (unknown2->first.token == unknown2->last.token);
        case TOKENS_KIND_INDIRECT:
            return (unknown2->first.ptoken == unknown2->last.ptoken);
    }

    printf("CPPLIB Internal error %s:%u\n", __FILE__, __LINE__);
    return (-1);
}

static const cpp_token *get_token_from_unknown2(cpp_reader *reader)
{
    cpp_unknown2 *unknown2 = reader->unknown2;

    switch (unknown2->tokens_kind)
    {
        case TOKENS_KIND_DIRECT:
            return (unknown2->first.token++);
        case TOKENS_KIND_INDIRECT:
            return (*(unknown2->first.ptoken++));
    }

    printf("CPPLIB %s:%u Internal error\n", __FILE__, __LINE__);
    return (NULL);
}

/* Tries to paste 2 tokens. Returns 0 on success.
 * pleft is changed to token without PASTE_LEFT. */
static int paste_tokens(cpp_reader *reader,
                        const cpp_token **pleft,
                        const cpp_token *right)
{
    char *memory, *end, *left_end;
    cpp_token *left;
    unsigned int len;

    len = cpp_token_len(*pleft) + cpp_token_len(right) + 1;
    memory = xmalloc(len);
    end = left_end = cpp_spell_token(reader, *pleft, memory, 1);

    /* Avoids comments.
     * It is simpler to add space than to change _cpp_lex_token_direct(). */
    if (((*pleft)->type == CPP_DIV) && (right->type != CPP_EQ))
        *(end++) = ' ';
    /* Special case, when right token is CPP_PADDING. */
    if (right->type != CPP_PADDING)
        end = cpp_spell_token(reader, right, end, 1);
    *end = '\n';

    _cpp_add_mffc(reader, memory, end - memory, /* from_stage3 */ 1);
    _cpp_clean_line(reader);

    reader->cur_token = _cpp_temp_token(reader);
    left = _cpp_lex_token_direct(reader);
    if (reader->mffc->pos != reader->mffc->end)
    {
        _cpp_remove_mffc(reader);
        _cpp_return_tokens(reader, 1);
        *left_end = '\0';

        *left = **pleft;
        *pleft = left;
        left->flags &= ~PASTE_LEFT;

        cpp_error(reader, CPP_DL_ERROR,
                  "pasting \"%s\" and \"%s\""
                  " does not give a valid preprocessor token",
                  memory, cpp_token_as_text(right));
        free(memory);
        return (1);
    }
    *pleft = left;
    _cpp_remove_mffc(reader);
    /* Memory pointer was given to the MFFC which already deallocated it :/ */
    return (0);
}

static void paste_all_tokens(cpp_reader *reader, const cpp_token *left)
{
    const cpp_token *right;
    cpp_unknown2 *unknown2 = reader->unknown2;

    if ((unknown2 == NULL) || (!(left->flags & PASTE_LEFT)))
    {
        printf("Internal error %s:%u\n", __FILE__, __LINE__);
        return;
    }

    do {
        /* Gets token from the current unknown2. */
        switch (unknown2->tokens_kind)
        {
            case TOKENS_KIND_DIRECT:
                right = unknown2->first.token++; break;
            case TOKENS_KIND_INDIRECT:
                right = *(unknown2->first.ptoken++); break;
        }

        if (right->type == CPP_PADDING)
        {
            if (right->flags & PASTE_LEFT)
            {
                printf("Internal error %s:%u\n", __FILE__, __LINE__);
                return;
            }
        }
        
        if (paste_tokens(reader, &left, right)) break;
        
    } while (right->flags & PASTE_LEFT);

    _cpp_add_unknown2_direct(reader, NULL, left, 1);
}

static int in_macro_expansion(cpp_reader *reader)
{
    if (reader == NULL) return (0);

    return (reader->about_to_expand_macro
            || macro_unknown2(reader->unknown2));
}

const cpp_token *cpp_get_token_1(cpp_reader *reader,
                                 location_t *vloc)
{
    const cpp_token *result;
    location_t loc;

    for (;;)
    {
        cpp_unknown *unknown;
        cpp_unknown2 *unknown2 = reader->unknown2;

        if (unknown2 == NULL)
        {
            result = _cpp_lex_token(reader);
        }
        else if (!end_unknown2(unknown2))
        {
            result = get_token_from_unknown2(reader);
            if (result->flags & PASTE_LEFT)
            {
                paste_all_tokens(reader, result);
                if (reader->state.in_directive) continue;
                result = padding_token(reader, result);
                break;
            }
        }
        else
        {
            _cpp_remove_unknown2(reader);
            
            if (reader->state.in_directive) continue;

            result = &(reader->avoid_paste);
            break;
        }

        if (result == NULL) return NULL;
        loc = result->src_loc;
        if (result->flags & NO_EXPAND) break;
        if (result->type != CPP_IDENT) break;
        unknown = result->value.unknown;

        if (unknown->type == UNKNOWN_VOID) break;
        if (!(unknown->flags & UNKNOWN_DISABLED))
        {
            int expanded = 0;

            if (!in_macro_expansion(reader))
            {
                reader->invocation_location = loc;
            }

            if (reader->state.prevent_expansion)
            {
                break;
            }

            expanded = expand_macro(reader, unknown, loc);
            if (expanded)
            {
                result = padding_token(reader, result);
                break;
            }
        }
        else
        {
            /* Marks this token as unexpandable. */
            cpp_token *t = _cpp_temp_token(reader);
            t->type = result->type;
            t->flags = result->flags | NO_EXPAND;
            t->value = result->value;
            result = t;
        }

        break;
    }

    if (vloc)
    {
        *vloc = loc;

        if (macro_unknown2(reader->unknown2) != NULL)
        {
            *vloc = reader->invocation_location;
        }
    }

    return (result);
}

const cpp_token *cpp_get_token(cpp_reader *reader)
{
    return (cpp_get_token_1(reader, NULL));
}

/* Returns token with location perhaps different from token->src_loc.
 * If token comes from macro expansion,
 * location of top most macro expansion is returned. */
const cpp_token *cpp_get_token_with_location(cpp_reader *reader,
                                             location_t *vloc)
{
    return (cpp_get_token_1(reader, vloc));
}

/* Maximum bytes required for token spelling
 * (without whitespace before token). */
unsigned int cpp_token_len(const cpp_token *token)
{
    switch (token_spelling[token->type].type)
    {
        default: return (6);
        case SPELLING_IDENT: return (token->value.unknown->cell.len * 10);
        case SPELLING_LITERAL: return (token->value.string.len);
    }
}

char *cpp_spell_token(cpp_reader *reader, const cpp_token *token,
                      char *memory, int who_knows)
{
    switch (token_spelling[token->type].type)
    {
        case SPELLING_OPERATOR: {
            const char *spelling;
            char c;

            if (token->flags & DIGRAPH)
            {
                spelling = digraph_spelling[token->type - CPP_FIRST_DIGRAPH];
            }
            else spelling = token_spelling[token->type].text;

            while ((c = *(spelling++)) != '\0') *(memory++) = c;

            break;
        }
        
        case SPELLING_IDENT:
            if (who_knows)
            {
                memcpy(memory,
                       token->value.unknown->cell.name,
                       token->value.unknown->cell.len);
                memory += token->value.unknown->cell.len;
            }

            break;
        
        case SPELLING_LITERAL:
            memcpy(memory,
                   token->value.string.text,
                   token->value.string.len);
            memory += token->value.string.len;
            break;
        
        case SPELLING_NONE:
            cpp_error(reader, CPP_DL_INTERNAL_ERROR,
                      "unspellable token %s",
                      token_spelling[token->type].text);
            break;
    }

    return (memory);
}

const char *cpp_token_as_text(const cpp_token *token)
{
    switch (token_spelling[token->type].type)
    {
        case SPELLING_OPERATOR:
            if (token->flags & DIGRAPH)
            {
                return (digraph_spelling[token->type - CPP_FIRST_DIGRAPH]);
            }
            else return (token_spelling[token->type].text);
        case SPELLING_IDENT: return (token->value.unknown->cell.name);
        case SPELLING_LITERAL: return (token->value.string.text);
        case SPELLING_NONE: return ("");
    }

    return ("Unknown token type!");
}

char *cpp_quote_string(char *dest, const char *source, size_t len)
{
    while (len--)
    {
        unsigned char c = *(source++);

        switch (c)
        {
            case '\n':
                c = 'n';
                /* Fallthrough. */

            case '\\':
            case '"':
                *(dest++) = '\\';
        }

        *(dest++) = c;
    }

    return (dest);
}

char *cpp_output_line_to_string(cpp_reader *reader,
                                const char *directive_name)
{
    const cpp_token *token;
    size_t out = 0;
    size_t allocated = 128;
    char *result = xmalloc(allocated);
    
    if (directive_name)
    {
        sprintf((char *)result, "#%s ", directive_name);
        out += strlen((const char *)directive_name) + 2;
    }

    token = cpp_get_token(reader);
    while (token->type != CPP_END)
    {
        char *end;
        /* +2 for space and terminating '\0'. */
        size_t len = cpp_token_len(token) + 2;

        if (out + len > allocated)
        {
            allocated *= 2;
            if (out + len > allocated) allocated = out + len;

            result = xrealloc(result, allocated);
        }

        end = cpp_spell_token(reader, token, &(result[out]), 0);
        out = end - result;

        token = cpp_get_token(reader);
        if (token->flags & AFTER_WHITE) result[out++] = ' ';
    }

    result[out] = '\0';
    return (result); 
}

void cpp_output_token(const cpp_token *token, FILE *output)
{
    switch (token_spelling[token->type].type)
    {
        case SPELLING_OPERATOR: {
            const char *spelling;
            int c;

            if (token->flags & DIGRAPH)
            {
                spelling = digraph_spelling[token->type - CPP_FIRST_DIGRAPH];
            }
            else spelling = token_spelling[token->type].text;
            
            c = *spelling;
            do {
                putc(c, output);
            } while ((c = *(++spelling)) != '\0');

            break;
        }
        
        case SPELLING_IDENT: {
            size_t i;
            const char *name = UNKNOWN_NAME(token->value.unknown);

            for (i = 0; i < UNKNOWN_LEN(token->value.unknown); i++)
            {
                putc(name[i], output);
            }

            break;
        }
        
        case SPELLING_LITERAL:
            fwrite(token->value.string.text, 1,
                   token->value.string.len, output);
            break;
        
        case SPELLING_NONE: break;
    }
}

int cpp_avoid_paste(cpp_reader *reader,
                    const cpp_token *token1,
                    const cpp_token *token2)
{
    enum cpp_tokentype a = token1->type, b = token2->type;
    char c = ' ';
    if (token2->flags & DIGRAPH)
        c = digraph_spelling[b - CPP_FIRST_DIGRAPH][0];
    else if (token_spelling[b].type == SPELLING_OPERATOR)
        c = token_spelling[b].text[0];

    /* Quickly finds anything that can paste with '='. */
    if ((a <= CPP_RSHIFT) && (c == '=')) return (1);
    
    switch (a)
    {
        case CPP_GREATER: return (c == '>');
        case CPP_LESS: return ((c == '<') || (c == ':') || (c == '%'));
        case CPP_PLUS: return (c == '+');
        case CPP_MINUS: return ((c == '-') || (c == '>'));
        case CPP_DIV: return ((c == '/') || (c == '*')); /* Comments. */
        case CPP_MODULO: return ((c == '>') || (c == ':'));
        case CPP_AND: return (c == '&');
        case CPP_OR: return (c == '|');
        case CPP_COLON: return ((c == ':') || (c == '>'));
        case CPP_DEREF: return (c == '*');
        case CPP_DOT: return ((c == '.') || (b == CPP_NUMBER));
        case CPP_HASH: return ((c == '#') || (c == '%')); /* Digraph. */
        case CPP_IDENT: return ((b == CPP_NUMBER)
                                || (b == CPP_IDENT)
                                || (b == CPP_CHAR) || (CPP_STRING));
        case CPP_NUMBER: return ((b == CPP_NUMBER)
                                 || (b == CPP_IDENT)
                                 || (c == '.')
                                 || (c == '+')
                                 || (c == '-'));
        case CPP_OTHER: return ((token1->value.string.text[0] == '\\')
                                && (b == CPP_IDENT));
        default: break;
    }
    
    return (0);
}

void *cpp_get_help(cpp_reader *reader)
{
    return (reader->help);
}

void cpp_set_help(cpp_reader *reader, void *help)
{
    reader->help = help;
}

void *cpp_get_help2(cpp_reader *reader)
{
    return (reader->help2);
}

void cpp_set_help2(cpp_reader *reader, void *help)
{
    reader->help2 = help;
}
