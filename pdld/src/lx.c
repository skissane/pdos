/******************************************************************************
 * @file            lx.c
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
#include "coff.h"
#include "lx.h"
#include "xmalloc.h"

#include "coff_bytearray.h"
#include "lx_bytearray.h"

static size_t size_of_headers = sizeof (struct IMAGE_DOS_HEADER_file) + sizeof (struct LX_HEADER_file);

static void write_sections (unsigned char *file, unsigned char *pos)
{
    struct section *section;

    for (section = all_sections; section; section = section->next) {
        if (!section->is_bss) {
            section_write (section, pos);
            pos += section->total_size;
        }
    }
}

void lx_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct IMAGE_DOS_HEADER_internal dos_hdr;
    struct LX_HEADER_internal lx_hdr;

    struct section *section;

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    {
        size_t total_section_size_to_write = 0;

        for (section = all_sections; section; section = section->next) {
            if (!section->is_bss) total_section_size_to_write += section->total_size;
        }

        file_size = size_of_headers + total_section_size_to_write;
    }

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    pos = file;

    memset (&dos_hdr, 0, sizeof (dos_hdr));

    dos_hdr.Magic[0] = 'M';
    dos_hdr.Magic[1] = 'Z';

    dos_hdr.SizeOfHeaderInParagraphs = sizeof (struct IMAGE_DOS_HEADER_file) / IMAGE_DOS_HEADER_PARAGRAPH_SIZE;

    dos_hdr.OffsetToNewEXEHeader = sizeof (struct IMAGE_DOS_HEADER_file);

    write_struct_IMAGE_DOS_HEADER (pos, &dos_hdr);
    pos += sizeof (struct IMAGE_DOS_HEADER_file);
    
    memset (&lx_hdr, 0, sizeof lx_hdr);
    
    lx_hdr.Magic[0] = 'L';
    lx_hdr.Magic[1] = 'X';
    lx_hdr.BOrd = BORD_LITTLE_ENDIAN;
    lx_hdr.WOrd = WORD_LITTLE_ENDIAN;
    lx_hdr.FormatLevel = 0;
    lx_hdr.CpuType = CPU_TYPE_386;
    lx_hdr.OsType = OS_TYPE_OS2;

    lx_hdr.DataPagesOffset = size_of_headers; 

    write_struct_LX_HEADER (pos, &lx_hdr);
    pos += sizeof (struct LX_HEADER_file);

    write_sections (file, pos);

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }

    free (file);
    fclose (outfile);
}
