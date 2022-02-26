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

static fixupS *fixup_new_internal(fragS *frag,
                                  unsigned long where,
                                  int size,
                                  symbolS *add_symbol,
                                  long add_number,
                                  int pcrel)
{
    fixupS *fixup = xmalloc(sizeof(*fixup));

    memset(fixup, '\0', sizeof(*fixup));

    fixup->frag = frag;
    fixup->where = where;
    fixup->size = size;
    fixup->add_symbol = add_symbol;
    fixup->add_number = add_number;
    fixup->pcrel = pcrel;

    fixup->next = NULL;

    /* Adds the fixup to the current fixup chain. */
    if (current_frag_chain->last_fixup)
    {
        current_frag_chain->last_fixup->next = fixup;
        current_frag_chain->last_fixup = fixup;
    }
    else
    {
        (current_frag_chain->last_fixup
         = current_frag_chain->first_fixup = fixup);
    }

    return (fixup);
}

fixupS *fixup_new(fragS *frag,
                  unsigned long where,
                  int size,
                  symbolS *add_symbol,
                  unsigned long add_number,
                  int pcrel)
{  
    return (fixup_new_internal(frag, where, size, add_symbol, add_number,
                               pcrel));
}

fixupS *fixup_new_expr(fragS *frag,
                       unsigned long where,
                       int size,
                       exprS *expr,
                       int pcrel)
{
    symbolS *add_symbol = NULL;
    long add_number = 0;

    switch (expr->type)
    {
        case Expr_type_constant:
            add_number = expr->add_number;
            break;

        case Expr_type_symbol:
            add_symbol = expr->add_symbol;
            break;

        default:
            as_error("+++fixup_new_expr UNIMPLEMENTED!\n");
            break;
    }
    
    return (fixup_new_internal(frag, where, size, add_symbol, add_number,
                               pcrel));
}

static unsigned long relax_align(unsigned long address,
                                 unsigned long alignment)
{
    unsigned long mask, new_address;

    mask = ~((~0) << alignment);
    new_address = (address + mask) & (~mask);

    return (new_address - address);
}

static void relax_section(sectionT section)
{
    fragS *root_frag, *frag;
    unsigned long address;

    section_set(section);
    root_frag = current_frag_chain->first_frag;
    
    /* Initial calculation of frag addresses. */
    address = 0;
    for (frag = root_frag; frag; frag = frag->next)
    {
        frag->address = address;
        address += frag->fixed_size;
        
        switch (frag->relax_type)
        {
            case relax_type_none_needed:
                /* Nothing needs to be done. */
                break;

            case relax_type_align:
            case relax_type_align_code:
                address += relax_align(address, frag->offset);
                break;
            
            case relax_type_machine_dependent:
                address += machine_dependent_estimate_size_before_relax(frag,
                                                                        section);
                break;

            default:
                as_error("+++relax_section\n");
                break;
        }
    }
    
    /* The relaxation itself. */
    {
        long change;
        int changed;
        
        do {
            change = 0;
            changed = 0;

            for (frag = root_frag; frag; frag = frag->next)
            {
                long growth = 0;
                unsigned long old_address;

                old_address = frag->address;
                frag->address += change;
                
                switch (frag->relax_type)
                {
                    case relax_type_none_needed:
                        /* Nothing needs is done. */
                        growth = 0;
                        break;

                    case relax_type_align:
                    case relax_type_align_code:
                    {
                        unsigned long old_offset, new_offset;

                        old_offset = relax_align((old_address
                                                  + frag->fixed_size),
                                                 frag->offset);
                        new_offset = relax_align((frag->address
                                                  + frag->fixed_size),
                                                 frag->offset);
                        growth = new_offset - old_offset;

                        break;
                    }
                    
                    case relax_type_machine_dependent:
                        growth = machine_dependent_relax_frag(frag,
                                                              section,
                                                              change);
                        break;

                    default:
                        as_error("+++relax_section\n");
                        break;
                }

                if (growth)
                {
                    change += growth;
                    changed = 1;
                }
            }
            
        } while (changed);
    }
}

