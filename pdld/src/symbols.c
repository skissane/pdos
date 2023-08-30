/******************************************************************************
 * @file            symbols.c
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

#include "ld.h"
#include "xmalloc.h"
#include "hashtab.h"

static struct hashtab *symbol_hashtab;

static hash_value_t hash_symbol (const void *p)
{
    const struct symbol *symbol = (const struct symbol *) p;
    return hashtab_help_default_hash_string (symbol->name);
}

static int equal_symbol (const void *p1, const void *p2)
{
    const struct symbol *symbol1 = (const struct symbol *) p1;
    const struct symbol *symbol2 = (const struct symbol *) p2;
    
    return strcmp (symbol1->name, symbol2->name) == 0;
}

static struct symbol *find_symbol (const char *name)
{
    struct symbol fake = { NULL };
    fake.name = (char *)name;
    
    return (struct symbol *) hashtab_find (symbol_hashtab, &fake);
}

void symbols_init (void)
{
    symbol_hashtab = hashtab_create_hashtab (0, hash_symbol, equal_symbol, &xmalloc, &free);
}

void symbols_destroy (void)
{
    hashtab_destroy_hashtab (symbol_hashtab);
}

struct symbol *symbol_find (const char *name)
{
    return find_symbol (name);
}

void symbol_add_to_hashtab (struct symbol *symbol)
{
    if (hashtab_insert (symbol_hashtab, symbol)) {
        ld_internal_error_at_source (__FILE__, __LINE__, "failed to insert symbol '%s' into hashtab", symbol->name);
    }
}

void symbol_remove_from_hashtab (struct symbol *symbol)
{
    hashtab_delete (symbol_hashtab, symbol);
}

void symbol_record_external_symbol (struct symbol *symbol)
{
    struct symbol *old_symbol = symbol_find (symbol->name);
    
    if (old_symbol == NULL) {
        symbol_add_to_hashtab (symbol);
        return;
    }

    if (symbol_is_undefined (old_symbol)) {
        symbol_remove_from_hashtab (old_symbol);
        symbol_add_to_hashtab (symbol);
        return;
    }

    if (symbol_is_undefined (symbol)) return;

    ld_error ("%s:(%s%s0x%lx): multiple definition of '%s'",
              symbol->part->of->filename,
              symbol->part ? symbol->part->section->name : "",
              symbol->part ? "+" : "",
              symbol->value,
              symbol->name);
    ld_note ("%s:(%s%s0x%lx): first defined here",
             old_symbol->part->of->filename,
             old_symbol->part ? old_symbol->part->section->name : "",
             old_symbol->part ? "+" : "",
             old_symbol->value);
}
        
int symbol_is_undefined (const struct symbol *symbol)
{
    return (symbol->section_number == UNDEFINED_SECTION_NUMBER);
}

address_type symbol_get_value_with_base (struct symbol *symbol)
{
    if (symbol->part) return ld_state->base_address + symbol->part->rva + symbol->value;
    else return symbol->value;
}

address_type symbol_get_value_no_base (struct symbol *symbol)
{
    if (symbol->part) return symbol->part->rva + symbol->value;
    else return symbol->value - ld_state->base_address;
}

void symbols_for_each_global (void (*symbol_callback) (struct symbol *))
{
    hashtab_for_each_element (symbol_hashtab, (void (*) (void *))symbol_callback);
}
