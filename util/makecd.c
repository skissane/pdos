/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  makecd - make a CD from a floppy or hard disk image              */
/*                                                                   */
/*  This program creates a bootable ISO                              */
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

    if (argc <= 3)
    {
        printf("usage: makecd <fd/hd> <floppy/hard disk image> <iso image>\n");
        printf("example: makecd fd floppy.img out.iso\n");
        return (EXIT_FAILURE);
    }
    if (strcmp(*(argv + 1), "fd") == 0)
    {
        floppy = 1;
    }
    else if (strcmp(*(argv + 1), "hd") == 0)
    {
        floppy = 0;
    }
    else
    {
        printf("must specify either fd (floppy) or hd (hard disk)\n");
        return (EXIT_FAILURE);
    }
    fp = fopen(*(argv + 2), "rb");
    if (fp == NULL)
    {
        printf("failed to open %s for reading\n", *(argv + 2));
        return (EXIT_FAILURE);
    }
    fq = fopen(*(argv + 3), "wb");
    if (fq == NULL)
    {
        printf("failed to open %s for writing\n", *(argv + 3));
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

    /* boot record volume descriptor */
    memset(buf, '\0', sizeof buf);
    buf[0] = 0;
    memcpy(buf + 1, "CD001", 5);
    buf[6] = 1;
    memcpy(buf + 7, "EL TORITO SPECIFICATION", 23);
    buf[0x47] = 19;
    /* buf[0x47 ... 4a] = boot catalog sector */
    fwrite(buf, sizeof buf, 1, fq);

    /* volume descriptor set terminator */
    memset(buf, '\0', sizeof buf);
    buf[0] = 255;
    memcpy(buf + 1, "CD001", 5);
    buf[6] = 1;
    fwrite(buf, sizeof buf, 1, fq);

    /* boot catalog */
    memset(buf, '\0', sizeof buf);
    /* validation entry */
    p = buf;
    p[0] = 1;
    p[1] = 0; /* 80x86 */
    /* p[1c ... 1d] = checksum; */
    p[0x1e] = 0x55;
    p[0x1f] = 0xAA;
#if 0
    checksum = 0;
    for (x = 0; x < 0x20; x++)
    {
        checksum += (unsigned char)p[x];
    }
#endif
    checksum = 0x55AA;
    p[0x1c] = checksum & 0xff;
    p[0x1d] = (checksum >> 8) & 0xff;
    /* initial/default entry */
    p += 0x20;
    p[0] = 0x88;
    p[1] = 2; /* 1.44 MB floppy */
        /* note - 4 = hard disk, 0 = no emulation */
        /* 3 = 2.88 MB floppy, 1 = 1.2 MB floppy */
    if (!floppy)
    {
        p[1] = 4;
    }
    p[4] = 0; /* system type from partition table - not sure
           what a floppy requires */
    if (!floppy)
    {
        p[4] = 0x0b; /* should probably read this from the image */
    }
    p[6] = 1; /* load 1 512-byte sector */
    p[7] = 0; /* high byte */
    p[8] = 20;
    /* p[8 ... b] = relative block address of disk image */
    /* I don't think we need a section header, section entries
       and section entry extensions */
    fwrite(buf, sizeof buf, 1, fq);

    /* not sure if we can start our floppy image now */
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
