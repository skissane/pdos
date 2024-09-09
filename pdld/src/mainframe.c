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
#include <ctype.h>

#include "ld.h"
#include "febc.h"
#include "tebc.h"
#include "bytearray.h"
#include "mainframe.h"
#include "xmalloc.h"

static int amode = 24;

int mainframe_get_amode (void)
{
    return amode;
}

#define DEFAULT_PART_ALIGNMENT 8

/* The following fields are faked for now
 * to make it easier to compare executables.
 */
#define LINKAGE_EDITOR_PROGRAM_NAME "5752SC104"
#define LINKAGE_EDITOR_VERSION 0x0308
#define LINKAGE_DATE 0x24162f /* YYDDD packed decimal. */

/* Hardcoded for now, not sure how to determine from object files. */
#define TRANSLATOR_PROGRAM_NAME "5741SC103"
#define TRANSLATOR_VERSION 0x0201
#define TRANSLATION_DATE LINKAGE_DATE

#define CHUNK_SIZE 6144
/* Member Data Record header size without Block Descriptor Word. */
#define MEMBER_DATA_RECORD_HEADER_SIZE (8 - 4)

/* Output CESD should not have duplicate symbols,
 * so they need to be marked after being seen.
 * (The symbols are output in the original order,
 *  so hashtab for_each() cannot be used for this. )
 */
#define SYMBOL_FLAG_INTERNAL1 (1U << 10)
#define SYMBOL_FLAG_INTERNAL2 (1U << 11)

address_type mvs_get_base_address (void)
{    
    return 0;
}

static unsigned char *write_member_data_record (unsigned char *pos,
                                                size_t size,
                                                unsigned char *current_sector_p,
                                                unsigned char **saved_pos_p)
{
    if (*saved_pos_p) {
        bytearray_write_2_bytes (*saved_pos_p + 4, pos - (*saved_pos_p + 4), BIG_ENDIAN);
    }

    if (size == 0) {
        /* End-of-file, so just update the current record. */
        *saved_pos_p = NULL;
        return pos;
    }
    
    /* Member Data Record (contains the load module), Block Descriptor Word is stripped. */
    pos -= 4;
    /* Record Descriptor Word,
     * length of Member Data Record including this field.
     *
     * While it is possible to calculate the length in advance,
     * saving the position and updating the field later is simpler. 
     */
    *saved_pos_p = pos;
    pos += 8;
    
    /* Member Data. */
    /* Flag, 0x00 is member data. */
    pos[0] = 0x00;
    /* Count field, 8 bytes, CCHHRKDD,
     * CC is sector count, R is sector number,
     * K (key) is unused, DD is data length (size of sector).
     */
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = (*current_sector_p)++;
    bytearray_write_2_bytes (pos + 4 + 6, size, BIG_ENDIAN);
    pos += 12;

    return pos;
}

