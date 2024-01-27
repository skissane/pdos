/******************************************************************************
 * @file            libas.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "as.h"
#include "cstr.h"
#include "libas.h"
#include "options.h"

enum option_index {

    AS_OPTION_IGNORED = 0,
    AS_OPTION_DEFSYM,
    AS_OPTION_HELP,
    AS_OPTION_INCLUDE,
    AS_OPTION_LISTING,
    AS_OPTION_OUTFILE,
    AS_OPTION_OFORMAT

};

static const struct as_option as_options[] = {

    { "a",          AS_OPTION_LISTING,      AS_OPTION_HAS_OPTIONAL_ARG  },
    { "-defsym",    AS_OPTION_DEFSYM,       AS_OPTION_HAS_ARG           },
    { "-help",      AS_OPTION_HELP,         AS_OPTION_NO_ARG            },
    { "I",          AS_OPTION_INCLUDE,      AS_OPTION_HAS_ARG           },
    { "o",          AS_OPTION_OUTFILE,      AS_OPTION_HAS_ARG           },
    { "-oformat",   AS_OPTION_OFORMAT,      AS_OPTION_HAS_ARG           },
    { NULL,         0,                      0                           }

};

struct defsym {
    struct defsym *next;
    const char *name;
    value_t value;
};

static struct defsym *defsyms = NULL;

static void _add_include_path (const char *pathname) {


    const char *in = pathname;
    const char *p;
    
    do {
    
        int c;
        
        CString str;
        cstr_new (&str);
        
        for (p = in; c = *p, c != '\0' && c != PATHSEP[0]; ++p) {
            cstr_ccat (&str, c);
        }
        
        if (str.size) {
        
            cstr_ccat (&str, '\0');
            dynarray_add (&state->nb_include_paths, &state->include_paths, xstrdup (str.data));
        
        }
        
        cstr_free (&str);
        in = (p + 1);
    
    } while (*p != '\0');

}

static void _convert_to_lower (char *out, const char *str) {

    int ch;
    
    while ((ch = *str++)) {
        *out++ = tolower (ch);
    }
    *out = '\0';

}

void _error (const char *fmt, ...) {

    va_list ap;
    
    if (program_name) {
        fprintf (stderr, "%s: ", program_name);
    }
    
    fprintf (stderr, "error: ");
    
    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    
    fprintf (stderr, "\n");
    exit (EXIT_FAILURE);

}

static void _print_help (const char *name)
{
    const char *default_format;
    char *p;
    
#if defined (USE_COFF_BY_DEFAULT)
    default_format = "coff";
#elif defined (USE_ELF_BY_DEFAULT)
    default_format = "elf";
#else
    default_format = "a.out";
#endif
    
    if ((p = strrchr (name, '/'))) {
        name = (p + 1);
    }
    
    printf ("Usage: %s [options] asmfile...\n\n", name);

    printf ("    -a[=FILE]             Print listings to stdout or a specified file\n");
    printf ("    --defsym SYM=VAL      Define symbol SYM to given value");
    printf ("    --help                Print this help information\n");
    printf ("    -I DIR                Add DIR to search list for .include directives\n");
    printf ("    -o OBJFILE            Name the object-file output OBJFILE (default a.out)\n");
    printf ("    --oformat FORMAT      Create an output file in format FORMAT (default %s)\n", default_format);
    printf ("                              Supported formats are: a.out, coff, elf\n");

    machine_dependent_print_help ();
    
    printf ("\n");
    exit (EXIT_SUCCESS);
}

static int _strstart (const char *val, const char **str) {

    const char *p = *str;
    const char *q = val;
    
    while (*q != '\0') {
    
        if (*p != *q) {
            return 0;
        }
        
        ++p;
        ++q;
    
    }
    
    *str = p;
    return 1;

}

void as_parse_args (int *pargc, char ***pargv, int optind)
{
    char **argv = *pargv;
    int argc = *pargc;
    
    const struct as_option *popt;
    const char *optarg, *r;
    
    const struct as_option *machine_dependent_options;

    if (argc == optind) {
        _print_help (argv[0]);
    }

    machine_dependent_options = machine_dependent_get_options ();
    
    while (optind < argc) {

        int is_machine_dependent_option = 0;
    
        r = argv[optind++];
        
        if (r[0] != '-' || r[1] == '\0') {
        
            dynarray_add (&state->nb_files, &state->files, xstrdup (r));
            continue;
        
        }
        
        for (popt = as_options; ; ++popt) {
        
            const char *p1 = popt->name;
            const char *r1 = (r + 1);
            
            if (p1 == NULL) {

                if (!is_machine_dependent_option) {
                    
                    is_machine_dependent_option = 1;
                    popt = machine_dependent_options;
                    p1 = popt->name;

                } else {
                    _error ("invalid option -- '%s'", r);
                }
                
            }
            
            if (!_strstart (p1, &r1)) {
                continue;
            }
            
            optarg = r1;
            
            if (popt->flags & AS_OPTION_HAS_ARG) {
            
                if (*r1 == '\0') {
                
                    if (optind >= argc) {
                        _error ("argument to '%s' is missing", r);
                    }
                    
                    optarg = argv[optind++];
                
                } else if (*r1 == '=') {
                    optarg = r1 + 1;
                }
            
            } else if (popt->flags & AS_OPTION_HAS_OPTIONAL_ARG) {
            
                if (*optarg == '=') {
                    ++optarg;
                }
            
            } else if (*r1 != '\0') {
                continue;
            }
            
            break;
        
        }

        if (is_machine_dependent_option) {
            machine_dependent_handle_option (popt, optarg);
            continue;
        }
        
        switch (popt->index) {

            case AS_OPTION_DEFSYM:
                {
                    struct defsym *defsym;
                    char *p = strchr (optarg, '=');

                    if (!p) {
                        _error ("bad defsym; format is --defsym name=value");
                    }
                    *p = '\0';
                    p++;

                    defsym = xmalloc (sizeof (*defsym));
                    defsym->next = defsyms;
                    defsym->name = optarg;
                    defsym->value = strtoul (p, NULL, 0);
                    defsyms = defsym;                    
                }
                break;                
            
            case AS_OPTION_HELP:
            
                _print_help (argv[0]);
                break;
            
            case AS_OPTION_INCLUDE:
            
                _add_include_path (optarg);
                break;
            
            case AS_OPTION_LISTING:
            
                state->generate_listing = 1;
                
                if (*optarg != '\0') {
                    state->listing = xstrdup (optarg);
                }
                
                break;
            
            case AS_OPTION_OUTFILE:
            
                if (state->outfile) {
                    _error ("multiple output files provided");
                }
                
                state->outfile = xstrdup (optarg);
                break;

            case AS_OPTION_OFORMAT: {
            
                char *temp = xmalloc (strlen (optarg) + 1);
                _convert_to_lower (temp, optarg);
                
                if (strcmp (temp, "a.out") == 0) {
                    state->format = AS_FORMAT_A_OUT;
                } else if (strcmp (temp, "coff") == 0) {
                    state->format = AS_FORMAT_COFF;
                } else if (strcmp (temp, "elf") == 0) {
                    state->format = AS_FORMAT_ELF;
                } else {
                
                    free (temp);
                    _error ("unsuppored format '%s' specified\n", optarg);
                
                }
                
                free (temp);
                break;
            
            }
            
            default:
            
                _error ("unsupported option '%s'", r);
                break;
        
        }
    
    }
    
    if (!state->outfile) { state->outfile = "a.out"; }
}

void as_use_defsyms (void)
{
    struct defsym *defsym, *next;

    for (defsym = defsyms; defsym; defsym = next) {
        struct symbol *symbol = symbol_find_or_make (defsym->name);

        symbol_set_frag (symbol, &zero_address_frag);
        symbol_set_section (symbol, absolute_section);
        symbol_set_value (symbol, defsym->value);

        next = defsym->next;
        free (defsym);
    }
}

void dynarray_add (int *nb_ptr, void *ptab, void *data) {

    int nb, nb_alloc;
    void **pp;
    
    nb = *nb_ptr;
    pp = *(void ***) ptab;
    
    if ((nb & (nb - 1)) == 0) {
    
        if (!nb) {
            nb_alloc = 1;
        } else {
            nb_alloc = nb * 2;
        }
        
        pp = xrealloc (pp, nb_alloc * sizeof (void *));
        *(void ***) ptab = pp;
    
    }
    
    pp[nb++] = data;
    *nb_ptr = nb;

}


char *skip_whitespace (char *p) {
    return *p == ' ' ? p + 1 : p;
}

void *xmalloc (size_t size) {

    void *ptr = malloc (size);
    
    if (!ptr && size) {
        _error ("memory full (malloc)\n");
    }
    
    return ptr;

}

void *xrealloc (void *ptr, size_t size) {

    void *new_ptr = realloc (ptr, size);
    
    if (!new_ptr && size) {
        _error ("memory full (realloc)\n");
    }
    
    return new_ptr;

}

int xstrcasecmp (const char *s1, const char *s2) {

    const unsigned char *p1;
    const unsigned char *p2;
    
    p1 = (const unsigned char *) s1;
    p2 = (const unsigned char *) s2;
    
    while (*p1 != '\0') {
    
        if (toupper (*p1) < toupper (*p2)) {
            return (-1);
        } else if (toupper (*p1) > toupper (*p2)) {
            return (1);
        }
        
        p1++;
        p2++;
    
    }
    
    if (*p2 == '\0') {
        return (0);
    } else {
        return (-1);
    }

}

char *xstrdup (const char *str) {

    char *ptr = xmalloc (strlen (str) + 1);
    strcpy (ptr, str);
    
    return ptr;

}

char *xstrsep (char **stringp, const char *delim) {

    char *rv = *stringp;
    
    if (rv) {
    
        *stringp += strcspn (*stringp, delim);
        
        if (**stringp) {
            *(*stringp)++ = '\0';
        } else {
            *stringp = 0;
        }
    
    }
    
    return rv;

}
