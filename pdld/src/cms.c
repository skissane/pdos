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
            
            total_section_size_to_write += section->total_size;
        }

        if (total_section_size_to_write > 65535) {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "total_section_size_to_write > 65535");
        }

        file_size += 4 + total_section_size_to_write;

        /* Loader table,
         * stores symbols with 8 byte name and 3 4 byte fields (so 20 bytes each),
         * sorted in descdending order.
         */
        file_size += 4;

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

    /* Record containg content of sections. */
    bytearray_write_2_bytes (pos, 4 + total_section_size_to_write, BIG_ENDIAN);
    pos += 4;
    
    for (section = all_sections; section; section = section->next) {
        if (section->is_bss) continue;
        
        section_write (section, pos);
        pos += section->total_size;
    }

    /* Loader table. */
    bytearray_write_2_bytes (pos, 4 + (num_symbols + 2) * 20, BIG_ENDIAN);
    pos += 4;

    {
        unsigned long section_symbol_index = 284;
        size_t sym_i = 0;
        struct temp_sym *temp_syms = xmalloc (num_symbols * sizeof *temp_syms);

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
            
            if (symbol->section_number == symbol - symbol->part->of->symbol_array) {
                /* Section symbols have strange names like ".000284". */
                char name[9];

                /* snprintf() would be better but C90 does not have it. */
                sprintf (name, ".%06lu", section_symbol_index--);
                /* 8 bytes long EBDIC symbol name. */
                {
                    int j;

                    memset (pos, tebc (' '), 8);
                    for (j = 0; j < 8 && name[j]; j++) {
                        pos[j] = tebc (name[j]);
                    }
                }

                /* Not sure what is the difference between those 3 fields.
                 * but the third one seems to be the absolute address of the entire section
                 * while the other two are absolute symbol values.
                 */
                bytearray_write_4_bytes (pos + 8, symbol_get_value_with_base (symbol), BIG_ENDIAN);
                bytearray_write_4_bytes (pos + 12, symbol_get_value_with_base (symbol), BIG_ENDIAN);
                bytearray_write_4_bytes (pos + 16, symbol->part->section->rva + ld_state->base_address, BIG_ENDIAN);
            } else {
                /* 8 bytes long EBDIC symbol name. */
                {
                    int j;

                    memset (pos, tebc (' '), 8);
                    for (j = 0; j < 8 && symbol->name[j]; j++) {
                        pos[j] = tebc (symbol->name[j]);
                    }
                }

                bytearray_write_4_bytes (pos + 8, 0, BIG_ENDIAN);
                bytearray_write_4_bytes (pos + 12, symbol_get_value_with_base (symbol), BIG_ENDIAN);
                bytearray_write_4_bytes (pos + 16, 0, BIG_ENDIAN);
            }

            pos += 20;
        }

        /* SYSREF and NUCON symbols. */
        {
            int j;
            const char *name;

            name = "SYSREF";
            memset (pos, tebc (' '), 8);
            for (j = 0; j < 8 && name[j]; j++) {
                pos[j] = tebc (name[j]);
            }

            bytearray_write_4_bytes (pos + 8, 0x600, BIG_ENDIAN);
            bytearray_write_4_bytes (pos + 12, 0x600, BIG_ENDIAN);
            bytearray_write_4_bytes (pos + 16, 0, BIG_ENDIAN);
            pos += 20;

            name = "NUCON";
            memset (pos, tebc (' '), 8);
            for (j = 0; j < 8 && name[j]; j++) {
                pos[j] = tebc (name[j]);
            }

            bytearray_write_4_bytes (pos + 8, 0, BIG_ENDIAN);
            bytearray_write_4_bytes (pos + 12, 0, BIG_ENDIAN);
            bytearray_write_4_bytes (pos + 16, 0, BIG_ENDIAN);
            pos += 20;
        }

        free (temp_syms);
    }

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}
