/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  mfterm - terminal mainly for mainframe                           */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *sf;
    int c;

    if (argc != 2)
    {
        printf("usage: mfterm <serial file>\n");
        printf("establishes a telnet ANSI terminal connection to mainframe\n");
        printf("e.g. mfterm com1:\n");
        return (EXIT_FAILURE);
    }
    sf = fopen(*(argv + 1), "r+b");
    if (sf == NULL)
    {
        printf("failed to open serial file\n");
        return (EXIT_FAILURE);
    }
    c = fgetc(sf);
    printf("c is %x\n", c);
    c = fgetc(sf);
    printf("c is %x\n", c);
    c = fgetc(sf);
    printf("c is %x\n", c);
    fclose(sf);
    return (0);
}
