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
#include <ctype.h>
#include <assert.h>
#include "diag.h"
#include "cclib.h"

void cc_report (cc_reader *reader, enum cc_diagnostic_level level,
                const char *msg, ...)
{
    const location_t *loc = &reader->curr_token->loc;
    char line[80];
    
    va_list arg;
    
    va_start (arg, msg);
    if (loc->file) {
        fprintf (stderr, TTY_BOLD "%s:%lu:%i: " TTY_RESET,
                 loc->file, loc->line, loc->column);
    } else {
        fprintf (stderr, TTY_BOLD "<unknown>: " TTY_RESET);
    }
    switch (level) {
        case CC_DL_NOTE: fprintf (stderr, TTY_CYAN "note: " TTY_RESET); break;
        case CC_DL_WARNING: fprintf (stderr, TTY_MAGENTA "warning: " TTY_RESET); break;
        case CC_DL_ERROR: fprintf (stderr, TTY_RED "error: " TTY_RESET); break;
    }
    vfprintf(stderr, msg, arg);
    va_end(arg);

    if (reader->input != NULL) {
        size_t line_cnt = 0;
        
        fprintf(stderr, ":" TTY_RESET "\n");
        rewind(reader->input);
        while (fgets(line, 80, reader->input) && line_cnt != loc->line)
            line_cnt++;

        if (line_cnt == loc->line)
        {
            char *p = strchr(line, '\n');
            size_t i;
            if (p) *p = '\0';
            fprintf(stderr, "%s\n", line);
            for (i = 0; i < loc->column; i++)
                fputc('-', stderr);
            fprintf(stderr, "^\n");
        }
    } else {
        fprintf(stderr, TTY_RESET "\n");
    }

    if (level == CC_DL_ERROR)
        exit(EXIT_FAILURE);
}
