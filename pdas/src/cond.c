/******************************************************************************
 * @file            cond.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "as.h"

struct if_stack {
    struct if_stack *prev;
    
    const char *filename;
    unsigned long line_number;
    const char *else_filename;
    unsigned long else_line_number;
    
    int ignoring;
    int prev_ignoring;
    int has_else;
};

static struct if_stack *cur_if_stack = NULL;

static void add_if_stack (void)
{
    struct if_stack *if_stack = xmalloc (sizeof *if_stack);
    
    memset (if_stack, 0, sizeof *if_stack);

    get_filename_and_line_number (&if_stack->filename, &if_stack->line_number);
    
    if (cur_if_stack) if_stack->prev_ignoring = cur_if_stack->ignoring;

    if_stack->prev = cur_if_stack;
    cur_if_stack = if_stack;
}

static void handler_ifdef (char **pp, int ifndef)
{
    const char *name;
    char ch;
    struct symbol *symbol;

    name = *pp;
    ch = get_symbol_name_end (pp);
    
    if (name == *pp) {
        as_error ("invalid identifier for \"%s\"",
                  ifndef ? ".ifndef" : ".ifdef");
        **pp = ch;
        ignore_rest_of_line (pp);
        return;
    }

    symbol = symbol_find (name);
    **pp = ch;

    add_if_stack ();

    if (cur_if_stack->prev_ignoring) {
        cur_if_stack->ignoring = 1;
    } else {
        int is_defined = 0;

        if (symbol
            && !symbol_is_undefined (symbol)
            && symbol_get_section (symbol) != reg_section) is_defined = 1;

        cur_if_stack->ignoring = is_defined ^ (!ifndef);
    }

    demand_empty_rest_of_line (pp);
}

int cond_can_ignore_line (const char *line)
{
    if (line[0] != '.') {
        if (!state->no_pseudo_dot) return cur_if_stack && cur_if_stack->ignoring;

        line--;
    }
    
    if (((line[1] == 'i'
          || line[1] == 'I')
         && (xstrncasecmp (line + 1, "if", 2) == 0
             || xstrncasecmp (line + 1, "ifdef", 5) == 0
             || xstrncasecmp (line + 1, "ifndef", 6) == 0))
        || ((line[1] == 'e'
             || line[1] == 'E')
            && (xstrncasecmp (line + 1, "else", 4) == 0
                || xstrncasecmp (line + 1, "endif", 5) == 0))) {
        return 0;
    }

    return cur_if_stack && cur_if_stack->ignoring;
}

void cond_end_of_file (void)
{
    if (cur_if_stack) {
        as_error ("end of file inside conditional");
        as_error_at (cur_if_stack->filename,
                     cur_if_stack->line_number,
                     "here is the start of the unterminated conditional");
        if (cur_if_stack->has_else) {
            as_error_at (cur_if_stack->else_filename,
                         cur_if_stack->else_line_number,
                         "here is the \"else\" of the unterminated conditional");
        }

        {
            struct if_stack *prev;

            for (; cur_if_stack; cur_if_stack = prev) {
                prev = cur_if_stack->prev;
                free (cur_if_stack);
            }
        }
    }
}

void cond_handler_if (char **pp)
{
    add_if_stack ();
    
    if (cur_if_stack->prev_ignoring) {
        cur_if_stack->ignoring = 1;
        ignore_rest_of_line (pp);
        return;
    } else {
        struct expr expr_s;

        expression_evaluate_and_read_into (pp, &expr_s);
        
        if (expr_s.type != EXPR_TYPE_CONSTANT) {
            as_error ("non-constant expression in \".if\" statement");
        }

        if (expr_s.add_number == 0) cur_if_stack->ignoring = 1;
    }

    demand_empty_rest_of_line (pp);
}

void cond_handler_ifdef (char **pp)
{
    handler_ifdef (pp, 0);
}

void cond_handler_ifndef (char **pp)
{
    handler_ifdef (pp, 1);
}

void cond_handler_else (char **pp)
{
    if (!cur_if_stack) {
        as_error ("\".else\" without matching \".if\"");
    } else if (cur_if_stack->has_else) {
        as_error ("duplicate \"else\"");
        as_error_at (cur_if_stack->filename,
                     cur_if_stack->line_number,
                     "here is the previous \"if\"");
        as_error_at (cur_if_stack->else_filename,
                     cur_if_stack->else_line_number,
                     "here is the previous \"else\"");
    } else {
        get_filename_and_line_number (&cur_if_stack->else_filename,
                                      &cur_if_stack->else_line_number);

        cur_if_stack->ignoring = cur_if_stack->prev_ignoring || !cur_if_stack->ignoring;
        cur_if_stack->has_else = 1;
    }

    demand_empty_rest_of_line (pp);
}

void cond_handler_endif (char **pp)
{
    if (!cur_if_stack) {
        as_error ("\".endif\" without \".if\"");
    } else {
        struct if_stack *prev;

        prev = cur_if_stack->prev;
        free (cur_if_stack);
        cur_if_stack = prev;
    }

    demand_empty_rest_of_line (pp);
}
