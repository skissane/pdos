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
#include    <stdarg.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "as.h"

unsigned long error_count = 0;

static void report (const char *filename, unsigned long line_number, const char *fmt, va_list ap, const char *name) {

    char *message;
    
    if (filename) {
    
        if (line_number == 0) {
            fprintf (stderr, "%s: ", filename);
        } else {
            fprintf (stderr, "%s:", filename);
        }
    
    }
    
    if (line_number > 0) {
        fprintf (stderr, "%lu: ", line_number);
    }
    
    /* +++This is a very bad way of doing this but without va_copy or vsnprintf or rewriting vsprintf nothing else can be done. */
    message = xmalloc ((name ? strlen (name) + 2 : 0) + 300 + 1);
    *message = '\0';
    
    if (name) {
    
        strcpy (message, name);
        strcat (message, ": ");
    
    }
    
    vsprintf (message + strlen (message), fmt, ap);
    fprintf (stderr, "%s\n", message);

    if (filename) {
        add_listing_message (message, filename, line_number);
    } else {
        free (message);
    }

}

unsigned long as_get_error_count (void) {
    return error_count;
}

void as_error (const char *fmt, ...) {

    va_list ap;
    
    const char *filename;
    unsigned long line_number;
    
    get_filename_and_line_number (&filename, &line_number);
    
    va_start (ap, fmt);
    report (filename, line_number, fmt, ap, "error");
    va_end (ap);
    
    error_count++;

}

void as_error_at (const char *filename, unsigned long line_number, const char *fmt, ...) {

    va_list ap;
    
    va_start (ap, fmt);
    report (filename, line_number, fmt, ap, "error");
    va_end (ap);
    
    error_count++;

}

void as_fatal_error_at (const char *filename, unsigned long line_number, const char *fmt, ...) {

    va_list ap;
    
    va_start (ap, fmt);

    if (filename) {
    
        if (line_number == 0) {
            fprintf (stderr, "%s: ", filename);
        } else {
            fprintf (stderr, "%s:", filename);
        }
    
    }
    
    if (line_number > 0) {
        fprintf (stderr, "%lu: ", line_number);
    }
    
    fprintf (stderr, "fatal error: ");
    vfprintf (stderr, fmt, ap);
    fprintf (stderr, "\n");
    
    va_end (ap);
    
    /* Prevents thinking that the file was assembled. */
    if (state->outfile) {
        remove (state->outfile);
    }
    
    exit (EXIT_FAILURE);

}

void as_warn (const char *fmt, ...) {

    va_list ap;
    
    const char *filename;
    unsigned long line_number;
    
    get_filename_and_line_number (&filename, &line_number);
    
    va_start (ap, fmt);
    report (filename, line_number, fmt, ap, "warning");
    va_end (ap);

}

void as_warn_at (const char *filename, unsigned long line_number, const char *fmt, ...) {

    va_list ap;
    
    va_start (ap, fmt);
    report (filename, line_number, fmt, ap, "warning");
    va_end (ap);

}

static void report_internal (const char *code_filename,
                             unsigned long code_line_number,
                             const char *filename,
                             unsigned long line_number,
                             const char *fmt,
                             va_list ap,
                             const char *name) {

    if (filename) {
    
        if (line_number == 0) {
            fprintf (stderr, "%s: ", filename);
        } else {
            fprintf (stderr, "%s:", filename);
        }
    
    }
    
    if (line_number > 0) {
        fprintf (stderr, "%lu: ", line_number);
    }

    fprintf (stderr, "%s: ", name);
    vfprintf (stderr, fmt, ap);

    fprintf (stderr, "\n(Source code file: %s line: %lu)\n",
             code_filename ? code_filename : "(not provided)", code_line_number);

}

void as_internal_error_at_source (const char *code_filename,
                                  unsigned long code_line_number,
                                  const char *fmt,
                                  ...) {

    va_list ap;

    const char *filename;
    unsigned long line_number;
    
    get_filename_and_line_number (&filename, &line_number);
    
    va_start (ap, fmt);
    report_internal (code_filename, code_line_number, filename, line_number, fmt, ap, "internal error");
    va_end (ap);

    /* Prevents thinking that the file was assembled. */
    if (state->outfile) {
        remove (state->outfile);
    }
    
    exit (EXIT_FAILURE);

}

void as_internal_error_at_source_at (const char *code_filename,
                                     unsigned long code_line_number,
                                     const char *filename,
                                     unsigned long line_number,
                                     const char *fmt,
                                     ...) {

    va_list ap;
    
    va_start (ap, fmt);
    report_internal (code_filename, code_line_number, filename, line_number, fmt, ap, "internal error");
    va_end (ap);

    /* Prevents thinking that the file was assembled. */
    if (state->outfile) {
        remove (state->outfile);
    }
    
    exit (EXIT_FAILURE);

}
