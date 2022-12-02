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
    fputc(0x50, outf);
    fputc(0x4b, outf);
    fputc(0x03, outf);
    fputc(0x04, outf);
    /* I don't know what this is, but it seems consistent */
    fwrite("\x0A\x00\x00\x00\x00\x00\xCB\x4C\x75\x55", 10, 1, outf);
    /* This doesn't appear to be archive size, but could be a timestamp */
    fwrite("\x00\x00\x00\x00", 4, 1, outf);
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
    fwrite("\x50\x4B\x01\x02\x1E\x00\x0A\x00", 8, 1, outf);
    fwrite("\x00\x00\x00\x00\xCB\x4C\x75\x55", 8, 1, outf);
    fwrite("\x7C\xC7\x8A\x45\xBB\x00\x00\x00", 8, 1, outf);
    fwrite("\xBB\x00\x00\x00\x0B\x00\x00\x00", 8, 1, outf);
    fwrite("\x00\x00\x00\x00\x00\x00\x20\x00", 8, 1, outf);
    fwrite("\x00\x00\x00\x00\x00\x00", 6, 1, outf);
    /* this is a filename. I made it aaaaaaa.bbb */
    fwrite("\x61\x61\x61\x61\x61\x61\x61\x2E\x62\x62\x62", 11, 1, outf);
    fwrite("\x50\x4B\x05\x06\x00\x00\x00\x00", 8, 1, outf);
    fwrite("\x01\x00\x01\x00\x39\x00\x00\x00", 8, 1, outf);
    fwrite("\xE4\x00\x00\x00\x00\x00", 6, 1, outf);
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
    printf("looking for %s\n", from);

    /* shouldn't need to do this - PDOS should allow ./ */
    if (strncmp(from, "./", 2) == 0)
    {
        ret = PosFindFirst(from + 2, 0x10);
    }
    else
    {
        ret = PosFindFirst(from, 0x10);
    }

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
            long offs;

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
            fseek(fp, 0, SEEK_END);
            offs = ftell(fp);
            /* this needs to be changed */
            fwrite(&offs, 4, 1, outf);
            fwrite(&offs, 4, 1, outf);
            p = in;
            if (strncmp(in, "./", 2) == 0)
            {
                p += 2;
            }
            /* honestly, who writes this shit? */
            offs = strlen(p);
            fwrite(&offs, 4, 1, outf);
            fprintf(outf, "%s", p);
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
