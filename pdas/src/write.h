/******************************************************************************
 * @file            write.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
struct fixup {

    struct frag *frag;
    
    unsigned long where;
    unsigned int size;
    
    struct symbol *add_symbol;
    struct symbol *sub_symbol;
    offset_t add_number;
    
    int pcrel;
    int done;
    
    reloc_type_t reloc_type;
    struct fixup *next;

};

struct fixup *fixup_new (struct frag *frag, unsigned long where, int size, struct symbol *add_symbol, offset_t add_number, int pcrel, reloc_type_t reloc_type);
struct fixup *fixup_new_expr (struct frag *frag, unsigned long where, int size, struct expr *expr, int pcrel, reloc_type_t reloc_type);

void write_object_file (void);
