/******************************************************************************
 * @file            link.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "ld.h"
#include "xmalloc.h"

static void collapse_subsections (void)
{
    struct section *section;

    for (section = all_sections; section; section = section->next) {
        struct subsection *subsection;

        for (subsection = section->all_subsections; subsection; subsection = subsection->next) {
            if (subsection->first_part) {
                *section->last_part_p = subsection->first_part;
                section->last_part_p = subsection->last_part_p;
            }

        }
    }
}

static void calculate_section_sizes_and_rvas (void)
{
    struct section *section;
    address_type rva = 0;

    if (ld_state->oformat == LD_OFORMAT_COFF) {
        rva = coff_get_first_section_rva ();
    } else if (ld_state->oformat == LD_OFORMAT_ELF) {
        rva = elf_get_first_section_rva ();
    }

    for (section = all_sections; section; section = section->next) {
        struct section_part *part;

        rva = ALIGN (rva, section->section_alignment);
        section->rva = rva;
        
        section->total_size = 0;
        for (part = section->first_part; part; part = part->next) {

            if (part->next && part->next->alignment > 1) {
                address_type new_rva;

                new_rva = ALIGN (rva + part->content_size, part->next->alignment);
                if (new_rva != rva + part->content_size) {
                    part->content = xrealloc (part->content, new_rva - rva);
                    memset (part->content + part->content_size,
                            0,
                            new_rva - rva - part->content_size);
                    part->content_size = new_rva - rva;
                }
            }
            
            part->rva = rva;
            section->total_size += part->content_size;
            rva += part->content_size;
        }
    }
}

static void relocate_sections (void)
{
    struct section *section;

    for (section = all_sections; section; section = section->next) {
        struct section_part *part;
        
        for (part = section->first_part; part; part = part->next) {
            if (ld_state->oformat == LD_OFORMAT_COFF
                || ld_state->oformat == LD_OFORMAT_LX) {
                coff_relocate_part (part);
            } else if (ld_state->oformat == LD_OFORMAT_ELF) {
                elf_relocate_part (part);
            }
        }
    }
}

static void calculate_entry_point (void)
{
    const struct section *section;

    if (ld_state->entry_symbol_name) {
        if (ld_state->entry_symbol_name[0] == '\0') {
            ld_state->entry_point -= ld_state->base_address;
            return;
        } else {
            const struct symbol *symbol;

            symbol = symbol_find (ld_state->entry_symbol_name);
            if (symbol) {
                ld_state->entry_point = symbol_get_value_no_base (symbol);
                return;
            }
        }
    }

    if (ld_state->entry_symbol_name == NULL) {
        if ((ld_state->entry_point = coff_calculate_entry_point ())) return;
    }

    section = section_find (".text");
    if (section) ld_state->entry_point = section->rva;

    if (ld_state->entry_symbol_name) {
        ld_warn ("cannot find entry symbol '%s'; defaulting to 0x%08lx",
                 ld_state->entry_symbol_name,
                 ld_state->base_address + ld_state->entry_point);
    }
}

void link (void)
{
    collapse_subsections ();

    calculate_section_sizes_and_rvas ();

    ld_state->base_address = coff_get_base_address ();

    relocate_sections ();

    calculate_entry_point ();
    
}
