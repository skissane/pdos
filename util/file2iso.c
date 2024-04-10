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

#define ROOT_LBN 17
#define DIR_LBN ROOT_LBN + 2
#define FILE_LBN DIR_LBN + 1

static void both_end32(unsigned char *p, unsigned long val);
static void both_end16(unsigned char *p, unsigned long val);

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
    both_end32(p, tot_len);
    p += 8 + 7;
    *p = 1 << 1; /* mark it as a directory - right? */
    both_end32(buf + 80, blocks);
    both_end16(buf + 120, 1);
    both_end16(buf + 124, 1);
    buf[881] = 1;
    fwrite(buf, sizeof buf, 1, fq);


    /* volume descriptor set terminator */
    memset(buf, '\0', sizeof buf);
    buf[0] = 255;
    memcpy(buf + 1, "CD001", 5);
    buf[6] = 1;
    fwrite(buf, sizeof buf, 1, fq);



    /* I'm hoping I don't need a path table */




    /* need directory info now */
    memset(buf, '\0', sizeof buf);
    p = buf;
    p[0] = tot_len;
    p = buf + 2;
    both_end32(p, FILE_LBN);

    p = buf + 10;
    both_end32(p, len_file);

    buf[32] = fnm_len;
    p = buf + 33;
    strcpy(p, fnm);
    strcat(p, ";1");
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