static void finish_frags_after_relaxation(sectionT section)
{
    fragS *root_frag, *frag;

    section_set(section);
    root_frag = current_frag_chain->first_frag;

    for (frag = root_frag; frag; frag = frag->next)
    {
        switch (frag->relax_type)
        {
            case relax_type_none_needed:
                /* Nothing needs to be done. */
                break;

            case relax_type_align:
            {
                unsigned long i;
                
                frag->offset = (frag->next->address
                                - (frag->address + frag->fixed_size));

                for (i = 0; i < frag->offset; i++)
                {
                    frag->buf[frag->fixed_size++] = 0x00;
                }

                break;
            }

            case relax_type_align_code:
            {
                unsigned long i;
                
                frag->offset = (frag->next->address
                                - (frag->address + frag->fixed_size));

                for (i = 0; i < frag->offset; i++)
                {
                    frag->buf[frag->fixed_size++] = 0x90;
                }
                break;
            }
            
            case relax_type_machine_dependent:
                machine_dependent_finish_frag(frag);
                break;

            default:
                as_error("+++finish_frags_after_relaxation\n");
                break;
        }
    }
}

/* Replaces local symbols in fixups with section symbols. */
static void adjust_reloc_symbols_of_section(sectionT section)
{
    fixupS *fixup;

    section_set(section);
    for (fixup = current_frag_chain->first_fixup;
         fixup;
         fixup = fixup->next)
    {
        if (fixup->done) continue;

        if (fixup->add_symbol)
        {
            symbolS *symbol = fixup->add_symbol;
            
            if (symbol_force_reloc(symbol)) continue;

            fixup->add_number += symbol_get_value(symbol);
            
            fixup->add_symbol = section_symbol(symbol_get_section(symbol));
        }
    }
}


/* Result is how many relocations are needed. */
static unsigned long fixup_section(sectionT section)
{
    fixupS *fixup;
    unsigned long section_reloc_count = 0;
    unsigned long add_number;
    sectionT add_symbol_section;

    section_set(section);
    for (fixup = current_frag_chain->first_fixup;
         fixup;
         fixup = fixup->next)
    {
        add_number = fixup->add_number;

        if (fixup->add_symbol)
        {
            add_symbol_section = symbol_get_section(fixup->add_symbol);

            if ((add_symbol_section == section)
                && !MACHINE_DEPENDENT_FORCE_RELOCATION_LOCAL(fixup))
            {
                add_number += symbol_get_value(fixup->add_symbol);
                fixup->add_number = add_number;
                if (fixup->pcrel)
                {
                    add_number -= machine_dependent_pcrel_from(fixup);
                    fixup->pcrel = 0;
                }
                fixup->add_symbol = NULL;
            }
        }

        if (fixup->pcrel)
        {
            add_number -= machine_dependent_pcrel_from(fixup);
        }
        
        machine_dependent_apply_fixup(fixup, add_number);
        
        if (fixup->done == 0)
        {
            section_reloc_count++;
        }
    }

    return (section_reloc_count);
}

void write_object_file(void)
{
    relax_section(text_section);
    relax_section(data_section);

    finish_frags_after_relaxation(text_section);
    finish_frags_after_relaxation(data_section);

#ifdef TARGET_OBJECT_FILE_A_OUT
    {
        unsigned long text_section_size, address;
        fragS *frag;

        section_set(text_section);
        text_section_size = (current_frag_chain->last_frag->address
                             + current_frag_chain->last_frag->fixed_size);

        section_set(data_section);
        address = text_section_size;
        for (frag = current_frag_chain->first_frag;
             frag;
             frag = frag->next)
        {
            frag->address = address;
            address += frag->fixed_size;
        }
    }
#endif

    adjust_reloc_symbols_of_section(text_section);
    adjust_reloc_symbols_of_section(data_section);

    fixup_section(text_section);
    fixup_section(data_section);
    
    object_dependent_write_object_file();
}
