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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static cpp_macro *new_macro(void)
{
    cpp_macro *macro = xmalloc(sizeof(*macro));

    macro->parameters = NULL;
    macro->count = 0;
    macro->parameter_count = 0;
    macro->function_like = 0;
    macro->variadic = 0;
    macro->extra_tokens = 0;

    return (macro);
}

static void destroy_macro(cpp_macro *macro)
{
    cpp_token *token;
    
    free (macro->parameters);
    free (macro);
}

typedef struct {
    cpp_unknown *unknown;
    int type;
    union _cpp_unknown_value value;
} macro_parameter_saved_data;

int _cpp_save_parameter(cpp_reader *reader,
                        int parameter_count,
                         cpp_unknown *unknown,
                        cpp_unknown ***parameters)
{
    unsigned int size;
    
    if (unknown->type == UNKNOWN_MACRO_PARAMETER)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "duplicate macro parameter \"%s\"",
                  UNKNOWN_NAME(unknown));
        return (1);
    }

    size = (sizeof(macro_parameter_saved_data)
                * (parameter_count + 1));
    if (size > reader->macro_memory_size)
    {
        reader->macro_memory = xrealloc(reader->macro_memory, size);

        reader->macro_memory_size = size;
    }

    {
        macro_parameter_saved_data *data = (void *)(reader->macro_memory);

        data += parameter_count;

        data->unknown = unknown;
        data->type = unknown->type;
        data->value = unknown->value;
    }

    *parameters = xrealloc(*parameters,
                           sizeof (**parameters) * (parameter_count + 1));

    (*parameters)[parameter_count] = unknown;

    unknown->type = UNKNOWN_MACRO_PARAMETER;
    unknown->value.parameter_index = parameter_count;
    
    return (0);
}

void _cpp_unsave_parameters(cpp_reader *reader, int parameter_count)
{
    while (parameter_count--)
    {
        cpp_unknown *unknown;
        macro_parameter_saved_data *data = (void *)(reader->macro_memory);

        data += parameter_count;

        unknown = data->unknown;
        unknown->type = data->type;
        unknown->value = data->value;
    }
}

static int collect_parameters(cpp_reader *reader, int *parameter_count,
                              int *variadic, cpp_unknown ***parameters)
{
    int after_ident = 0;
    
    *parameter_count = 0;
    *variadic = 0;

    *parameters = NULL;
    
    for (;;)
    {
        const cpp_token *token = _cpp_lex_token(reader);

        switch (token->type)
        {
            case CPP_IDENT: {
                if (after_ident || *variadic) goto bad;
                after_ident = 1;
                if (_cpp_save_parameter(reader, *parameter_count,
                                         token->value.unknown, parameters))
                {
                    return (1);
                }
                (*parameter_count)++;
                break;
            }

            case CPP_COMMA: {
                if (!after_ident || *variadic) goto bad;
                after_ident = 0;
                break;
            }

            case CPP_ELLIPSIS: {
                if (*variadic) goto bad;
                if (after_ident)
                {
                    cpp_error(reader, CPP_DL_ERROR,
                              "named variadic macros are not permitted");
                    return (1);
                }
                if (_cpp_save_parameter(reader,
                                        *parameter_count,
                                        reader->spec_unknowns.n___VA_ARGS__,
                                        parameters))
                {
                    return (1);
                }
                (*parameter_count)++;
                *variadic = 1;
                reader->state.va_args_fine = 1;
                break;
            }

            case CPP_RPAREN: {
                if (after_ident || !*parameter_count || *variadic)
                {
                    return (0);
                }
                goto bad;
            }

            default:
            bad: {
                const char *messages[] = {
                    "expected parameter name, found \"%s\"",
                    "expected ',' or ')', found \"%s\"",
                    "expected parameter name before end of line",
                    "expected ')' before end of line",
                    "expected ')' after \"...\""};
                int i = after_ident;
                const char *as_text = NULL;
                if (*variadic) i = 4;
                else if (token->type == CPP_END) i += 2;
                else as_text = cpp_token_as_text(token);

                cpp_error(reader, CPP_DL_ERROR, messages[i], as_text);
                return (1);
            }
        }
    }
}

static cpp_macro *macro_add_token(cpp_reader *reader, cpp_macro *macro)
{
    cpp_token *token;

    macro = xrealloc(macro,
                     sizeof(*macro) + sizeof(cpp_token) * (macro->count));
    {
        cpp_token *saved_cur_token = reader->cur_token;

        reader->cur_token = &(macro->tokens[macro->count]);
        token = _cpp_lex_token_direct(reader);

        reader->cur_token = saved_cur_token;
    }

    if (token->type == CPP_IDENT
        && token->value.unknown->type == UNKNOWN_MACRO_PARAMETER)
    {
        cpp_unknown *unknown = token->value.unknown;
        token->type = CPP_MACRO_ARGUMENT;
        token->value.macro_argument.arg_number = (token->value.unknown
                                                  ->value.parameter_index);
        token->value.macro_argument.spelling = unknown;
    }
    return (macro);
}

