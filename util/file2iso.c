/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  file2iso - make an ISO CD from a single file                     */
/*                                                                   */
/*********************************************************************/

/* Documentation for the El Torito spec can be found here:

https://web.archive.org/web/20180112220141/
https://download.intel.com/support/motherboards/desktop/sb/specscdrom.pdf

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buf[2048];
    FILE *fp;
    FILE *fq;
    int x;
    char *p;
    unsigned int checksum;
    int floppy;
    char *fnm;

    if (argc <= 2)
    {
        printf("usage: file2iso <input file> <output file>\n");
        printf("example: file2iso in.txt out.iso\n");
        return (EXIT_FAILURE);
    }
    fnm = *(argv + 1);
    fp = fopen(fnm, "rb");
    if (fp == NULL)
    {
        printf("failed to open %s for reading\n", fnm);
        return (EXIT_FAILURE);
    }
    fq = fopen(*(argv + 2), "wb");
    if (fq == NULL)
    {
        printf("failed to open %s for writing\n", *(argv + 2));
        return (EXIT_FAILURE);
    }
    /* 16 sectors are unused */
    memset(buf, '\0', sizeof buf);
    for (x = 0; x < 16; x++)
    {
        fwrite(buf, sizeof buf, 1, fq);
    }

    /* primary volume descriptor */
    memset(buf, '\0', sizeof buf);
    buf[0] = 1;
    memcpy(buf + 1, "CD001", 5);
    buf[6] = 1;
    fwrite(buf, sizeof buf, 1, fq);

    /* volume descriptor set terminator */
    memset(buf, '\0', sizeof buf);
    buf[0] = 255;
    memcpy(buf + 1, "CD001", 5);
    buf[6] = 1;
    fwrite(buf, sizeof buf, 1, fq);





    /* need directory info now */
    memset(buf, '\0', sizeof buf);
    /* there seems to be 0x14 bytes of file information, followed
       by the filename */
    p = buf + 0x14;
    strcpy(p, fnm);
    while (*p != '\0')
    {
        *p = toupper((unsigned char)*p);
        p++;
    }
    fwrite(buf, sizeof buf, 1, fq);

    /* I don't think we need a section header, section entries
       and section entry extensions */




    /* copy file across */
    memset(buf, '\0', sizeof buf);
    while (fread(buf, 1, sizeof buf, fp) != 0)
    {
        fwrite(buf, sizeof buf, 1, fq);
        memset(buf, '\0', sizeof buf);
    }
    if (ferror(fq))
    {
        printf("write error\n");
        return (EXIT_FAILURE);
    }
    printf("success\n");
    return (0);
}
