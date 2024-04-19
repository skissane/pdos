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
#include <limits.h>

#include "ld.h"
#include "xmalloc.h"
#include "bytearray.h"

static void reloc_arm_32 (struct section_part *part,
                          struct reloc_entry *rel,
                          struct symbol *symbol);
static void reloc_arm_26_pcrel (struct section_part *part,
                                struct reloc_entry *rel,
                                struct symbol *symbol);

static void reloc_aarch64_hi21_page_pcrel (struct section_part *part,
                                           struct reloc_entry *rel,
                                           struct symbol *symbol);
static void reloc_aarch64_generic (struct section_part *part,
                                   struct reloc_entry *rel,
                                   struct symbol *symbol);

static void reloc_generic (struct section_part *part,
                           struct reloc_entry *rel,
                           struct symbol *symbol);

const struct reloc_howto reloc_howtos[RELOC_TYPE_END] = {
    { 0, 0, 0, 0, NULL, "RELOC_TYPE_IGNORED" },
    
    { 8, 0, 0, 0, NULL, "RELOC_TYPE_64" },
    { 4, 0, 0, 0, NULL, "RELOC_TYPE_32" },

    { 4, 1, 0, 0, NULL, "RELOC_TYPE_PC32" },

    { 4, 0, 1, 0, NULL, "RELOC_TYPE_32_NO_BASE" },

    { 4, 0, 0, 0, &reloc_arm_32, "RELOC_TYPE_ARM_32" },
    { 3, 1, 0, 2, &reloc_arm_26_pcrel, "RELOC_TYPE_ARM_PC26" },

    { 4, 1, 0, 9, &reloc_aarch64_hi21_page_pcrel, "RELOC_TYPE_AARCH64_ADR_PREL_PG_HI21", 0x60ffffe0 },
    { 4, 0, 0, 0, &reloc_aarch64_generic, "RELOC_TYPE_AARCH64_ADD_ABS_LO12_NC", 0x3ffc00, 10 },
    { 2, 0, 0, 0, &reloc_aarch64_generic, "RELOC_TYPE_AARCH64_LDST8_ABS_LO12_NC", 0xfff },
    { 2, 0, 0, 1, &reloc_aarch64_generic, "RELOC_TYPE_AARCH64_LDST16_ABS_LO12_NC", 0xffe },
    { 4, 0, 0, 2, &reloc_aarch64_generic, "RELOC_TYPE_AARCH64_LDST32_ABS_LO12_NC", 0xffc00, 10 },
    { 4, 0, 0, 3, &reloc_aarch64_generic, "RELOC_TYPE_AARCH64_LDST64_ABS_LO12_NC", 0x7fc00, 10 },
    { 4, 1, 0, 2, &reloc_aarch64_generic, "RELOC_TYPE_AARCH64_JUMP26", 0x3ffffff },
    { 4, 1, 0, 2, &reloc_aarch64_generic, "RELOC_TYPE_AARCH64_CALL26", 0x3ffffff },
};

static void reloc_arm_32 (struct section_part *part,
                          struct reloc_entry *rel,
                          struct symbol *symbol)
{    
    if (symbol->part->of == part->of) {
        address_type addend;
        
        bytearray_read_4_bytes (&addend, part->content + rel->offset, LITTLE_ENDIAN);
        
        if (addend) {
            /* If the target symbol is in same object file and the addend is not zero,
             * the addend has the value of the symbol added in by assembler,
             * so the value of the symbol needs to be subtracted before doing the relocation.
             */
            addend -= symbol->value;
            bytearray_write_4_bytes (part->content + rel->offset, addend, LITTLE_ENDIAN);
        }
    }

    /* Rest of the relocation is the same as regular 32-bit relocation. */
    reloc_generic (part, rel, symbol);
}

static void reloc_arm_26_pcrel (struct section_part *part,
                                struct reloc_entry *rel,
                                struct symbol *symbol)
{
    address_type result;
    
    bytearray_read_3_bytes (&result, part->content + rel->offset, LITTLE_ENDIAN);
    result <<= 2;
    /* The field is signed, so sign extend it. */
    result = (result ^ 0x2000000) - 0x2000000;
    result += rel->addend;
    result += symbol_get_value_no_base (symbol);
    /* The size of the field must not be subtracted
     * even though it is PC relative relocation.
     */
    result -= part->rva + rel->offset;
    result >>= 2;
    bytearray_write_3_bytes (part->content + rel->offset, result, LITTLE_ENDIAN);
}

static void reloc_aarch64_hi21_page_pcrel (struct section_part *part,
                                           struct reloc_entry *rel,
                                           struct symbol *symbol)
{
    address_type result;
    unsigned long field;
    address_type bottom_2_bits = 0;

    /* The lowest 2 bits of immediate operand for ADRP are 0x60000000
     * while the rest of immediate starts at 0x20.
     * (adrp x0, 0x0000 is 0x90000000
     *  adrp x0, 0x1000 is 0xb0000000,
     *  adrp x0, 0x2000 is 0xd0000000,
     *  adrp x0, 0x3000 is 0xf0000000,
     *  adrp x0, 0x4000 is 0x90000020,
     *  adrp x0, 0x5000 is 0xb0000020,
     *  adrp x0, 0x6000 is 0xd0000020,
     *  adrp x0, 0x7000 is 0xf0000020...)
     */