void mvs_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct section *section;
    struct object_file *of;
    size_t cesd_i;
    size_t cesd_size;
    size_t csect_hmaspzap_size, csect_linkage_size, csect_translator_size;
    size_t control_record_size;
    size_t total_section_size_to_write = 0;
    size_t num_section_symbols = 0;
    size_t num_relocs = 0;
    unsigned char current_sector = 0x16; /* Arbitrary? */
    unsigned char *saved_pos = NULL;

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }
    
    {
        file_size = 0;

        file_size += SIZEOF_struct_COPYR1_file - 4;
        file_size += SIZEOF_struct_COPYR2_file - 4;
        file_size += 8 + 276 + 12 - 4; /* Directory Block Record. */
        file_size += MEMBER_DATA_RECORD_HEADER_SIZE;

        /* Load module, the program itself. */
        /* CESD Records (symbol table). */
        cesd_i = 1;
        for (of = all_object_files; of; of = of->next) {
            size_t i;

            for (i = 1; i < of->symbol_count; i++) {
                struct symbol *symbol = of->symbol_array + i;

                if (!symbol->name && symbol->section_number != i) continue;
                if (symbol_is_undefined (symbol)) {
                    symbol = mainframe_symbol_find (symbol->name);
                }
                if (symbol->flags & SYMBOL_FLAG_INTERNAL1) continue;

                symbol->flags |= SYMBOL_FLAG_INTERNAL1;
                if (symbol->section_number == i) {
                    /* Symbols referencing section symbols
                     * need to know the CESD symbol index.
                     */
                    symbol->value = cesd_i;
                    if (symbol->part->section->target_index == 0) {
                        symbol->part->section->target_index = cesd_i;
                    }
                    num_section_symbols++;
                } else {
                    /* Relocation Dictionary Record uses CESD symbol index
                     * to refer to target symbols, so the index needs to be stored
                     * and symbol size field works for that purpose. 
                     */
                    symbol->size = cesd_i;
                }
                cesd_i++;
            }
        }

        /* One CESD Record can have at most 248 bytes:
         * 8 bytes of header + 240 bytes of ESD Data.
         * One symbol takes up 16 bytes of ESD Data,
         * so one CESD Record stores at most 15 symbols (15 * 16 = 240)
         * and if there are more symbols than 15,
         * multiple CESD Records must be used.
         */
        file_size += ((cesd_i - 1) / 15 + !!((cesd_i - 1) % 15)) * (SIZEOF_struct_member_data_header_file
                                                                    + MEMBER_DATA_RECORD_HEADER_SIZE);
        cesd_size = ((cesd_i - 1) / 15 + !!((cesd_i - 1) % 15)) * 8;
        cesd_size += 16 * (cesd_i - 1);
        file_size += cesd_size;

        file_size += SIZEOF_struct_member_data_header_file;
        file_size += csect_hmaspzap_size = 3 + 0xF8;
        file_size += SIZEOF_struct_member_data_header_file;
        file_size += csect_linkage_size = 3 + SIZEOF_struct_linkage_editor_data_file;
        file_size += SIZEOF_struct_member_data_header_file;
        file_size += csect_translator_size = 3 + num_section_symbols * 2 + 1 + SIZEOF_struct_translator_description_file;

        file_size += SIZEOF_struct_member_data_header_file;
        file_size += control_record_size = 16 + num_section_symbols * 4;

        for (section = all_sections; section; section = section->next) {
            if (section->is_bss) continue;

            file_size += (((ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT) / CHUNK_SIZE
                           + !!(ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT) % CHUNK_SIZE)) - 1)
                          * (SIZEOF_struct_member_data_header_file
                             + 20 /* Dictionary Record. */));
            file_size += ((ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT) / CHUNK_SIZE
                           + !!(ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT) % CHUNK_SIZE))
                          * (SIZEOF_struct_member_data_header_file
                             + MEMBER_DATA_RECORD_HEADER_SIZE));
            total_section_size_to_write += ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT);
        }
        
        file_size += total_section_size_to_write;

        for (section = all_sections; section; section = section->next) {
            struct section_part *part;
    
            for (part = section->first_part; part; part = part->next) {
                num_relocs += part->relocation_count;
            }
        }

        if (num_relocs) {
            /* One RLD Record can have at most 256 bytes:
             * 16 bytes of header + 240 bytes od RLD Data.
             * One relocation takes up 8 bytes,
             * so one RLD Record stores at most 30 relocations (30 * 8 = 240).
             */
            file_size += (num_relocs / 30 + !!(num_relocs % 30)) * (SIZEOF_struct_member_data_header_file
                                                                    + MEMBER_DATA_RECORD_HEADER_SIZE);
            file_size += (num_relocs / 30 + !!(num_relocs % 30)) * 16;
            file_size += 8 * num_relocs;
        }

        /* Member Data Record EOF. */
        file_size += SIZEOF_struct_member_data_header_file;
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

    /* Last 16 bytes of basic section of Data Extent Block (DEB),
     * highly device dependent, so just copied from working executable.
     */
    pos[8] = 0x01; /* debnmext */
    bytearray_write_3_bytes (pos + 8 + 1, 0x0, BIG_ENDIAN); /* debusrpb */
    pos[8 + 4] = 0xff; /* debprotg */
    bytearray_write_3_bytes (pos + 8 + 5, 0x0, BIG_ENDIAN); /* debdcbd */
    pos[8 + 8] = 0x8f; /* debexscl */
    bytearray_write_3_bytes (pos + 8 + 9, 0x0b6644, BIG_ENDIAN); /* debappb */
    /* The following field (debucbad) is an irrelevant memory address
     * which randomly changes, so it must be ignored when comparing executables.
     */
    bytearray_write_4_bytes (pos + 8 + 12, 0x049470e8, BIG_ENDIAN); /* debucbad */
    
    /* First 16 extent descriptors from original DEB,
     * highly device dependent, so just copied from working executable.
     */
    pos[24] = 0x50; /* debdvmod31 */
    pos[24 + 1] = 0x00; /* debhmtrkhi */
    bytearray_write_2_bytes (pos + 24 + 2, 0x72a0, BIG_ENDIAN); /* debstrcc */
    bytearray_write_2_bytes (pos + 24 + 4, 0, BIG_ENDIAN); /* debstrhh */
    bytearray_write_2_bytes (pos + 24 + 6, 0x1, BIG_ENDIAN); /* debendcc */
    bytearray_write_2_bytes (pos + 24 + 8, 0, BIG_ENDIAN); /* debendhh */
    bytearray_write_2_bytes (pos + 24 + 10, 0x0a, BIG_ENDIAN); /* debnmtrk */

    bytearray_write_4_bytes (pos + 24 + 12, 0x001d012c, BIG_ENDIAN); /* unknown */

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
    /* 8 bytes long EBDIC uppercase program name. */
    {
        int i;

        memset (pos + 30, tebc (' '), 8);
        for (i = 0; i < 8 && ld_state->output_filename[i]; i++) {
            if (i > 0 && ld_state->output_filename[i] == '.') break;
            pos[30 + i] = tebc (toupper (ld_state->output_filename[i]));
        }
    }

    /* PDS member entry. */
    /* 4 unknown bytes. */
    memcpy (pos + 38,
            "\x00\x00\x16\x2C",
            4);
    /* PDS2TTRT - TTR OF FIRST BLOCK OF TEXT
     * (Unknown meaning.)
     */
    bytearray_write_3_bytes (pos + 38 + 4, 0x1b, BIG_ENDIAN);
    /* PDS2ZERO - ZERO */
    pos[38 + 7] = 0;
    /* PDS2TTRN - TTR OF NOTE LIST OR SCATTER/TRANSLATION TABLE.
     * USED FOR MODULES IN SCATTER LOAD FORMAT OR OVERLAY STRUCTURE ONLY.
     * (Scatter format is not used here at all, so set to 0.)
     */
    bytearray_write_3_bytes (pos + 38 + 8, 0, BIG_ENDIAN);
    /* PDS2NL - NUMBER OF ENTRIES IN NOTE LIST
     * FOR MODULES IN OVERLAY STRUCTURE, OTHERWISE ZERO
     */
    pos[38 + 11] = 0;

    /* PDS2ATR1 */
    pos[38 + 12] = PDS2EXEC;
    if (!num_relocs) pos[38 + 12] |= PDS21BLK;

    /* PDS2ATR2 */
    pos[38 + 13] = PDS2FLVL | PDS2ORG0 | PDS2LEF;
    if (ld_state->entry_point == 0) pos[38 + 13] |= PDS2EP0;
    if (!num_relocs) pos[38 + 13] |= PDS2NRLD;

    /* PDS2STOR - TOTAL CONTIGUOUS MAIN STORAGE REQUIREMENT OF MODULE */
    bytearray_write_3_bytes (pos + 38 + 14, total_section_size_to_write, BIG_ENDIAN);
    /* PDS2FTBL - LENGTH OF FIRST BLOCK OF TEXT */
    bytearray_write_2_bytes (pos + 38 + 17,
                             total_section_size_to_write > CHUNK_SIZE ? CHUNK_SIZE : total_section_size_to_write,
                             BIG_ENDIAN);
    /* PDS2EPA - 24 bit entry point. */
    bytearray_write_3_bytes (pos + 57, ld_state->entry_point, BIG_ENDIAN);
    /* Unknown. */
    pos[60] = 0x88;
    pos[63] = 0x01;
    /* Again 8 bytes of 0xFF, unknown meaning. */
    memset (pos + 66, 0xFF, 8);
    
    pos += 8 + 276 + 12;
    
    /* The load module, CESD Records (symbol table). */
    cesd_i = 1;
    for (of = all_object_files; of; of = of->next) {
        size_t i;

        for (i = 1; i < of->symbol_count; i++) {
            struct symbol *symbol = of->symbol_array + i;

            if (!symbol->name && symbol->section_number != i) continue;
            if (symbol_is_undefined (symbol)) {
                symbol = mainframe_symbol_find (symbol->name);
            }
            if (symbol->flags & SYMBOL_FLAG_INTERNAL2) continue;

            if ((cesd_i - 1) % 15 == 0) {
                size_t this_size = cesd_size > 248 ? 248 : cesd_size;

                pos = write_member_data_record (pos, this_size, &current_sector, &saved_pos);
                
                /* CESD Record. */
                pos[0] = 0x20; /* Identification, CESD. */
                pos[1] = 0; /* Flag, use segment numbers in CESD data. */
                bytearray_write_2_bytes (pos + 4,
                                         cesd_i,
                                         BIG_ENDIAN); /* ESDID of the first ESD item in this record. */
                bytearray_write_2_bytes (pos + 6, this_size - 8, BIG_ENDIAN); /* ESD data size. */

                cesd_size -= this_size;
                pos += 8;
            }

            symbol->flags |= SYMBOL_FLAG_INTERNAL2;
            if (symbol->section_number == i) {
                /* Adjusted original section part symbols (so one section can have multiple of them). */
                memset (pos, tebc (' '), 8); /* Empty 8 bytes long name. */
                pos[8] = 0x4; /* Type, Private Code. */
                bytearray_write_3_bytes (pos + 9, symbol->part->rva, BIG_ENDIAN); /* 24 bit Address. */
                pos[12] = symbol->part->section->target_index; /* Segment number (1-based section index). */
                bytearray_write_3_bytes (pos + 13, symbol->size, BIG_ENDIAN); /* 24 bit (original) Length. */
            } else {
                /* 8 bytes long EBDIC symbol name. */
                {
                    int j;

                    memset (pos, tebc (' '), 8);
                    for (j = 0; j < 8 && symbol->name[j]; j++) {
                        pos[j] = tebc (symbol->name[j]);
                    }
                }
                pos[8] = 0x3; /* Type, Label Reference. */
                if (symbol->part) {
                    bytearray_write_3_bytes (pos + 9, symbol->part->rva, BIG_ENDIAN); /* 24 bit Address. */
                    pos[12] = symbol->part->section->target_index; /* Segment number (1-based section index). */
                }
                /* Supposedly 2 byte ID for LR but it is actually 3 bytes.
                 * CESD index of section symbol from the object module
                 * in which was this symbol defined.
                 */
                if (symbol->part) {
                    struct symbol *section_symbol;

                    section_symbol = symbol->part->of->symbol_array + symbol->section_number;
                    bytearray_write_3_bytes (pos + 13, section_symbol->value, BIG_ENDIAN);
                }
            }
            cesd_i++;
            pos += 16;
        }
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
    pos += 3;
    {
        int i;

        memset (pos, tebc (' '), 10);
        for (i = 0; i < 10 && LINKAGE_EDITOR_PROGRAM_NAME[i]; i++) {
            pos[i] = tebc (LINKAGE_EDITOR_PROGRAM_NAME[i]);
        }
    }
    bytearray_write_2_bytes (pos + 10, LINKAGE_EDITOR_VERSION, BIG_ENDIAN);
    bytearray_write_3_bytes (pos + 12, LINKAGE_DATE, BIG_ENDIAN);
    pos += SIZEOF_struct_linkage_editor_data_file;

    /* Member Data. */
    pos[0] = 0x00;
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = current_sector++;
    bytearray_write_2_bytes (pos + 4 + 6, csect_translator_size, BIG_ENDIAN);
    pos += 12;

    /* CSECT Translator-supplied data. */
    pos[0] = 0x80; /* Identification, CSECT. */
    pos[1] = csect_translator_size - 1; /* Byte count of IDR data, including this field. */
    pos[2] = 0x4 | 0x80; /* Sub-Type Indicator, should be 0x4, Translator-supplied data, 0x80 means it is last IDR. */
    pos += 3;
    /* Array of 2 byte long section symbol indexes to which the data applies. */
    for (of = all_object_files; of; of = of->next) {
        size_t i;

        for (i = 1; i < of->symbol_count; i++) {
            struct symbol *symbol = of->symbol_array + i;
            if (symbol->section_number == i) {
                bytearray_write_2_bytes (pos, symbol->value, BIG_ENDIAN);
                pos += 2;
            }
        }
    }
    /* 0x8000 marks the last index. */
    if (num_section_symbols) pos[-2] |= 0x80;
    pos[0] = 0x0; /* One translator description_follows. */
    pos++;

    {
        int i;

        memset (pos, tebc (' '), 10);
        for (i = 0; i < 10 && TRANSLATOR_PROGRAM_NAME[i]; i++) {
            pos[i] = tebc (TRANSLATOR_PROGRAM_NAME[i]);
        }
    }
    bytearray_write_2_bytes (pos + 10, TRANSLATOR_VERSION, BIG_ENDIAN);
    bytearray_write_3_bytes (pos + 12, TRANSLATION_DATE, BIG_ENDIAN);
    pos += SIZEOF_struct_translator_description_file;

    /* Member Data. */
    pos[0] = 0x00;
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = current_sector++;
    bytearray_write_2_bytes (pos + 4 + 6, control_record_size, BIG_ENDIAN);
    pos += 12;

    /* Control Record. */
    pos[0] = 0x1 ; /* Identification, 0x1 means Control Record. */
    if (!num_relocs) {
        /* 0xC means it precedes last text record of the module (EOM = end of module).
         * This should be set if there are no other records in the file after this one
         * but if Relocation Dictionary Records exist, they are present later on,
         * so this should not be set in that case.
         */
        pos[0] |= 0xC;
    }
    bytearray_write_2_bytes (pos + 4, control_record_size - 16, BIG_ENDIAN); /* Size of control data. */
    /* Channel Command Word,
     * 8-bit channel Command Code (should be 0x6),
     * 24-bit address,
     * 8-bit flag (should be 0x40),
     * 24-bit count.
     */
    pos[8] = 0x06;
    bytearray_write_3_bytes (pos + 8 + 1, all_sections ? all_sections->rva : 0, BIG_ENDIAN);
    pos[8 + 4] = 0x40;
    bytearray_write_3_bytes (pos + 8 + 5,
                             all_sections
                             ? (ALIGN (all_sections->total_size, DEFAULT_PART_ALIGNMENT) > CHUNK_SIZE
                                ? CHUNK_SIZE
                                : ALIGN (all_sections->total_size, DEFAULT_PART_ALIGNMENT))
                             : 0,
                             BIG_ENDIAN);
    pos += 16;
    /* CESD section symbol indexes and original section sizes (rounded up to 8 bytes). */
    for (of = all_object_files; of; of = of->next) {
        size_t i;

        for (i = 1; i < of->symbol_count; i++) {
            struct symbol *symbol = of->symbol_array + i;
            if (symbol->section_number == i) {
                bytearray_write_2_bytes (pos, symbol->value, BIG_ENDIAN);
                bytearray_write_2_bytes (pos + 2, ALIGN (symbol->size, DEFAULT_PART_ALIGNMENT), BIG_ENDIAN);
                pos += 4;
            }
        }
    }

    for (section = all_sections; section; section = section->next) {
        unsigned char *tmp;
        size_t section_size;
        
        if (section->is_bss) continue;

        tmp = xcalloc (ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT), 1);
        section_write (section, tmp);

        section_size = ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT);
        while (section_size) {
            size_t this_size = section_size > CHUNK_SIZE ? CHUNK_SIZE : section_size;

            if (section_size == ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT)) {
                pos = write_member_data_record (pos, this_size, &current_sector, &saved_pos);
            } else {
                pos = write_member_data_record (pos, 20, &current_sector, &saved_pos);

                /* Dictionary Record.
                 *
                 * More complex Relocation Dictionary record
                 * which signals that the following record is a text record
                 * (necessary for all text records except the first one
                 *  because the first one has Control Record).
                 * It is simpler to have all relocations in one place after all text records,
                 * so the relocation data are not used here.
                 */
                pos[0] = 0x3; /* Identification, 0x3 means Dictionary Record. */
                if (!num_relocs && this_size == section_size) {
                    pos[0] |= 0xC; /* There are no more records in the file after the following text record. */
                }
                bytearray_write_2_bytes (pos + 4, 4, BIG_ENDIAN); /* Count, in bytes, of control information after relocation data. */
                bytearray_write_2_bytes (pos + 6, 0, BIG_ENDIAN); /* Count-in bytes of relocation data. */
                /* Channel Command Word,
                 * 8-bit channel Command Code (should be 0x6),
                 * 24-bit address,
                 * 8-bit flag (should be 0x40),
                 * 24-bit count.
                 */
                pos[8] = 0x06;
                bytearray_write_3_bytes (pos + 8 + 1,
                                         section->rva + ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT) - section_size,
                                         BIG_ENDIAN);
                pos[8 + 4] = 0x40;
                bytearray_write_3_bytes (pos + 8 + 5,
                                         this_size,
                                         BIG_ENDIAN);
                pos += 16;

                bytearray_write_2_bytes (pos, section->target_index, BIG_ENDIAN); /* CESD index of the section. */
                bytearray_write_2_bytes (pos + 2, this_size, BIG_ENDIAN); /* Length of the following text record. */
                pos += 4;

                /* Member Data. */
                pos[0] = 0x00;
                bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
                pos[4 + 4] = current_sector++;
                bytearray_write_2_bytes (pos + 4 + 6, this_size, BIG_ENDIAN);
                pos += 12;
            }

            memcpy (pos,
                    tmp + ALIGN (section->total_size, DEFAULT_PART_ALIGNMENT) - section_size,
                    this_size);
            section_size -= this_size;
            pos += this_size;
        }

        free (tmp);
    }
    
    if (num_relocs) {
        size_t reloc_i = 1;

        for (section = all_sections; section; section = section->next) {
            struct section_part *part;
    
            for (part = section->first_part; part; part = part->next) {
                size_t i;
        
                for (i = 0; i < part->relocation_count; i++) {
                    struct reloc_entry *reloc;
                    struct symbol *target_symbol;

                    if ((reloc_i - 1) % 30 == 0) {
                        size_t this_relocs = num_relocs > 30 ? 30 : num_relocs;

                        pos = write_member_data_record (pos, 16 + this_relocs * 8, &current_sector, &saved_pos);

                        /* Relocation Dictionary Record. */
                        pos[0] = 0x2; /* Identification, 0x2 means Relocation Dictionary Record. */
                        if (num_relocs == this_relocs) {
                            pos[0] |= 0xC; /* Last record in the file. */
                        }
                        bytearray_write_2_bytes (pos + 6, this_relocs * 8, BIG_ENDIAN); /* Count-in bytes of relocation data. */
                        num_relocs -= this_relocs;
                        pos += 16;
                    }

                    reloc = part->relocation_array + i;
                    target_symbol = reloc->symbol;
                    if (symbol_is_undefined (target_symbol)) {
                        target_symbol = mainframe_symbol_find (target_symbol->name);
                    }
                    /* CESD index for target symbol (obtained from repurposed symbol size field). */
                    if (target_symbol->part
                        && target_symbol->section_number == target_symbol - target_symbol->part->of->symbol_array) {
                        /* Section symbols have their CESD index stored in value field. */
                        bytearray_write_2_bytes (pos, target_symbol->value, BIG_ENDIAN);
                    } else {
                        bytearray_write_2_bytes (pos, target_symbol->size, BIG_ENDIAN);
                    }
                    /* CESD index for source section. */
                    bytearray_write_2_bytes (pos + 2, part->section->target_index, BIG_ENDIAN);
                    /* 24-bit address in section. */
                    bytearray_write_3_bytes (pos + 5, reloc->offset + part->rva - part->section->rva, BIG_ENDIAN);
                    if (reloc->howto == &reloc_howtos[RELOC_TYPE_32]) {
                        pos[4] = 0x1C;
                    } else {
                        ld_internal_error_at_source (__FILE__, __LINE__,
                                                     "%s cannot be converted to mainframe relocation",
                                                     reloc->howto->name);
                    }
                    
                    pos += 8;
                    reloc_i++;
                }
            }
        }
    }
    
    /* Member Data Record EOF, same as regular Member Data Header,
     * only with Data Length (size of sector) set to 0.
     */
    pos[0] = 0x00;
    bytearray_write_2_bytes (pos + 4, 1, BIG_ENDIAN);
    pos[4 + 4] = current_sector++;
    bytearray_write_2_bytes (pos + 4 + 6, 0, BIG_ENDIAN);
    pos += 12;

    pos = write_member_data_record (pos, 0, &current_sector, &saved_pos);

    /* Repurposed section symbol value field
     * must be restored so section symbols
     * have correct values when printing map.
     */
    for (of = all_object_files; of; of = of->next) {
        size_t i;

        for (i = 1; i < of->symbol_count; i++) {
            struct symbol *symbol = of->symbol_array + i;

            if (!symbol->name && symbol->section_number != i) continue;
            if (symbol_is_undefined (symbol)) continue;
            
            if (symbol->section_number == i) {
                symbol->value = 0;
            } 
        }
    }

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("corrupted input file"); } while (0)

