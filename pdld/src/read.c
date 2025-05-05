/******************************************************************************
 * @file            read.c
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
#include "xmalloc.h"

/* For archive reading. */
#include "coff.h"
#include "bytearray.h"
#include "coff_bytearray.h"

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("%s: corrupted input file", archive_name); } while (0)

static int read_file (unsigned char *file, size_t file_size, const char *filename);

static void strip_trailing_spaces (char *str)
{
    char *p = str + strlen (str);

    while (p > str && p[-1] == ' ') p--;

    *p = '\0';
}

struct lm_offset_name_entry {
    unsigned long offset;
    char *name;
};

struct archive_member_header {
    char *name;
    unsigned long size;
};

struct archive_longnames {
    const char *names;
    unsigned long size;
};

static int read_archive_member_header (const unsigned char *pos,
                                       struct archive_member_header *hdr,
                                       const struct archive_longnames *longnames)
{
    struct IMAGE_ARCHIVE_MEMBER_HEADER_internal member_header;
    char *tmp;

    read_struct_IMAGE_ARCHIVE_MEMBER_HEADER (&member_header, pos);

    hdr->name = xstrndup (member_header.Name, sizeof (member_header.Name));
    strip_trailing_spaces (hdr->name);
    if (hdr->name[0] == '/' && isdigit (hdr->name[1])) {
        unsigned long i = strtoul (hdr->name + 1, NULL, 10);

        if (!longnames->names) {
            ld_warn ("archive member has longname but archive lacks longnames member");
        } else if (i >= longnames->size) {
            free (hdr->name);
            ld_error ("longname index %lu larger than longnames size %lu",
                      i, longnames->size);
            return 1;
        } else {
            const char *p, *p2;
            
            free (hdr->name);
            /* According to the specification,
             * individual long names should be null-terminated
             * but some tools do not follow that
             * and use '\n' as terminator.
             */
            p = strchr (longnames->names + i, '\0');
            p2 = strchr (longnames->names + i, '\n');
            if (p2 && p2 < p) p = p2;

            hdr->name = xstrndup (longnames->names + i, p - longnames->names - i);
            strip_trailing_spaces (hdr->name);
        }
    }
    
    tmp = xstrndup (member_header.Size, sizeof (member_header.Size));
    strip_trailing_spaces (tmp);
    hdr->size = strtoul (tmp, NULL, 10);
    free (tmp);

    return 0;
}

