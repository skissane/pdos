/******************************************************************************
 * @file            frags.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
struct frag {

    address_t address;
    
    value_t fixed_size;
    value_t size;
    
    unsigned char *buf;
    
    relax_type_t relax_type;
    relax_subtype_t relax_subtype;
    
    struct symbol *symbol;
    
    offset_t offset;
    value_t opcode_offset_in_buf;
    
    /* Filename and line number where the frag became variant. */
    const char *filename;
    unsigned long line_number;
    
    int relax_marker;
    struct frag *next;

};

#define     FRAG_BUF_REALLOC_STEP       16

extern struct frag zero_address_frag;
extern struct frag *current_frag;

struct frag *frag_alloc (void);

int frags_offset_is_fixed (const struct frag *frag1, const struct frag *frag2, offset_t *offset_p);
int frags_is_greater_than_offset (value_t offset2, const struct frag *frag2, value_t offset1, const struct frag *frag1, offset_t *offset_p);

unsigned char *frag_alloc_space (value_t space);
unsigned char *frag_increase_fixed_size (value_t increase);

unsigned char *finished_frag_increase_fixed_size_by_frag_offset (struct frag *frag);

void frag_align (offset_t alignment, int fill_char, offset_t max_bytes_to_skip);
void frag_align_code (offset_t alignment, offset_t max_bytes_to_skip);
void frag_append_1_char (unsigned char ch);
void frag_new (void);
void frag_destroy (struct frag *frag);
void frag_set_as_variant (relax_type_t relax_type, relax_subtype_t relax_subtype, struct symbol *symbol, offset_t offset, value_t opcode_offset_in_buf);
