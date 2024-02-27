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
    return (0);
}

void APIENTRY DosExit(int a, int b)
{
    exit(b);
}

ULONG APIENTRY DosOpen(char *fnm, ULONG *handle, ULONG *action1,
               ULONG newsize, ULONG fileattr, ULONG action2,
               ULONG mode, ULONG resvd)
{
    return (0);
}

ULONG APIENTRY DosClose(short handle)
{
    return (0);
}

ULONG APIENTRY DosRead(ULONG hfile, void *ptr,
                       ULONG toread, ULONG *tempRead)
{
    return (0);
}

ULONG APIENTRY DosDelete(char *name)
{
    return (0);
}

ULONG APIENTRY DosMove(char *a, char *b)
{
    return (0);
}

ULONG APIENTRY DosExecPgm(char *err_obj, USHORT sz, USHORT flags,
                          char *string, void *junk1, RESULTCODES *results,
                          char *string2)
{
    return (0);
}

ULONG APIENTRY DosSetFilePtr(ULONG hfile, LONG newpos,
                              int dir, ULONG *retpos)
{
    return (0);
}

ULONG APIENTRY DosGetDateTime(DATETIME *dt)
{
    return (0);
}

ULONG APIENTRY DosDevIOCtl(ULONG handle,
                           ULONG category,
                           ULONG function,
                           void *parmptr,
                           ULONG parmmax,
                           ULONG *parmlen,
                           void *dataptr,
                           ULONG datamax,
                           ULONG *datalen)
{
    return (0);
}

ULONG APIENTRY DosAllocMem(void *base, ULONG size, ULONG flags)
{
    return (0);
}

ULONG APIENTRY DosFreeMem(void *base)
{
    return (0);
}

ULONG APIENTRY DosScanEnv(void *name, void *result)
{
    return (0);
}

ULONG APIENTRY DosSetRelMaxFH(LONG *req, ULONG *max)
{
    return (0);
}