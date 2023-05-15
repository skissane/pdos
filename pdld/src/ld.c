/******************************************************************************
 * @file            ld.c
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

const char *program_name = NULL;

static struct ld_state ld_state_s = {0};
struct ld_state *ld_state = &ld_state_s;

int main (int argc, char **argv)
{
    char **input_filenames;
    int i;

    if (argc && *argv) {
        program_name = *argv;
    }

    ld_state->output_filename = "a.exe";

    input_filenames = ld_parse_args (argc, argv, 1);
    if (input_filenames == NULL) {
        if (ld_state->no_input_files_is_fine) exit (EXIT_SUCCESS);
        ld_fatal_error ("no input files");
    }
        
    symbols_init ();

    for (i = 0; i < argc; i++) {
        if (input_filenames[i]) coff_read (input_filenames[i]);
    }

    coff_before_link ();

    link ();

    coff_after_link ();
    
    coff_write (ld_state->output_filename);

    if (ld_state->output_map_filename) map_write (ld_state->output_map_filename);
    
    sections_destroy ();
    symbols_destroy ();

    free (input_filenames);

    if (ld_get_error_count ()) {
        remove (ld_state->output_filename);
        exit (EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}
