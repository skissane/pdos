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

fragS *current_frag;

fragS zero_address_frag = {0};

fragS *frag_alloc(void)
{
    fragS *frag = xmalloc(sizeof(*frag));

    memset(frag, '\0', sizeof(*frag));

    return (frag);
}

void frag_new(void)
{
    fragS *prev_frag = current_frag;

    current_frag = frag_alloc();

    prev_frag->next = current_frag;
    current_frag_chain->last_frag = current_frag;
}

void frag_set_as_variant(relax_typeT relax_type,
                         relax_subtypeT relax_subtype,
                         symbolS *symbol,
                         unsigned long offset,
                         unsigned long opcode_offset_in_buf)
{
    current_frag->relax_type = relax_type;
    current_frag->relax_subtype = relax_subtype;
    current_frag->symbol = symbol;
    current_frag->offset = offset;
    current_frag->opcode_offset_in_buf = opcode_offset_in_buf;

    frag_new();
}

void frag_align(unsigned long alignment)
{
    frag_alloc_space(1 << alignment);
    frag_set_as_variant(relax_type_align, 0, NULL, alignment, 0);
}

void frag_align_code(unsigned long alignment)
{
    frag_alloc_space(1 << alignment);
    frag_set_as_variant(relax_type_align_code, 0, NULL, alignment, 0);
}

void frag_alloc_space(unsigned long space)
{
    if (current_frag->fixed_size + space >= current_frag->size)
    {
        current_frag->size += ((space > FRAG_BUF_REALLOC_STEP)
                               ? space
                               : FRAG_BUF_REALLOC_STEP);
        current_frag->buf = xrealloc(current_frag->buf, current_frag->size);
    }
}

void frag_increase_fixed_size(unsigned long increase)
{
    frag_alloc_space(increase);
    
    current_frag->fixed_size += increase;
}

void frag_append_1_char(unsigned char c)
{
    if (current_frag->fixed_size == current_frag->size)
    {
        current_frag->size += FRAG_BUF_REALLOC_STEP;
        current_frag->buf = xrealloc(current_frag->buf, current_frag->size);
    }

    current_frag->buf[current_frag->fixed_size++] = c;
}
