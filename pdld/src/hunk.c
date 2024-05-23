/******************************************************************************
 * @file            hunk.c
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
#include <time.h>

#include "ld.h"
#include "bytearray.h"
#include "hunk.h"
#include "xmalloc.h"

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("corrupted input file"); } while (0)

static int estimate (unsigned char *file,
                     size_t file_size,
                     const char *filename,
                     size_t *num_hunks_p,
                     size_t *num_symbols_p)
{
    unsigned char *pos;
    size_t num_hunks = 0;
    size_t num_symbols = 0;
    
    {
        /* HUNK_UNIT contains the object name, so it needs to be skipped. */
        unsigned long name_size;
        
        pos = file + 4;
        CHECK_READ (pos, 4);

        bytearray_read_4_bytes (&name_size, pos, BIG_ENDIAN);
        pos += 4;
        name_size *= 4;
        
        CHECK_READ (pos, name_size);
        pos += name_size;
    }

    while (pos < file + file_size) {
        unsigned long type;

        CHECK_READ (pos, 4);
        bytearray_read_4_bytes (&type, pos, BIG_ENDIAN);
        pos += 4;

        if (type == HUNK_UNIT) break;

        if (type == HUNK_END) {
            num_hunks++;
            num_symbols++;
            continue;
        }

        if (type == HUNK_EXT) {
            while (1) {
                unsigned char symbol_type;
                unsigned long name_size;
                
                CHECK_READ (pos, 4);
                bytearray_read_4_bytes (&name_size, pos, BIG_ENDIAN);
                pos += 4;

                if (name_size == 0) break;
                
                symbol_type = (name_size >> 24) & 0xff;
                name_size &= 0xffffff;
                name_size *= 4;

                pos += name_size;

                if (symbol_type == EXT_DEF
                    || symbol_type == EXT_ABS) {
                    pos += 4;
                } else if (symbol_type == EXT_REF32
                           || symbol_type == EXT_COMMON
                           || symbol_type == EXT_RELREF32) {
                    unsigned long num_ref;

                    if (symbol_type == EXT_COMMON) {
                        CHECK_READ (pos, 4);
                        pos += 4;
                    }

                    CHECK_READ (pos, 4);
                    bytearray_read_4_bytes (&num_ref, pos, BIG_ENDIAN);
                    pos += 4;
                    
                    CHECK_READ (pos, num_ref * 4);
                    pos += num_ref * 4;
                } else {
                    ld_internal_error_at_source (__FILE__, __LINE__, "+++symbol type %lu unsupported", symbol_type);
                }

                num_symbols++;
            }
            
            continue;
        }

        switch (type) {
            case HUNK_NAME:
            case HUNK_CODE:
            case HUNK_DATA: {
                unsigned long size;
                
                CHECK_READ (pos, 4);
                bytearray_read_4_bytes (&size, pos, BIG_ENDIAN);
                size *= 4;
                pos += 4;

                pos += size;
                break;
            }
                
            case HUNK_BSS: {                
                CHECK_READ (pos, 4);
                pos += 4;

                break;
            }

            case HUNK_RELOC32: {                
                while (1) {
                    unsigned long size;
                    
                    CHECK_READ (pos, 4);
                    bytearray_read_4_bytes (&size, pos, BIG_ENDIAN);
                    size *= 4;
                    pos += 4;

                    if (size == 0) break;

                    CHECK_READ (pos, 4 + size);
                    pos += 4 + size;
                }

                break;
            }

            default:
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++unrecognized hunk %#lx", type);
                break;
        }
    }
    
    *num_hunks_p = num_hunks;
    *num_symbols_p = num_symbols;

    return 0;
}            

