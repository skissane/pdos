/******************************************************************************
 * @file            options.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/

/* To be called when error occurs duting processing an option. */
void _error (const char *fmt, ...);

struct as_option {

    const char *name;
    int index, flags;

};

#define     AS_OPTION_NO_ARG            0x0000
#define     AS_OPTION_HAS_ARG           0x0001
#define     AS_OPTION_HAS_OPTIONAL_ARG  0x0002

const struct as_option *machine_dependent_get_options (void);
void machine_dependent_print_help (void);
void machine_dependent_handle_option (const struct as_option *popt, const char *optarg);
