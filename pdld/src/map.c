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

static int symbol_compare (const void *a, const void *b)
{
    const struct symbol *sa = a;
    const struct symbol *sb = b;

    /* Put all auxiliary symbols at the end because they are just fillers and not displayed anyway. */
    if (sa->auxiliary && sb->auxiliary) return 0;
    if (sa->auxiliary) return 1;
    if (sb->auxiliary) return -1;

    if (symbol_get_value_with_base (sa) < symbol_get_value_with_base (sb)) return -1;
    if (symbol_get_value_with_base (sa) > symbol_get_value_with_base (sb)) return 1;

    return 0;
}

static void sort_symbols (void)
{
    struct object_file *of;

    for (of = all_object_files; of; of = of->next) {
        qsort (of->symbol_array, of->symbol_count, sizeof (*of->symbol_array), &symbol_compare);
    }
}

void map_write (const char *filename)
{
    FILE *outfile;
    struct section *section;

    if (strcmp (filename, "") == 0) outfile = stdout;
    else if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    /* After this point indexes into symbol arrays become invalid
     * but that is fine because all linking was already done. */
    sort_symbols ();

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

                if (symbol->value == 0
                    && strncmp (section->name, symbol->name, strlen (section->name)) == 0) continue;

                fprintf (outfile, "%-16s%#08lx %10s %s\n",
                         "",
                         symbol_get_value_with_base (symbol),
                         "",
                         symbol->name);
            }
        }

        fprintf (outfile, "\n");
    }

    fprintf (outfile, "\n");
    {
        const struct section *last_section = NULL;

        for (section = all_sections; section; section = section->next) {
            last_section = section;
        }
        
        fprintf (outfile, "Size of Module: %#08lx\n",
                 last_section ? (last_section->rva + last_section->total_size) : 0);
    }
        
    fprintf (outfile, "Entry Point: %#08lx\n", ld_state->entry_point + ld_state->base_address);

    if (strcmp (filename, "") != 0) fclose (outfile);
    return;
}
