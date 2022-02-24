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

#include <stdarg.h>

static unsigned long error_count = 0;

unsigned long as_get_error_count(void)
{
    return (error_count);
}

void as_error(char *format, ...)
{
    va_list args;
    
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    error_count++;
}
