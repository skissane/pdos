/******************************************************************************
 * @file            xmalloc.c
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
#include <ctype.h>

void *xmalloc (size_t size)
{
    void *p = malloc (size);

    if (p == NULL) {
        fprintf (stderr, "Failed to allocate memory!\n");
        exit (EXIT_FAILURE);
    }
    
    return p;
}

void *xcalloc (size_t num, size_t size)
{
    void *p;
    
    p = xmalloc (num * size);
    memset (p, '\0', num * size);
    
    return p;
}

void *xrealloc (void *p, size_t size)
{
    p = realloc (p, size);

    if (p == NULL) {
        fprintf (stderr, "Failed to allocate memory!\n");
        exit (EXIT_FAILURE);
    }

    return p;
}

char *xstrdup (const char *str)
{
    size_t size = strlen (str) + 1;
    char *p = xmalloc (size);

    memcpy (p, str, size);

    return p;
}

size_t strnlen (const char *str, size_t max_len)
{
    size_t len;
    
    for (len = 0; (len < max_len) && str[len]; len++);

    return len;
}

char *xstrndup (const char *str, size_t max_len)
{
    size_t len = strnlen (str, max_len);
    char *p = xmalloc (len + 1);

    p[len] = '\0';
    memcpy (p, str, len);

    return p;
}

int xstrcasecmp (const char *s1, const char *s2)
{
    const unsigned char *p1;
    const unsigned char *p2;

    p1 = (const unsigned char *)s1;
    p2 = (const unsigned char *)s2;
    while (*p1 != '\0') {
        if (toupper (*p1) < toupper (*p2)) return -1;
        else if (toupper (*p1) > toupper (*p2)) return 1;
        p1++;
        p2++;
    }
    if (*p2 == '\0') return 0;
    else return -1;
}
