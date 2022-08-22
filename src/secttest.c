/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  secttest - test writing and rereading sectors to see if your USB */
/*  stick really has that many sectors                               */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "pos.h"

int main(int argc, char **argv)
{
    char buf[512];
    int drive;
    unsigned long sect;
    unsigned long x;
    int rc;
    
    if (argc <= 2)
    {
        printf("usage: secttest <drive> <max sector>\n");
        printf("example: secttest 81 1000000\n");
        printf("warning - this is a destructive test - all data wiped\n");
        printf("second hard drive is 81, etc\n");
        return (EXIT_FAILURE);
    }
    
    drive = strtol(*(argv + 1), NULL, 16);
    sect = strtoul(*(argv + 2), NULL, 0);
    memset(buf, '\0', sizeof buf);
    for (x = 0; x <= sect; x++)
    {
        *(unsigned long *)buf = x;
        rc = PosAbsoluteDriveWrite(drive, x, 1, buf);
        if (rc != 0)
        {
            printf("write of sector %lu failed with rc %d\n", x, rc);
            return (EXIT_FAILURE);
        }
    }
    for (x = 0; x <= sect; x++)
    {
        *(unsigned long *)buf = x;
        rc = PosAbsoluteDriveRead(drive, x, 1, buf);
        if (rc != 0)
        {
            printf("read of sector %lu failed with rc %d\n", x, rc);
            return (EXIT_FAILURE);
        }
        if (*(unsigned long *)buf != x)
        {
            printf("integrity check failed on sector %ld\n", x);
            return (EXIT_FAILURE);
        }
    }
    printf("passed integrity check\n");
    return (0);
}
