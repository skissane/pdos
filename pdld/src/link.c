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
#include "bytearray.h"

const struct reloc_howto reloc_howtos[RELOC_TYPE_END] = {
    { 0, 0, 0, NULL, "RELOC_TYPE_IGNORED" },
    
    { 8, 0, 0, NULL, "RELOC_TYPE_64" },
    { 4, 0, 0, NULL, "RELOC_TYPE_32" },

    { 4, 1, 0, NULL, "RELOC_TYPE_PC32" },

    { 4, 0, 1, NULL, "RELOC_TYPE_32_NO_BASE" },

};

static void relocate_part (struct section_part *part)
{
    struct reloc_entry *relocs;
    size_t i;
    
    relocs = part->relocation_array;
    for (i = 0; i < part->relocation_count; i++) {
        struct symbol *symbol;
        address_type result;

        if (relocs[i].howto->size == 0) continue;

        if (relocs[i].howto->special_function) {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "special relocation functions are not yet supported");
        }

        symbol = relocs[i].symbol;
        if (symbol_is_undefined (symbol)) {
            if ((symbol = symbol_find (symbol->name)) == NULL) {
                symbol = relocs[i].symbol;
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "external symbol '%s' not found in hashtab",
                                             symbol->name);
            }
            if (symbol_is_undefined (symbol)) {
                ld_error ("%s:(%s+0x%lx): undefined reference to '%s'",
                          part->of->filename,
                          part->section->name,
                          relocs[i].offset,
                          symbol->name);
                continue;
            }
        }

        switch (relocs[i].howto->size) {
            case 8:
                /* It should be actually 8 bytes but 64-bit int is not yet available. */
                bytearray_read_4_bytes (&result, part->content + relocs[i].offset, LITTLE_ENDIAN);
                break;

            case 4:
                bytearray_read_4_bytes (&result, part->content + relocs[i].offset, LITTLE_ENDIAN);
                break;

            default:
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "invalid relocation size");
        }

        result += relocs[i].addend;
        
        if (relocs[i].howto->pc_relative
            || relocs[i].howto->no_base) {
            result += symbol_get_value_no_base (symbol);
        } else {
            result += symbol_get_value_with_base (symbol);
        }

        if (relocs[i].howto->pc_relative) {
            result -= part->rva + relocs[i].offset;
            result -= relocs[i].howto->size;
        }
        
        switch (relocs[i].howto->size) {
            case 8:
                /* It should be actually 8 bytes but 64-bit int is not yet available. */
                bytearray_write_4_bytes (part->content + relocs[i].offset, result, LITTLE_ENDIAN);
                break;

            case 4:
                bytearray_write_4_bytes (part->content + relocs[i].offset, result, LITTLE_ENDIAN);
                break;

            default:
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "invalid relocation size");
        }
    }
}

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
            relocate_part (part);
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
