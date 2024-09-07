/******************************************************************************
 * @file            cms.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ld.h"
#include "tebc.h"
#include "bytearray.h"
#include "xmalloc.h"

#define MAX_RECORD_SIZE 32752
#define MAX_RECORD_SYMS (MAX_RECORD_SIZE / 20)
#define MAX_RECORD_RELOCS (MAX_RECORD_SIZE / 4)

struct temp_sym {
    struct symbol *symbol;
    address_type address;
};

static int temp_sym_compar (const void *a, const void *b)
{
    address_type aa = ((const struct temp_sym *)a)->address;
    address_type ba = ((const struct temp_sym *)b)->address;

    /* The goal is descdending order, not ascending. */
    if (aa < ba) return 1;
    if (ba == aa) return 0;
    return -1;
}

address_type cms_get_base_address (void)
{    
    return 0x20000;
}

static void write_ebcdic_symbol_name (unsigned char *pos, const char *name)
{
    int i;

    memset (pos, tebc (' '), 8);
    for (i = 0; i < 8 && name[i]; i++) {
        pos[i] = tebc (name[i]);
    }
}

static size_t get_num_relocs (void)
{
    struct section *section;
    size_t num_relocs = 0;
    
    for (section = all_sections; section; section = section->next) {
        struct section_part *part;
        
        for (part = section->first_part; part; part = part->next) {
            size_t i;
            
            for (i = 0; i < part->relocation_count; i++) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]
                    || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_24]) {
                    num_relocs++;
                }
            }
        }
    }

    return num_relocs;
}

static unsigned char *write_relocs (unsigned char *pos, size_t num_relocs)
{
    struct section *section;
    size_t rel_i = 0;
    
    for (section = all_sections; section; section = section->next) {
        struct section_part *part;
        
        for (part = section->first_part; part; part = part->next) {
            size_t i;
            
            for (i = 0; i < part->relocation_count; i++) {
                unsigned char flags;
                
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_IGNORED]) {
                    continue;
                }

                /* Not sure what meaning exactly does the flags field have, the values are guessed. */
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    flags = 0x23;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_24]) {
                    flags = 0x22;
                } else continue;

                if (rel_i % MAX_RECORD_RELOCS == 0) {
                    size_t this_relocs = num_relocs > MAX_RECORD_RELOCS ? MAX_RECORD_RELOCS : num_relocs;

                    bytearray_write_2_bytes (pos, 4 + this_relocs * 4, BIG_ENDIAN);
                    pos += 4;
                    num_relocs -= this_relocs;
                }

                pos[0] = flags;
                bytearray_write_3_bytes (pos + 1,
                                         ld_state->base_address + part->rva + part->relocation_array[i].offset,
                                         BIG_ENDIAN);
                pos += 4;
                rel_i++;
            }
        }
    }

    return pos;
}

