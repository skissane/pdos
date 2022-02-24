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

#include "as.h"

char *input_line_pointer;

char lex_table[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,      /* 0123456789:;<=>? */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      /* @ABCDEFGHIJKLMNO */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 3,      /*# PQRSTUVWXYZ[\]^_ */
    0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      /* `abcdefghijklmno */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0       /* pqrstuvwxyz{|}~  */
};

char get_symbol_end(void)
{
    if (is_name_beginner(*input_line_pointer++))
    {
        char c;
        
        while (is_name_part(*input_line_pointer++)) ;

        c = *--input_line_pointer;

        *input_line_pointer = '\0';

        return (c);
    }

    input_line_pointer--;
}

static void skip_rest_of_line(void)
{
    char c = *input_line_pointer++;
    while ((c != '\n')
           && (c != '\0')) c = *input_line_pointer++;
}

static void handler_ascii(void)
{
    char c = *input_line_pointer++;

    if (c == '"')
    {
        while ((c = *input_line_pointer++) != '"')
        {
            switch (c)
            {
                case '\n':
                case '\0':
                    as_error("+++ %s %i\n", __FILE__, __LINE__);
                    skip_rest_of_line();
                    return;

                case '\\':
                    c = *input_line_pointer++;
                    switch (c)
                    {
                        case '0':
                            frag_append_1_char('\0');
                            break;
                    }
                    break;

                default: frag_append_1_char(c); break;
            }
        }
    }

    skip_rest_of_line();
}

static void handler_comm(void)
{
    sectionT saved_section = current_section;
    char c;
    char *name, *p;
    exprS expr;
    unsigned long size;
    symbolS *symbol;

    name = input_line_pointer;
    c = get_symbol_end();
    p = input_line_pointer;
    *p = c;

    if (name == p)
    {
        as_error("Expected symbol name!\n");
        return;
    }

    SKIP_WHITESPACE();

    if (*input_line_pointer == ',') input_line_pointer++;

    expr_read_into(&expr);
    if (expr.type != Expr_type_constant)
    {
        as_error("+++handler_comm\n");
        return;
    }

    *p = '\0';
    symbol = symbol_find_or_make(name);
    *p = c;

    if (symbol_is_undefined(symbol))
    {
        section_set(bss_section);

        symbol->section = bss_section;
        symbol->frag = current_frag;
        symbol->value = current_frag->fixed_size;
        /* Difference between .comm and .lcomm
         * is that in .comm the symbol is set as global. */
        symbol_set_external(symbol);

        frag_increase_fixed_size(expr.add_number);
    }
    else
    {
        as_error("Symbol `%s' is already defined!\n", symbol->name);
    }

    section_set(saved_section);
}

static void handler_constant(int size)
{
    exprS expr;

    do
    {
        expr_read_into(&expr);

        if (expr.type == Expr_type_constant)
        {
            int i;

            for (i = 0; i < size; i++)
            {
                frag_append_1_char((expr.add_number >> (8 * i)) & 0xff);
            }
        }
        else if (expr.type == Expr_type_symbol)
        {
            fixup_new_expr(current_frag,
                           current_frag->fixed_size,
                           size,
                           &expr,
                           0/* pcrel */);

            frag_increase_fixed_size(size);
        }
        else
        {
            as_error("+++handler_constant");
        }
    }
    while (*input_line_pointer++ == ',');
}

static void handler_byte(void)
{
    handler_constant(1);
}

static void handler_lcomm(void)
{
    sectionT saved_section = current_section;
    char c;
    char *name, *p;
    exprS expr;
    unsigned long size;
    symbolS *symbol;

    name = input_line_pointer;
    c = get_symbol_end();
    p = input_line_pointer;
    *p = c;

    if (name == p)
    {
        as_error("Expected symbol name!\n");
        return;
    }

    SKIP_WHITESPACE();

    if (*input_line_pointer == ',') input_line_pointer++;

    expr_read_into(&expr);
    if (expr.type != Expr_type_constant)
    {
        as_error("+++handler_lcomm\n");
        return;
    }

    *p = '\0';
    symbol = symbol_find_or_make(name);
    *p = c;

    if (symbol_is_undefined(symbol))
    {
        section_set(bss_section);

        symbol->section = bss_section;
        symbol->frag = current_frag;
        symbol->value = current_frag->fixed_size;

        frag_increase_fixed_size(expr.add_number);
    }
    else
    {
        as_error("Symbol `%s' is already defined!\n", symbol->name);
    }

    section_set(saved_section);
}