static int estimate (unsigned char *file,
                     size_t file_size,
                     const char *filename,
                     size_t *num_symbols_p,
                     size_t *local_sym_i_p)
{
    unsigned char *record_pos;
    size_t num_symbols = 0;
    size_t num_local_symbols = 0;

    for (record_pos = file; record_pos + 80 <= file + file_size; record_pos += 80) {
        char record_name[4] = {0};
        unsigned char *pos = record_pos;

        if (pos[0] != 0x2) {
            ld_error ("%s: first byte of record is not 0x2", filename);
            return 1;
        }

        record_name[0] = febc (pos[1]);
        record_name[1] = febc (pos[2]);
        record_name[2] = febc (pos[3]);

        if (strcmp (record_name, "ESD") == 0) {
            unsigned short num_bytes, esdid;

            bytearray_read_2_bytes (&num_bytes, pos + 10, BIG_ENDIAN);
            bytearray_read_2_bytes (&esdid, pos + 14, BIG_ENDIAN);

            /* The ESD IDs do not have to be contiguous.
             * 0x4040 means two spaces in EBCDIC, blank,
             * so it should be ignored because all symbols
             * are LD type which does not use ESD IDs.
             */
            if (esdid != 0x4040 && esdid > num_symbols) num_symbols = esdid;
            
            if (num_bytes > 72 - 16) {
                ld_error ("%s: invalid number of bytes in ESD %u", filename, num_bytes);
                return 1;
            }
            
            for (pos += 16; pos + 16 <= record_pos + 16 + num_bytes; pos += 16) {
                /* Symbols with type LD do not have ESD ID and do not increment it. */
                if (pos[8] == ESD_DATA_TYPE_LD) num_local_symbols++;
                else num_symbols++;;
            }
        } else if (strcmp (record_name, "TXT") == 0) {
            /* Nothing needs to be done. */
        } else if (strcmp (record_name, "END") == 0) {
            break;
        } else if (strcmp (record_name, "RLD") == 0) {
            /* Nothing needs to be done. */
        } else {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "%s: unsupported record '%s'",
                                         filename, record_name);
        }
    }

    *num_symbols_p = num_symbols + 1 + num_local_symbols;
    *local_sym_i_p = num_symbols + 1;

    return 0;
}