void cms_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct section *section;
    struct object_file *of;
    size_t total_section_size_to_write = 0;
    size_t num_symbols = 0;
    size_t num_relocs = 0;

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    {
        /* 80 byte record with loader information.
         * All records have Record Descriptor Word
         * (2 byte length including the RDW, 2 zero bytes)
         * at the beginning.
         */
        file_size = 4 + 80;
        
        for (section = all_sections; section; section = section->next) {
            if (section->is_bss) continue;
            
            file_size += (section->total_size / MAX_RECORD_SIZE
                          + !!(section->total_size % MAX_RECORD_SIZE)) * 4;
            total_section_size_to_write += section->total_size;
        }

        file_size += total_section_size_to_write;

        /* Loader table,
         * stores symbols with 8 byte name and 3 4 byte fields (so 20 bytes each),
         * sorted in descdending order.
         * It should be the last record in file but when too many symbols exist,
         * it needs to be split into multiple records.
         */
        for (of = all_object_files; of; of = of->next) {
            size_t i;

            for (i = 1; i < of->symbol_count; i++) {
                const struct symbol *symbol = of->symbol_array + i;

                if (!symbol->name && symbol->section_number != i) continue;
                if (symbol_is_undefined (symbol)) continue;
                
                num_symbols++;
            }
        }
        /* +2 symbols, SYSREF and NUCON. */
        file_size += (num_symbols + 2) * 20;
        file_size += ((num_symbols + 2) / MAX_RECORD_SYMS
                      + !!((num_symbols + 2) % MAX_RECORD_SYMS)) * 4;

        /* Relocations have 4 bytes: 1 byte flags and 3 bytes address.
         * They seem to be base relocations,
         * so they do not need to reference the symbol table.
         */
        if (ld_state->emit_relocs) {
            num_relocs = get_num_relocs ();
            file_size += num_relocs * 4;
            file_size += (num_relocs / MAX_RECORD_RELOCS
                          + !!(num_relocs % MAX_RECORD_RELOCS)) * 4;
        }
    }

    file = xcalloc (file_size, 1);

    /* 80 byte record with loader information. */
    pos = file;
    bytearray_write_2_bytes (pos, 4 + 80, BIG_ENDIAN);
    pos += 4;
    /* Entry point with base address included. */
    bytearray_write_4_bytes (pos, ld_state->base_address + ld_state->entry_point, BIG_ENDIAN);
    /* Base address. */
    bytearray_write_4_bytes (pos + 4, ld_state->base_address, BIG_ENDIAN);
    /* Highest address (base address + total size of sections) twice. */
    bytearray_write_4_bytes (pos + 8, ld_state->base_address + total_section_size_to_write, BIG_ENDIAN);
    bytearray_write_4_bytes (pos + 12, ld_state->base_address + total_section_size_to_write, BIG_ENDIAN);

    /* Unknown. */
    bytearray_write_4_bytes (pos + 16, 0x125D6, BIG_ENDIAN);
    memcpy (pos + 28,
            "\x00\x00\x01\x61\x4C\x91\x00\x08\x00\x00\x00\x00\x00\x08\x00\x00"
            "\x02\xC5\xE2\xC4\x00\x00\x00\x00\x00\x02\x84\x0C\x00\x00\x0B\x18"
            "\xE9\xF0\xF0\xF0\xF2\xF8\xF4\x40",
            40);
    pos += 80;

    for (section = all_sections; section; section = section->next) {
        unsigned char *tmp;
        size_t section_size;
        
        if (section->is_bss) continue;

        tmp = xcalloc (section->total_size, 1);
        section_write (section, tmp);

        section_size = section->total_size;
        while (section_size) {
            size_t this_size = section_size > MAX_RECORD_SIZE ? MAX_RECORD_SIZE : section_size;

            /* Record containg content of sections. */
            bytearray_write_2_bytes (pos, 4 + this_size, BIG_ENDIAN);
            pos += 4;

            memcpy (pos,
                    tmp + section->total_size - section_size,
                    this_size);
            section_size -= this_size;
            pos += this_size;
        }

        free (tmp);
    }

    /* Loader table. */
    {
        unsigned long section_symbol_index = 284;
        size_t sym_i = 0;
        struct temp_sym *temp_syms = xmalloc (num_symbols * sizeof *temp_syms);
        size_t table_syms = num_symbols + 2;

        for (of = all_object_files; of; of = of->next) {
            size_t i;

            for (i = 1; i < of->symbol_count; i++) {
                struct symbol *symbol = of->symbol_array + i;

                if (!symbol->name && symbol->section_number != i) continue;
                if (symbol_is_undefined (symbol)) continue;

                temp_syms[sym_i].symbol = symbol;
                temp_syms[sym_i].address = symbol_get_value_with_base (symbol);
                sym_i++;
            }
        }

        qsort (temp_syms, num_symbols, sizeof *temp_syms, &temp_sym_compar);

        for (sym_i = 0; sym_i < num_symbols; sym_i++) {
            struct symbol *symbol = temp_syms[sym_i].symbol;

            if (sym_i % MAX_RECORD_SYMS == 0) {
                size_t this_syms = table_syms > MAX_RECORD_SYMS ? MAX_RECORD_SYMS : table_syms;
                bytearray_write_2_bytes (pos, 4 + this_syms * 20, BIG_ENDIAN);
                pos += 4;
                table_syms -= this_syms;
            }
            
            if (symbol->section_number == symbol - symbol->part->of->symbol_array) {
                /* Section symbols have strange names like ".000284". */
                char name[9];

                /* snprintf() would be better but C90 does not have it. */
                sprintf (name, ".%06lu", section_symbol_index--);
                /* 8 bytes long EBDIC symbol name. */
                write_ebcdic_symbol_name (pos, name);

                /* Not sure what is the difference between those 3 fields.
                 * but the third one seems to be the absolute address of the entire section
                 * while the other two are absolute symbol values.
                 */
                bytearray_write_4_bytes (pos + 8, symbol_get_value_with_base (symbol), BIG_ENDIAN);
                bytearray_write_4_bytes (pos + 12, symbol_get_value_with_base (symbol), BIG_ENDIAN);
                bytearray_write_4_bytes (pos + 16, symbol->part->section->rva + ld_state->base_address, BIG_ENDIAN);
            } else {
                /* 8 bytes long EBDIC symbol name. */
                write_ebcdic_symbol_name (pos, symbol->name);

                bytearray_write_4_bytes (pos + 8, 0, BIG_ENDIAN);
                bytearray_write_4_bytes (pos + 12, symbol_get_value_with_base (symbol), BIG_ENDIAN);
                bytearray_write_4_bytes (pos + 16, 0, BIG_ENDIAN);
            }

            pos += 20;
        }

        /* SYSREF and NUCON symbols. */
        {
            const char *name;

            if (sym_i % MAX_RECORD_SYMS == 0) {
                size_t this_syms = table_syms > MAX_RECORD_SYMS ? MAX_RECORD_SYMS : table_syms;
                bytearray_write_2_bytes (pos, 4 + this_syms * 20, BIG_ENDIAN);
                pos += 4;
                table_syms -= this_syms;
            }

            name = "SYSREF";
            write_ebcdic_symbol_name (pos, name);

            bytearray_write_4_bytes (pos + 8, 0x600, BIG_ENDIAN);
            bytearray_write_4_bytes (pos + 12, 0x600, BIG_ENDIAN);
            bytearray_write_4_bytes (pos + 16, 0, BIG_ENDIAN);
            pos += 20;
            sym_i++;

            if (sym_i % MAX_RECORD_SYMS == 0) {
                size_t this_syms = table_syms > MAX_RECORD_SYMS ? MAX_RECORD_SYMS : table_syms;
                bytearray_write_2_bytes (pos, 4 + this_syms * 20, BIG_ENDIAN);
                pos += 4;
                table_syms -= this_syms;
            }

            name = "NUCON";
            write_ebcdic_symbol_name (pos, name);

            bytearray_write_4_bytes (pos + 8, 0, BIG_ENDIAN);
            bytearray_write_4_bytes (pos + 12, 0, BIG_ENDIAN);
            bytearray_write_4_bytes (pos + 16, 0, BIG_ENDIAN);
            pos += 20;
        }

        free (temp_syms);
    }

    if (ld_state->emit_relocs) write_relocs (pos, num_relocs);

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}
