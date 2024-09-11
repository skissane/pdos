/******************************************************************************
 * @file            vse.c
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
#include <ctype.h>

#include "ld.h"
#include "tebc.h"
#include "bytearray.h"
#include "xmalloc.h"
#include "mainframe.h"

#define RECORD_SIZE 80
#define MAX_TXT_IN_BYTES 56
#define MAX_RLD_IN_BYTES 56
/* 4 bytes are needed for the beginning ESD IDs in each record. */
#define MAX_RLD_RELOCS ((MAX_RLD_IN_BYTES - 4) / 4)

#define PHASE_STR " PHASE "
#define AFTER_PHASE_STR ",S+X'000050'"

address_type vse_get_base_address (void)
{    
    return 0x4000C8;
}

unsigned char *write_ebcdic_str_no_null (unsigned char *pos, const char *str)
{
    size_t i;

    for (i = 0; str[i]; i++) {
        pos[i] = tebc (str[i]);
    }

    return pos + i;
}

unsigned char *terminate_record (unsigned char *file, size_t *record_i_p)
{
    unsigned char *pos = file + (*record_i_p + 1) * RECORD_SIZE - 4;
    char text[5];

    sprintf (text, "%04lu", (unsigned long)*record_i_p);
    *record_i_p += 1;

    return write_ebcdic_str_no_null (pos, text);
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

static unsigned char *write_relocs (unsigned char *file,
                                    unsigned char *pos,
                                    size_t *record_i_p,
                                    size_t num_relocs)
{
    struct section *section;
    size_t this_relocs;

    if (!num_relocs) return pos;

    this_relocs = num_relocs > MAX_RLD_RELOCS ? MAX_RLD_RELOCS : num_relocs;
    num_relocs -= this_relocs;

    pos[0] = 0x2;
    write_ebcdic_str_no_null (pos + 1, "RLD");
    bytearray_write_2_bytes (pos + 10, 4 + this_relocs * 4, BIG_ENDIAN);
    pos += 16;
    bytearray_write_2_bytes (pos, 1, BIG_ENDIAN);
    bytearray_write_2_bytes (pos + 2, 1, BIG_ENDIAN);
    pos += 4;
    
    for (section = all_sections; section; section = section->next) {
        struct section_part *part;
        
        for (part = section->first_part; part; part = part->next) {
            size_t i;
            
            for (i = 0; i < part->relocation_count; i++) {
                unsigned char flags;
                
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_IGNORED]) {
                    continue;
                }

                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    flags = 0xC;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_24]) {
                    flags = 0x8;
                } else continue;

                /* The last RLD data item should not have the flag
                 * that next item has same ESD IDs set.
                 */
                if (this_relocs != 1) flags |= 0x1;

                if (this_relocs == 0) {
                    pos = terminate_record (file, record_i_p);
                    if (!num_relocs) return pos;
                    
                    pos[0] = 0x2;
                    write_ebcdic_str_no_null (pos + 1, "RLD");
                    bytearray_write_2_bytes (pos + 10, 4 + this_relocs * 4, BIG_ENDIAN);
                    pos += 16;
                    bytearray_write_2_bytes (pos, 1, BIG_ENDIAN);
                    bytearray_write_2_bytes (pos + 1, 1, BIG_ENDIAN);
                    pos += 4;
                }

                pos[0] = flags;
                bytearray_write_3_bytes (pos + 1,
                                         ld_state->base_address + part->rva + part->relocation_array[i].offset,
                                         BIG_ENDIAN);
                pos += 4;
                this_relocs--;
            }
        }
    }

    pos = terminate_record (file, record_i_p);

    return pos;
}

