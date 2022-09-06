/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  secttest - test writing and rereading sectors to see if your USB */
/*  stick really has that many sectors. Puts random data also, to    */
/*  defeat any compression being done. A USB manufacturer could      */
/*  theoretically guard against this exact program, so a custom      */
/*  variation would need to be done to be reasonably sure of         */
/*  redefeating them. Depends how paranoid you are. Don't think      */
/*  about it too much or you'll wake up screaming and end up in the  */
/*  funny farm.
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "pos.h"

int main(int argc, char **argv)
{
    char buf[512];
    int drive;
    int numint;
    int n;
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
    srand(1); /* consistency is fine. Don't need to use time() */
    numint = (sizeof buf - sizeof(long)) / sizeof(int);
    for (x = 0; x <= sect; x++)
    {
        *(unsigned long *)buf = x;
        if (x != 0) /* protect MBR - let it be zeroed */
        {
            /* defeat compression */
            for (n = 0; n < numint; n++)
            {
                *(int *)(buf + sizeof(long) + n * sizeof(int)) = rand();
            }
        }
        rc = PosAbsoluteDriveWrite(drive, x, 1, buf);
        if (rc != 0)
        {
            printf("write of sector %lu failed with rc %d\n", x, rc);
            return (EXIT_FAILURE);
        }
    }
    srand(1);
    for (x = 0; x <= sect; x++)
    {
        rc = PosAbsoluteDriveRead(drive, x, 1, buf);
        if (rc != 0)
        {
            printf("read of sector %lu failed with rc %d\n", x, rc);
            return (EXIT_FAILURE);
        }
        if (*(unsigned long *)buf != x)
        {
            printf("integrity check failed on sector %lu\n", x);
            return (EXIT_FAILURE);
        }
        if (x == 0)
        {
            for (n = 0; n < sizeof buf; n++)
            {
                if (buf[n] != 0)
                {
                    printf("integrity check failed2 on sector %lu\n", x);
                    return (EXIT_FAILURE);
                }
            }
        }
        else
        {
            /* see if the random numbers are still intact */
            for (n = 0; n < numint; n++)
            {
                if (*(int *)(buf + sizeof(long) + n * sizeof(int)) != rand())
                {
                    printf("integrity check failed3 on sector %lu\n", x);
                    return (EXIT_FAILURE);
                }
            }
        }
    }
    printf("passed integrity check\n");
    return (0);
}
