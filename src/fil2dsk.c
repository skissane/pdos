/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  fil2dsk - write a file onto a raw hard disk, with NUL-padding    */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "pos.h"
#include "bos.h"

int main(int argc, char **argv)
{
    FILE *fp;
    char buf[512];
    int drive;
    unsigned long sect = 0;
    int rc;
    size_t cnt;
    
    if (argc <= 2)
    {
        printf("usage: fil2dsk <file> <drive num>\n");
        printf("example: fil2dsk temp.zip 81\n");
        printf("80 = C drive, 81 = D drive etc\n");
        printf("assuming drives aren't partitioned etc\n");
        return (EXIT_FAILURE);
    }
    fp = fopen(*(argv + 1), "rb");
    if (fp == NULL)
    {
        printf("failed to open %s for reading\n", *(argv + 1));
        return (EXIT_FAILURE);
    }
    drive = strtol(*(argv + 2), NULL, 16);
    rc = 0;
    while (rc == 0)
    {
        memset(buf, '\0', sizeof buf);
        if (!feof(fp))
        {
            cnt = fread(buf, 1, sizeof buf, fp);
        }
        rc = PosAbsoluteDriveWrite(drive, sect, 1, buf);
        if (rc == 0)
        {
            cnt = 0;
        }
        sect++;
    }
    if (feof(fp) && (cnt == 0))
    {
        printf("successful\n");
    }
    else
    {
        printf("failed with rc %d\n", rc);
        return (EXIT_FAILURE);
    }
    return (0);
}
