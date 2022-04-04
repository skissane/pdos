/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  boot - boot a disk                                               */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <pos.h>

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("usage: boot <disk>\n");
        printf("e.g. boot d\n");
        return (EXIT_FAILURE);
    }
    PosDoBoot(toupper((unsigned char)argv[1][0]) - 'A');
    return (EXIT_FAILURE); /* if we get here, it is a failure */
}
