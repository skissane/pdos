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

void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL)
    {
        fprintf(stderr, "failed to allocate %u bytes\n", size);
        abort();
    }
    return (p);
}

void *xcalloc(size_t size)
{
    void *p = xmalloc(size);
    memset(p, '\0', size);
    return p;
}

void *xrealloc(void *p, size_t size)
{
    p = realloc(p, size);
    if (p == NULL)
    {
        fprintf(stderr, "failed to reallocate %u bytes\n", size);
        abort();
    }
    return (p);
}

char *xstrdup(const char *str)
{
    size_t len = strlen(str);
    char *out = xmalloc(len + 1);
    strcpy(out, str);
    return (out);
}

char *xstrndup(const char *str, size_t len)
{
    char *out = xmalloc(len + 1);
    strncpy(out, str, len);
    return (out);
}

