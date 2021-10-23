/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include "c_ppout.h"
#include "cpplib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NOLINE
#define LINEPREF "#"
#else
#define LINEPREF "#line"
#endif

struct pp_output {
    FILE *out;
    const cpp_token *prev;
    const cpp_token *source; /* Source token for spacing. */
    unsigned long line;
    int printed; /* 1, if something was output on the line. */
    const char *src_file;
};

static int print_line(pp_output *pp_o, location_t loc, const char *flags)
{
    if (pp_o->printed)
    {
        putc('\n', pp_o->out);
        pp_o->printed = 0;
    }

    {
        const char *filename = loc.file;
        size_t filename_len = strlen(filename);
        unsigned char *new_name = malloc(filename_len * 4 + 1);
        unsigned char *s;

        if (new_name == NULL)
        {
            printf("failed to allocate memory\n");
            abort();
        }

        pp_o->line = loc.line;
        pp_o->src_file = filename;

        s = cpp_quote_string(new_name,
                             (const unsigned char *) filename,
                             filename_len);
        *s = '\0';

        fprintf(pp_o->out, LINEPREF " %lu \"%s\"%s\n",
                pp_o->line == 0 ? 1 : pp_o->line,
                new_name, flags);

        free(new_name);
    }
    
    return (1);
}

static int maybe_print_line(pp_output *pp_o, location_t loc)
{
    int linemarker_emitted = 0;
    int orig_line = loc.line;
    const char *orig_file = loc.file;

    if (pp_o->printed)
    {
        putc('\n', pp_o->out);
        pp_o->line++;
        pp_o->printed = 0;
    }

    if ((orig_line >= pp_o->line)
        && (orig_line < pp_o->line + 8)
        && (orig_file == pp_o->src_file))
    {
        while (orig_line > pp_o->line)
        {
            putc('\n', pp_o->out);
            pp_o->line++;
        }
    }
    else linemarker_emitted = print_line(pp_o, loc, "");
    
    return (linemarker_emitted);
}

static int do_line_change(pp_output *pp_o, cpp_reader *reader,
                             const cpp_token *token, location_t loc,
                             int collecting_args)
{
    int linemarker_emitted = 0;
    
    if ((token->type == CPP_END) || collecting_args)
    {
        return (0);
    }

    linemarker_emitted = maybe_print_line(pp_o, loc);
    pp_o->prev = NULL;
    pp_o->source = NULL;

    {
        int spaces = loc.column - 2;
        pp_o->printed = 1;

        while (--spaces >= 0)
        {
            putc(' ', pp_o->out);
        }
    }
    
    return (linemarker_emitted);
}

static void line_change(cpp_reader *reader, const cpp_token *token,
                         int collecting_args)
{
    do_line_change(cpp_get_help2(reader),
                   reader,
                   token,
                   token->src_loc,
                   collecting_args);
}

static void file_change(cpp_reader *reader,
                        location_t loc,
                        int reason)
{
    const char *flags = "";

    switch(reason)
    {
        case CPP_REASON_ENTER: flags = " 1"; break;
        case CPP_REASON_LEAVE: flags = " 2"; break;
    }
    
    print_line(cpp_get_help2(reader), loc, flags);
}

pp_output *init_pp_output(cpp_reader *reader, FILE *out)
{
    pp_output *pp_o = malloc(sizeof(*pp_o));

    if (pp_o == NULL)
    {
        printf("failed to allocate memory\n");
        abort();
    }

    cpp_get_callbacks(reader)->line_change = &line_change;
    cpp_get_callbacks(reader)->file_change = &file_change;
    cpp_set_help2(reader, pp_o);

    pp_o->line = 1;
    pp_o->printed = 0;
    pp_o->prev = NULL;
    pp_o->src_file = "";
    pp_o->out = out;

    return (pp_o);
}

void preprocess_file(pp_output *pp_o, cpp_reader *reader)
{
    int avoid_paste = 0;
    int linemarker_emitted = 0;
    int do_line_adjustments = 1;

    pp_o->source = NULL;
    for (;;)
    {
        location_t loc;
        const cpp_token *token = cpp_get_token_with_location(reader, &loc);

        if (token->type == CPP_END) break;
        
        if (token->type == CPP_PADDING)
        {
            avoid_paste = 1;
            if ((pp_o->source == NULL)
                || (!(pp_o->source->flags & AFTER_WHITE)
                    && (token->value.source == NULL)))
            {
                pp_o->source = token->value.source;
            }
            continue;
        }

        if (avoid_paste)
        {
            unsigned long orig_line = loc.line;

            if (pp_o->source == NULL)
            {
                pp_o->source = token;
            }

            if (do_line_adjustments
                && (orig_line != pp_o->line))
            {
                linemarker_emitted = do_line_change(pp_o,
                                                    reader,
                                                    token,
                                                    loc,
                                                    0);
                putc(' ', pp_o->out);
                pp_o->printed = 1;
            }
            else if ((pp_o->source->flags & AFTER_WHITE)
                     || (pp_o->prev
                         && cpp_avoid_paste(reader, pp_o->prev, token))
                     || ((pp_o->prev == NULL)
                         && (token->type == CPP_HASH)))
            {
                putc(' ', pp_o->out);
                pp_o->printed = 1;
            }
        }
        else if (token->flags & AFTER_WHITE)
        {
            unsigned long orig_line = loc.line;

            if (do_line_adjustments
                && (orig_line != pp_o->line))
            {
                linemarker_emitted = do_line_change(pp_o,
                                                    reader,
                                                    token,
                                                    loc,
                                                    0);
            }
                
            putc(' ', pp_o->out);
            pp_o->printed = 1;
        }

        avoid_paste = 0;
        pp_o->source = NULL;
        pp_o->prev = token;

        if (do_line_adjustments
            && !linemarker_emitted)
        {
            ;
        }

        cpp_output_token(token, pp_o->out);
        linemarker_emitted = 0;
        pp_o->printed = 1;
    }

    if (pp_o->printed) putc('\n', pp_o->out);

    if (pp_o->out != stdout) fclose(pp_o->out);
    free(pp_o);
}
