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
#include "xmalloc.h"

symbolS *symbols = NULL;

/* This is system is used so symbols are
 * in the order they are added and not in the reverse order. */
static symbolS **pointer_to_pointer_to_next_symbol = &symbols;
void symbol_add_to_chain(symbolS *symbol)
{
    *pointer_to_pointer_to_next_symbol = symbol;
    pointer_to_pointer_to_next_symbol = &(symbol->next);
}

symbolS *symbol_find(const char *name)
{
    symbolS *symbol;

    for (symbol = symbols;
         symbol && strcmp(symbol->name, name);
         symbol = symbol->next) ;

    return (symbol);
}

symbolS *symbol_create(const char *name,
                       sectionT section,
                       unsigned long value,
                       fragS *frag)
{
    symbolS *symbol = xmalloc(sizeof(*symbol));

    memset(symbol, '\0', sizeof(*symbol));

    symbol->name = xstrdup(name);
    symbol->section = section;
    symbol->value = value;
    symbol->frag = frag;

    return (symbol);
}

symbolS *symbol_make(const char *name)
{
    return (symbol_create(name, undefined_section, 0, &zero_address_frag));
}

symbolS *symbol_find_or_make(const char *name)
{
    symbolS *symbol;

    symbol = symbol_find(name);

    if (symbol == NULL)
    {
        symbol = symbol_make(name);
        symbol_add_to_chain(symbol);
    }

    return (symbol);
}

symbolS *symbol_label(const char *name)
{
    symbolS *symbol;

    if ((symbol = symbol_find(name)))
    {
        if (symbol->section == undefined_section)
        {
            symbol->section = current_section;
            symbol->value = current_frag->fixed_size;
            symbol->frag = current_frag;
        }
        else
        {
            printf("Symbol `%s' is already defined!\n", name);
        }
    }
    else
    {
        symbol = symbol_create(name, current_section,
                               current_frag->fixed_size, current_frag);
        symbol_add_to_chain(symbol);
    }

    return (symbol);
}

void symbol_set_external(symbolS *symbol)
{
    symbol->flags |= SYMBOL_FLAG_EXTERNAL;
}

sectionT symbol_get_section(symbolS *symbol)
{
    return (symbol->section);
}

unsigned long symbol_get_value(symbolS *symbol)
{
    return (symbol->frag->address + symbol->value);
}

unsigned long symbol_get_symbol_table_index(symbolS *symbol)
{
    return (symbol->symbol_table_index);
}

void symbol_set_symbol_table_index(symbolS *symbol, unsigned long index)
{
    symbol->symbol_table_index = index;
}

int symbol_is_external(symbolS *symbol)
{
    return (!!(symbol->flags & SYMBOL_FLAG_EXTERNAL));
}

int symbol_is_undefined(symbolS *symbol)
{
    return (symbol->section == undefined_section);
}

int symbol_is_section_symbol(symbolS *symbol)
{
    return (symbol->flags & SYMBOL_FLAG_SECTION_SYMBOL);
}

int symbol_force_reloc(symbolS *symbol)
{
    return ((symbol->section == undefined_section));
}
