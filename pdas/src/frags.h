/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

/* How large steps are used when increasing frag buffer size.
 * +++Determine the optimal value. */
#define FRAG_BUF_REALLOC_STEP 16

/* A code fragment consisting of a known amount of chars,
 * followed by an unknown amount of chars.
 * Maximum possible amount of chars is always known
 * and at least that much space is reserved. */
typedef struct frag {
    /* Address in section in object file. */
    unsigned long address;
    
    /* The known amount of chars.
     * Fixed during relaxation. */
    unsigned long fixed_size;
    /* Current size of buf. */
    unsigned long size;
    /* Buffer to hold the fragment's content. */
    unsigned char *buf;

    relax_typeT relax_type;
    relax_subtypeT relax_subtype;
    symbolS *symbol;
    unsigned long offset;
    unsigned long opcode_offset_in_buf;
    
    struct frag *next;
} fragS;

extern fragS *current_frag;

extern fragS zero_address_frag;

fragS *frag_alloc(void);
void frag_new(void);

void frag_set_as_variant(relax_typeT relax_type,
                         relax_subtypeT relax_subtype,
                         symbolS *symbol,
                         unsigned long offset,
                         unsigned long opcode_offset_in_buf);

void frag_align(unsigned long alignment);
void frag_align_code(unsigned long alignment);

void frag_alloc_space(unsigned long space);
void frag_increase_fixed_size(unsigned long increase);

void frag_append_1_char(unsigned char c);
