/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include "symtab.h"
#include "xmalloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

symtab *symtab_create_symtab(size_t size,
                             symtab_cell *(*alloc_cell)(void))
{
    symtab *tab = xmalloc(sizeof(*tab));
    size_t i;

    if (tab == NULL) return (NULL);

    tab->size = size;
    tab->alloc_cell = alloc_cell;
    tab->cells = xmalloc(sizeof(*(tab->cells)) * size);

    if (tab->cells == NULL)
    {
        free(tab);
        return (NULL);
    }

    for (i = 0; i < size; i++)
    {
        tab->cells[i] = NULL;
    }

    return (tab);
}

symtab_cell *symtab_find(symtab *tab,
                         const char *name,
                         size_t len)
{
    int value = 0;
    size_t i;
    symtab_cell *cell, **prev_cell;

    for (i = 0; i < len; i++)
    {
        value = (value * 43) + (name[i] - 87);
    }
    if (value < 0) value = -value;

    cell = tab->cells[value % (tab->size)];
    prev_cell = &(tab->cells[value % (tab->size)]);
    for (; cell; prev_cell = &(cell->next), cell = cell->next)
        if (cell->len == len && strncmp(cell->name, name, len) == 0)
            return (cell);

    cell = tab->alloc_cell();
    cell->name = xmalloc(len + 1);

    strncpy(cell->name, name, len);
    cell->len = len;
    cell->next = NULL;

    *prev_cell = cell;

    return (cell);
}

void symtab_destroy_symtab(symtab *tab, void (*free_cell)(symtab_cell *))
{
    for (; tab->size--;)
    {
        symtab_cell *cell = tab->cells[tab->size];
        symtab_cell *next_cell = NULL;

        for (; cell; cell = next_cell)
        {
            next_cell = cell->next;
            free_cell(cell);
        }
    }

    free(tab->cells);
    free(tab);
}
    
    
