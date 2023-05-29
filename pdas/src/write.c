/******************************************************************************
 * @file            write.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include    <stddef.h>

#include    "as.h"
#include    "cfi.h"

static struct fixup *fixup_new_internal (struct frag *frag, unsigned long where, int size, struct symbol *add_symbol, long add_number, int pcrel, reloc_type_t reloc_type) {

    struct fixup *fixup = xmalloc (sizeof (*fixup));
    
    fixup->frag         = frag;
    fixup->where        = where;
    fixup->size         = size;
    fixup->add_symbol   = add_symbol;
    fixup->add_number   = add_number;
    fixup->pcrel        = pcrel;
    fixup->done         = 0;
    fixup->reloc_type   = reloc_type;
    fixup->next         = NULL;
    
    if (current_frag_chain->last_fixup) {
    
        current_frag_chain->last_fixup->next = fixup;
        current_frag_chain->last_fixup = fixup;
    
    } else {
        current_frag_chain->last_fixup = current_frag_chain->first_fixup = fixup;
    }
    
    return fixup;

}

static unsigned long relax_align (unsigned long address, unsigned long alignment) {

    unsigned long mask, new_address;
    
    mask = ~(~((unsigned int) 0) << alignment);
    new_address = (address + mask) & ~mask;
    
    return new_address - address;

}

static void relax_section (section_t section)
{
    struct frag *root_frag, *frag;
    unsigned long address, frag_count, max_iterations;
    unsigned long alignment_needed;
    
    int changed;
    
    section_set (section);
    
    root_frag = current_frag_chain->first_frag;
    address   = 0;
    
    for (frag_count = 0, frag = root_frag; frag; frag_count++, frag = frag->next) {
    
        frag->relax_marker  = 0;
        frag->address       = address;
        
        address += frag->fixed_size;
        
        switch (frag->relax_type) {
        
            case RELAX_TYPE_NONE_NEEDED:
            
                break;
            
            case RELAX_TYPE_ALIGN:
            case RELAX_TYPE_ALIGN_CODE:
            
                alignment_needed = relax_align (address, frag->offset);

                if (frag->relax_subtype != 0 && alignment_needed > frag->relax_subtype) {
                    alignment_needed = 0;
                }

                address += alignment_needed;
                break;
            
            case RELAX_TYPE_ORG:
            case RELAX_TYPE_SPACE:
            
                break;

            case RELAX_TYPE_CFI:

                address += cfi_estimate_size_before_relax (frag);
                break;
            
            case RELAX_TYPE_MACHINE_DEPENDENT:
            
                address += machine_dependent_estimate_size_before_relax (frag, section);
                break;
            
            default:
            
                as_internal_error_at_source_at (__FILE__, __LINE__, frag->filename, frag->line_number,
                                                "invalid relax type");
                break;
        
        }
    
    }
    
    /**
     * Prevents an infinite loop caused by frag growing because of a symbol that moves when the frag grows.
     *
     * Example:
     *
     *      .org _abc + 2
     *      _abc:
     */
    max_iterations = frag_count * frag_count;
    
    /* Too many frags might cause an overflow. */
    if (max_iterations < frag_count) {
        max_iterations = frag_count;
    }
    
    do {
        long change = 0;
        changed = 0;
        
        for (frag = root_frag; frag; frag = frag->next) {
        
            long growth = 0;
            unsigned long old_address;
            
            unsigned long old_offset;
            unsigned long new_offset;
            
            frag->relax_marker = !frag->relax_marker;
            
            old_address = frag->address;
            frag->address += change;
            
            switch (frag->relax_type) {
            
                case RELAX_TYPE_NONE_NEEDED:
                
                    growth = 0;
                    break;
                
                case RELAX_TYPE_ALIGN:
                case RELAX_TYPE_ALIGN_CODE:
                
                    old_offset = relax_align (old_address + frag->fixed_size, frag->offset);
                    new_offset = relax_align (frag->address + frag->fixed_size, frag->offset);
                    
                    if (frag->relax_subtype != 0) {

                        if (old_offset > frag->relax_subtype) {
                            old_offset = 0;
                        }
                        if (new_offset > frag->relax_subtype) {
                            new_offset = 0;
                        }
                    }

                        
                    growth = new_offset - old_offset;
                    break;
                
                case RELAX_TYPE_ORG: {
                
                    unsigned long target = frag->offset;
                    
                    if (frag->symbol) {
                        target += symbol_get_value (frag->symbol);
                    }
                    
                    growth = target - (frag->next->address + change);
                    
                    if (frag->address + frag->fixed_size > target) {
                    
                        as_error_at (frag->filename, frag->line_number, "attempt to move .org backwards");
                        growth = 0;
                        
                        /* Changes the frag so no more errors appear because of it. */
                        frag->relax_type = RELAX_TYPE_ALIGN;
                        frag->offset = 0;
                        frag->fixed_size = frag->next->address + change - frag->address;
                    
                    }
                    
                    break;
                
                }
                
                case RELAX_TYPE_SPACE:
                
                    growth = 0;
                    
                    if (frag->symbol) {
                    
                        long amount = symbol_get_value (frag->symbol);
                        
                        if (symbol_get_section (frag->symbol) != absolute_section || symbol_is_undefined (frag->symbol)) {
                        
                            as_error_at (frag->filename, frag->line_number, ".space specifies non-absolute value");
                            
                            /* Prevents the error from repeating. */
                            frag->symbol = NULL;
                        
                        } else if (amount < 0) {
                        
                            as_warn_at (frag->filename, frag->line_number, ".space with negative value, ignoring");
                            frag->symbol = NULL;
                        
                        } else {
                            growth = old_address + frag->fixed_size + amount - frag->next->address;
                        }
                    
                    }
                    
                    break;

                case RELAX_TYPE_CFI:

                    growth = cfi_relax_frag (frag);
                    break;
                
                case RELAX_TYPE_MACHINE_DEPENDENT:
                
                    growth = machine_dependent_relax_frag (frag, section, change);
                    break;
                
                default:
                
                    as_internal_error_at_source_at (__FILE__, __LINE__, frag->filename, frag->line_number,
                                                    "invalid relax type");
                    break;
            
            }
            
            if (growth) {
            
                change += growth;
                changed = 1;
            
            }
        
        }
    
    } while (changed && --max_iterations);
    
    if (changed) {
        as_fatal_error_at (NULL, 0,
                           "Infinite loop encountered whilst attempting to compute the addresses in section %s",
                           section_get_name (section));
    }
}

