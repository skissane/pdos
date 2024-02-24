/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  doscalls - minimal implementation of OS/2 under PDOS/386         */
/*                                                                   */
/*********************************************************************/

#undef __SUPPRESS__

#include <os2.h>

#include <stdio.h>
#include <stdlib.h>

ULONG APIENTRY DosWrite(ULONG hfile, void *ptr,
                         ULONG towrite, ULONG *tempWritten)
{
    fwrite(ptr, towrite, 1, stdout);
    return (1);
}

void APIENTRY DosExit(int a, int b)
{
    exit(b);
}
