/******************************************************************************
 * @file            binary.c
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
#include "xmalloc.h"

address_type binary_get_base_address (void)
{    
    return 0;
}

void binary_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct section *section;

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    file_size = 0;
    for (section = all_sections; section; section = section->next) {
        if (file_size < section->rva + section->total_size) {
            file_size = section->rva + section->total_size;
        }
    }

    file = xcalloc (file_size, 1);
    
    for (section = all_sections; section; section = section->next) {
        if (section->is_bss) continue;

        pos = file + section->rva;
        section_write (section, pos);
    }
    
    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}
