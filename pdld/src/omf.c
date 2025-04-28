/******************************************************************************
 * @file            omf.c
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
#include "omf.h"
#include "xmalloc.h"

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("%s: corrupted input file", filename); } while (0)

static int estimate (unsigned char *file,
                     size_t file_size,
                     const char *filename,
                     size_t *num_segments_p,
                     size_t *num_extdefs_p,
                     size_t *num_pubdefs_p,
                     size_t *num_lnames_p)
{
    unsigned char *pos;
    size_t num_segments = 0;
    size_t num_extdefs = 0;
    size_t num_pubdefs = 0;
    size_t num_lnames = 0;

    pos = file;

    while (pos < file + file_size) {
        unsigned char record_type;
        int big_fields;
        unsigned short record_len;
        
        CHECK_READ (file, 3);
        record_type = pos[0];
        big_fields = record_type & 1;
        record_type &= ~1;
        bytearray_read_2_bytes (&record_len, pos + 1, LITTLE_ENDIAN);
        CHECK_READ (file, record_len);
        {
            size_t i;
            unsigned char checksum = 0;

            for (i = 0; i < record_len + 3; i++) checksum += pos[i];
            if (checksum != 0) ld_warn ("%s: invalid checksum", filename);
        }
        pos += 3;

        if (record_type == RECORD_TYPE_EXTDEF) {
            unsigned char *extdef_name = pos;
            const unsigned char *end = pos + record_len - 1;
            
            while (extdef_name != end) {
                unsigned char extdef_name_len = extdef_name[0];
                if (extdef_name + 1 + extdef_name_len + 1 > end) {
                    ld_fatal_error ("%s: incorrect string length", filename);
                }
                num_extdefs++;
                extdef_name += 1 + extdef_name_len + 1;
            }
        } else if (record_type == RECORD_TYPE_PUBDEF) {
            unsigned char *pubdef_name = pos;
            const unsigned char *end = pos + record_len - 1;

            if (big_fields) {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "%s: big fields not supported for record %#x",
                                             filename, record_type);
            }
            
            while (pubdef_name != end) {
                unsigned char pubdef_name_len = pubdef_name[2];
                if (pubdef_name + 2 + 1 + pubdef_name_len + 3 > end) {
                    ld_fatal_error ("%s: incorrect string length", filename);
                }
                num_pubdefs++;
                pubdef_name += 2 + 1 + pubdef_name_len + 3;
            }
        } else if (record_type == RECORD_TYPE_LNAMES) {
            unsigned char *lname = pos;
            const unsigned char *end = pos + record_len - 1;
            
            while (lname != end) {
                unsigned char lname_len = lname[0];
                if (lname + 1 + lname_len > end) {
                    ld_fatal_error ("%s: incorrect string length", filename);
                }
                num_lnames++;
                lname += 1 + lname_len;
            }                
        } else if (record_type == RECORD_TYPE_SEGDEF) {
            num_segments++;
        }

        pos += record_len;
    }

    *num_segments_p = num_segments;
    *num_extdefs_p = num_extdefs;
    *num_pubdefs_p = num_pubdefs;
    *num_lnames_p = num_lnames;

    return 0;
}

static int read_omf_object (unsigned char *file,
                            size_t file_size,
                            const char *filename)
{
    size_t num_segments, i_segments;
    size_t num_extdefs, i_extdefs;
    size_t num_pubdefs, i_pubdefs;
    size_t num_lnames, i_lnames;
    char **lnames;
    
    unsigned char *pos;

    struct object_file *of;
    struct section_part **part_p_array;

    if (estimate (file, file_size, filename,
                  &num_segments, &num_extdefs, &num_pubdefs, &num_lnames)) return 1;

    /* All indexes are 1-based. */
    num_lnames++;
    lnames = xcalloc (num_lnames, sizeof *lnames);
    i_lnames = 1;

    /* The layout of symbol table here is:
     * empty symbol
     * section symbols
     * public symbols
     * undefined symbols
     */
    num_segments++;
    i_segments = 1;
    i_pubdefs = num_segments;
    i_extdefs = i_pubdefs + num_pubdefs;

    part_p_array = xmalloc (sizeof (*part_p_array) * num_segments);
    of = object_file_make (num_segments + num_pubdefs + num_extdefs, filename);

    pos = file;
    while (pos < file + file_size) {
        unsigned char record_type;
        int big_fields;
        unsigned short record_len;
        
        record_type = pos[0];
        big_fields = record_type & 1;
        record_type &= ~1;
        bytearray_read_2_bytes (&record_len, pos + 1, LITTLE_ENDIAN);
        pos += 3;

        if (record_type == RECORD_TYPE_THEADR) {
            /* 1 byte String length.
             * <variable> Name string.
             */
            unsigned char string_length = pos[0];
            if (string_length > record_len - 2) {
                ld_fatal_error ("%s: incorrect string length", filename);
            }
        } else if (record_type == RECORD_TYPE_COMENT) {
            /* Nothing needs to be done. */
        } else if (record_type == RECORD_TYPE_MODEND) {
            if (pos + record_len != file + file_size) {
                ld_fatal_error ("%s: MODEND record is not the last record");
            }
        } else if (record_type == RECORD_TYPE_EXTDEF) {
            unsigned char *extdef_name = pos;
            const unsigned char *end = pos + record_len - 1;
            
            while (extdef_name != end) {
                unsigned char extdef_name_len = extdef_name[0];
                struct symbol *symbol = of->symbol_array + i_extdefs++;
                
                symbol->name = xstrndup (extdef_name + 1, extdef_name_len);
                symbol->value = 0;
                symbol->part = NULL;
                symbol->section_number = UNDEFINED_SECTION_NUMBER;
                symbol_record_external_symbol (symbol);
                
                extdef_name += 1 + extdef_name_len + 1;
            }
        } else if (record_type == RECORD_TYPE_PUBDEF) {
            unsigned char *pubdef_name = pos;
            const unsigned char *end = pos + record_len - 1;
            
            while (pubdef_name != end) {
                unsigned char base_segment_index;
                unsigned char pubdef_name_len;
                unsigned short public_offset;
                struct symbol *symbol = of->symbol_array + i_pubdefs++;

                base_segment_index = pubdef_name[1];
                pubdef_name_len = pubdef_name[2];
                bytearray_read_2_bytes (&public_offset, pubdef_name + 3 + pubdef_name_len, LITTLE_ENDIAN);

                if (base_segment_index >= i_segments || !base_segment_index) {
                    ld_fatal_error ("%s: invalid base segment index", filename);
                }

                symbol->name = xstrndup (pubdef_name + 3, pubdef_name_len);
                symbol->value = public_offset;
                symbol->part = part_p_array[base_segment_index];
                symbol->section_number = base_segment_index;
                symbol_record_external_symbol (symbol);

                pubdef_name += 3 + pubdef_name_len + 3;
            }
        } else if (record_type == RECORD_TYPE_LNAMES) {
            unsigned char *lname = pos;
            const unsigned char *end = pos + record_len - 1;
            
            while (lname != end) {
                unsigned char lname_len = lname[0];
                lnames[i_lnames++] = xstrndup (lname + 1, lname_len);
                lname += 1 + lname_len;
            }                
        } else if (record_type == RECORD_TYPE_SEGDEF) {
            unsigned char attributes;
            unsigned long segment_len;
            unsigned short segment_name_index;
            unsigned short class_name_index;
            struct section *section;
            struct section_part *part;
            
            attributes = pos[0];
            if (!(attributes & SEGMENT_ATTR_P)) {
                ld_fatal_error ("%s: OMF Use16 segments are not supported", filename);
            }

            if ((attributes >> 5) == 0) {
                ld_fatal_error ("%s: OMF absolute segments are not supported", filename);
            }
            
            if (big_fields) {
                bytearray_read_4_bytes (&segment_len, pos + 1, LITTLE_ENDIAN);
                segment_name_index = pos[5];
                class_name_index = pos[6];
            } else {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "%s: non-big fields not supported for record %#x",
                                             filename, record_type);
            }

            if (segment_name_index >= i_lnames || !segment_name_index) {
                ld_fatal_error ("%s: invalid segment name index", filename);
            }

            if (class_name_index >= i_lnames || !class_name_index) {
                ld_fatal_error ("%s: invalid class name index", filename);
            }

            section = section_find_or_make (lnames[segment_name_index]);
            
            if (strcmp (lnames[class_name_index], "CODE") == 0) {
                section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_READONLY | SECTION_FLAG_CODE;
            } else if (strcmp (lnames[class_name_index], "DATA") == 0) {
                section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_DATA;
            } else {
                section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_DATA;
            }

            part = section_part_new (section, of);

            switch (attributes >> 5) {
                case 1: part->alignment = 1; break;
                case 2: part->alignment = 2; break;
                case 3: part->alignment = 16; break;
                case 4: part->alignment = 4096; break;
                case 5: part->alignment = 4; break;
                
                default:
                    ld_fatal_error ("%s: invalid segment alignment", filename);
                    break;
            }
            
            part->content_size = segment_len;
            part->content = xmalloc (part->content_size);

            {
                struct symbol *symbol = of->symbol_array + i_segments;
                
                symbol->name = xstrdup (section->name);
                symbol->flags = SYMBOL_FLAG_SECTION_SYMBOL;
                symbol->value = 0;
                symbol->size = 0;
                symbol->part = part;
                symbol->section_number = i_segments;
            }

            section_append_section_part (section, part);
            part_p_array[i_segments++] = part;
        } else if (record_type == RECORD_TYPE_GRPDEF) {
            /* Nothing needs to be done. */
        } else if (record_type == RECORD_TYPE_FIXUPP) {
            unsigned char *subrec = pos;
            
            if (!big_fields) {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "%s: non-big fields not supported for record %#x",
                                             filename, record_type);
            }

            if (subrec[0] & 0x80) {
                if (!(subrec[0] & 0x40) /* self-relative */
                    && (((subrec[0] >> 2) & 0xf) == 9) /* 32-bit offset */) {
                    ld_warn ("%s: OMF relocations are not yet supported", filename);
                } else {
                    ld_internal_error_at_source (__FILE__, __LINE__,
                                                 "%s: unsupported FIXUP",
                                                 filename);
                }
            } else {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "%s: THREAD subrecords are not yet supported",
                                             filename);
            }
        } else if (record_type == RECORD_TYPE_LEDATA) {
            unsigned char seg_index;
            unsigned long offset;
            unsigned short size;
            const unsigned char *data_bytes;
            struct section_part *part;

            seg_index = pos[0];
            if (big_fields) {
                bytearray_read_4_bytes (&offset, pos + 1, LITTLE_ENDIAN);
                size = record_len - 6;
                data_bytes = pos + 5;
            } else {
                unsigned short field2;
                bytearray_read_2_bytes (&field2, pos + 1, LITTLE_ENDIAN);
                offset = field2;
                size = record_len - 4;
                data_bytes = pos + 3;
            }

            if (seg_index >= i_segments || !seg_index) {
                ld_fatal_error ("%s: invalid seg index", filename);
            }
            part = part_p_array[seg_index];

            if (offset + size > part->content_size) {
                ld_fatal_error ("%s: invalid LEDATA offset and size combination for segment", filename);
            }

            memcpy (part->content + offset, data_bytes, size);
        } else {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "%s: not yet supported record type %#x",
                                         filename, record_type | big_fields);
        }
        
        pos += record_len;
    }

    for (i_lnames = 0; i_lnames < num_lnames; i_lnames++) {
        free (lnames[i_lnames]);
    }
    free (lnames);

    free (part_p_array);

    return 0;
}        

int omf_read (unsigned char *file, size_t file_size, const char *filename)
{
    CHECK_READ (file, 3);
    if (file[0] == RECORD_TYPE_THEADR) {
        if (read_omf_object (file, file_size, filename)) return INPUT_FILE_ERROR;
        return INPUT_FILE_FINISHED;
    } else {
        return INPUT_FILE_UNRECOGNIZED;
    }

    return INPUT_FILE_FINISHED;
}
