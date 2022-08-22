/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  fdisk - prepare a disk                                           */
/*                                                                   */
/*  This program creates a 950 MB partitioned FAT32 disk on a drive  */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buf[512];
    FILE *fq;
    int x;
    
    if (argc <= 1)
    {
        printf("usage: fdisk <outfile>\n");
        printf("example: fdisk test.img\n");
        printf("example: fdisk drv82: (be real careful)\n");
        printf("writes control structures for a 950 MB FAT32 partition\n");
        return (EXIT_FAILURE);
    }
    fq = fopen(*(argv + 1), "wb");
    if (fq == NULL)
    {
        printf("failed to open %s for writing\n", *(argv + 1));
        return (EXIT_FAILURE);
    }
    memset(buf, '\0', sizeof buf);
    memcpy(buf + 0x1be, "\x80\x02\x03\x00\x0C\xBB\x03\x78"
                        "\x80\x00\x00\x00\x00\x98\x1D\x00", 16);
    memcpy(buf + 0x1fe, "\x55\xAA", 2);
    fwrite(buf, sizeof buf, 1, fq);
    memset(buf, '\0', sizeof buf);
    for (x = 0; x < 127; x++)
    {
        fwrite(buf, sizeof buf, 1, fq);
    }
    memcpy(buf + 11,
           "\x00\x02\x08\x42"
           "\x11\x02\x00\x00"
           "\x00\x00\xF8\x00"
           "\x00\x3F\x00\xFF"

           "\x00\x80\x00\x00"
           "\x00\x00\x98\x1D"
           "\x00\x5F\x07\x00"
           "\x00\x00\x00\x00"

           "\x00\x02\x00\x00"
           "\x00\x01\x00\x06"
           "\x00\x00\x00\x00"
           "\x00\x00\x00\x00"

           "\x00\x00\x00\x00"
           "\x00\x80\x00\x29"
           "\xCD\x44\xFA\x84"
           "\x4E\x4F\x20\x4E"

           "\x41\x4D\x45\x20"
           "\x20\x20\x20\x46"
           "\x41\x54\x33\x32"
           "\x20\x20\x20", 79);

    memcpy(buf + 0x1fe, "\x55\xAA", 2);
    fwrite(buf, sizeof buf, 1, fq);
    memset(buf, '\0', sizeof buf);
    for (x = 0; x < 4417; x++)
    {
        fwrite(buf, sizeof buf, 1, fq);
    }
    memset(buf, 0xFF, 12);
    fwrite(buf, sizeof buf, 1, fq);
    if (ferror(fq))
    {
        printf("write error\n");
        return (EXIT_FAILURE);
    }
    printf("success\n");
    return (0);
}
