/* Released to the Public Domain */
/*
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static char *process_arg(const char *p, char *o)
{
    const char *q = p;

    while (!isspace(*q) && (*q != '\0') && (*q != ','))
    {
        if (*q == '%') q++;
        *(o++) = *(q++);
    }

    return (o);
}   

static char *process_args(const char *p, char *o)
{
    const char *q = strchr(p, ',');

    if (q == NULL)
    {
        /* Single argument. */
        return (process_arg(p, o));
    }

    q++;
    while (isspace(*q)) q++;

    o = process_arg(q, o);
    *(o++) = ',';
    *(o++) = ' ';
    
    return (process_arg(p, o));
}

int main(int argc, char **argv)
{
    FILE *f_in, *f_out;
    char buf[300], buf_out[300];
    
    if (argc != 3)
    {
        printf("usage: %s file output_file\n", argv[0]);
        return (0);
    }

    f_in = fopen(argv[1], "r");
    if (f_in == NULL)
    {
        printf("failed to open file %s\n", argv[1]);
        return (0);
    }

    f_out = fopen(argv[2], "w");
    if (f_out == NULL)
    {
        printf("failed to create file %s\n", argv[2]);
        fclose(f_in);
        return (0);
    }

    while (fgets(buf, 300, f_in))
    {
        const char *p = buf;
        char *o = buf_out;

        if (*p == '/')
        {
            /* A comment. */
            fputs(buf, f_out);
            continue;
        }

        /* Copies instruction with preceding and following whitespace. */
        while (isspace(*p)) p++;
        while (!isspace(*p)) p++;
        while (isspace(*p)) p++;
        memcpy(o, buf, p - buf);
        o += p - buf;

        if (*p != '\0')
        {
            o = process_args(p, o);
            *(o++) = '\n';
            *(o++) = '\0';
        }
        else *o = '\0';
        fputs(buf_out, f_out);
    }

    fclose(f_in);
    fclose(f_out);

    return (0);
}
