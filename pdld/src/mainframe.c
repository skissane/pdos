/******************************************************************************
 * @file            mainframe.c
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
#include "febc.h"
#include "tebc.h"
#include "bytearray.h"
#include "xmalloc.h"

/* Bytearray structs are not currently really used
 * because only few fields need to be written.
 */
#define SIZEOF_struct_COPYR1_file 60 /* In old format, in new format it is 64. */
#define SIZEOF_struct_COPYR2_file 284

#define LINKAGE_EDITOR_DATA "\xF5\xF7\xF5\xF2\xE2\xC3\xF1\xF0\xF4\x40\x03\x08\x24\x14\x5F"
#define USER_SUPPLIED_DATA "\x80\x01\x00\xF5\xF7\xF4\xF1\xE2\xC3\xF1\xF0\xF3\x40\x02\x01\x24\x14\x5F"

void mainframe_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct section *section;
    size_t cesd_size;
    size_t csect_hmaspzap_size, csect_linkage_size, csect_user_size;
    size_t control_record_size;
    unsigned char current_sector = 0x16; /* Arbitrary? */

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    {
        size_t total_section_size_to_write = 0;
        int segment_number = 1;

        file_size = 0;

        file_size += SIZEOF_struct_COPYR1_file - 4;
        file_size += SIZEOF_struct_COPYR2_file - 4;
        file_size += 8 + 276 + 12 - 4; /* Directory Block Record. */
        file_size += 8 - 4; /* Member Data Record header. */

        file_size += 12; /* Member Data header. */
        /* Load module, the program itself. */
        cesd_size = 8; /* CESD Record. */
        for (section = all_sections; section; section = section->next) {
            section->target_index = segment_number++;
            cesd_size += 16; /* CESD data. */
        }
        file_size += cesd_size;

        file_size += 12; /* Member Data header. */
        file_size += csect_hmaspzap_size = 3 + 0xF8;
        file_size += 12; /* Member Data header. */
        file_size += csect_linkage_size = 3 + sizeof (LINKAGE_EDITOR_DATA) - 1;
        file_size += 12; /* Member Data header. */
        file_size += csect_user_size = 3 + sizeof (USER_SUPPLIED_DATA) - 1;

        file_size += 12; /* Member Data header. */
        file_size += control_record_size = 16 + 4 * (segment_number - 1);

        for (section = all_sections; section; section = section->next) {
            if (!section->is_bss) {
                file_size += 12; /* Member Data header. */
                total_section_size_to_write += ALIGN (section->total_size, 16);
            }
        }
        
        file_size += total_section_size_to_write;

        file_size += 12; /* Member Data Record EOF. */
    }

    file = xcalloc (file_size, 1);

    /* COPYR1 record, Block Descriptor Word is stripped. */
    pos = file - 4;
    
    /* Segment Descriptor Word, length of segment.
     * Length of COPYR1 including this field.
     */
    bytearray_write_2_bytes (pos + 4, SIZEOF_struct_COPYR1_file - 4, BIG_ENDIAN);

    memcpy (pos + 9, "\xCA\x6D\x0F", 3);

    /* Data set organization. */
    pos[12] = 0x2;
    pos[13] = 0;
    /* Block size, not sure how to determine. */
    bytearray_write_2_bytes (pos + 14, 0x4800, BIG_ENDIAN);
    /* Record format, not sure what it means. */
    pos[18] = 0x80 | 0x40;
    /* Block size of container set, not sure how to determine. */
    bytearray_write_2_bytes (pos + 22, 0x4814, BIG_ENDIAN);
    /* Information about the device.
     * Currently just copied from working executable
     * because DEVTYPE macro is not available.
     */
    memcpy (pos + 24,
            "\x30\x50\x20\x0B\x00\x00\x4A\x7D\x02\x30\x00\x1E\x4B\x36\x01\x0B\x52\x08\x02\x00",
            20);

    pos += SIZEOF_struct_COPYR1_file;
    
    /* COPYR2 record, Block Descriptor Word is stripped. */
    pos -= 4;
    /* Segment Descriptor Word, length of segment.
     * Length of COPYR2 including this field.
     */
    bytearray_write_2_bytes (pos + 4, SIZEOF_struct_COPYR2_file - 4, BIG_ENDIAN);

    /* Last 16 bytes of basic section of Data Extend Block (DEB),
     * highly device dependent, so just copied from working executable.
     */
    memcpy (pos + 8,
            "\x01\x00\x00\x00\xFF\x00\x00\x00\x8F\x0B\x66\x44\x04\x94\x70\xE8",
            16);
    /* First 16 extent descriptors from original DEB,
     * highly device dependent, so just copied from working executable.
     */
    memcpy (pos + 24,
            "\x50\x00\x72\xA0\x00\x00\x00\x01\x00\x00\x00\x0A\x00\x1D\x01\x2C",
            16);
    memset (pos + 24 + 16, 0, 256 - 16);

    pos += SIZEOF_struct_COPYR2_file;

    /* Directory Block Record, Block Descriptor Word is stripped. */
    pos -= 4;
    /* Record Descriptor Word,
     * length of Directory Block Record including this field.
     */
    bytearray_write_2_bytes (pos + 4, 8 + 276 + 12 - 4, BIG_ENDIAN);
    /* Count field, 8 bytes, CCHHRKDD,
     * K (key length) should be 0x8, DD (data length) should be 0x100.
     */
    pos[12 + 5] = 0x8;
    bytearray_write_2_bytes (pos + 12 + 6, 0x100, BIG_ENDIAN);

    /* 8 bytes of 0xFF, not sure what it is. */
    memset (pos + 20, 0xFF, 8);
    /* Unknown length field. */
    bytearray_write_2_bytes (pos + 28, 0x32, BIG_ENDIAN);
    /* 8 bytes long EBDIC program name. */
    {
        int i;

        memset (pos + 30, '\0', 8);
        for (i = 0; i < 8 && ld_state->output_filename[i]; i++) {
            if (i > 0 && ld_state->output_filename[i] == '.') break;
            pos[30 + i] = tebc (ld_state->output_filename[i]);
        }
    }
    /* 19 unknown bytes. */
    memcpy (pos + 38,
            "\x00\x00\x16\x2C\x00\x00\x1B\x00\x00\x00\x00\x00\x03\xF2\x00\x00\x20\x00\x20",
            19);
    /* 24 bit entry point. */
    bytearray_write_3_bytes (pos + 57, ld_state->entry_point, BIG_ENDIAN);
    /* Unknown. */
    pos[60] = 0x88;
    pos[63] = 0x01;
    /* Again 8 bytes of 0xFF, unknown meaning. */
    memset (pos + 66, 0xFF, 8);
    
    pos += 8 + 276 + 12;

    /* Member Data Record (contains the load module), Block Descriptor Word is stripped. */
    pos -= 4;
    /* Record Descriptor Word,
     * length of Member Data Record including this field.*/
    bytearray_write_2_bytes (pos + 4, file_size - (pos + 4 - file), BIG_ENDIAN);

    pos += 8;
    
    /* Member Data. */
    /* Flag, 0x00 is member data. */
    pos[0] = 0x00;
    /* Count field, 8 bytes, CCHHRKDD,
     * CC is sector count, R is sector number,
     * K (key) is unused, DD is data length (size of sector).
     */
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = current_sector++;
    bytearray_write_2_bytes (pos + 4 + 6, cesd_size, BIG_ENDIAN);
    pos += 12;
    
    /* The load module, CESD record. */
    pos[0] = 0x20; /* Identification, CESD. */
    pos[1] = 0; /* Flag, use segment numbers in CESD data. */
    bytearray_write_2_bytes (pos + 4,
                             all_sections ? all_sections->target_index : 0,
                             BIG_ENDIAN); /* ESDID of the first ESD item. */
    bytearray_write_2_bytes (pos + 6, 16, BIG_ENDIAN); /* ESD data size. */

    pos += 8;

    /* CESD data, something like section table. */
    for (section = all_sections; section; section = section->next) {
        memset (pos, tebc (' '), 8); /* Empty 8 bytes long name. */
        pos[8] = 0x4; /* Type, Private Code. */
        bytearray_write_3_bytes (pos + 9, section->rva, BIG_ENDIAN); /* 24 bit Address. */
        pos[12] = 1; /* Segment number (1-based section index?). */
        bytearray_write_3_bytes (pos + 13, section->total_size, BIG_ENDIAN); /* 24 bit Length. */
        pos += 16;
    }

    /* Member Data. */
    pos[0] = 0x00;
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = current_sector++;
    bytearray_write_2_bytes (pos + 4 + 6, csect_hmaspzap_size, BIG_ENDIAN);
    pos += 12;

    /* CSECT HMASPZAP. */
    pos[0] = 0x80; /* Identification, CSECT. */
    pos[1] = csect_hmaspzap_size - 1; /* Byte count of IDR data, including this field. */
    pos[2] = 0x1; /* Sub-Type Indicator, should be 0x1, data supplied by HMASPZAP. */
    memset (pos + 3, 0, csect_hmaspzap_size - 3); /* 0xF8 zeros. */
    pos += csect_hmaspzap_size;

    /* Member Data. */
    pos[0] = 0x00;
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = current_sector++;
    bytearray_write_2_bytes (pos + 4 + 6, csect_linkage_size, BIG_ENDIAN);
    pos += 12;

    /* CSECT Linkage Editor data. */
    pos[0] = 0x80; /* Identification, CSECT. */
    pos[1] = csect_linkage_size - 1; /* Byte count of IDR data, including this field. */
    pos[2] = 0x2; /* Sub-Type Indicator, should be 0x2, Linkage Editor data. */
    memcpy (pos + 3, LINKAGE_EDITOR_DATA, sizeof (LINKAGE_EDITOR_DATA) - 1);
    pos += csect_linkage_size;

    /* Member Data. */
    pos[0] = 0x00;
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = current_sector++;
    bytearray_write_2_bytes (pos + 4 + 6, csect_user_size, BIG_ENDIAN);
    pos += 12;

    /* CSECT User-supplied data. */
    pos[0] = 0x80; /* Identification, CSECT. */
    pos[1] = csect_user_size - 1; /* Byte count of IDR data, including this field. */
    pos[2] = 0x4 | 0x80; /* Sub-Type Indicator, should be 0x4, User-supplied data, 0x80 means it is last IDR. */
    memcpy (pos + 3, USER_SUPPLIED_DATA, sizeof (USER_SUPPLIED_DATA) - 1);
    pos += csect_user_size;

    /* Member Data. */
    pos[0] = 0x00;
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = current_sector++;
    bytearray_write_2_bytes (pos + 4 + 6, control_record_size, BIG_ENDIAN);
    pos += 12;

    /* Control Record. */
    pos[0] = 0x1 | 0xC; /* Identification, 0x1 means Control Record, 0xC means it precedes last text record of the module. */
    bytearray_write_2_bytes (pos + 4, control_record_size - 16, BIG_ENDIAN); /* Size of control data. */
    /* Channel Command Word, unknown meaning, just copied. */
    pos[8] = 0x06;
    pos[8 + 4] = 0x40;
    pos[8 + 7] = 0x20;
    pos += 16;
    for (section = all_sections; section; section = section->next) {
        /* CESD entry number (1-based section index) and section size (rounded up to 16 bytes). */
        bytearray_write_2_bytes (pos, section->target_index, BIG_ENDIAN);
        bytearray_write_2_bytes (pos + 2, ALIGN (section->total_size, 16), BIG_ENDIAN);
        pos += 4;
    }    

    for (section = all_sections; section; section = section->next) {
        if (section->is_bss) continue;

        /* Member Data. */
        pos[0] = 0x00;
        bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
        pos[4 + 4] = current_sector++;
        bytearray_write_2_bytes (pos + 4 + 6, ALIGN (section->total_size, 16), BIG_ENDIAN);
        pos += 12;
        
        section_write (section, pos);
        pos += ALIGN (section->total_size, 16);
    }

    /* Member Data Record EOF. */
    pos[5] = 0x1;
    pos[8] = 0x1C;

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("corrupted input file"); } while (0)

