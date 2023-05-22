/******************************************************************************
 * @file            frags.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "as.h"

struct frag zero_address_frag = { 0 };
struct frag *current_frag;

struct frag *frag_alloc (void)
{
    struct frag *frag = xmalloc (sizeof (*frag));
    
    memset (frag, 0, sizeof (*frag));
    
    return frag;
}

/**
 * Returns 1 if the addresses of frags are directly proportional.
 * *offset_p is set to the difference in addresses not counted in frag->address.
 */
int frags_offset_is_fixed (const struct frag *frag1, const struct frag *frag2, offset_t *offset_p) {

    const struct frag *frag;
    offset_t offset;
    
    offset = frag1->address - frag2->address;
    
    if (frag1 == frag2) {
    
        *offset_p = offset;
        return 1;
    
    }
    
    /* Checks if frag2 is after frag1. */
    frag = frag1;
    
    while (frag->relax_type == RELAX_TYPE_NONE_NEEDED) {
    
        offset += frag->fixed_size;
        frag = frag->next;
        
        if (frag == NULL) {
            break;
        }
        
        if (frag == frag2) {
        
            *offset_p = offset;
            return 1;
        
        }
    
    }
    
    /* Checks if frag1 is after frag2. */
    offset = frag1->address - frag2->address;
    frag = frag2;
    
    while (frag->relax_type == RELAX_TYPE_NONE_NEEDED) {
    
        offset -= frag->fixed_size;
        frag = frag->next;
        
        if (frag == NULL) {
            break;
        }
        
        if (frag == frag1) {
        
            *offset_p = offset;
            return 1;
        
        }
    
    }
    
    return 0;

}

/**
 * Returns 1 if it is possible to determine whether frag2 left_offset
 * is after (strict >) frag1 right_offset at the current time.
 * *offset_p is set to the negated difference in frag addresses.
 *
 * This is a fallback for frags_offset_is_fixed().
 */
int frags_is_greater_than_offset (value_t offset2, const struct frag *frag2, value_t offset1, const struct frag *frag1, offset_t *offset_p) {

    offset_t difference;
    const struct frag *frag;
    
    /* Checks for something that should be impossible. */
    if (frag2 == frag1 || offset1 > frag1->fixed_size) {
        return 0;
    }
    
    difference = offset2 - offset1;
    
    for (frag = frag1;;) {
    
        difference += frag->fixed_size;
        frag = frag->next;
        
        if (frag == frag2) {
        
            if (difference == 0) {
                return 0;
            }
            
            break;
        
        }
        
        if (frag == NULL) {
            return 0;
        }
    
    }
    
    *offset_p = offset2 - offset1 - difference;
    return 1;

}

unsigned char *frag_alloc_space (value_t space) {

    if (current_frag->fixed_size + space >= current_frag->size) {
    
        current_frag->size += ((space > FRAG_BUF_REALLOC_STEP) ? space : FRAG_BUF_REALLOC_STEP);
        current_frag->buf   = xrealloc (current_frag->buf, current_frag->size);
    
    }
    
    return current_frag->buf + current_frag->fixed_size;

}

unsigned char *frag_increase_fixed_size (value_t increase) {

    frag_alloc_space (increase);
    current_frag->fixed_size += increase;
    
    return (current_frag->buf + current_frag->fixed_size - increase);

}

unsigned char *finished_frag_increase_fixed_size_by_frag_offset (struct frag *frag) {

    frag->fixed_size += frag->offset;

    if (frag->fixed_size > frag->size) {
        
        frag->buf = xrealloc (frag->buf, frag->fixed_size);
        frag->size = frag->fixed_size;

    }

    return (frag->buf + frag->fixed_size - frag->offset);
}

void frag_align (offset_t alignment, int fill_char, offset_t max_bytes_to_skip) {

    (frag_alloc_space (1 << alignment))[0] = fill_char;
    frag_set_as_variant (RELAX_TYPE_ALIGN, (relax_subtype_t) max_bytes_to_skip, NULL, alignment, 0);

}

void frag_align_code (offset_t alignment, offset_t max_bytes_to_skip) {

    (frag_alloc_space (1 << alignment))[0] = 0x90;
    frag_set_as_variant (RELAX_TYPE_ALIGN_CODE, (relax_subtype_t) max_bytes_to_skip, NULL, alignment, 0);

}

void frag_append_1_char (unsigned char ch) {

    if (current_frag->fixed_size == current_frag->size) {
    
        current_frag->size += FRAG_BUF_REALLOC_STEP;
        current_frag->buf   = xrealloc (current_frag->buf, current_frag->size);
    
    }
    
    current_frag->buf[current_frag->fixed_size++] = ch;

}

void frag_new (void) {

    struct frag *prev_frag = current_frag;
    
    current_frag = frag_alloc ();
    current_frag->relax_type = RELAX_TYPE_NONE_NEEDED;
    current_frag->next = NULL;
    
    prev_frag->next = current_frag;
    current_frag_chain->last_frag = current_frag;

}

void frag_destroy (struct frag *frag)
{
    if (frag == NULL) return;

    free (frag->buf);
    free (frag);
}

void frag_set_as_variant (relax_type_t relax_type, relax_subtype_t relax_subtype, struct symbol *symbol, offset_t offset, value_t opcode_offset_in_buf) {

    current_frag->relax_type           = relax_type;
    current_frag->relax_subtype        = relax_subtype;
    current_frag->symbol               = symbol;
    current_frag->offset               = offset;
    current_frag->opcode_offset_in_buf = opcode_offset_in_buf;
    
    get_filename_and_line_number (&(current_frag->filename), &(current_frag->line_number));
    frag_new ();

}