static void handler_long(void)
{
    handler_constant(4);
}

static void handler_data(void)
{
    section_set(data_section);
}

static void handler_global(void)
{
    char c;
    char *s = input_line_pointer;
    symbolS *symbol;

    c = get_symbol_end();
    symbol = symbol_find_or_make(s);
    symbol_set_external(symbol);

    *input_line_pointer++ = c;
}

static void handler_space(void)
{
    exprS expr;

    expr_read_into(&expr);

    if (expr.type == Expr_type_constant)
    {
        unsigned long bytes;

        for (bytes = 0; bytes < expr.add_number; bytes++)
        {
            frag_append_1_char('\0');
        }
    }
    else
    {
        as_error("+++handler_space\n");
    }
}

static void handler_text(void)
{
    section_set(text_section);
}

static void handler_ignore(void)
{
    skip_rest_of_line();
}

typedef struct {
    const char *name;
    void (*handler)(void);
} Pseudo_op_entry;

const Pseudo_op_entry pseudo_op_table[] = {
    {"ascii", &handler_ascii},
    {"byte", &handler_byte},
    {"comm", &handler_comm},
    {"data", &handler_data},
    {"file", &handler_ignore},
    {"globl", &handler_global},
    {"global", &handler_global},
    {"ident", &handler_ignore},
    {"lcomm", &handler_lcomm},
    {"long", &handler_long},
    {"space", &handler_space},
    {"text", &handler_text},
    {0, 0}
};

void read_a_source_file(const char *filename)
{
    FILE *f;
    char buf[100];

    f = fopen(filename, "r");
    if (f == NULL)
    {
        as_error("Failed to open %s!\n", filename);
        return;
    }

    while (fgets(buf, 100, f))
    {
        char *buf_end = buf + strlen(buf);
        input_line_pointer = buf;

        while (input_line_pointer < buf_end)
        {
            char c;

            c = *input_line_pointer++;

            while ((c == ' ')
                   || (c == '\t')
                   || (c == '\f')) c = *input_line_pointer++;

            if (is_name_beginner(c))
            {
                char *s = --input_line_pointer;

                c = get_symbol_end();

                if (c == ':')
                {
                    symbol_label(s);
                    *input_line_pointer++ = c;
                }
                else
                {
                    if (*s == '.')
                    {
                        const Pseudo_op_entry *poe;
                        
                        s++;
                        
                        for (poe = pseudo_op_table;
                             poe->name && strcmp(poe->name, s);
                             poe++);

                        if (poe->name == NULL)
                        {
                            as_error("Unknown pseudo-op: `%s'\n", --s);
                            *input_line_pointer = c;
                            skip_rest_of_line();
                            continue;
                        }
                        else
                        {
                            *input_line_pointer++ = c;
                            SKIP_WHITESPACE();
                            (*(poe->handler))();
                        }
                    }
                    else
                    {
                        *input_line_pointer++ = c;

                        while ((c != '\n')
                               && (c != '\0')) c = *input_line_pointer++;
                        c = *--input_line_pointer;
                        *input_line_pointer = '\0';

                        machine_dependent_assemble_line(s);

                        *input_line_pointer++ = c;
                    }
                }
                
                continue;
            }

            if (c == '\n') continue;

            if (c == '/')
            {
                /* A comment. */
                c = *input_line_pointer++;
                while ((c != '\n')
                       && (c != '\0')) c = *input_line_pointer++;
                continue;
            }

            --input_line_pointer;
            as_error("Ignoring rest of line: %s\n", input_line_pointer);
            break;
        }
    }

    if (fclose(f))
    {
        as_error("Failed to close file %s!\n", filename);
    }
}
