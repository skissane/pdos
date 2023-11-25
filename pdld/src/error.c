/******************************************************************************
 * @file            error.c
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
#include <stdarg.h>

#include "ld.h"

static int error_count = 0;

static void fprint_program_name (FILE *to)
{
    if (program_name) {
        fprintf (to, "%s: ", program_name);
    }
}

static void report_internal (const char *code_filename,
                             unsigned long code_line_number,
                             const char *fmt,
                             va_list ap,
                             const char *name)
{
    fprint_program_name (stderr);

    fprintf (stderr, "%s: ", name);
    vfprintf (stderr, fmt, ap);

    fprintf (stderr, "\n(Source code file: %s line: %lu)\n",
             code_filename ? code_filename : "(not provided)", code_line_number);
}

NORETURN void ld_internal_error_at_source (const char *code_filename,
                                           unsigned long code_line_number,
                                           const char *fmt,
                                           ...)
{
    va_list ap;
    
    va_start (ap, fmt);
    report_internal (code_filename, code_line_number, fmt, ap, "internal error");
    va_end (ap);

    /* Prevents thinking that the output file was linked. */
    if (ld_state->output_filename) remove (ld_state->output_filename);
    
    exit (EXIT_FAILURE);
}

static void report (const char *fmt,
                    va_list ap,
                    const char *name)
{
    fprint_program_name (stderr);

    fprintf (stderr, "%s: ", name);
    vfprintf (stderr, fmt, ap);
    fprintf (stderr, "\n");
}

NORETURN void ld_fatal_error (const char *fmt, ...)
{
    va_list ap;
    
    va_start (ap, fmt);
    report (fmt, ap, "fatal error");
    va_end (ap);

    /* Prevents thinking that the output file was linked. */
    if (ld_state->output_filename) remove (ld_state->output_filename);
    
    exit (EXIT_FAILURE);
}

void ld_error (const char *fmt, ...)
{
    va_list ap;
    
    va_start (ap, fmt);
    report (fmt, ap, "error");
    va_end (ap);

    error_count++;
}

void ld_warn (const char *fmt, ...)
{
    va_list ap;
    
    va_start (ap, fmt);
    report (fmt, ap, "warning");
    va_end (ap);
}

void ld_note (const char *fmt, ...)
{
    va_list ap;
    
    va_start (ap, fmt);
    report (fmt, ap, "note");
    va_end (ap);
}

int ld_get_error_count (void)
{
    return error_count;
}
