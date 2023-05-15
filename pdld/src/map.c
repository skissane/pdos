/******************************************************************************
 * @file            map.c
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

void map_write (const char *filename)
{
    FILE *outfile;
    struct section *section;

    if (strcmp (filename, "") == 0) outfile = stdout;
    else if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    for (section = all_sections; section; section = section->next) {
        struct section_part *part;
        int printed_chars;
        
        printed_chars = fprintf (outfile, "%-16s", section->name);

        if (printed_chars > 16) {
            fprintf (outfile, "\n%-16s", "");
        }

        fprintf (outfile, "%#08lx %#10lx\n",
                 ld_state->base_address + section->rva,
                 section->total_size);
        
        for (part = section->first_part; part; part = part->next) {
            struct symbol *symbol;
            
            printed_chars = fprintf (outfile, " %-15s", section->name);

            if (printed_chars > 16) {
                fprintf (outfile, "\n%-16s", "");
            }

            fprintf (outfile, "%#08lx %#10lx %s\n",
                     ld_state->base_address + part->rva,
                     part->content_size,
                     part->of->filename);

            for (symbol = part->of->symbol_array;
                 symbol < part->of->symbol_array + part->of->symbol_count;
                 symbol++) {
                if (symbol->auxiliary || symbol->part != part) continue;

                fprintf (outfile, "%-16s%#08lx %10s %s\n",
                         "",
                         symbol_get_value_with_base (symbol),
                         "",
                         symbol->name);
            }
        }

        fprintf (outfile, "\n");
    }

    if (strcmp (filename, "") != 0) fclose (outfile);
    return;
}