static cpp_macro *create_definition(cpp_reader *reader)
{
    cpp_macro *macro = NULL;
    cpp_token first;
    cpp_token *token;

    cpp_unknown **parameters;
    int parameter_count = 0;
    int variadic = 0;

    int good = 0;
    int following_paste = 0;
    unsigned int extra_tokens = 0;

    {
        cpp_token *saved_cur_token = reader->cur_token;

        reader->cur_token = &first;
        token = _cpp_lex_token_direct(reader);

        reader->cur_token = saved_cur_token;
    }

    if (token->flags & AFTER_WHITE) /* Part of the expansion. */;
    else if (token->type == CPP_LPAREN)
    {
        if (collect_parameters(reader,
                               &parameter_count,
                               &variadic,
                               &parameters))
        {
            goto end;
        }

        token = NULL;
    }
    else if (token->type != CPP_END)
    {
        cpp_error(reader, CPP_DL_WARNING,
                  "missing whitespace after macro name");
    }

    macro = new_macro();

    if (token == NULL)
    {
        macro->parameter_count = parameter_count;
        macro->variadic = variadic;
        macro->parameters = parameters;
        macro->function_like = 1;
    }
    else
    {
        macro->tokens[0] = *token;
        token = &(macro->tokens[0]);
        macro->count = 1;
    }

    for (;; token = NULL)
    {
        if (token == NULL)
        {
            macro = macro_add_token(reader, macro);
            token = &(macro->tokens[macro->count++]);
        }

        if ((macro->count > 1)
            && (token[-1].type == CPP_HASH)
            && (macro->function_like))
        {
            if (token->type == CPP_MACRO_ARGUMENT)
            {
                if (token->flags & AFTER_WHITE)
                {
                    token->flags |= SP_AFTER_WHITE;
                }
                if (token[-1].flags & DIGRAPH)
                {
                    token->flags |= SP_DIGRAPH;
                }
                token->flags &= ~AFTER_WHITE;
                token->flags |= STRINGIFY_ARGUMENT;
                token->flags |= token[-1].flags & AFTER_WHITE;
                token[-1] = token[0];
                macro->count--;
            }
            else
            {
                cpp_error(reader, CPP_DL_ERROR,
                          "'#' is not followed by a macro parameter");
                goto end;
            }
        }
        
        if (token->type == CPP_END)
        {
            if (following_paste)
            {
                cpp_error(reader, CPP_DL_ERROR,
                          "'##' cannot appear at the end"
                          " of a macro expansion");
                goto end;
            }
            break;
        }

        if (token->type == CPP_PASTE)
        {
            if (macro->count == 1)
            {
                cpp_error(reader, CPP_DL_ERROR,
                          "'##' cannot appear at the beginning"
                          " of a macro expansion");
                goto end;
            }

            if (following_paste)
            {
                /* Consecutive CPP_PASTE.
                 * This one will be moved to the end. */
                extra_tokens++;
                token->value.token_number = macro->count - 1;
            }
            else
            {
                /* Discards CPP_PASTE. */
                macro->count--;
                token[-1].flags |= PASTE_LEFT;
                if (token->flags & AFTER_WHITE)
                    token[-1].flags |= SP_AFTER_WHITE;
                if (token->flags & DIGRAPH)
                    token[-1].flags |= SP_DIGRAPH;
            }

            following_paste = 1;
        }
        else following_paste = 0;
    }

    good = 1;

    /* CPP_END is not counted. */
    macro->count--;

    if (macro->count)
    {
        /* Whitespace before first token is ignored. */
        macro->tokens[0].flags &= ~AFTER_WHITE;
    }

    if (extra_tokens)
    {
        /* Places ## in sequence at the end of list. */
        cpp_token *temp = xmalloc(sizeof(cpp_token) * extra_tokens);
        unsigned int norm_index = 0, extra_index = 0;
        unsigned int index;

        for (index = 0; index < macro->count; index++)
        {
            if (macro->tokens[index].type == CPP_PASTE)
                temp[extra_index++] = macro->tokens[index];
            else
                macro->tokens[norm_index++] = macro->tokens[index];
        }

        memcpy(&(macro->tokens[norm_index]), temp,
               sizeof(cpp_token) * extra_tokens);

        free(temp);
        
        macro->extra_tokens = 1;
    }

end:
    reader->state.va_args_fine = 0;
    if (good == 0 && macro)
    {
        destroy_macro(macro);
        macro = NULL;
    }
    
    _cpp_unsave_parameters(reader, parameter_count);

    return (macro);
}

int warn_of_redefinition(cpp_reader *reader, cpp_unknown *unknown,
                        const cpp_macro *macro2)
{
    unsigned int i;
    cpp_macro *macro1 = unknown->value.macro;

    if (cpp_builtin_macro_s(unknown)) return (1);

    /* Redefinition of macro is allowed only when the new definition
     * is the same as the old one. */
    if ((macro1->parameter_count != macro2->parameter_count)
        || (macro1->function_like != macro2->function_like)
        || (macro1->variadic != macro2->variadic))
        return (1);

    for (i = macro1->parameter_count; i--; )
    {
        if (macro1->parameters[i] != macro2->parameters[i])
            return (1);
    }

    if (macro1->count != macro2->count) return (1);

    for (i = macro1->count; i--; )
    {
        if (!_cpp_equiv_tokens(&(macro1->tokens[i]), &(macro2->tokens[i])))
            return (1);
    }

    return (0);
}

int _cpp_define_macro(cpp_reader *reader, cpp_unknown *unknown)
{
    cpp_macro *macro = create_definition(reader);
    if (macro == NULL) return (1);

    if (cpp_macro_s(unknown))
    {
        if (warn_of_redefinition(reader, unknown, macro))
        {
            cpp_error_with_line(reader, CPP_DL_WARNING,
                                reader->directive_line, 0,
                                "\"%s\" redefined", UNKNOWN_NAME(unknown));
        }

        if (unknown->type == UNKNOWN_MACRO) _cpp_undefine_macro (unknown);
    }

    unknown->type = UNKNOWN_MACRO;
    unknown->value.macro = macro;
    return (0);
}

void _cpp_undefine_macro(cpp_unknown *unknown)
{
    if (unknown->type == UNKNOWN_MACRO) destroy_macro (unknown->value.macro);
    unknown->type = UNKNOWN_VOID;
    unknown->flags &= ~UNKNOWN_DISABLED;
}
