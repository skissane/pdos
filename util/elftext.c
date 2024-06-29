/******************************************************************************
 * @file            elftext.c
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
#include "elf.h"
#include "elf_bytearray.h"

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

static int endianess;

#define ld_error printf

static int read_elf_object (unsigned char *file,
                            size_t file_size,
                            const char *filename,
                            const char *out_filename)
{
    struct Elf32_Ehdr_internal ehdr;
    struct Elf32_Shdr_internal shdr;

    const char *section_name_string_table = NULL;
    Elf32_Word section_name_string_table_size;

    unsigned char *pos;

    Elf32_Half i;

    endianess = LITTLE_ENDIAN;

    pos = file;
    CHECK_READ (pos, SIZEOF_struct_Elf32_Ehdr_file);
    read_struct_Elf32_Ehdr (&ehdr, pos, endianess);

    if (ehdr.e_ident[EI_CLASS] == ELFCLASS64) {
        printf ("64-bit ELF is not supported");
        return 1;
    }

    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        ld_error ("%s: Unsupported ELF file class", filename);
        return 1;
    }

    if (ehdr.e_ident[EI_DATA] == ELFDATA2LSB) {
        /* Nothing needs to be done. */
    } else if (ehdr.e_ident[EI_DATA] == ELFDATA2MSB) {
        endianess = BIG_ENDIAN;
        read_struct_Elf32_Ehdr (&ehdr, pos, endianess);
    } else {
        ld_error ("%s: Unsupported ELF data encoding", filename);
        return 1;
    }

    if (ehdr.e_ident[EI_VERSION] != EV_CURRENT) {
        ld_error ("%s: Unsupported ELF version", filename);
        return 1;
    }

    if (ehdr.e_version != EV_CURRENT) {
        ld_error ("%s: e_version is not EV_CURRENT", filename);
        return 1;
    }

    if (ehdr.e_ehsize < sizeof (ehdr)) {
        ld_error ("%s: e_ehsize is too small", filename);
        return 1;
    }

    if (ehdr.e_shoff == 0 || ehdr.e_shentsize == 0 || ehdr.e_shnum == 0) {
        ld_error ("%s: missing section header table", filename);
        return 1;
    }

    if (ehdr.e_shentsize < SIZEOF_struct_Elf32_Shdr_file) {
        ld_error ("%s: e_shentsize is too small", filename);
        return 1;
    }

    if (ehdr.e_shstrndx == 0 || ehdr.e_shstrndx >= ehdr.e_shnum) {
        ld_error ("%s: missing section name string table", filename);
        return 1;
    }

    pos = file + ehdr.e_shoff;
    CHECK_READ (pos, ehdr.e_shentsize * ehdr.e_shnum);

    pos += ehdr.e_shentsize * ehdr.e_shstrndx;
    read_struct_Elf32_Shdr (&shdr, pos, endianess);

    if (shdr.sh_type != SHT_STRTAB) {
        ld_error ("section name string table does not have SHT_STRTAB type");
        return 1;
    }

    section_name_string_table_size = shdr.sh_size;
    pos = file + shdr.sh_offset;
    CHECK_READ (pos, section_name_string_table_size);
    section_name_string_table = (char *)pos;

    for (i = 1; i < ehdr.e_shnum; i++) {
        const char *section_name;
        
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        read_struct_Elf32_Shdr (&shdr, pos, endianess);

        section_name = section_name_string_table + shdr.sh_name;

        if (strcmp (section_name, ".text") == 0) {
            FILE *outfile;

            pos = file + shdr.sh_offset;
            CHECK_READ (pos, shdr.sh_size);

            if (!(outfile = fopen (out_filename, "wb"))) {
                fprintf (stderr, "cannot open '%s' for writing\n", out_filename);
                free (file);
                return EXIT_FAILURE;
            }
            
            if (fwrite (pos, shdr.sh_size, 1, outfile) != 1) {
                fprintf (stderr, "writing '%s' file failed\n", out_filename);
                free (file);
                return EXIT_FAILURE;
            }
            
            fclose (outfile);
        }
    }

    return 0;
}

int main (int argc, char **argv)
{
    unsigned char *file;
    size_t file_size;
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

    CHECK_READ (file, 4);

    if (!(file[EI_MAG0] == ELFMAG0
          && file[EI_MAG1] == ELFMAG1
          && file[EI_MAG2] == ELFMAG2
          && file[EI_MAG3] == ELFMAG3)) {
        fprintf (stderr, "invalid input file\n");
        free (file);
        return EXIT_FAILURE;
    }

    if (read_elf_object (file, file_size, in_filename, out_filename)) {
        fprintf (stderr, "\nerror occured while processing input\n");
        free (file);
        return EXIT_FAILURE;
    }
    
    free (file);

    return 0;
}
