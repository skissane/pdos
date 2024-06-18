/******************************************************************************
 * @file            atari.c
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
#include "bytearray.h"
#include "atari.h"
#include "xmalloc.h"

#include "atari_bytearray.h"

void atari_init ()
{
    /* Force the order of sections because the output requires it.
     * This also ensures that .text is at RVA 0.
     */
    section_find_or_make (".text");
    section_find_or_make (".data");
    section_find_or_make (".bss");
}

address_type atari_get_base_address (void)
{    
    return 0;
}

static int reloc_offset_compare (const void *a, const void *b)
{
    const unsigned long *ra = a;
    const unsigned long *rb = b;

    if (*ra < *rb) return -1;
    if (*ra > *rb) return 1;

    return 0;
}

static unsigned long *get_reloc_offsets (size_t *num_relocs_p)
{
    struct section_part *part;
    struct section *text_section, *data_section;
    unsigned long *reloc_offsets;
    size_t num_relocs = 0;

    text_section = section_find (".text");
    data_section = section_find (".data");
    
    if (text_section) {
        for (part = text_section->first_part; part; part = part->next) {
            size_t i;
            
            for (i = 0; i < part->relocation_count; i++) {
                if (part->relocation_array[i].howto != &reloc_howtos[RELOC_TYPE_32]) continue;

                num_relocs++;
            }
        }
    }
    if (data_section) {
        for (part = data_section->first_part; part; part = part->next) {
            size_t i;
            
            for (i = 0; i < part->relocation_count; i++) {
                if (part->relocation_array[i].howto != &reloc_howtos[RELOC_TYPE_32]) continue;

                num_relocs++;
            }
        }
    }

    *num_relocs_p = num_relocs;
    if (!num_relocs) return NULL;

    reloc_offsets = xmalloc (num_relocs * sizeof *reloc_offsets);
    num_relocs = 0;
    if (text_section) {
        for (part = text_section->first_part; part; part = part->next) {
            size_t i;
            
            for (i = 0; i < part->relocation_count; i++) {
                if (part->relocation_array[i].howto != &reloc_howtos[RELOC_TYPE_32]) continue;

                reloc_offsets[num_relocs] = part->rva + part->relocation_array[i].offset;
                num_relocs++;
            }
        }
    }
    if (data_section) {
        for (part = data_section->first_part; part; part = part->next) {
            size_t i;
            
            for (i = 0; i < part->relocation_count; i++) {
                if (part->relocation_array[i].howto != &reloc_howtos[RELOC_TYPE_32]) continue;

                reloc_offsets[num_relocs] = part->rva + part->relocation_array[i].offset;
                num_relocs++;
            }
        }
    }

    qsort (reloc_offsets, num_relocs, sizeof *reloc_offsets, &reloc_offset_compare);

    return reloc_offsets;
}

void atari_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct section *text_section, *data_section, *bss_section;
    size_t num_relocs;
    unsigned long *reloc_offsets = NULL;
    struct PH_internal ph = {0};
    size_t reloc_size = 4;

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    text_section = section_find (".text");
    data_section = section_find (".data");
    bss_section = section_find (".bss");

    ph.ph_branch = 0x601a;
    /* Sections might not be contiguous because of RVA alignment.
     * and some might not even exist (but their order is certain).
     */
    if (data_section) {
        ph.ph_tlen = data_section->rva;
        if (bss_section) {
            ph.ph_dlen = bss_section->rva - data_section->rva;
        } else {
            ph.ph_dlen = data_section->total_size;
        }
    } else {
        if (bss_section) {
            ph.ph_tlen = bss_section->rva;
        } else {
            ph.ph_tlen = text_section ? text_section->total_size : 0;
        }
        ph.ph_dlen = 0;
    }
    
    ph.ph_blen = bss_section ? bss_section->total_size : 0;
    ph.ph_slen = 0;

    reloc_offsets = get_reloc_offsets (&num_relocs);
    if (num_relocs) {
        size_t i;
        unsigned long offset = reloc_offsets[0];
        
        for (i = 1; i < num_relocs; i++) {
            unsigned long diff = reloc_offsets[i] - offset;

            while (diff > 255) {
                diff -= 254;
                reloc_size++;
            }
            reloc_size++;
            offset = reloc_offsets[i];
        }
        reloc_size++;
    }

    file_size = SIZEOF_struct_PH_file;
    file_size += ph.ph_tlen + ph.ph_dlen;
    file_size += reloc_size;

    file = xcalloc (file_size, 1);
    
    pos = file;
    write_struct_PH (pos, &ph);
    pos += SIZEOF_struct_PH_file;
    
    if (text_section) section_write (text_section, pos);
    pos += ph.ph_tlen;
    if (data_section) section_write (data_section, pos);
    pos += ph.ph_dlen;

    if (num_relocs) {
        size_t i;
        unsigned long offset = reloc_offsets[0];

        bytearray_write_4_bytes (pos, offset, BIG_ENDIAN);
        pos += 4;
        
        for (i = 1; i < num_relocs; i++) {
            unsigned long diff = reloc_offsets[i] - offset;

            while (diff > 255) {
                diff -= 254;
                pos++[0] = 1;
            }
            pos++[0] = diff;
            offset = reloc_offsets[i];
        }
        pos[0] = 0;
    } else {
        /* Empty relocation table should be marked with a LONG value of 0. */
        bytearray_write_4_bytes (pos, 0, BIG_ENDIAN);
    }

    free (reloc_offsets);
    
    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}
