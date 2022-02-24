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

struct section {
    symbolS *symbol;

    frag_chainS frag_chain;
};

static struct section sections[4];
static symbolS section_symbols[1];

sectionT undefined_section;
sectionT text_section;
sectionT data_section;
sectionT bss_section;

sectionT current_section;
frag_chainS *current_frag_chain;

void sections_init(void)
{
    undefined_section = &sections[0];
    text_section = &sections[1];
    data_section = &sections[2];
    bss_section = &sections[3];

    undefined_section->symbol = &section_symbols[1];
    {
        undefined_section->symbol->name = ".undef";
        undefined_section->symbol->section = undefined_section;
        undefined_section->symbol->value = 0;
        
        undefined_section->symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
    }

    text_section->symbol = symbol_create(".text", text_section,
                                         0, &zero_address_frag);
    text_section->symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
    symbol_add_to_chain(text_section->symbol);

    (text_section->frag_chain.last_frag
     = text_section->frag_chain.first_frag = frag_alloc());
    (text_section->frag_chain.last_fixup
     = text_section->frag_chain.first_fixup = NULL);

    data_section->symbol = symbol_create(".data", data_section,
                                         0, &zero_address_frag);
    data_section->symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
    symbol_add_to_chain(data_section->symbol);
    
    (data_section->frag_chain.last_frag
     = data_section->frag_chain.first_frag = frag_alloc());
    (data_section->frag_chain.last_fixup
     = data_section->frag_chain.first_fixup = NULL);

    bss_section->symbol = symbol_create(".bss", bss_section,
                                        0, &zero_address_frag);
    bss_section->symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
    symbol_add_to_chain(bss_section->symbol);
    
    (bss_section->frag_chain.last_frag
     = bss_section->frag_chain.first_frag = frag_alloc());
    (bss_section->frag_chain.last_fixup
     = bss_section->frag_chain.first_fixup = NULL);
}

void section_set(sectionT section)
{
    current_section = section;
    current_frag_chain = &(current_section->frag_chain);
    current_frag = current_frag_chain->last_frag;
}

symbolS *section_symbol(sectionT section)
{
    return (section->symbol);
}
