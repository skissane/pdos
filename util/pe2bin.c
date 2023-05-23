/******************************************************************************
 * @file            coff.c
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

#include "xmalloc.h"
#include "bytearray.h"
#include "coff.h"

#define READ_MEM_INCREMENT 60000 /* 1/2 of size of a certain archive file. */

int read_file_into_memory (const char *filename, unsigned char **memory_p, size_t *size_p)
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

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) { fprintf (stderr, "corrupted input file\n"); free (file); exit (EXIT_FAILURE); } } while (0)

int main (int argc, char **argv)
{
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;
    unsigned long OffsetToNewEXEHeader, SizeOfImage, AddressOfEntryPoint;
    FILE *outfile;
    const char *in_filename, *out_filename;
    
    if (argc != 3)
    {
        printf("usage: %s file output_file\n", argv[0]);
        return (0);
    }

    in_filename = argv[1];
    out_filename = argv[2];
    
    if (read_file_into_memory (in_filename, &file, &file_size)) {
        fprintf (stderr, "failed to read '%s'\n", in_filename);
        return EXIT_FAILURE;
    }

    pos = file;
    CHECK_READ (file, 2);
    if (memcmp (pos, "MZ", 2) && memcmp (pos, "ZM", 2)) {
        fprintf (stderr, "invalid input file\n");
        free (file);
        return EXIT_FAILURE;
    }

    pos += offsetof (struct IMAGE_DOS_HEADER_file, OffsetToNewEXEHeader);
    CHECK_READ (pos, 4);
    bytearray_read_4_bytes (&OffsetToNewEXEHeader, pos, LITTLE_ENDIAN);

    pos = (file + OffsetToNewEXEHeader + 4
           + sizeof (struct coff_header_file)
           + offsetof (struct optional_header_file, SizeOfImage));
    CHECK_READ (pos, 4);
    bytearray_read_4_bytes (&SizeOfImage, pos, LITTLE_ENDIAN);

    pos = (file + OffsetToNewEXEHeader + 4
           + sizeof (struct coff_header_file)
           + offsetof (struct optional_header_file, AddressOfEntryPoint));
    CHECK_READ (pos, 4);
    bytearray_read_4_bytes (&AddressOfEntryPoint, pos, LITTLE_ENDIAN);

    if (SizeOfImage < file_size) SizeOfImage = file_size;

    file = xrealloc (file, SizeOfImage);
    memset (file + file_size, '\0', SizeOfImage - file_size);
    file_size = SizeOfImage;
    file[0] = 0xE9;
    bytearray_write_4_bytes (file + 1, AddressOfEntryPoint - 5, LITTLE_ENDIAN);

    if (!(outfile = fopen (out_filename, "wb"))) {
        fprintf (stderr, "cannot open '%s' for writing\n", out_filename);
        free (file);
        return EXIT_FAILURE;
    }
    if (outfile) {
        if (fwrite (file, file_size, 1, outfile) != 1) {
            fprintf (stderr, "writing '%s' file failed\n", out_filename);
            free (file);
            return EXIT_FAILURE;
        }
        fclose (outfile);
    }
    
    free (file);
    
    return 0;
}
