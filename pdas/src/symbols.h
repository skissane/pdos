/******************************************************************************
 * @file            symbols.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
struct symbol {

    struct symbol *next_to_free;

    char *name;
    
    struct expr value;
    section_t section;
    
    struct frag *frag;
    unsigned long flags;
    
#define     SYMBOL_FLAG_EXTERNAL                            0x01
#define     SYMBOL_FLAG_SECTION_SYMBOL                      0x02
    
    struct symbol *next;
    
    unsigned long symbol_table_index;
    int write_name_to_string_table;
    
    int resolving;
    int resolved;

    void *object_format_dependent_data;

};

extern struct symbol *symbols;
extern int finalize_symbols;

void symbols_destroy (void);

struct expr *symbol_get_value_expression (struct symbol *symbol);
struct frag *symbol_get_frag (struct symbol *symbol);

struct symbol *symbol_create (const char *name, section_t section, unsigned long value, struct frag *frag);
struct symbol *symbol_find (const char *name);
struct symbol *symbol_find_or_make (const char *name);
struct symbol *symbol_label (const char *name);
struct symbol *symbol_make (const char *name);
struct symbol *symbol_temp_new_now (void);
struct symbol *symbol_temp_make (void);

section_t symbol_get_section (struct symbol *symbol);
char *symbol_get_name (struct symbol *symbol);

int get_symbol_snapshot (struct symbol **symbol_p, value_t *value_p, section_t *section_p, struct frag **frag_p);

int symbol_force_reloc (struct symbol *symbol);
int symbol_is_external (struct symbol *symbol);
int symbol_is_resolved (struct symbol *symbol);
int symbol_is_section_symbol (struct symbol *symbol);
int symbol_is_undefined (struct symbol *symbol);
int symbol_uses_other_symbol (struct symbol *symbol);
int symbol_uses_reloc_symbol (struct symbol *symbol);

unsigned long symbol_get_symbol_table_index (struct symbol *symbol);

value_t symbol_get_value (struct symbol *symbol);
value_t symbol_resolve_value (struct symbol *symbol);

void symbol_add_to_chain (struct symbol *symbol);
void symbol_set_external (struct symbol *symbol);
void symbol_set_frag (struct symbol *symbol, struct frag *frag);
void symbol_set_section (struct symbol *symbol, section_t section);
void symbol_set_symbol_table_index (struct symbol *symbol, unsigned long index);
void symbol_set_value (struct symbol *symbol, value_t value);
void symbol_set_value_expression (struct symbol *symbol, struct expr *expr);
void symbol_set_value_now (struct symbol *symbol);
