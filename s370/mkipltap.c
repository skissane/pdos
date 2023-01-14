/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  mkipltap - make an tape containing the PDOS OS that can be IPLed */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;
    FILE *fq;
    FILE *fp;
    char *buf;

    if (argc <= 1)
    {
        printf("usage: mkipltap <vtape>\n");
        printf("e.g. mkipltap tav10004:\n");
        printf("note - must be tav not tap\n");
        printf("tav auto-converts V to U\n");
        return (EXIT_FAILURE);
    }
    buf = malloc(18452 + 4);
    if (buf == NULL)
    {
        printf("couldn't allocate 18456 bytes\n");
        return (EXIT_FAILURE);
    }
    *(short *)(buf + 2) = 0;
    fq = fopen(*(argv + 1), "wb");
    if (fq == NULL)
    {
        printf("failed to open tape %s\n", *(argv + 1));
        return (EXIT_FAILURE);
    }
    fp = fopen("PDOS.SYS", "rb");
    if (fp == NULL)
    {
        printf("failed to open PDOS.SYS for reading\n");
        return (EXIT_FAILURE);
    }
    while ((i = fread(buf + 4, 1, 18452, fp)) > 0)
    {
        *(short *)buf = i + 4;
        fwrite(buf, 1, i + 4, fq);
    }
    fclose(fp);
    if (ferror(fq))
    {
        printf("write error\n");
        return (EXIT_FAILURE);
    }
    fclose(fq);
    printf("finished writing tape\n");
    return (EXIT_SUCCESS);
}
