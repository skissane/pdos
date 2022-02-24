/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "as.h"

char *output_file_name;
int flag_generate_listing;

static void perform_an_assembly_pass(int argc, char **argv)
{
    /* Skips argv[0]. */
    argc--;
    argv++;
    while (argc--)
    {
        if (*argv)
        {
            read_a_source_file(*argv);
        }
        argv++;
    }
}

void help(void)
{
    printf("Usage: pdas [option...] [asmfile...]\n");
    printf("Options:\n");
    printf("  -a                  Turns on listing.\n");
    printf("  -h, --help          Print this message and exit.\n");
    printf("  -o OBJFILE          Names the object file output OBJFILE"
           " (default a.out).\n");
    
}

int main(int argc, char **argv)
{
    output_file_name = "a.out";
    flag_generate_listing = 0;

    {
        /* argc and argv are modified so only input file names remain. */
        int new_argc = 0;
        int i;
        
        for (i = 0; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                switch (argv[i][1])
                {
                    case 'a':
                        flag_generate_listing = 1;
                        break;
                    
                    case 'h':
                        help();
                        return (0);

                    case 'o':
                        if (argv[i][2] == '\0')
                        {
                            output_file_name = argv[++i];
                        }
                        else
                        {
                            output_file_name = argv[i] + 2;
                        }
                        break;

                    case '-':
                        if (strcmp(argv[i] + 2, "help") == 0)
                        {
                            help();
                            return (0);
                        }
                        else printf("Unknown option `%s'\n", argv[i]);
                        break;

                    default:
                        printf("Unknown option `%s'\n", argv[i]);
                        break;
                }
                
                continue;
            }

            argv[new_argc] = argv[i];
            new_argc++;
        }

        argc = new_argc;
    }

    sections_init();
    current_section = text_section;
                    
    perform_an_assembly_pass(argc, argv);

    write_object_file();

    if (as_get_error_count()) return (EXIT_FAILURE);
    
    return (0);
}
