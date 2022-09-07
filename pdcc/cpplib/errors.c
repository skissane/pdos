/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include "cpplib.h"
#include "internal.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

static void cpp_diagnostics(cpp_reader *reader,
                            enum cpp_diagnostic_level level,
                            int ignored,
                            const char *message,
                            va_list *vl)
{
    location_t loc;
    
    assert(reader->callbacks.diagnostics);
    if (reader->cur_token == reader->cur_tokenrow->start)
    {
        loc.file = NULL;
    }
    else
    {
        loc = reader->cur_token[-1].src_loc;
    }
    
    reader->callbacks.diagnostics(reader, level, ignored,
                                  loc, message, vl);
}

void cpp_error(cpp_reader *reader, enum cpp_diagnostic_level level,
               const char *message, ...)
{
    va_list vl;

    va_start(vl, message);

    cpp_diagnostics(reader, level, 0, message, &vl);

    va_end(vl);
}

static void cpp_diagnostics_with_line(cpp_reader *reader,
                                      enum cpp_diagnostic_level level,
                                      int ignored,
                                      location_t src_loc,
                                      unsigned int column,
                                      const char *message,
                                      va_list *vl)
{
    assert(reader->callbacks.diagnostics);
    if (column) src_loc.column = column;
    reader->callbacks.diagnostics(reader, level, ignored,
                                  src_loc, message, vl);
}

void cpp_error_with_line(cpp_reader *reader, enum cpp_diagnostic_level level,
                         location_t loc, unsigned int column,
                         const char *message, ...)
{
    va_list vl;

    va_start(vl, message);

    cpp_diagnostics_with_line(reader, level, 0, loc, column, message, &vl);

    va_end(vl);
}
