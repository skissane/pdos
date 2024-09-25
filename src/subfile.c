/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  subfile - substitute a file at a particular point                */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv)
{
    FILE *fp1;
    FILE *fp2;
    FILE *fq;
    long in1;
    long cnt;
    unsigned long x;
    unsigned long startpos;
    int c;
    
    if (argc <= 4)
    {
        printf("usage: subfile <in1> <in2> <out> <subpos>\n");
        return (EXIT_FAILURE);
    }

    fp1 = fopen(*(argv + 1), "rb");
    assert(fp1 != NULL);
    fp2 = fopen(*(argv + 2), "rb");
    assert(fp2 != NULL);
    fq = fopen(*(argv + 3), "wb");
    assert(fq != NULL);
    startpos = strtoul(*(argv + 4), NULL, 0);
    assert(startpos != 0);

    for (x = 0; x < startpos; x++)
    {
        c = fgetc(fp1);
        if (c == EOF) break;
        fputc(c, fq);
    }

    while ((c = fgetc(fp2)) != EOF)
    {
        fgetc(fp1);
        fputc(c, fq);
    }

    while ((c = fgetc(fp1)) != EOF)
    {
        fputc(c, fq);
    }
    
    return (0);
}
