/******************************************************************************
 * @file            genimp.c
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

#include "xmalloc.h"
#include "bytearray.h"
#include "coff.h"

#include "coff_bytearray.h"

/* "__imp_" added in front of the "_" prefix means
 * that the symbol is imported directly from Import Address Table
 * instead of from stub generated in .text. */
#define IMP_PREFIX_STR "__imp_"
#define IMP_PREFIX_LEN 6

enum export_type {
    EXPORT_TYPE_CODE,
    EXPORT_TYPE_DATA,
    EXPORT_TYPE_CONST
};

struct export_name {
    char *name;
    char *name_no_at;
    unsigned short ordinal;
    enum export_type export_type;
};

static const char *dll_filename;
static const char *output_filename;
static int leading_underscore = 0;
static unsigned short wanted_Machine = IMAGE_FILE_MACHINE_I386;
static int kill_at = 0;

#define ALIGN(to_align, alignment) \
 (((to_align) / (alignment) + (((to_align) % (alignment)) ? 1 : 0)) * (alignment))

static void write_archive_member_header (unsigned char *pos, const char *Name, unsigned long Size, unsigned long lu_timestamp)
{
    struct IMAGE_ARCHIVE_MEMBER_HEADER_internal member_hdr;

    memset (&member_hdr, ' ', sizeof (member_hdr));
    
    member_hdr.Name[sprintf (member_hdr.Name, Name)] = ' ';
    member_hdr.Date[sprintf (member_hdr.Date, "%lu", lu_timestamp)] = ' ';
    member_hdr.UserID[0] = '0';
    member_hdr.GroupID[0] = '0';
    member_hdr.Mode[0] = '0';
    member_hdr.Size[sprintf (member_hdr.Size, "%lu", Size)] = ' ';
    memcpy (member_hdr.EndOfHeader, IMAGE_ARCHIVE_MEMBER_HEADER_END_OF_HEADER, sizeof (member_hdr.EndOfHeader));
    
    write_struct_IMAGE_ARCHIVE_MEMBER_HEADER (pos, &member_hdr);
}

static void write_implib (struct export_name *export_names, size_t num_names, unsigned long OrdinalBase)
{
    const char *filename;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    unsigned long lu_timestamp = 0;

    unsigned long linker_member_size;
    unsigned long num_linker_member_offsets;
    unsigned char *offset_pos;
    unsigned char *string_table_pos;
    size_t i;

    filename = output_filename;

    file_size = strlen (IMAGE_ARCHIVE_START);
    linker_member_size = 1 * 4;

    num_linker_member_offsets = 0;
    for (i = 0; i < num_names; i++) {
        file_size += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file);
        file_size += sizeof (struct IMPORT_OBJECT_HEADER_file);
        if (leading_underscore) file_size++;
        file_size += strlen (export_names[i].name) + 1;
        file_size += strlen (dll_filename) + 1;

        if (export_names[i].export_type == EXPORT_TYPE_CODE) {
            if (leading_underscore) linker_member_size++;
            linker_member_size += 4 + strlen (export_names[i].name) + 1;
            num_linker_member_offsets++;
        }
        if (leading_underscore) linker_member_size++;
        linker_member_size += 4 + IMP_PREFIX_LEN + strlen (export_names[i].name) + 1;
        num_linker_member_offsets++;

        file_size = ALIGN (file_size, 2);
    }

    linker_member_size = ALIGN (linker_member_size, 2);
    file_size += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file) + linker_member_size;

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    pos = file;

    memcpy (pos, IMAGE_ARCHIVE_START, strlen (IMAGE_ARCHIVE_START));
    pos += strlen (IMAGE_ARCHIVE_START);

    write_archive_member_header (pos, IMAGE_ARCHIVE_LINKER_MEMBER_Name, linker_member_size, lu_timestamp);
    pos += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file);

    bytearray_write_4_bytes (pos, num_linker_member_offsets, BIG_ENDIAN);
    pos += 4;

    offset_pos = pos;
    string_table_pos = pos + num_linker_member_offsets * 4;
    pos = file + strlen (IMAGE_ARCHIVE_START) + sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file) + linker_member_size;

    for (i = 0; i < num_names; i++) {

        if (export_names[i].export_type == EXPORT_TYPE_CODE) {
            bytearray_write_4_bytes (offset_pos, pos - file, BIG_ENDIAN);
            offset_pos += 4;
            
            if (leading_underscore) string_table_pos++[0] = '_';
            strcpy ((char *)string_table_pos, export_names[i].name);
            string_table_pos += strlen (export_names[i].name) + 1;
        }

        {
            bytearray_write_4_bytes (offset_pos, pos - file, BIG_ENDIAN);
            offset_pos += 4;
            
            memcpy (string_table_pos, IMP_PREFIX_STR, IMP_PREFIX_LEN);
            string_table_pos += IMP_PREFIX_LEN;
            if (leading_underscore) string_table_pos++[0] = '_';
            strcpy ((char *)string_table_pos, export_names[i].name);
            string_table_pos += strlen (export_names[i].name) + 1;
        }

        write_archive_member_header (pos, "IMPORT/",
                                     sizeof (struct IMPORT_OBJECT_HEADER_file)
                                     + (leading_underscore ? 1 : 0)
                                     + strlen (export_names[i].name) + 1
                                     + strlen (dll_filename) + 1,
                                     lu_timestamp);
        pos += sizeof (struct IMAGE_ARCHIVE_MEMBER_HEADER_file);

        {
            struct IMPORT_OBJECT_HEADER_internal import_hdr;

            import_hdr.Magic1 = IMAGE_FILE_MACHINE_UNKNOWN;
            import_hdr.Magic2 = IMPORT_OBJECT_HDR_MAGIC2;
            import_hdr.Version = 0;
            import_hdr.Machine = wanted_Machine;

            import_hdr.TimeDateStamp = lu_timestamp;
            
            import_hdr.SizeOfData = strlen (export_names[i].name) + 1 + strlen (dll_filename) + 1;
            if (leading_underscore) import_hdr.SizeOfData++;

            if (export_names[i].ordinal) {
                import_hdr.OrdinalHint = export_names[i].ordinal;
            } else {
                import_hdr.OrdinalHint = OrdinalBase + i;
            }
            
            switch (export_names[i].export_type) {
                case EXPORT_TYPE_CODE: import_hdr.Type = IMPORT_CODE; break;
                case EXPORT_TYPE_DATA: import_hdr.Type = IMPORT_DATA; break;
                case EXPORT_TYPE_CONST: import_hdr.Type = IMPORT_CONST; break;
            }

            if (export_names[i].ordinal) import_hdr.Type |= IMPORT_ORDINAL << 2;
            else if (kill_at) import_hdr.Type |= IMPORT_NAME_UNDECORATE << 2;
            else if (leading_underscore) import_hdr.Type |= IMPORT_NAME_NOPREFIX << 2;
            else import_hdr.Type |= IMPORT_NAME << 2;

            write_struct_IMPORT_OBJECT_HEADER (pos, &import_hdr);
            pos += sizeof (struct IMPORT_OBJECT_HEADER_file);

            if (leading_underscore) pos++[0] = '_';
            strcpy ((char *)pos, export_names[i].name);
            pos += strlen (export_names[i].name) + 1;
            strcpy ((char *)pos, dll_filename);
            pos += strlen (dll_filename) + 1;
        }

        pos = file + ALIGN (pos - file, 2);
    }

    {
        FILE *outfile;
        
        if (!(outfile = fopen (filename, "wb"))) {
            fprintf (stderr, "cannot open '%s' for writing", filename);
            return;
        }

        if (fwrite (file, file_size, 1, outfile) != 1) {
            fprintf (stderr, "writing '%s' file failed", filename);
        }

        fclose (outfile);
    }

    free (file);
    return;
}