static int read_mainframe_object (unsigned char *file,
                                  size_t file_size,
                                  const char *filename,
                                  unsigned char **end_pos_p)
{
    size_t num_symbols;
    
    unsigned char *record_pos;
    struct object_file *of;
    unsigned short esdid = 0;
    size_t local_sym_i;

    int ret = 1;

    /* It is impossible to know how many symbols
     * are there without parsing all ESDs,
     * so the file is parsed twice.
     */
    if (estimate (file, file_size, filename, &num_symbols, &local_sym_i)) return 1;

    if (ld_state->target_machine == LD_TARGET_MACHINE_MAINFRAME
        || ld_state->target_machine == LD_TARGET_MACHINE_UNKNOWN) {
        ld_state->target_machine = LD_TARGET_MACHINE_MAINFRAME;
    } else {
        ld_error ("%s: Mainframe format used but other objects are not for mainframe", filename);
        return 1;
    }

    of = object_file_make (num_symbols, filename);

    for (record_pos = file; record_pos + 80 <= file + file_size; record_pos += 80) {
        char record_name[4] = {0};
        unsigned char *pos = record_pos;

        if (pos[0] != 0x2) {
            ld_error ("%s: first byte of record is not 0x2", filename);
            return 1;
        }

        record_name[0] = febc (pos[1]);
        record_name[1] = febc (pos[2]);
        record_name[2] = febc (pos[3]);

        if (strcmp (record_name, "ESD") == 0) {
            unsigned short num_bytes, new_esdid;

            bytearray_read_2_bytes (&num_bytes, pos + 10, BIG_ENDIAN);
            bytearray_read_2_bytes (&new_esdid, pos + 14, BIG_ENDIAN);

            if (new_esdid != 0x4040) esdid = new_esdid;

            if (num_bytes > 72 - 16) {
                ld_error ("%s: invalid number of bytes in ESD %u", filename, num_bytes);
                return 1;
            }

            for (pos += 16; pos + 16 <= record_pos + 16 + num_bytes; pos += 16) {
                struct symbol *symbol;
                unsigned char type;
                char *name = NULL;

                if (esdid >= num_symbols) {
                    ld_internal_error_at_source (__FILE__, __LINE__,
                                         "%s: ESD ID %u exceeds num_symbols",
                                         filename, esdid);
                }
                
                type = pos[8];
                if (type == ESD_DATA_TYPE_LD) {
                    /* LD type symbols do not use ESD IDs
                     * and are never referenced by relocations in the same file
                     * because LD symbols are only used to expose symbols
                     * to other object files (they are defined external symbols).
                     * To prevent them from interfering with ESD ID system used
                     * for other symbols, they are stored in symbol array
                     * after all other symbols using separate index.
                     */
                    symbol = of->symbol_array + local_sym_i;
                } else {
                    symbol = of->symbol_array + esdid;
                }
                if (type == ESD_DATA_TYPE_LD
                    || type == ESD_DATA_TYPE_ER) {
                    int i;

                    name = xmalloc (9);

                    for (i = 0; i < 8 && pos[i]; i++) {
                        name[i] = febc (pos[i]);
                        if (name[i] == ' ') break;
                    }
                    name[i] = '\0';
                }

                symbol->name = name;

                if (type == ESD_DATA_TYPE_LD) {
                    unsigned long address;
                    unsigned long section_esdid;

                    bytearray_read_3_bytes (&address, pos + 9, BIG_ENDIAN);
                    bytearray_read_3_bytes (&section_esdid, pos + 13, BIG_ENDIAN);

                    if (section_esdid >= num_symbols) {
                        ld_internal_error_at_source (__FILE__, __LINE__,
                                                     "%s: ESD ID %u exceeds num_symbols",
                                                     filename, section_esdid);
                    }

                    symbol->part = of->symbol_array[section_esdid].part;
                    if (!symbol->part) {
                        ld_error ("%s: ESD Data item type LD has invalid SD identifier",
                                  filename);
                        return 1;
                    }

                    symbol->value = address;
                    symbol->size = 0;
                    symbol->section_number = section_esdid;
                    symbol_record_external_symbol (symbol);
                } else if (type == ESD_DATA_TYPE_ER) {
                    symbol->value = 0;
                    symbol->size = 0;
                    symbol->section_number = UNDEFINED_SECTION_NUMBER;
                    symbol->part = NULL;
                    symbol_record_external_symbol (symbol);
                } else if (type == ESD_DATA_TYPE_PC) {
                    struct section *section;
                    struct section_part *part;
                    unsigned long address;
                    unsigned long len;

                    bytearray_read_3_bytes (&address, pos + 9, BIG_ENDIAN);
                    bytearray_read_3_bytes (&len, pos + 13, BIG_ENDIAN);

                    section = section_find_or_make (".text");
                    section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_READONLY | SECTION_FLAG_CODE;
                    part = section_part_new (section, of);

                    part->alignment = DEFAULT_PART_ALIGNMENT;
                    part->content_size = len;
                    part->content = xcalloc (part->content_size, 1);

                    section_append_section_part (section, part);

                    symbol->name = xstrdup (".text");
                    symbol->value = 0;
                    /* Even though section parts seem to require 8-byte alignment,
                     * the original part size needs to be preserved for output CESD.
                     */
                    symbol->size = len;
                    symbol->section_number = esdid;
                    symbol->part = part;
                }

                if (type == ESD_DATA_TYPE_LD) local_sym_i++;
                else esdid++;
            }
        } else if (strcmp (record_name, "TXT") == 0) {
            struct section_part *part;
            unsigned long address;
            unsigned short num_bytes, esd_ident;

            bytearray_read_3_bytes (&address, pos + 5, BIG_ENDIAN);
            bytearray_read_2_bytes (&num_bytes, pos + 10, BIG_ENDIAN);
            bytearray_read_2_bytes (&esd_ident, pos + 14, BIG_ENDIAN);
            pos += 16;

            if (num_bytes > 56) {
                ld_error ("%s: invalid number of bytes in TXT record",
                          filename);
                return 1;
            }

            part = of->symbol_array[esd_ident].part;
            if (!part) {
                ld_error ("%s: Text record has invalid ESD identifier",
                          filename);
                return 1;
            }

            if (address + num_bytes > part->content_size) {
                ld_error ("%s: Text record address and number of bytes exceeds ESD-specified length",
                          filename);
                return 1;
            }
            
            memcpy (part->content + address, pos, num_bytes);
            pos += num_bytes;
        } else if (strcmp (record_name, "END") == 0) {
            ret = 0;
            record_pos += 80;
            break;
        } else if (strcmp (record_name, "RLD") == 0) {
            unsigned short num_bytes;
            unsigned short target_id, src_id;
            int ids_same_as_previous = 0;

            bytearray_read_2_bytes (&num_bytes, pos + 10, BIG_ENDIAN);

            if (num_bytes > 72 - 16) {
                ld_error ("%s: invalid number of bytes in RLD %u", filename, num_bytes);
                return 1;
            }
            
            for (pos += 16; pos + 8 <= record_pos + 16 + num_bytes; pos += 8) {
                unsigned char flags;
                unsigned long address;

                struct section_part *part;
                struct reloc_entry *reloc;

                if (!ids_same_as_previous) {
                    bytearray_read_2_bytes (&target_id, pos, BIG_ENDIAN);
                    bytearray_read_2_bytes (&src_id, pos + 2, BIG_ENDIAN);
                }
                flags = pos[4];
                bytearray_read_3_bytes (&address, pos + 5, BIG_ENDIAN);

                part = of->symbol_array[src_id].part;
                if (!part) {
                    ld_error ("%s: RLD record has invalid ESD identifier",
                              filename);
                    return 1;
                }

                part->relocation_count++;
                part->relocation_array = xrealloc (part->relocation_array,
                                                   part->relocation_count
                                                   * sizeof *part->relocation_array);
                memset (part->relocation_array + part->relocation_count - 1,
                        0,
                        1 * sizeof *part->relocation_array);

                reloc = part->relocation_array + part->relocation_count - 1;
                reloc->symbol = of->symbol_array + target_id;
                reloc->offset = address;
                reloc->addend = 0;

                if ((flags & 0x30) == 0x10
                    && (flags & 0xC) == 0xC) {
                    /* Adcon type V - absolute, 4 bytes. */
                    reloc->howto = &reloc_howtos[RELOC_TYPE_32];
                } else if ((flags & 0x30) == 0x0
                    && (flags & 0xC) == 0xC) {
                    /* Adcon type A - absolute, 4 bytes.
                     * The difference is that A does not allow external linkage
                     * but that is ignored here.
                     */
                    reloc->howto = &reloc_howtos[RELOC_TYPE_32];
                } else if ((flags & 0x30) == 0x0
                    && (flags & 0xC) == 0x8) {
                    /* Adcon type A - absolute, 3 bytes.
                     */
                    ld_warn ("%s: AL3 address '%s' encountered",
                             filename, reloc->symbol->name);
                    reloc->howto = &reloc_howtos[RELOC_TYPE_24];
                } else {
                    ld_internal_error_at_source (__FILE__, __LINE__,
                                                 "%s: unsupported relocation flags %#x",
                                                 filename, flags);
                }

                if (flags & 0x1) {
                    /* Relocation pointer and Position pointer (ESD IDs)
                     * for the next item are omitted because they are the same.
                     */
                    ids_same_as_previous = 1;
                    pos -= 4;
                } else {
                    ids_same_as_previous = 0;
                }
            }
        } else {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "%s: unsupported record '%s'",
                                         filename, record_name);
        }
    }

    if (ret) {
        ld_error ("%s: object module missing END record", filename);
    }

    *end_pos_p = record_pos;
    
    return ret;
}

