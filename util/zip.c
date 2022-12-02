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

#include <crc32.h>

static char from[FILENAME_MAX];

static char filespec[FILENAME_MAX];

static DTA *dta;

static FILE *outf;

static int compress = 0;

static int stage = 1;

static long startcd;

static long cdlen;

static short numfiles;

static long logupto; /* logical upto */

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
    numfiles = 0;
    logupto = 0;
    dolevel();
    numfiles = 0;
    logupto = 0;
    stage = 2;
    startcd = ftell(outf);
    cdlen = 0;
    dolevel();
    fwrite("\x50\x4B\x05\x06\x00\x00\x00\x00", 8, 1, outf);
    /* this needs to change */
    fwrite(&numfiles, 2, 1, outf);
    fwrite(&numfiles, 2, 1, outf);
    /* this needs to change */
    fwrite(&cdlen, 4, 1, outf);
    /* this needs to change */
    fwrite(&startcd, 4, 1, outf);
    fwrite("\x00\x00", 2, 1, outf);
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
            long filelen;
            long fnmlen;
            CRC32 crc;
            int c;

            strcpy(in, from);
            strcat(in, "/");
            strcat(in, dta->lfn);
            if (dta->lfn[0] == '\0')
            {
                strcat(in, dta->file_name);
            }
            p = in;
            if (strncmp(in, "./", 2) == 0)
            {
                p += 2;
            }
            fnmlen = strlen(p);
            if (stage == 1)
            {
                printf("%s\n", p);
            }
            fp = fopen(in, "rb");
            if (fp == NULL)
            {
                printf("failed to open %s for reading\n", in);
                exit(EXIT_FAILURE);
            }
            if (stage == 1)
            {
                fwrite("\x50\x4B\x03\x04\x0A\x00\x00\x00", 8, 1, outf);
                fwrite("\x00\x00\xCB\x4C\x75\x55", 6, 1, outf);
                crc32Init(&crc);
                while ((c = getc(fp)) != EOF)
                {
                    crc32Update(&crc, c);
                }
                crc32Finalize(&crc);
                fputc(crc32Byte4(&crc), outf);
                fputc(crc32Byte3(&crc), outf);
                fputc(crc32Byte2(&crc), outf);
                fputc(crc32Byte1(&crc), outf);
            }
            else if (stage == 2)
            {
                fwrite("\x50\x4B\x01\x02\x1E\x00\x0A\x00", 8, 1, outf);
                fwrite("\x00\x00\x00\x00\xCB\x4C\x75\x55", 8, 1, outf);
                fwrite("\x7C\xC7\x8A\x45", 4, 1, outf);
                fseek(fp, 0, SEEK_END);
            }
            filelen = ftell(fp);
            /* this needs to be changed */
            fwrite(&filelen, 4, 1, outf);
            fwrite(&filelen, 4, 1, outf);
            fwrite(&fnmlen, 4, 1, outf);
            if (stage == 2)
            {
                fwrite("\x00\x00\x00\x00\x00\x00\x20\x00", 8, 1, outf);
                fwrite("\x00\x00", 2, 1, outf);
                /* this needs to change */
                fwrite(&logupto, 4, 1, outf);
            }
            fprintf(outf, "%s", p);
            if(stage == 1)
            {
                rewind(fp);
                while ((cnt = fread(buf, 1, sizeof buf, fp)) > 0)
                {
                    fwrite(buf, 1, cnt, outf);
                }
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
            numfiles++;
            logupto += filelen;
            logupto += fnmlen;
            logupto += 30; /* overhead */
            cdlen += fnmlen;
            cdlen += 46; /* overhead */
        }
        ret = PosFindNext();
    }
    return (0);
}