static int read_mainframe_object (unsigned char *file,
                                  size_t file_size,
                                  const char *filename)
{   
    unsigned char *record_pos;
    struct object_file *of;

    of = object_file_make (0, filename);

    for (record_pos = file; record_pos + 80 <= file + file_size; record_pos += 80) {
        char record_name[4] = {0};
        unsigned char *pos = record_pos;

        if (pos[0] != 0x2) {
            ld_error ("%s: first byte of record is not 0x2", filename);
            return 1;
        }
        pos++;

        record_name[0] = febc (pos[0]);
        record_name[1] = febc (pos[1]);
        record_name[2] = febc (pos[2]);
        pos += 3;

        if (strcmp (record_name, "ESD") == 0) {
            /* Do nothing for now. */
        } else if (strcmp (record_name, "TXT") == 0) {
            struct section *section;
            struct section_part *part;
            unsigned long address;
            unsigned short num_bytes, esd_ident;

            pos++;
            bytearray_read_3_bytes (&address, pos, BIG_ENDIAN);
            pos += 3;

            pos += 2;
            bytearray_read_2_bytes (&num_bytes, pos, BIG_ENDIAN);
            pos += 2;

            pos += 2;
            bytearray_read_2_bytes (&esd_ident, pos, BIG_ENDIAN);
            pos += 2;

            if (num_bytes > 56) {
                ld_error ("%s: invalid number of bytes in TXT record",
                          filename);
                return 1;
            }

            section = section_find_or_make (".text");
            section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_READONLY | SECTION_FLAG_CODE;
            part = section_part_new (section, of);

            part->content_size = num_bytes;
            part->content = xmalloc (part->content_size);
            memcpy (part->content, pos, part->content_size);
            pos += num_bytes;

            section_append_section_part (section, part);
        } else if (strcmp (record_name, "END") == 0) {
            /* Do nothing for now. */
        } else {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "%s: unsupported record '%s'",
                                         filename, record_name);
        }
    }
    
    return 0;
}

int mainframe_read (unsigned char *file, size_t file_size, const char *filename)
{
    CHECK_READ (file, 4);

    if (file[0] == 0x2
        && febc (file[1]) == 'E'
        && febc (file[2]) == 'S'
        && febc (file[3]) == 'D') {
        if (read_mainframe_object (file, file_size, filename)) return INPUT_FILE_ERROR;
        return INPUT_FILE_FINISHED;
    }

    return INPUT_FILE_UNRECOGNIZED;
}