int mainframe_read (unsigned char *file, size_t file_size, const char *filename)
{
    CHECK_READ (file, 4);

    if (file[0] == 0x2
        && febc (file[1]) == 'E'
        && febc (file[2]) == 'S'
        && febc (file[3]) == 'D') {
        /* Minframe object file can contain multiple object modules. */
        while (file_size >= 80) {
            unsigned char *new_pos;
            int ret = read_mainframe_object (file, file_size, filename, &new_pos);
            if (ret == 1) {
                return INPUT_FILE_ERROR;
            } else {
                file_size -= new_pos - file;
                file = new_pos;
                continue;
            }  
        }
        return INPUT_FILE_FINISHED;
    }

    return INPUT_FILE_UNRECOGNIZED;
}

struct symbol *mainframe_symbol_find (const char *name)
{
    /* External references for MVS object code are at most 8 characters long,
     * uppercase and use '@' instead of '_'.
     * On the other hand, (mainframe) ELF cares about case
     * and does not use '@' instead of '_'.
     * To make those two input formats compatible,
     * the code below converts MVS name
     * to uppercase ELF name and lowercase ELF name
     * and ELF name to uppercase MVS name and lowercase MVS name.
     *
     * Note that this method is not case insensitive.
     * Mixed case names cannot be matched one way
     * ("BIGOPEN" will not match "BigOpen" but "BigOpen" will match "BIGOPEN").
     */
    struct symbol *symbol;
    size_t i;
    char new_name[9] = {0};

