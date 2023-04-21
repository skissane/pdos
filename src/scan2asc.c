/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  scan2asc - provide a mapping of scan code to ASCII               */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>

#include "bos.h"

static unsigned char map[256];

int main(void)
{
    int scancode;
    int ascii;
    FILE *fq;

    printf("press every key you can think of, except ESC\n");
    printf("then press ESC\n");
    printf("result will be in scanmap.dat\n");
    
    while (1)
    {
        BosReadKeyboardCharacter(&scancode,&ascii);
        map[scancode] = ascii;
        if (ascii == 0x1b) break;
    }
    fq = fopen("scanmap.dat", "wb");
    if (fq == NULL)
    {
        printf("failed to open scanmap.dat\n");
        return (EXIT_FAILURE);
    }
    fwrite(map, 1, sizeof map, fq);
    fclose(fq);
    return (0);
}
