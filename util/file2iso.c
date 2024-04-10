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

/* Documentation for ISO 9660 can be found here:

https://wiki.osdev.org/ISO_9660

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PVN_LBN 16
#define PATH_LBN PVN_LBN + 2
#define DIR_LBN PATH_LBN + 2
#define FILE_LBN DIR_LBN + 1

#define PATHENT_SIZE 10

static void both_end32(unsigned char *p, unsigned long val);
static void both_end16(unsigned char *p, unsigned long val);
static void int32_lsb(unsigned char *p, unsigned long val);
static void int32_msb(unsigned char *p, unsigned long val);
static void int16_lsb(unsigned char *p, unsigned long val);
static void int16_msb(unsigned char *p, unsigned long val);

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
    int fnm_len;
    int tot_len;
    long len_file;
    long blocks;

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



    /* calculate what we will need */
    fnm_len = strlen(fnm);
    fnm_len += 2; /* for ;1 terminator */
    if ((fnm_len % 2) == 0)
    {
        fnm_len++;
    }
    tot_len = 33 + fnm_len;
    if ((tot_len % 2) != 0)
    {
        tot_len++;
    }
    fseek(fp, 0, SEEK_END);
    len_file = ftell(fp);
    rewind(fp);

    blocks = len_file / 2048;
    if ((len_file % 2048) != 0)
    {
        blocks++;
    }
    blocks += FILE_LBN;



    /* primary volume descriptor */
    memset(buf, '\0', sizeof buf);
    buf[0] = 1;
    memcpy(buf + 1, "CD001", 5);
    buf[6] = 1;
    /* root directory entry */
    p = buf + 156;
    p[0] = 34;
    p += 2;
    both_end32(p, DIR_LBN);
    p += 8;
    both_end32(p, 2048 /* tot_len ? */);
    p += 8;
    p[0] = 0x7c;
    p[1] = 1;
    p[2] = 1;
    p + 7;
    *p = 1 << 1; /* mark it as a directory - right? */
    p++;
    p += 2;
    both_end16(p, 1);
    p += 4;
    *p = 0x01;
    both_end32(buf + 80, blocks);
    both_end16(buf + 120, 1);
    both_end16(buf + 124, 1);
    both_end32(buf + 132, PATHENT_SIZE); /* path table size */
    int32_lsb(buf + 140, PATH_LBN);
    int32_msb(buf + 148, PATH_LBN + 1);
    buf[881] = 1;
    memset(buf + 8, ' ', 72 - 8);
    buf[8] = 'X';
    memset(buf + 318, ' ', 813 - 318);
    memcpy(buf + 813, "20240101000000000", 17);
    memcpy(buf + 830, "20240101000000000", 17);
    fwrite(buf, sizeof buf, 1, fq);


    /* volume descriptor set terminator */
    memset(buf, '\0', sizeof buf);
    buf[0] = 255;
    memcpy(buf + 1, "CD001", 5);
    buf[6] = 1;
    fwrite(buf, sizeof buf, 1, fq);



    /* first path table */
    memset(buf, '\0', sizeof buf);
    buf[0] = 1; /* root directory is 1 byte */
    int32_lsb(buf + 2, DIR_LBN);
    int16_lsb(buf + 6, 1); /* self-referencing I think */
    fwrite(buf, sizeof buf, 1, fq);



    /* second path table */
    int32_msb(buf + 2, DIR_LBN);
    int16_msb(buf + 6, 1); /* self-referencing I think */
    fwrite(buf, sizeof buf, 1, fq);



    /* need directory info now */
    memset(buf, '\0', sizeof buf);
    p = buf;
    p[0] = tot_len;
    p = buf + 2;
    both_end32(p, FILE_LBN);

    buf[18] = buf[19] = 1;

    p = buf + 10;
    both_end32(p, len_file);

    p = buf + 28;
    both_end16(p, 1);

    p = buf + 33;
    strcpy(p, fnm);
    strcat(p, ";1");
    while (*p != '\0')
    {
        *p = toupper((unsigned char)*p);
        p++;
    }
    buf[32] = strlen(buf + 33);
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

static void both_end32(unsigned char *p, unsigned long val)
{
    p[0] = p[7] = val & 0xff;
    val >>= 8;
    p[1] = p[6] = val & 0xff;
    val >>= 8;
    p[2] = p[5] = val & 0xff;
    val >>= 8;
    p[3] = p[4] = val & 0xff;
    return;
}

static void both_end16(unsigned char *p, unsigned long val)
{
    p[0] = p[3] = val & 0xff;
    val >>= 8;
    p[1] = p[2] = val & 0xff;
    return;
}

static void int32_lsb(unsigned char *p, unsigned long val)
{
    p[0] = val & 0xff;
    val >>= 8;
    p[1] = val & 0xff;
    val >>= 8;
    p[2] = val & 0xff;
    val >>= 8;
    p[3] = val & 0xff;
    return;
}

static void int32_msb(unsigned char *p, unsigned long val)
{
    p[3] = val & 0xff;
    val >>= 8;
    p[2] = val & 0xff;
    val >>= 8;
    p[1] = val & 0xff;
    val >>= 8;
    p[0] = val & 0xff;
    return;
}

static void int16_lsb(unsigned char *p, unsigned long val)
{
    p[0] = val & 0xff;
    val >>= 8;
    p[1] = val & 0xff;
    return;
}

static void int16_msb(unsigned char *p, unsigned long val)
{
    p[1] = val & 0xff;
    val >>= 8;
    p[0] = val & 0xff;
    return;
}