    symbol = symbol_find (name);
    if (symbol && !symbol_is_undefined (symbol)) return symbol;
    if (strlen (name) > 8) return symbol_find (name);

    for (i = 0; i < 8 && name[i]; i++) {
        new_name[i] = toupper (name[i]);
        if (new_name[i] == '@') new_name[i] = '_';
        else if (new_name[i] == '_') new_name[i] = '@';
    }

    symbol = symbol_find (new_name);
    if (symbol && !symbol_is_undefined (symbol)) return symbol;

    for (i = 0; i < 8 && name[i]; i++) {
        new_name[i] = tolower (name[i]);
        if (new_name[i] == '@') new_name[i] = '_';
        else if (new_name[i] == '_') new_name[i] = '@';
    }

    symbol = symbol_find (new_name);
    if (symbol && !symbol_is_undefined (symbol)) return symbol;

    return symbol_find (name);
}

int mainframe_symbol_check_undefined (const char *name)
{
    /* This is more complicated than the above code
     * because it is used to check whether archive member
     * should be imported.
     * Any version of the symbol being defined returns 0
     * and none of the versions being found returns 0 too.
     * Only when all versions of the symbol are undefined or not found
     * and at least one version is found should return 1.
     */
    const struct symbol *symbol;
    size_t i;
    char new_name[9] = {0};
    int symbol_exists = 0;

    symbol = symbol_find (name);
    if (symbol && !symbol_is_undefined (symbol)) return 0;
    if (strlen (name) > 8) return 0;
    if (symbol) symbol_exists = 1;

    for (i = 0; i < 8 && name[i]; i++) {
        new_name[i] = toupper (name[i]);
        if (new_name[i] == '@') new_name[i] = '_';
        else if (new_name[i] == '_') new_name[i] = '@';
    }

    symbol = symbol_find (new_name);
    if (symbol && !symbol_is_undefined (symbol)) return 0;
    if (symbol) symbol_exists = 1;

    for (i = 0; i < 8 && name[i]; i++) {
        new_name[i] = tolower (name[i]);
        if (new_name[i] == '@') new_name[i] = '_';
        else if (new_name[i] == '_') new_name[i] = '@';
    }

    symbol = symbol_find (new_name);
    if (symbol && !symbol_is_undefined (symbol)) return 0;
    if (symbol) symbol_exists = 1;

    return symbol_exists;
}

