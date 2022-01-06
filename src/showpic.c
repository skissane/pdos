/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  showpic - show a picture                                         */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <bos.h>

int main(int argc, char **argv)
{
    unsigned int oldmode;
    unsigned int columns;
    unsigned int page;
    int c;
    int x;
    int y;
    int color;
    int backcol = 3;
    int forecol = 0;
    int type = 1;
    static char buf[322];
    FILE *fp;

    if (argc <= 1)
    {
        printf("usage: showpic <picture file>\n");
        return (EXIT_FAILURE);
    }
    fp = fopen(*(argv + 1), "r");
    if (fp == NULL)
    {
        printf("failed to open %s\n", *(argv + 1));
        return (EXIT_FAILURE);
    }
    setvbuf(stdin, NULL, _IONBF, 0);
    BosGetVideoMode(&columns, &oldmode, &page);
    BosSetVideoMode(4);
    for (y = 0; y < 200; y++)
    {
        fgets(buf, sizeof buf, fp);
        for (x = 0; x < 320; x++)
        {
            if (buf[x] == 'X')
            {
                color = forecol;
            }
            else
            {
                color = backcol;
            }
            BosWriteGraphicsPixel(page, color, y, x);
        }
    }
    getc(stdin);
    BosSetVideoMode(oldmode);
    fclose(fp);
    return (0);
}
