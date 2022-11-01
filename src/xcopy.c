/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  xcopy - copy a lot of files                                      */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pos.h>

static char from[FILENAME_MAX];
static char to[FILENAME_MAX];

static DTA *dta;

static int dolevel(void);

int main(int argc, char **argv)
{
    char *p;

    if (argc != 5)
    {
        printf("usage: xcopy <filespec> /s /e /n\n");
        printf("copies file structure into current directory\n");
        return (EXIT_FAILURE);
    }
    dta = PosGetDTA();
    strcpy(from, *(argv + 1));
    p = strstr(from, "*.*");
    if (p == NULL)
    {
        printf("must end with *.*\n");
        return (EXIT_FAILURE);
    }
    if ((p-- == from) || (*p != '\\'))
    {
        printf("must have backslash\n");
        return (EXIT_FAILURE);
    }
    *p = '\0';
    strcpy(to, ".");
    dolevel();
    return (0);
}

static int dolevel(void)
{
    char oldfrom[FILENAME_MAX];
    char oldto[FILENAME_MAX];
    DTA olddta;
    int ret;
    char *p;

    strcpy(oldfrom, from);
    strcpy(oldto, to);
    olddta = *dta;

    p = from + strlen(from);
    strcpy(p, "\\");
    strcpy(p + 1, "*.*");
    ret = PosFindFirst(from, 0x10);
    *p = '\0';
    while (ret == 0)
    {
        if ((dta->attrib & FILE_ATTR_DIRECTORY) != 0)
        {
            strcat(from, "\\");
            strcat(from, dta->lfn);
            strcat(to, "\\");
            strcat(to, dta->lfn);
            PosMakeDir(to);
            dolevel();
            strcpy(from, oldfrom);
            strcpy(to, oldto);
            *dta = olddta;
        }
        else
        {
            FILE *fp;
            FILE *fq;
            char in[FILENAME_MAX];
            char out[FILENAME_MAX];
            char buf[512];
            size_t cnt;

            strcpy(in, from);
            strcat(in, "\\");
            strcat(in, dta->lfn);
            strcpy(out, to);
            strcat(out, "\\");
            strcat(out, dta->lfn);
            fp = fopen(in, "rb");
            if (fp == NULL)
            {
                printf("failed to open %s for reading\n", in);
                exit(EXIT_FAILURE);
            }
            fq = fopen(out, "wb");
            if (fq == NULL)
            {
                printf("failed to open %s for writing\n", out);
                exit(EXIT_FAILURE);
            }
            while ((cnt = fread(buf, 1, sizeof buf, fp)) > 0)
            {
                fwrite(buf, 1, cnt, fq);
            }
            if (ferror(fp))
            {
                printf("error on read of %s\n", in);
                exit(EXIT_FAILURE);
            }
            if (ferror(fq))
            {
                printf("error on write of %s\n", out);
                exit(EXIT_FAILURE);
            }
            fclose(fp);
            fclose(fq);
        }
        ret = PosFindNext();
    }
    return (0);
}
