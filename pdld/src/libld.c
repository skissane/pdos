/******************************************************************************
 * @file            libld.c
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
#include "options.h"

enum option_index {

    LD_OPTION_IGNORED = 0,
    LD_OPTION_HELP,
    LD_OPTION_OUTPUT,
    LD_OPTION_OUT_IMPLIB,
    LD_OPTION_SHARED_LIBRARY,
    LD_OPTION_VERSION,
    LD_OPTION_VERSION_LONG

};

static const struct short_option short_options[] = {
    
    { 'o', LD_OPTION_OUTPUT, OPTION_HAS_ARG},
    { 's', LD_OPTION_IGNORED, OPTION_NO_ARG},
    { 'v', LD_OPTION_VERSION, OPTION_NO_ARG},
    { '\0', 0, 0}

};

#define STR_AND_LEN(str) (str), (sizeof (str) - 1)
static const struct long_option long_options[] = {
    
    { STR_AND_LEN("Bshareable"), LD_OPTION_SHARED_LIBRARY, OPTION_NO_ARG},
    { STR_AND_LEN("help"), LD_OPTION_HELP, OPTION_NO_ARG},
    { STR_AND_LEN("nostdlib"), LD_OPTION_IGNORED, OPTION_NO_ARG},
    { STR_AND_LEN("output"), LD_OPTION_OUTPUT, OPTION_HAS_ARG},
    { STR_AND_LEN("out-implib"), LD_OPTION_OUT_IMPLIB, OPTION_HAS_ARG},
    { STR_AND_LEN("shared"), LD_OPTION_SHARED_LIBRARY, OPTION_NO_ARG},
    { STR_AND_LEN("strip-all"), LD_OPTION_IGNORED, OPTION_NO_ARG},
    { STR_AND_LEN("version"), LD_OPTION_VERSION_LONG, OPTION_NO_ARG},
    { NULL, 0, 0}

};
#undef STR_AND_LEN

static void print_help (void)
{
    printf ("Usage: %s [options] file...\n", program_name ? program_name : "pdld");
    printf ("Options:\n");
    printf ("  --help                      Print option help\n");
    printf ("  -nostdlib                   Ignored\n");
    printf ("  -o FILE, --output FILE      Set output file name\n");
    printf ("  --out-implib FILE           Generate import library\n");
    printf ("  -shared, -Bshareable        Create a shared library\n");
    printf ("  -s, --strip-all             Ignored\n");
    printf ("  -v, --version               Print version information\n");
    
    coff_print_help ();
    
    exit (EXIT_SUCCESS);
}

static void use_option (enum option_index option_index, char *arg)
{
    switch (option_index) {

        case LD_OPTION_IGNORED:
            break;

        case LD_OPTION_HELP:
            print_help ();
            break;

        case LD_OPTION_OUTPUT:
            ld_state->output_filename = arg;
            break;

        case LD_OPTION_OUT_IMPLIB:
            ld_state->output_implib_filename = arg;
            break;

        case LD_OPTION_SHARED_LIBRARY:
            ld_state->create_shared_library = 1;
            break;

        case LD_OPTION_VERSION:
            printf ("pdld %i.%i\n", LD_MAJOR_VERSION, LD_MINOR_VERSION);
            ld_state->no_input_files_is_fine = 1;
            break;

        case LD_OPTION_VERSION_LONG:
            printf ("pdld %i.%i\n", LD_MAJOR_VERSION, LD_MINOR_VERSION);
            printf ("Released to the public domain.\n");
            exit (EXIT_SUCCESS);
            break;

    }
}

char **ld_parse_args (int argc, char **argv, int start_index)
{
    char **input_filenames;
    int i_arg;
    int at_least_one_input_filename = 0;

    if (argc == start_index) return NULL;

    input_filenames = xmalloc (sizeof (*input_filenames) * argc);
    memset (input_filenames, 0, sizeof (*input_filenames) * argc);

    for (i_arg = start_index; i_arg < argc; i_arg++) {
        char *after_hyphen;
        
        if (argv[i_arg][0] != '-') {
            input_filenames[i_arg] = argv[i_arg];
            at_least_one_input_filename = 1;
            continue;
        }

        after_hyphen = argv[i_arg] + 1;

        if (argv[i_arg][1] != '-') {
            const struct short_option *s_option;
            
            for (s_option = short_options; s_option->letter; s_option++) {
                if (s_option->letter == argv[i_arg][1]) break;
            }

            if (s_option->letter) {
                if ((s_option->flags & OPTION_NO_ARG) && argv[i_arg][2] == '\0') {
                    use_option (s_option->index, NULL);
                    continue;
                }
                if (s_option->flags & OPTION_HAS_ARG) {
                    char *arg;
                    if (argv[i_arg][2]) arg = argv[i_arg] + 2;
                    else if (i_arg + 1 < argc) arg = argv[++i_arg];
                    else {
                        ld_error ("argument to '-%c' is missing", s_option->letter);
                        continue;
                    }
                    use_option (s_option->index, arg);
                    continue;
                }
            }
        } else after_hyphen = argv[i_arg] + 2;

        {
            const struct long_option *l_option;
            const struct long_option *object_dependent_options;

            object_dependent_options = coff_get_long_options (); 

            for (l_option = long_options;
                 l_option->name
                 || (object_dependent_options
                     && (l_option = object_dependent_options,
                         object_dependent_options = NULL,
                         l_option->name));
                 l_option++) {
                if (strncmp (l_option->name, after_hyphen, l_option->name_len) == 0
                    && (after_hyphen[l_option->name_len] == '\0'
                        || after_hyphen[l_option->name_len] == '=')) break;
            }

            if ((l_option->flags & OPTION_NO_ARG)
                && after_hyphen[l_option->name_len] == '\0') {
                if (object_dependent_options) use_option (l_option->index, NULL);
                else coff_use_option (l_option->index, NULL);
                continue;
            }
            if (l_option->flags & OPTION_HAS_ARG) {
                char *arg;
                if (after_hyphen[l_option->name_len] == '=') arg = after_hyphen + l_option->name_len + 1;
                else if (i_arg + 1 < argc) arg = argv[++i_arg];
                else {
                    ld_error ("argument to '%s' is missing", argv[i_arg]);
                    continue;
                }
                if (object_dependent_options) use_option (l_option->index, arg);
                else coff_use_option (l_option->index, arg);
                continue;
            }
        }

        ld_error ("unrecognized command-line option '%s'", argv[i_arg]);
        ld_note ("use the --help option for usage information");
    }

    if (at_least_one_input_filename == 0) {
        free (input_filenames);
        input_filenames = NULL;
    }

    return input_filenames;
}        

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