void vse_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct section *section;
    size_t total_section_size_to_write = 0;
    size_t record_i = 1;
    size_t num_relocs = 0;
    
    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    {
        /* Beginning PHASE record. */
        file_size = RECORD_SIZE;
        /* Symbol table. */
        file_size += RECORD_SIZE;

        for (section = all_sections; section; section = section->next) {
            if (section->is_bss) continue;
            
            total_section_size_to_write += section->total_size;
            file_size += (section->total_size / MAX_TXT_IN_BYTES
                          + !!(section->total_size % MAX_TXT_IN_BYTES)) * RECORD_SIZE;
        }

        num_relocs = get_num_relocs ();
        file_size += (num_relocs / MAX_RLD_RELOCS
                      + !!(num_relocs % MAX_RLD_RELOCS)) * RECORD_SIZE;

        /* END record. */
        file_size += RECORD_SIZE;
        /* Ending record. */
        file_size += RECORD_SIZE;
    }
    
    file = xmalloc (file_size);
    /* Instead of '\0' EBCDIC ' ' should be used to fill empty space. */
    memset (file, tebc (' '), file_size);

    pos = file;
    {
        size_t i;

        pos = write_ebcdic_str_no_null (pos, PHASE_STR);

        /* 8 bytes long EBDIC uppercase program name. */
        for (i = 0; i < 8 && ld_state->output_filename[i]; i++) {
            if (i > 0 && ld_state->output_filename[i] == '.') break;
            pos[i] = tebc (toupper (ld_state->output_filename[i]));
        }

        pos += i;
        write_ebcdic_str_no_null (pos, AFTER_PHASE_STR);
    }
    pos = file + RECORD_SIZE;

    /* Symbol table, contains single SD symbol with program name. */
    pos[0] = 0x2;
    write_ebcdic_str_no_null (pos + 1, "ESD");
    bytearray_write_2_bytes (pos + 10, 16, BIG_ENDIAN);
    bytearray_write_2_bytes (pos + 14, 1, BIG_ENDIAN);
    pos += 16;
    
    {
        size_t i;

        /* 8 bytes long EBDIC uppercase program name. */
        for (i = 0; i < 8 && ld_state->output_filename[i]; i++) {
            if (i > 0 && ld_state->output_filename[i] == '.') break;
            pos[i] = tebc (toupper (ld_state->output_filename[i]));
        }
        pos[8] = ESD_DATA_TYPE_SD;
        bytearray_write_3_bytes (pos + 9, ld_state->base_address, BIG_ENDIAN);
        bytearray_write_3_bytes (pos + 13, total_section_size_to_write, BIG_ENDIAN);
    }
    pos = terminate_record (file, &record_i);

    for (section = all_sections; section; section = section->next) {
        unsigned char *tmp;
        size_t section_size;
        
        if (section->is_bss) continue;

        tmp = xcalloc (section->total_size, 1);
        section_write (section, tmp);

        section_size = section->total_size;
        while (section_size) {
            size_t this_size = section_size > MAX_TXT_IN_BYTES ? MAX_TXT_IN_BYTES : section_size;

            pos[0] = 0x2;
            write_ebcdic_str_no_null (pos + 1, "TXT");

            bytearray_write_3_bytes (pos + 5, ld_state->base_address + section->total_size - section_size, BIG_ENDIAN);
            bytearray_write_2_bytes (pos + 10, this_size, BIG_ENDIAN);
            bytearray_write_2_bytes (pos + 14, 1, BIG_ENDIAN);
            pos += 16;

            memcpy (pos,
                    tmp + section->total_size - section_size,
                    this_size);
            section_size -= this_size;
            pos = terminate_record (file, &record_i);
        }

        free (tmp);
    }

    pos = write_relocs (file, pos, &record_i, num_relocs);

    /* END record. */
    pos[0] = 0x2;
    write_ebcdic_str_no_null (pos + 1, "END");
    /* Entry point. */
    bytearray_write_3_bytes (pos + 5, ld_state->base_address + ld_state->entry_point, BIG_ENDIAN);
    /* ESD ID of SD for the section with entry point. */
    bytearray_write_2_bytes (pos + 14, 1, BIG_ENDIAN);
    
    pos = terminate_record (file, &record_i);

    /* Ending record. */
    pos[0] = tebc ('/');
    pos[1] = tebc ('*');

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}
