/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  bios - generic BIOS that exports the C library                   */
/*                                                                   */
/*********************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "bios.h"
#include "exeload.h"

static BIOS bios = { printf };

static int (*genstart)(BIOS *bios);

int main(int argc, char **argv)
{
    unsigned char *p;
    unsigned char *entry_point;
    int rc;

    printf("bios starting\n");
    if (argc < 3)
    {
        printf("Usage: bios pdos.exe config.fil\n");
        return (EXIT_FAILURE);
    }
    p = calloc(1, 1000000);
    if (p == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    if (exeloadDoload(&entry_point, argv[1], &p) != 0)
    {
        printf("failed to load executable\n");
        return (EXIT_FAILURE);
    }
    genstart = (void *)entry_point;
    /* printf("first byte of code is %02X\n", *(unsigned char *)entry_point); */
#if 1
    rc = genstart(&bios);
#else
    rc = 0;
#endif
    printf("return from called program is %d\n", rc);
    printf("bios exiting\n");
    return (0);
}
