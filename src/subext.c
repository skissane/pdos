/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  subext - substitute a specially-formatted file on a disk image   */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv)
{
    FILE *fp;
    FILE *sf;
    long in1;
    char mybuf[16];
    long cnt;
    char testbuf[16];
    unsigned long x;
    unsigned long startpos;
    char scanbuf[17];
    unsigned long upto;
    int c;
    long curpos;
    int first;
    
    if (argc <= 2)
    {
        printf("usage: subext <orig> <subfile>\n");
        printf("substitute a specially formatted file on an ext2 etc filesystem\n");
        printf("e.g. subext ram_in newfile\n");
        return (EXIT_FAILURE);
    }

    fp = fopen(*(argv + 1), "r+b");
    assert(fp != NULL);
    sf = fopen(*(argv + 2), "rb");
    assert(sf != NULL);

    upto = 0;
    sprintf(scanbuf, "%08lu%08lu", upto, upto + 1);
    while ((c = fread(mybuf, 1, sizeof mybuf, sf)) > 0)
    {
        curpos = ftell(fp);
        first = 1;
        while (1)
        {
            /* this logic isn't perfect */
            if (fread(testbuf, 1, sizeof testbuf, fp) != sizeof testbuf)
            {
                printf("unexpected EOF - aborting\n");
                printf("looking for %s\n", scanbuf);
                return (EXIT_FAILURE);
            }
            if (memcmp(testbuf, scanbuf, sizeof testbuf) == 0)
            {
                fseek(fp, curpos, SEEK_SET);
                fwrite(mybuf, 1, c, fp);
                fseek(fp, 0, SEEK_CUR);
                break;
            }
            if (first)
            {
                first = 0;
                rewind(fp);
                curpos = 0;
            }
            else
            {
                curpos = ftell(fp);
            }
        }
        upto += 2;
        sprintf(scanbuf, "%08lu%08lu", upto, upto + 1);
    }
    printf("probably successful\n");
    return (0);
}