static void finish_frags_after_relaxation (section_t section) {

    struct frag *root_frag, *frag;
    
    section_set (section);
    root_frag = current_frag_chain->first_frag;
    
    for (frag = root_frag; frag; frag = frag->next) {
    
        switch (frag->relax_type) {
        
            case RELAX_TYPE_NONE_NEEDED:
            
                break;
            
            case RELAX_TYPE_ALIGN:
            case RELAX_TYPE_ALIGN_CODE:
            case RELAX_TYPE_ORG:
            case RELAX_TYPE_SPACE: {
            
                offset_t i;
                unsigned char *p;
                unsigned char fill;
                
                frag->offset = frag->next->address - (frag->address + frag->fixed_size);
                
                if (((long)(frag->offset)) < 0) {
                
                    as_error_at (frag->filename, frag->line_number, "attempt to .org/.space backward ("PRIiOFFSET")", frag->offset);
                    frag->offset = 0;
                
                }

                p = finished_frag_increase_fixed_size_by_frag_offset (frag);
                fill = *p;
                
                for (i = 0; i < frag->offset; i++) {
                
                    p[i] = fill;
                
                }
                
                break;
            
            }

            case RELAX_TYPE_CFI:

                cfi_finish_frag (frag);
                break;

            case RELAX_TYPE_MACHINE_DEPENDENT:
            
                machine_dependent_finish_frag (frag);
                break;
            
            default:
            
                as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "invalid relax type");
                break;
        
        }
    
    }

}

static void adjust_reloc_symbols_of_section (section_t section) {

    struct fixup *fixup;
    section_set (section);
    
    for (fixup = current_frag_chain->first_fixup; fixup; fixup = fixup->next) {
    
        if (fixup->done) { continue; }
        
        if (fixup->add_symbol) {
        
            struct symbol *symbol = fixup->add_symbol;
            
            /* Resolves symbols that have not been resolved yet (expression symbols). */
            symbol_resolve_value (symbol);
            
            if (symbol_uses_reloc_symbol (symbol)) {
            
                fixup->add_number += symbol_get_value_expression (symbol)->add_number;
                symbol = symbol_get_value_expression (symbol)->add_symbol;
                fixup->add_symbol = symbol;
            
            }
            
            if (symbol_force_reloc (symbol)) {
                continue;
            }
            
            if (symbol_get_section (symbol) == absolute_section) {
                continue;
            }
            
            fixup->add_number += symbol_get_value (symbol);
            fixup->add_symbol = section_symbol (symbol_get_section (symbol));
        
        }
    
    }

}