#include "options.h"

enum option_index {

    MAINFRAME_OPTION_IGNORED = 0,
    MAINFRAME_OPTION_AMODE24,
    MAINFRAME_OPTION_AMODE31

};

#define STR_AND_LEN(str) (str), (sizeof (str) - 1)
static const struct long_option long_options[] = {
    
    { STR_AND_LEN("amode24"), MAINFRAME_OPTION_AMODE24, OPTION_NO_ARG},
    { STR_AND_LEN("amode31"), MAINFRAME_OPTION_AMODE31, OPTION_NO_ARG},
    { NULL, 0, 0}

};
#undef STR_AND_LEN

void mainframe_print_help (void)
{
    printf ("mainframe:\n");
    printf ("  --amode24, --amode31              Set AMODE24/AMODE31\n");
}

static void use_option (enum option_index option_index, char *arg)
{
    switch (option_index) {

        case MAINFRAME_OPTION_IGNORED:
            break;

        case MAINFRAME_OPTION_AMODE24:
            amode = 24;
            break;

        case MAINFRAME_OPTION_AMODE31:
            amode = 31;
            break;

    }
}

void mainframe_use_option (int option_index, char *arg)
{
    use_option (option_index, arg);
}

const struct long_option *mainframe_get_long_options (void)
{
    return long_options;
}
