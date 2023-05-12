/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  killat - kill at signs in stdcall functions                      */
/*                                                                   */
/*  You can use an editor to get the input file into shape           */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buf[2048];
    FILE *fp;
    FILE *fq;
    FILE *fr;
    FILE *fs;
    FILE *ft;

    if (argc <= 5)
    {
        printf("usage: killat <funclist> <asm> <def> <wat> <wat2>\n");
        printf("input file (funclist) looks like:\n");
        printf("_CreateProcessA@40\n");
        printf("You can do link -map (with Visual Studio) to get this, "
               "and use an editor\n");
        printf("or grep \" f \" kernel32.map | grep kernel32.obj\n");
        printf("to clean it up\n");
        return (EXIT_FAILURE);
    }
    fp = fopen(*(argv + 1), "r");
    if (fp == NULL)
    {
        printf("failed to open %s for read\n", *(argv + 1));
        return (EXIT_FAILURE);
    }
    fq = fopen(*(argv + 2), "w");
    if (fq == NULL)
    {
        printf("failed to open %s for write\n", *(argv + 2));
        return (EXIT_FAILURE);
    }
    fr = fopen(*(argv + 3), "w");
    if (fr == NULL)
    {
        printf("failed to open %s for write\n", *(argv + 3));
        return (EXIT_FAILURE);
    }
    fs = fopen(*(argv + 4), "w");
    if (fs == NULL)
    {
        printf("failed to open %s for write\n", *(argv + 4));
        return (EXIT_FAILURE);
    }
    ft = fopen(*(argv + 5), "w");
    if (ft == NULL)
    {
        printf("failed to open %s for write\n", *(argv + 5));
        return (EXIT_FAILURE);
    }
    while (fgets(buf, sizeof buf, fp) != NULL)
    {
        char *p;
        int x;
        int tot;

        p = strchr(buf, '\n');
        if (p == NULL) break;
        *p = '\0';

#if 0

extrn __imp__CreateDirectoryA@8:ptr
public _CreateDirectoryA
_CreateDirectoryA:
push 8[esp]
push 8[esp]
call [__imp__CreateDirectoryA@8]
ret
ret

#endif


#if 1
do {
        p = strchr(buf, '@');
        if (p == NULL) break;
        fprintf(fq, "extrn __imp_%s:ptr\n", buf);
        fprintf(fq, "public ");
        fwrite(buf, 1, p - buf, fq);
        fprintf(fq, "\n");
        fwrite(buf, 1, p - buf, fq);
        fprintf(fq, ":\n");
        tot = atoi(p + 1) / 4;
        for (x = 0; x < tot; x++)
        {
            fprintf(fq, "push %s[esp]\n", p + 1);
        }
        fprintf(fq, "call [__imp_%s]\n", buf);
        fprintf(fq, "ret\nret\n\n");
} while (0);
#endif

#if 0
        p = strchr(buf, '@');
        if (p == NULL) break;
        fprintf(fq, "extrn %s:proc\npublic ", buf);
        fwrite(buf + 1, 1, p - buf - 1, fq);
        fprintf(fq, "\n");
        fwrite(buf + 1, 1, p - buf - 1, fq);
        fprintf(fq, ": jmp %s\n\n", buf);
#endif

#if 0
        fprintf(fq, "extrn %s:proc\n", buf);
        fprintf(fq, "public %s\n", buf + 1);
        fprintf(fq, "%s: jmp %s\n\n", buf + 1, buf);
#endif

do {
        p = strchr(buf, '@');
        if (p == NULL) break;
        fprintf(fr, "EXPORTS ");
        fwrite(buf + 1, 1, p - buf - 1, fr);
        fprintf(fr, "\n");
} while (0);

#if 0
        fprintf(fr, "EXPORTS %s=", buf);
        fwrite(buf + 1, 1, p - buf - 1, fr);
        fprintf(fr, "\n");
#endif
#if 0
        fprintf(fr, "EXPORTS ");
        fwrite(buf + 1, 1, p - buf - 1, fr);
        fprintf(fr, "=%s\n", buf);
#endif
#if 0
        fprintf(fr, "EXPORTS %s\n", buf);
#endif
#if 0
        fprintf(fr, "EXPORTS ");
        fwrite(buf + 1, 1, p - buf - 1, fr);
        fprintf(fr, "\n");
#endif

do {
        p = strchr(buf, '@');
        if (p == NULL) break;
        fprintf(fs, "++");
        fwrite(buf + 1, 1, p - buf - 1, fs);
        fprintf(fs, ".kernel32.");
        fprintf(fs, "%s\n", buf);
} while (0);

        fprintf(ft, "++%s.msvcrt._%s\n", buf, buf);

    }
    return (0);
}
