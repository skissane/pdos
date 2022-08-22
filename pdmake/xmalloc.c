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
        fprintf(stderr, "Failed to allocate memory!\n");
        exit(1);
    }

    return (p);
}

void *xrealloc(void *p, size_t size)
{
    p = realloc(p, size);

    if (p == NULL)
    {
        fprintf(stderr, "Failed to allocate memory!\n");
        exit(1);
    }

    return (p);
}

char *xstrdup(char *str)
{
    size_t size = strlen(str) + 1;
    char *p = xmalloc(size);

    memcpy(p, str, size);

    return (p);
}

size_t strnlen(const char *str, size_t max_len)
{
    size_t len;

    for (len = 0; (len < max_len) && str[len]; len++);

    return (len);
}

char *xstrndup(char *str, size_t max_len)
{
    size_t len = strnlen(str, max_len);
    char *p = xmalloc(len + 1);

    p[len] = '\0';
    memcpy(p, str, len);

    return (p);
}
