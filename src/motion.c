/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  motion - show a moving picture                                   */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <bos.h>

#define SIZEPIC 96

static char arr[SIZEPIC][SIZEPIC];

int main(int argc, char **argv)
{
    unsigned int oldmode;
    unsigned int columns;
    unsigned int page;
    int c;
    int x;
    int y;
    int z;
    int color;
    int backcol = 3;
    int forecol = 0;
    int type = 1;
    static char buf[SIZEPIC+2];
    FILE *fp;

    if (argc <= 1)
    {
        printf("usage: motion <picture file>\n");
        return (EXIT_FAILURE);
    }
    fp = fopen(*(argv + 1), "r");
    if (fp == NULL)
    {
        printf("failed to open %s\n", *(argv + 1));
        return (EXIT_FAILURE);
    }
    BosGetVideoMode(&columns, &oldmode, &page);
    BosSetVideoMode(4);
    for (y = 0; y < SIZEPIC; y++)
    {
        fgets(buf, sizeof buf, fp);
        memcpy(arr[y], buf, SIZEPIC);
    }
    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 320; x++)
        {
            BosWriteGraphicsPixel(page, backcol, y, x);
        }
    }
    for (z = 0; z <= 320 - SIZEPIC; z++)
    {
        for (y = 0; y < SIZEPIC; y++)
        {
            for (x = 0; x < SIZEPIC; x++)
            {
                if (arr[y][x] == 'X')
                {
                    color = forecol;
                }
                else
                {
                    color = backcol;
                }
                BosWriteGraphicsPixel(page,
                                      color,
                                      (200 - SIZEPIC) / 2 + y,
                                      z + x);
            }
        }
        /* sleep here */
        for (y = 0; y < SIZEPIC; y++)
        {
            BosWriteGraphicsPixel(page,
                                  backcol,
                                  (200 - SIZEPIC) / 2 + y,
                                  z);
        }
    }
    BosSetVideoMode(oldmode);
    fclose(fp);
    return (0);
}
