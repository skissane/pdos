/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include <stddef.h>

typedef struct symtab_cell {
    char *name;
    size_t len;
    struct symtab_cell *next;
} symtab_cell;

typedef struct {
    symtab_cell **cells;
    int size;

    symtab_cell *(*alloc_cell)(void);
} symtab;

symtab *symtab_create_symtab(size_t size,
                             symtab_cell *(*alloc_cell)(void));
void symtab_destroy_symtab(symtab *tab, void (*free_cell)(symtab_cell *));
symtab_cell *symtab_find(symtab *tab,
                         const char *name,
                         size_t len);

#endif /* SYMTAB_H */
