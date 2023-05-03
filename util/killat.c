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

    if (argc <= 3)
    {
        printf("usage: killat <funclist> <asm> <def>\n");
        printf("input file looks like:\n");
        printf("_CreateProcessA@40\n");
        printf("note that the output assembler file is not needed\n");
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
    while (fgets(buf, sizeof buf, fp) != NULL)
    {
        char *p;

        p = strchr(buf, '\n');
        if (p == NULL) break;
        *p = '\0';
        p = strchr(buf, '@');
        if (p == NULL) break;
        fprintf(fq, "extrn %s:proc\npublic ", buf);
        fwrite(buf + 1, 1, p - buf - 1, fq);
        fprintf(fq, "\n");
        fwrite(buf + 1, 1, p - buf - 1, fq);
        fprintf(fq, ": jmp %s\n\n", buf);

        fprintf(fr, "EXPORTS ");
        fwrite(buf + 1, 1, p - buf - 1, fr);
        fprintf(fr, "\n");

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
    }
    return (0);
}