    result = (symbol_get_value_with_base (symbol) + rel->addend) & ~(address_type)0xfff;
    result -= (ld_state->base_address + part->rva + rel->offset) & ~(address_type)0xfff;
    
    if ((bottom_2_bits = result & 0x3000)) {
        result &= ~bottom_2_bits;
        bottom_2_bits >>= 12;
    }
    result >>= 9;
    result |= bottom_2_bits << 29;

    bytearray_read_4_bytes (&field, part->content + rel->offset, LITTLE_ENDIAN);
    field = ((field & ~(address_type)rel->howto->dst_mask)
             | (((field & rel->howto->dst_mask) + result) & rel->howto->dst_mask));
    bytearray_write_4_bytes (part->content + rel->offset, field, LITTLE_ENDIAN);
}

static void reloc_aarch64_generic (struct section_part *part,
                                   struct reloc_entry *rel,
                                   struct symbol *symbol)
{
    address_type result, field;

    result = rel->addend;
    result += symbol_get_value_with_base (symbol);
    if (rel->howto->pc_relative) {
        result -= ld_state->base_address + part->rva + rel->offset;
    }
    
    result >>= rel->howto->final_right_shift;
    result <<= rel->howto->final_left_shift;

    switch (rel->howto->size) {
        case 8:
            /* It should be actually 8 bytes but 64-bit int is not yet available. */
            {
                unsigned long field4;
                bytearray_read_4_bytes (&field4, part->content + rel->offset, LITTLE_ENDIAN);
                field = field4;
            }
            break;

        case 4:
            {
                unsigned long field4;
                bytearray_read_4_bytes (&field4, part->content + rel->offset, LITTLE_ENDIAN);
                field = field4;
            }
            break;

        case 2:
            {
                unsigned short field2;
                bytearray_read_2_bytes (&field2, part->content + rel->offset, LITTLE_ENDIAN);
                field = field2;
            }
            break;

        default:
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "invalid relocation size");
    }

    field = ((field & ~(address_type)rel->howto->dst_mask)
             | (((field & rel->howto->dst_mask) + result) & rel->howto->dst_mask));

    switch (rel->howto->size) {
        case 8:
            /* It should be actually 8 bytes but 64-bit int is not yet available. */
            bytearray_write_4_bytes (part->content + rel->offset, field, LITTLE_ENDIAN);
            break;

        case 4:
            bytearray_write_4_bytes (part->content + rel->offset, field, LITTLE_ENDIAN);
            break;

        case 2:
            bytearray_write_2_bytes (part->content + rel->offset, field, LITTLE_ENDIAN);
            break;

        default:
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "invalid relocation size");
    }
    
}

static void reloc_generic (struct section_part *part,
                           struct reloc_entry *rel,
                           struct symbol *symbol)
{
    address_type result;
    
    switch (rel->howto->size) {
        case 8:
            /* It should be actually 8 bytes but 64-bit int is not yet available. */
            bytearray_read_4_bytes (&result, part->content + rel->offset, LITTLE_ENDIAN);
            break;

        case 4:
            bytearray_read_4_bytes (&result, part->content + rel->offset, LITTLE_ENDIAN);
            break;

        case 3:
            bytearray_read_3_bytes (&result, part->content + rel->offset, LITTLE_ENDIAN);
            break;

        default:
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "invalid relocation size");
    }

    result += rel->addend;
        
    if (rel->howto->pc_relative
        || rel->howto->no_base) {
        result += symbol_get_value_no_base (symbol);
    } else {
        result += symbol_get_value_with_base (symbol);
    }

    if (rel->howto->pc_relative) {
        result -= part->rva + rel->offset;
        result -= rel->howto->size;
    }

    if (rel->howto->size < sizeof result) {
        address_type mask = (((address_type)1) << (CHAR_BIT * rel->howto->size)) - 1;
        result &= mask;
    }
    result >>= rel->howto->final_right_shift;
    
    switch (rel->howto->size) {
        case 8:
            /* It should be actually 8 bytes but 64-bit int is not yet available. */
            bytearray_write_4_bytes (part->content + rel->offset, result, LITTLE_ENDIAN);
            break;

        case 4:
            bytearray_write_4_bytes (part->content + rel->offset, result, LITTLE_ENDIAN);
            break;

        case 3:
            bytearray_write_3_bytes (part->content + rel->offset, result, LITTLE_ENDIAN);
            break;

        default:
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "invalid relocation size");
    }
}

static void relocate_part (struct section_part *part)
{
    struct reloc_entry *relocs;
    size_t i;
    
    relocs = part->relocation_array;
    for (i = 0; i < part->relocation_count; i++) {
        struct symbol *symbol;

        if (relocs[i].howto->size == 0) continue;

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

        if (relocs[i].howto->special_function) {
            (*relocs[i].howto->special_function) (part, &relocs[i], symbol);
            continue;
        }

        reloc_generic (part, &relocs[i], symbol);
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

    if (!ld_state->use_custom_base_address) {
        switch (ld_state->oformat) {
            case LD_OFORMAT_COFF:
                ld_state->base_address = coff_get_base_address ();
                break;

            case LD_OFORMAT_LX:
                ld_state->base_address = lx_get_base_address ();
                break;

            default:
                ld_state->base_address = coff_get_base_address ();
                break;
        }
    }

    relocate_sections ();

    calculate_entry_point ();
    
}