static int read_hunk_object (unsigned char *file,
                             size_t file_size,
                             const char *filename,
                             unsigned char **end_pos_p)
{
    char *hunk_filename;
    size_t num_hunks;
    size_t num_symbols;
    size_t symbol_i;
    
    unsigned char *pos;

    struct object_file *of;
    struct section *section = NULL;
    struct section_part *part = NULL;
    char *section_name = NULL;
    size_t section_i = 0;
    int ret = 0;
    struct section *bss_section = NULL;

    /* It is impossible to know how many sections (hunks)
     * and symbols are there without parsing whole file,
     * so the file is parsed twice.
     */
    estimate (file, file_size, filename, &num_hunks, &num_symbols);
    if (num_hunks == 0) return 0;

    if (ld_state->target_machine == LD_TARGET_MACHINE_M68K
        || ld_state->target_machine == LD_TARGET_MACHINE_UNKNOWN) {
        ld_state->target_machine = LD_TARGET_MACHINE_M68K;
    } else {
        ld_error ("%s: Hunk format used but other objects are not for m68k", filename);
        return 1;
    }
    
    {
        /* HUNK_UNIT contains the object name which is important when reading archives. */
        unsigned long name_size;
        
        pos = file + 4;
        CHECK_READ (pos, 4);

        bytearray_read_4_bytes (&name_size, pos, BIG_ENDIAN);
        pos += 4;
        name_size *= 4;
        
        CHECK_READ (pos, name_size);
        hunk_filename = xstrndup (pos, name_size);
        pos += name_size;
        
        if (strcmp (filename, hunk_filename)) {
            char *new_filename;
            size_t archive_name_len = strlen (filename);
            size_t member_name_len = strlen (hunk_filename);
            
            new_filename = xmalloc (archive_name_len + 1 + member_name_len + 1 + 1);
            memcpy (new_filename, filename, archive_name_len);
            new_filename[archive_name_len] = '(';
            memcpy (new_filename + archive_name_len + 1, hunk_filename, member_name_len);
            new_filename[archive_name_len + 1 + member_name_len] = ')';
            new_filename[archive_name_len + 1 + member_name_len + 1] = '\0';

            free (hunk_filename);
            hunk_filename = new_filename;
        }
        
    }
    
    of = object_file_make (num_symbols, hunk_filename);
    free (hunk_filename);
    /* It is simpler to place section (hunk) symbols at the beginning
     * and access them like that instead of mixing them with other symbols.
     */
    symbol_i = num_hunks;

    while (pos < file + file_size) {
        unsigned long type;

        CHECK_READ (pos, 4);
        bytearray_read_4_bytes (&type, pos, BIG_ENDIAN);
        pos += 4;

        if (type == HUNK_UNIT) {
            pos -= 4;
            *end_pos_p = pos;
            ret = 2;
            break;
        }

        if (type == HUNK_END) {
            section = NULL;
            part = NULL;
            free (section_name);
            section_name = NULL;
            section_i++;

            continue;
        }

        if (type == HUNK_NAME) {
            unsigned long name_size;
            
            if (section) {
                ld_error ("%s: HUNK_NAME after section name was set", filename);
                return 1;
            }

            CHECK_READ (pos, 4);
            bytearray_read_4_bytes (&name_size, pos, BIG_ENDIAN);
            pos += 4;
            name_size *= 4;

            CHECK_READ (pos, name_size);
            free (section_name);
            section_name = xstrndup ((char *)pos, name_size);
            pos += name_size;

            continue;
        }

        if (type == HUNK_CODE
            || type == HUNK_DATA
            || type == HUNK_BSS) {
            unsigned long size;

            CHECK_READ (pos, 4);
            bytearray_read_4_bytes (&size, pos, BIG_ENDIAN);
            pos += 4;
            size *= 4;
            
            if (section) {
                ld_error ("%s: %s after section type was set", filename,
                          (type == HUNK_CODE)
                          ? "HUNK_CODE"
                          : ((type == HUNK_DATA)
                             ? "HUNK_DATA"
                             : "HUNK_BSS"));
                return 1;
            }

            if (!section_name) {
                /* HUNK_NAME is optional, so if it is missing,
                 * name must be decided based on the type.
                 */
                if (type == HUNK_CODE) {
                    section_name = xstrdup (".text");
                } else if (type == HUNK_DATA) {
                    section_name = xstrdup (".data");
                } else {
                    section_name = xstrdup (".bss");
                }
            }

            section = section_find_or_make (section_name);
            free (section_name);
            section_name = NULL;

            if (section->section_alignment < 0x1000) {
                /* Same alignment as for COFF input,
                 * needed for loading .text and .data
                 * on separate memory pages.
                 */
                section->section_alignment = 0x1000;
            }

            if (type == HUNK_CODE) {
                section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_READONLY | SECTION_FLAG_CODE;
            } else if (type == HUNK_DATA) {
                section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_DATA;
            } else {
                section->flags = SECTION_FLAG_ALLOC;
                section->is_bss = 1;
            }

            part = section_part_new (section, of);

            part->content_size = size;
            if (type != HUNK_BSS) {
                part->content = xmalloc (part->content_size);
                CHECK_READ (pos, part->content_size);
                memcpy (part->content, pos, part->content_size);
                pos += size;
            }

            section_append_section_part (section, part);

            {
                struct symbol *symbol = of->symbol_array + section_i;
                
                symbol->name = xstrdup (section->name);

                symbol->value = 0;
                symbol->size = 0;
                symbol->part = part;
                /* Hunk index is 0-based
                 * but 0 currently means undefined section elsewhere.
                 */
                symbol->section_number = section_i + 1;
            }

            continue;
        }

        if (!section) {
            ld_error ("%s: hunk type %#lx without preceding HUNK_CODE, HUNK_DATA or HUNK_BSS",
                      filename, type);
            return 1;
        }
            

        if (type == HUNK_EXT) {
            while (1) {
                unsigned char symbol_type;
                unsigned long name_size;
                struct symbol *symbol;
                
                CHECK_READ (pos, 4);
                bytearray_read_4_bytes (&name_size, pos, BIG_ENDIAN);
                pos += 4;

                if (name_size == 0) break;

                symbol = of->symbol_array + symbol_i++;
                
                symbol_type = (name_size >> 24) & 0xff;
                name_size &= 0xffffff;
                name_size *= 4;

                CHECK_READ (pos, name_size);
                symbol->name = xstrndup (pos, name_size);
                pos += name_size;

                if (symbol_type == EXT_DEF
                    || symbol_type == EXT_ABS) {
                    unsigned long value;
                    
                    CHECK_READ (pos, 4);
                    bytearray_read_4_bytes (&value, pos, BIG_ENDIAN);
                    pos += 4;

                    symbol->value = value;
                    symbol->size = 0;
                    if (symbol_type == EXT_DEF) {
                        symbol->part = part;
                        symbol->section_number = section_i + 1;
                        symbol_record_external_symbol (symbol);
                    } else if (symbol_type == EXT_ABS) {
                        symbol->part = NULL;
                        symbol->section_number = ABSOLUTE_SECTION_NUMBER;
                        /* Do not record absolute symbols as external,
                         * they seem to not be actually used after being defined
                         * and it is not clear how to handle duplicates.
                         */
                    }
                } else if (symbol_type == EXT_REF32
                           || symbol_type == EXT_COMMON
                           || symbol_type == EXT_RELREF32) {
                    unsigned long num_ref, i;
                    struct reloc_entry *reloc;
                    size_t old_reloc_count = part->relocation_count;

                    if (symbol_type == EXT_COMMON) {
                        unsigned long common_size;
                        struct symbol *old_symbol = symbol_find (symbol->name);

                        CHECK_READ (pos, 4);
                        bytearray_read_4_bytes (&common_size, pos, BIG_ENDIAN);
                        pos += 4;

                        symbol->size = common_size;

                        if (!old_symbol || symbol_is_undefined (old_symbol)) {
                            struct section_part *bss_part;
                            
                            if (bss_section == NULL) {
                                bss_section = section_find_or_make (".bss");

                                if (bss_section->section_alignment < 0x1000) {
                                    bss_section->section_alignment = 0x1000;
                                }
                                bss_section->flags = SECTION_FLAG_ALLOC;
                                bss_section->is_bss = 1;
                            }
                            
                            bss_part = section_part_new (bss_section, of);
                            section_append_section_part (bss_section, bss_part);

                            bss_part->content_size = symbol->size;
                            bss_part->alignment = 4; /* Arbitrary. */
                            symbol->part = bss_part;
                            symbol->value = 0;
                            /* The section number except for the special values
                             * does not matter, so no need to use the actual .bss section number.
                             */
                            symbol->section_number = num_hunks + 1;
                        } else {
                            if (symbol->size > old_symbol->size) {
                                old_symbol->part->content_size = old_symbol->size = symbol->size;
                            }
                            
                            symbol->value = 0;
                            symbol->section_number = UNDEFINED_SECTION_NUMBER;
                            symbol->part = NULL;
                        }
                        symbol_record_external_symbol (symbol);
                    } else {
                        symbol->value = 0;
                        symbol->size = 0;
                        symbol->section_number = UNDEFINED_SECTION_NUMBER;
                        symbol->part = NULL;
                        symbol_record_external_symbol (symbol);
                    }

                    CHECK_READ (pos, 4);
                    bytearray_read_4_bytes (&num_ref, pos, BIG_ENDIAN);
                    pos += 4;

                    /* Likely case for EXT_COMMON. */
                    if (num_ref == 0) continue;
                    
                    part->relocation_count += num_ref;
                    part->relocation_array = xrealloc (part->relocation_array,
                                                       part->relocation_count
                                                       * sizeof *part->relocation_array);
                    memset (part->relocation_array + old_reloc_count,
                            0,
                            (part->relocation_count - old_reloc_count) * sizeof *part->relocation_array);

                    reloc = part->relocation_array + old_reloc_count;

                    CHECK_READ (pos, num_ref * 4);
                    for (i = 0; i < num_ref; i++, reloc++) {
                        unsigned long offset;

                        bytearray_read_4_bytes (&offset, pos, BIG_ENDIAN);
                        pos += 4;
                        
                        reloc->symbol = symbol;
                        reloc->offset = offset;
                        reloc->addend = 0;
                        if (symbol_type == EXT_REF32
                            || symbol_type == EXT_COMMON) {
                            reloc->howto = &reloc_howtos[RELOC_TYPE_32];
                        } else if (symbol_type == EXT_RELREF32) {
                            /* EXT_RELREF32 is relative to the beginning of the hunk,
                             * not to the beginning of the field like ELF
                             * or to the end of the field like x86 COFF,
                             * so do not subtract the offset and the field size.
                             */
                            reloc->addend += offset + 4;
                            reloc->howto = &reloc_howtos[RELOC_TYPE_PC32];
                        }
                    }
                } else {
                    ld_internal_error_at_source (__FILE__, __LINE__, "+++symbol type %lu unsupported", symbol_type);
                }
            }
            
            continue;
        }

        if (type == HUNK_RELOC32) {
            while (1) {
                unsigned long num_relocs;
                unsigned long target_i;
                
                CHECK_READ (pos, 4);
                bytearray_read_4_bytes (&num_relocs, pos, BIG_ENDIAN);
                pos += 4;

                if (num_relocs == 0) break;

                CHECK_READ (pos, 4 + num_relocs * 4);
                bytearray_read_4_bytes (&target_i, pos, BIG_ENDIAN);
                pos += 4;

                if (target_i < num_hunks) {
                    struct reloc_entry *reloc;
                    size_t old_reloc_count = part->relocation_count;

                    part->relocation_count += num_relocs;
                    part->relocation_array = xrealloc (part->relocation_array,
                                                       part->relocation_count
                                                       * sizeof *part->relocation_array);
                    memset (part->relocation_array + old_reloc_count,
                            0,
                            (part->relocation_count - old_reloc_count) * sizeof *part->relocation_array);

                    reloc = part->relocation_array + old_reloc_count;
                    for (; num_relocs--; reloc++) {
                        unsigned long offset;

                        bytearray_read_4_bytes (&offset, pos, BIG_ENDIAN);
                        pos += 4;
                        
                        reloc->symbol = part->of->symbol_array + target_i;
                        reloc->offset = offset;
                        reloc->addend = 0;
                        reloc->howto = &reloc_howtos[RELOC_TYPE_32];
                    }
                } else {
                    ld_error ("%s: invalid hunk number %lu in HUNK_RELOC32",
                              filename, target_i);
                    pos += num_relocs * 4;
                }
            }
            
            continue;
        }

        ld_internal_error_at_source (__FILE__, __LINE__, "+++unrecognized hunk %#lx", type);
    }

    free (section_name);

    return ret;
}

int hunk_read (unsigned char *file, size_t file_size, const char *filename)
{
    unsigned long Magic;
    
    CHECK_READ (file, 4);

    bytearray_read_4_bytes (&Magic, file, BIG_ENDIAN);

    if (Magic == HUNK_UNIT) {
        unsigned char *new_pos;

        /* Hunk archives are just multiple object files
         * starting with HUNK_UNIT merged into a single file,
         * so keep reading the file until end is encountered.
         */
        while (1) {
            int ret = read_hunk_object (file, file_size, filename, &new_pos);
            if (ret == 1) {
                return INPUT_FILE_ERROR;
            } else if (ret == 2) {
                file_size -= new_pos - file;
                file = new_pos;
                continue;
            }        
            return INPUT_FILE_FINISHED;
        }
    } else {
        return INPUT_FILE_UNRECOGNIZED;
    }

    return INPUT_FILE_FINISHED;
}
