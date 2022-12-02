/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  zip - zip up files                                               */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pos.h>

static char from[FILENAME_MAX];

static char filespec[FILENAME_MAX];

static DTA *dta;

static FILE *outf;

static int compress = 0;

static int dolevel(void);

int main(int argc, char **argv)
{
    char *p;

    if (argc != 4)
    {
        printf("usage: zip <-0rX/-9rX> <destfile> <filespec>\n");
        printf("zips up files into a zip archive\n");
        printf("e.g. zip -9rX \\download\\fred.zip *.c\n");
        printf("or zip -0rX \\download\\fred.zip mydir\n");
        return (EXIT_FAILURE);
    }
    dta = PosGetDTA();
    if (strcmp(*(argv + 1), "-0rX") == 0)
    {
        compress = 0;
    }
    else if (strcmp(*(argv + 1), "-9rX") == 0)
    {
        compress = 1;
    }
    else
    {
        printf("need either -0rX or -9rX\n");
        return (EXIT_FAILURE);
    }
    outf = fopen(*(argv + 2), "wb");
    if (outf == NULL)
    {
        printf("failed to open %s for writing\n", *(argv + 2));
        return (EXIT_FAILURE);
    }
    p = strchr(*(argv + 3), '.');
    if (p == NULL)
    {
        strcpy(from, *(argv + 3));
        strcpy(filespec, "*");
    }
    else
    {
        strcpy(from, ".");
        strcpy(filespec, *(argv + 3));
    }
    dolevel();
    if (ferror(outf))
    {
        printf("error on output file\n");
        return (EXIT_FAILURE);
    }
    fclose(outf);
    return (0);
}

static int dolevel(void)
{
    char oldfrom[FILENAME_MAX];
    DTA olddta;
    int ret;
    char *p;

    strcpy(oldfrom, from);

    p = from + strlen(from);
    strcpy(p, "/");
    strcpy(p + 1, filespec);
    ret = PosFindFirst(from, 0x10);
    *p = '\0';
    while (ret == 0)
    {
        if ((dta->attrib & FILE_ATTR_DIRECTORY) != 0)
        {
            strcat(from, "/");
            strcat(from, dta->lfn);
            if (dta->lfn[0] == '\0')
            {
                strcat(from, dta->file_name);
            }
            if ((strcmp(dta->file_name, ".") == 0)
                || (strcmp(dta->file_name, "..") == 0))
            {
                /* ignore */
            }
            else
            {
                olddta = *dta;
                dolevel();
                *dta = olddta;
            }
            strcpy(from, oldfrom);
        }
        else
        {
            FILE *fp;
            char in[FILENAME_MAX];
            char buf[512];
            size_t cnt;

            strcpy(in, from);
            strcat(in, "/");
            strcat(in, dta->lfn);
            if (dta->lfn[0] == '\0')
            {
                strcat(in, dta->file_name);
            }
            printf("%s\n", in);
            fp = fopen(in, "rb");
            if (fp == NULL)
            {
                printf("failed to open %s for reading\n", in);
                exit(EXIT_FAILURE);
            }
            p = in;
            if (strncmp(in, "./", 2) == 0)
            {
                p += 2;
            }
            fprintf(outf, "%s", p);
            fputc('\0', outf);
            while ((cnt = fread(buf, 1, sizeof buf, fp)) > 0)
            {
                fwrite(buf, 1, cnt, outf);
            }
            if (ferror(fp))
            {
                printf("error on read of %s\n", in);
                exit(EXIT_FAILURE);
            }
            if (ferror(outf))
            {
                printf("write error\n");
                exit(EXIT_FAILURE);
            }
            fclose(fp);
        }
        ret = PosFindNext();
    }
    return (0);
}