#define READ_MEM_INCREMENT 60000 /* 1/2 of size of a certain archive file. */

static int read_file_into_memory (const char *filename, unsigned char **memory_p, size_t *size_p)
{
    size_t mem_size = READ_MEM_INCREMENT;
    size_t read_bytes = 0;
    size_t change;
    unsigned char *memory;
    FILE *infile;

    if ((infile = fopen (filename, "rb")) == NULL) return 1;

    memory = xmalloc (mem_size + 2);
    while ((change = fread (memory + read_bytes, 1, mem_size - read_bytes, infile)) > 0) {

        read_bytes += change;
        if (read_bytes == mem_size) {
            mem_size += READ_MEM_INCREMENT;
            memory = xrealloc (memory, mem_size + 2);
        }

    }

    /* Protection against corrupted string tables. */
    memory[read_bytes] = '\0';
    memory[read_bytes + 1] = '\0';

    fclose (infile);
    *memory_p = memory;
    *size_p = read_bytes;

    return 0;
}

static int process_input (void *input_file,
                          struct export_name **export_names_p,
                          size_t *num_names_p)
{
    struct export_name *enames = NULL;
    size_t num_enames = 0;
    size_t max_enames = 0;
    char *input = input_file;
    
    while (*input) {
        char *p;

        /* Format is "++symbol_name.dll_name.ignored.ordinal". */
        while (*input == '+') input++;
        p = strchr (input, '.');
        if (!p) return 1;
        *p = '\0';

        if (num_enames == max_enames) {
            max_enames = max_enames * 2 + 20;
            enames = xrealloc (enames, sizeof *enames * max_enames);
        }

        enames[num_enames].name = input;
        enames[num_enames].name_no_at = input;
        input = p + 1;

        p = strchr (input, '.');
        if (!p) return 1;
        *p = '\0';

        dll_filename = input;
        input = p + 1;

        p = strchr (input, '.');
        if (!p) return 1;
        *p = '\0';
        input = p + 1;

        enames[num_enames].ordinal = (unsigned short)strtol (input, &p, 0);
        input = p;
        
        enames[num_enames].export_type = EXPORT_TYPE_CODE;
        num_enames++;
        
        while (isspace (*input)) input++;
    }

    *export_names_p = enames;
    *num_names_p = num_enames;

    return 0;
}

int main (int argc, char **argv)
{
    unsigned char *input_file;
    size_t input_file_size;
    struct export_name *enames = NULL;
    size_t num_enames;
    
    if (argc != 3) {
        printf ("usage: %s input_file output_file\n", argv[0]);
        return 0;
    }

    if (read_file_into_memory (argv[1], &input_file, &input_file_size)) {
        fprintf (stderr, "failed to read '%s'\n", argv[1]);
        return 1;
    }

    if (process_input (input_file, &enames, &num_enames)) {
        fprintf (stderr, "error processing input\n");
        return 1;
    }

    output_filename = argv[2];
    write_implib (enames, num_enames, 0);

    free (input_file);
    free (enames);
    
    return 0;
}