static struct lm_offset_name_entry *read_linker_member (unsigned char *file,
                                                        size_t file_size,
                                                        const char *archive_name,
                                                        unsigned long *NumberOfSymbols_p)
{
    unsigned long NumberOfSymbols;
    struct lm_offset_name_entry *offset_name_table;

    unsigned char *pos;

    unsigned long i;
    unsigned char *string_table_pos;

    pos = file;

    CHECK_READ (pos, 4);
    bytearray_read_4_bytes (&NumberOfSymbols, pos, BIG_ENDIAN);
    pos += 4;

    offset_name_table = xmalloc (sizeof (*offset_name_table) * NumberOfSymbols);
    string_table_pos = pos + NumberOfSymbols * 4;

    for (i = 0; i < NumberOfSymbols; i++) {

        CHECK_READ (pos, 4);
        bytearray_read_4_bytes (&offset_name_table[i].offset, pos, BIG_ENDIAN);
        pos += 4;
        
        CHECK_READ (string_table_pos, 1);
        offset_name_table[i].name = (char *)string_table_pos;
        string_table_pos += strlen ((char *)string_table_pos) + 1;

    }

    *NumberOfSymbols_p = NumberOfSymbols;

    return offset_name_table;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define ALREADY_READ_NAME "\0Already read"

static int read_archive_member (unsigned char *file,
                                size_t file_size,
                                unsigned char *pos,
                                const char *archive_name,
                                const struct archive_longnames *longnames)
{
    struct archive_member_header hdr;
    int ret;
    char *filename;

    CHECK_READ (pos, SIZEOF_struct_IMAGE_ARCHIVE_MEMBER_HEADER_file);
    /* COFF COMDAT can result in symbols which should be in the member being discarded
     * what would lead to reading the member twice,
     * so this marks the member as read to prevent it.
     */
    if (memcmp (pos, ALREADY_READ_NAME, sizeof (ALREADY_READ_NAME)) == 0) return INPUT_FILE_NO_NEW_SYMBOLS;
    if (read_archive_member_header (pos, &hdr, longnames)) return INPUT_FILE_ERROR;
    memcpy (pos, ALREADY_READ_NAME, sizeof (ALREADY_READ_NAME));
    pos += SIZEOF_struct_IMAGE_ARCHIVE_MEMBER_HEADER_file;

    {
        size_t archive_name_len = strlen (archive_name);
        size_t member_name_len = strlen (hdr.name);

        /* Outside of members starting with '/' the '/' serves as name terminator
         * according to the specification. */
        if (member_name_len && hdr.name[member_name_len - 1] == '/' && hdr.name[0] != '/') member_name_len--;

#if 0
        filename = xmalloc (archive_name_len + 1 + member_name_len + 1 + 1);
        memcpy (filename, archive_name, archive_name_len);
        filename[archive_name_len] = '(';
        memcpy (filename + archive_name_len + 1, hdr.name, member_name_len);
        filename[archive_name_len + 1 + member_name_len] = ')';
        filename[archive_name_len + 1 + member_name_len + 1] = '\0';
#else
        filename = xmalloc (archive_name_len + 3 + 2 * sizeof (unsigned long) + 1 + member_name_len + 1 + 1);
        memcpy (filename, archive_name, archive_name_len);
        {
            char *p = filename + archive_name_len;
            p += sprintf (p, "+%#lx", (unsigned long)(pos - file));
            *p = '(';
            memcpy (p + 1, hdr.name, member_name_len);
            p[1 + member_name_len] = ')';
            p[1 + member_name_len + 1] = '\0';
        }
#endif
    }

    ret = read_file (pos, MIN (hdr.size, file_size - (pos - file)), filename);
    if (ret == INPUT_FILE_UNRECOGNIZED) {
        ld_error ("%s: unrecognized archive member object format", filename);
        ret = INPUT_FILE_ERROR;
    }

    free (hdr.name);
    free (filename);

    return ret;
}    

static void read_archive (unsigned char *file, size_t file_size, const char *archive_name)
{
    struct archive_member_header hdr;

    struct lm_offset_name_entry *offset_name_table;
    unsigned long NumberOfSymbols;
    unsigned long i;
    
    unsigned char *pos;

    struct archive_longnames longnames = {NULL, 0};

    unsigned long start_header_object_offset = 0;
    unsigned long end_header_object_offset = 0;
    int is_traditional_import_library = 0;

    pos = file + strlen (IMAGE_ARCHIVE_START);
    /* Empty archive is also allowed to not contain any members at all,
     * so error should not be reported
     * if the file ends exactly after IMAGE_ARCHIVE_START.
     */
    if (pos == file + file_size) return;
    CHECK_READ (pos, SIZEOF_struct_IMAGE_ARCHIVE_MEMBER_HEADER_file);
    if (read_archive_member_header (pos, &hdr, &longnames)) return;
    pos += SIZEOF_struct_IMAGE_ARCHIVE_MEMBER_HEADER_file;

    if (strcmp (hdr.name, IMAGE_ARCHIVE_LINKER_MEMBER_Name) == 0) {
        offset_name_table = read_linker_member (pos, MIN (hdr.size, file_size - (pos - file)), archive_name, &NumberOfSymbols);
    } else {
        offset_name_table = NULL;
        ld_error ("%s: IMAGE_ARCHIVE_LINKER_MEMBER missing from archive", archive_name);
    }

    free (hdr.name);

    if (offset_name_table == NULL) return;

repeat:
    pos += hdr.size;
    /* Empty archive is allowed to contain just the first linker member
     * and nothing more, so error should not be reported
     * if the file ends exactly after the first linker member.
     * (Empty archive might contain second linker member
     *  and/or longnames member too, so this handles such case too.)
     */
    if (pos == file + file_size) return;
    /* Each archive member header starts on the first even address
     * after the end of the previous archive member.
     */
    pos += (pos - file) & 1;
    CHECK_READ (pos, SIZEOF_struct_IMAGE_ARCHIVE_MEMBER_HEADER_file);
    if (read_archive_member_header (pos, &hdr, &longnames)) return;
    pos += SIZEOF_struct_IMAGE_ARCHIVE_MEMBER_HEADER_file;

    if (strcmp (hdr.name, IMAGE_ARCHIVE_LINKER_MEMBER_Name) == 0) {
        /* Skips the optional second linker member
         * which contains the same information as the first linker member,
         * only in different format.
         */
        free (hdr.name);
        goto repeat;
    } else if (strcmp (hdr.name, IMAGE_ARCHIVE_LONGNAMES_MEMBER_Name) == 0) {
        longnames.names = (char *)pos;
        if (hdr.size > 0) {
            CHECK_READ (pos, hdr.size);
            longnames.size = hdr.size;
            /* Longnames should be null-terminated
             * but input cannot be trusted.
             */
            pos[hdr.size - 1] = '\0';
        }
    }

    free (hdr.name);

    {        
        /* This is necessary because the member containing symbol "__IMPORT_DESCRIPTOR_something"
         * or "__head_something" contains the first part of the .idata content
         * and the member containing symbol "something_NULL_THUNK_DATA" or "_something_iname"
         * contains the terminators for the .idata content.
         * (Applies only to the traditional import library format,
         * for the short format whole .idata is automatically generated.)
         */
        for (i = 0; i < NumberOfSymbols && (!start_header_object_offset || !end_header_object_offset); i++) {
            if (strncmp (offset_name_table[i].name, "__IMPORT_DESCRIPTOR_", sizeof ("__IMPORT_DESCRIPTOR_") - 1) == 0
                || strncmp (offset_name_table[i].name, "__head_", 7) == 0
                || (ld_state->target_machine != LD_TARGET_MACHINE_I386
                    && strncmp (offset_name_table[i].name, "_head_", 6) == 0)) {
                start_header_object_offset = offset_name_table[i].offset;
            } else if ((strlen (offset_name_table[i].name) > sizeof ("_NULL_THUNK_DATA") - 1
                        && strcmp (offset_name_table[i].name
                                   + strlen (offset_name_table[i].name)
                                   - (sizeof ("_NULL_THUNK_DATA") - 1),
                                   "_NULL_THUNK_DATA") == 0)
                       || (strlen (offset_name_table[i].name) > 6
                           && strcmp (offset_name_table[i].name
                                      + strlen (offset_name_table[i].name)
                                      - 6, "_iname") == 0)) {
                end_header_object_offset = offset_name_table[i].offset;
            }
        }
    }

    while (1) {
        int change = 0;

        for (i = 0; i < NumberOfSymbols; i++) {
            int ret;

            if (ld_state->oformat == LD_OFORMAT_CMS
                || ld_state->oformat == LD_OFORMAT_MVS
                || ld_state->oformat == LD_OFORMAT_VSE) {
                /* Mainframe-only output formats need less strict matching. */
                if (!mainframe_symbol_check_undefined (offset_name_table[i].name)) continue;
            } else {
                const struct symbol *symbol = symbol_find (offset_name_table[i].name);

                if (symbol == NULL) continue;
                if (!symbol_is_undefined (symbol)) continue;
            }

            if (offset_name_table[i].offset == start_header_object_offset
                || offset_name_table[i].offset == end_header_object_offset) continue;
            
            pos = file + offset_name_table[i].offset;
            if (start_header_object_offset) {
                /* Some short import libraries contain the "__IMPORT_DESCRIPTOR_something" member etc.
                 * despite them being necessary only for traditional import libraries.
                 * Those members must not be imported in that case
                 * as they would interfere with automatically generated .idata.
                 */
                unsigned char *test = pos + SIZEOF_struct_IMAGE_ARCHIVE_MEMBER_HEADER_file;
                if (!coff_is_import_header (test, file_size - (test - file))) {
                    read_archive_member (file, file_size, file + start_header_object_offset, archive_name, &longnames);
                    is_traditional_import_library = 1;
                }
                start_header_object_offset = 0;
            }
            
            ret = read_archive_member (file, file_size, pos, archive_name, &longnames);
            if (ret == INPUT_FILE_ERROR) return;
            /* If the archive member is a real object (not short import entry),
             * it might require more symbols. */
            if (ret == 1) change = 1;
        }

        if (change == 0) break;
    }

    if (is_traditional_import_library) {
        if (end_header_object_offset)
            read_archive_member (file, file_size, file + end_header_object_offset, archive_name, &longnames);
    }

    coff_archive_end ();

    free (offset_name_table);
}

#undef CHECK_READ
#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("%s: corrupted input file", filename); } while (0)

static int read_file (unsigned char *file, size_t file_size, const char *filename)
{
    int ret;
    
    CHECK_READ (file, strlen (IMAGE_ARCHIVE_START));
    
    if (memcmp (file, IMAGE_ARCHIVE_START, strlen (IMAGE_ARCHIVE_START)) == 0) {
        read_archive (file, file_size, filename);
        return INPUT_FILE_FINISHED;
    }

    if ((ret = aout_read (file, file_size, filename)) != INPUT_FILE_UNRECOGNIZED) return ret;
    if ((ret = coff_read (file, file_size, filename)) != INPUT_FILE_UNRECOGNIZED) return ret;
    if ((ret = elf_read (file, file_size, filename)) != INPUT_FILE_UNRECOGNIZED) return ret;
    if ((ret = hunk_read (file, file_size, filename)) != INPUT_FILE_UNRECOGNIZED) return ret;
    if ((ret = macho_read (file, file_size, filename)) != INPUT_FILE_UNRECOGNIZED) return ret;
    if ((ret = mainframe_read (file, file_size, filename)) != INPUT_FILE_UNRECOGNIZED) return ret;
    if ((ret = omf_read (file, file_size, filename)) != INPUT_FILE_UNRECOGNIZED) return ret;

    return INPUT_FILE_UNRECOGNIZED;
}
 
void read_input_file (const char *filename)
{
    unsigned char *file;
    size_t file_size;

    if (read_file_into_memory (filename, &file, &file_size)) {
        ld_error ("failed to read file '%s' into memory", filename);
        return;
    }

    if (read_file (file, file_size, filename) == INPUT_FILE_UNRECOGNIZED) {
        ld_error ("%s: unrecognized file format", filename);
    }

    free (file);
}
