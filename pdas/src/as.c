/******************************************************************************
 * @file            as.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include    <stddef.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "as.h"
#include    "libas.h"
#include    "cfi.h"

struct as_state *state;
const char *program_name = 0;

int main (int argc, char **argv)
{
    char **pargv = argv;
    int pargc = argc;
    
    int i;
    
    if (argc && *argv) {
        char *p;
        program_name = *argv;
        
        if ((p = strrchr (program_name, '/'))) {
            program_name = (p + 1);
        }
    }
    
    state = xmalloc (sizeof (*state));
    memset (state, 0, sizeof (*state));
    
#if      defined (USE_COFF_BY_DEFAULT)
    state->format = AS_FORMAT_COFF;
#elif      defined (USE_ELF_BY_DEFAULT)
    state->format = AS_FORMAT_ELF;
#endif
    
    as_parse_args (&pargc, &pargv, 1);
    
    sections_init ();
    process_init ();
    
    machine_dependent_init ();

    as_use_defsyms ();
    
    if (state->nb_files == 0) {
        if (program_name) {
            fprintf (stderr, "%s: ", program_name);
        }
        
        fprintf (stderr, "error: no input files provided\n");
        exit (EXIT_FAILURE);
    }
    
    for (i = 0; i < state->nb_files; i++) {
        if (process (state->files[i])) {
            if (program_name) {
                fprintf (stderr, "%s: ", program_name);
            }
            
            as_error_at (NULL, 0, "error: failed to open '%s' for reading", state->files[i]);
            continue;
        }
    }

    cfi_finish ();
    
    write_object_file ();
    
    if (state->generate_listing) {
        generate_listing ();
        listing_destroy ();
    }

    machine_dependent_destroy ();
    process_destroy ();
    sections_destroy ();
    symbols_destroy ();
    
    if (as_get_error_count ()) {
        remove (state->outfile);
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
