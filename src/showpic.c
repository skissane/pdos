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

    if (argc <= 1)
    {
        printf("usage: showpic <picture file>\n");
        return (EXIT_FAILURE);
    }
    setvbuf(stdin, NULL, _IONBF, 0);
    BosGetVideoMode(&columns, &oldmode, &page);
    BosSetVideoMode(4);
    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 320; x++)
        {
            color = 0;
            if ((((x / 10) % 2) == 1)
                && (((y / 10) % 2) == 1))
            {
                color = 1;
            }
            BosWriteGraphicsPixel(page, color, y, x);
        }
    }
    getc(stdin);
    BosSetVideoMode(oldmode);
    return (0);
}
