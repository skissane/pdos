/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

typedef struct symbol {
    char *name;
    sectionT section;
    unsigned long value;
    /* Attached code fragment. */
    struct frag *frag;

    unsigned long flags;
#define SYMBOL_FLAG_EXTERNAL 0x1
#define SYMBOL_FLAG_SECTION_SYMBOL 0x2
    
    struct symbol *next;

    /* The following fields are only used when writing the object file. */
    unsigned long symbol_table_index;
    int write_name_to_string_table;
} symbolS;

extern symbolS *symbols;

void symbol_add_to_chain(symbolS *symbol);
symbolS *symbol_create(const char *name,
                       sectionT section,
                       unsigned long value,
                       struct frag *frag);
symbolS *symbol_find_or_make(const char *name);
symbolS *symbol_label(const char *name);

void symbol_set_external(symbolS *symbol);
sectionT symbol_get_section(symbolS *symbol);
unsigned long symbol_get_value(symbolS *symbol);
void symbol_set_value(symbolS *symbol, unsigned long value);
unsigned long symbol_get_symbol_table_index(symbolS *symbol);
void symbol_set_symbol_table_index(symbolS *symbol, unsigned long index);

int symbol_is_external(symbolS *symbol);
int symbol_is_undefined(symbolS *symbol);
int symbol_is_section_symbol(symbolS *symbol);

int symbol_force_reloc(symbolS *symbol);
