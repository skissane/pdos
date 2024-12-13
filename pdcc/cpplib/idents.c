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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static symtab_cell *alloc_cell(void)
{
    void *cell = xmalloc(sizeof(cpp_unknown));
    memset(cell, '\0', sizeof(cpp_unknown));
    return (cell);
}

static void free_cell(symtab_cell *cell)
{    
    if (cell->name) {
        cpp_unknown *unknown = CPP_UNKNOWN (cell);
        
        if (unknown->type == UNKNOWN_MACRO) {
            _cpp_undefine_macro (unknown);
        }
        
        free (cell->name);
    }
    free (cell);
}

void _cpp_init_symtab(cpp_reader *reader, symtab *tab)
{
    struct spec_unknowns *s;
    
    if (tab == NULL)
    {
        tab = symtab_create_symtab(1024, &alloc_cell);
        reader->own_table = 1;
    }
    else reader->own_table = 0;

    reader->tab = tab;

    _cpp_init_directives(reader);

    s = &(reader->spec_unknowns);
#define STR_AND_LEN(str) (str), sizeof(str) - 1
    s->n_defined = cpp_find(reader, STR_AND_LEN("defined"));
    s->n___VA_ARGS__ = cpp_find(reader, STR_AND_LEN("__VA_ARGS__"));
}

void _cpp_destroy_symtab(cpp_reader *reader)
{
    if (reader->own_table)
    {
        symtab_destroy_symtab(reader->tab, &free_cell);
    }
}

cpp_unknown *cpp_find(cpp_reader *reader, const char *name, size_t len)
{
    return (CPP_UNKNOWN(symtab_find(reader->tab, name, len)));
}