static unsigned long fixup_section (section_t section)
{
    struct fixup *fixup;
    section_t add_symbol_section;
    
    unsigned long section_reloc_count = 0;
    
    section_set (section);
    
    for (fixup = current_frag_chain->first_fixup; fixup; fixup = fixup->next) {
        unsigned long add_number;
    
        add_number = fixup->add_number;
        
        if (fixup->add_symbol) {
        
            add_symbol_section = symbol_get_section (fixup->add_symbol);
            
            if ((add_symbol_section == section) && !machine_dependent_force_relocation_local (fixup)) {
            
                add_number += symbol_get_value (fixup->add_symbol);
                fixup->add_number = add_number;
                
                if (fixup->pcrel) {
                
                    add_number -= machine_dependent_pcrel_from (fixup);
                    fixup->pcrel = 0;
                
                }
                
                fixup->add_symbol = NULL;
            
            } else if (add_symbol_section == absolute_section) {
            
                add_number += symbol_get_value (fixup->add_symbol);
                
                fixup->add_number = add_number;
                fixup->add_symbol = NULL;
            
            }
        
        }
        
        if (fixup->pcrel) {
            add_number -= machine_dependent_pcrel_from (fixup);
        }
        
        machine_dependent_apply_fixup (fixup, add_number);
        
        if (fixup->done == 0) {
            section_reloc_count++;
        }
    
    }
    
    return section_reloc_count;
}

struct fixup *fixup_new (struct frag *frag, unsigned long where, int size, struct symbol *add_symbol, long add_number, int pcrel, reloc_type_t reloc_type) {
    return fixup_new_internal (frag, where, size, add_symbol, add_number, pcrel, reloc_type);
}

struct fixup *fixup_new_expr (struct frag *frag, unsigned long where, int size, struct expr *expr, int pcrel, reloc_type_t reloc_type) {

    struct symbol *add_symbol = NULL;
    offset_t add_number = 0;
    
    switch (expr->type) {
    
        case EXPR_TYPE_ABSENT:
        
            break;
        
        case EXPR_TYPE_CONSTANT:
        
            add_number = expr->add_number;
            break;
        
        case EXPR_TYPE_SYMBOL:
        
            add_symbol = expr->add_symbol;
            add_number = expr->add_number;
            break;
        
        case EXPR_TYPE_SYMBOL_RVA:
        
            add_symbol = expr->add_symbol;
            add_number = expr->add_number;
            
            reloc_type = RELOC_TYPE_RVA;
            break;
        
        default:
        
            add_symbol = make_expr_symbol (expr);
            break;
        
    }
    
    return fixup_new_internal (frag, where, size, add_symbol, add_number, pcrel, reloc_type);

}

void write_object_file (void) {

    struct symbol *symbol;
    section_t section;
    
    sections_chain_subsection_frags ();
    
    for (section = sections; section; section = section_get_next_section (section)) {
        relax_section (section);
    }
    
    for (section = sections; section; section = section_get_next_section (section)) {
        finish_frags_after_relaxation (section);
    }
    
    if (state->format == AS_FORMAT_A_OUT) {
    
        struct frag *frag;
        unsigned long address, text_section_size;
        
        section_set (text_section);
        
        text_section_size = current_frag_chain->last_frag->address + current_frag_chain->last_frag->fixed_size;
        address = text_section_size;
        
        section_set (data_section);
        
        for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
        
            frag->address = address;
            address += frag->fixed_size;
        
        }
        
        section_set (bss_section);
        
        for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
        
            frag->address = address;
            address += frag->fixed_size;
        
        }
    
    }
    
    finalize_symbols = 1;

    for (symbol = symbols; symbol; symbol = symbol->next) {
        symbol_resolve_value (symbol);
    }
    
    for (section = sections; section; section = section_get_next_section (section)) {
        adjust_reloc_symbols_of_section (section);
    }
    
    for (section = sections; section; section = section_get_next_section (section)) {
        fixup_section (section);
    }
    
    if (state->format == AS_FORMAT_A_OUT) {
        write_a_out_file ();
    } else if (state->format == AS_FORMAT_COFF) {
        write_coff_file ();
    } else if (state->format == AS_FORMAT_ELF) {
        write_elf_file ();
    }

}
