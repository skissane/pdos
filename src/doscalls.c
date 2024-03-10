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

/* note that we don't need a separate table of OS/2 handles
   to file pointers - we rely on the handles less than 3 being
   standard and not clashing with file pointers and then the
   rest are equated. We should possibly create an initialization
   function, similar to initlinux, so that we can more easily
   index into a table containing just stdin, stdout and stderr,
   but for now we'll just code that every time */

/* add special handle 3 for opening of KBD$ */

#define INCL_KBD
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>

#include <pos.h>

ULONG APIENTRY DosWrite(ULONG hfile, void *ptr,
                         ULONG towrite, ULONG *tempWritten)
{
    FILE *f;

    if (hfile == 0) f = stdin;
    else if (hfile == 1) f = stdout;
    else if (hfile == 2) f = stderr;
    else f = (FILE *)hfile;

    *tempWritten = fwrite(ptr, 1, towrite, f);
    fflush(f);
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
    if (strcmp(fnm, "KBD$") == 0)
    {
        *handle = 3;
        return (0);
    }

    if ((mode & OPEN_ACCESS_WRITEONLY) != 0)
    {
        *handle = (ULONG)fopen(fnm, "wb");
    }
    else
    {
        *handle = (ULONG)fopen(fnm, "r+b");
    }

    if ((void *)*handle == NULL)
    {
        return (1);
    }
    return (0);
}

ULONG APIENTRY DosClose(ULONG hfile)
{
    FILE *f;

    /* keyboard is exception */
    if (hfile == 3) return (0);

    if (hfile == 0) f = stdin;
    else if (hfile == 1) f = stdout;
    else if (hfile == 2) f = stderr;
    else f = (FILE *)hfile;
    fclose((FILE *)hfile);
    return (0);
}

ULONG APIENTRY DosRead(ULONG hfile, void *ptr,
                       ULONG toread, ULONG *tempRead)
{
    FILE *f;

    if (hfile == 0) f = stdin;
    else if (hfile == 1) f = stdout;
    else if (hfile == 2) f = stderr;
    else f = (FILE *)hfile;

    *tempRead = fread(ptr, 1, toread, f);
    return (0);
}

ULONG APIENTRY DosDelete(char *name)
{
    remove(name);
    return (0);
}

ULONG APIENTRY DosMove(char *a, char *b)
{
    rename(a, b);
    return (0);
}

ULONG APIENTRY DosExecPgm(char *err_obj, USHORT sz, USHORT flags,
                          char *string, void *junk1, RESULTCODES *results,
                          char *string2)
{
    system(string);
    return (0);
}

ULONG APIENTRY DosSetFilePtr(ULONG hfile, LONG newpos,
                              int dir, ULONG *retpos)
{
    FILE *f;

    if (hfile == 0) f = stdin;
    else if (hfile == 1) f = stdout;
    else if (hfile == 2) f = stderr;
    else f = (FILE *)hfile;

    *retpos = fseek(f, newpos, dir);
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
    static int linebuf = 1;
    static KBDKEYINFO cd;

    if (function == 0x51)
    {
        if (linebuf)
        {
            linebuf = 0;
            setvbuf(stdin, NULL, _IONBF, 0);
        }
        else
        {
            linebuf = 1;
            setvbuf(stdin, NULL, _IOLBF, 0);
        }
    }
    /* this algorithm won't work for people who are using e0 as a
       legitimate character. Need to find out what e0 goes through
       with as a scancode, and set that scancode here, and get
       PDPCLIB to test that scancode first */
    else if (function == 0x74)
    {
        /* make sure to use a non-standard 0 (OS/2 uses 1) for a
           scancode of an ESC so that PDPCLIB does not attempt to
           double it. Currently all our scancodes are 0 since
           PDPCLIB doesn't need them */
        cd.chChar = fgetc(stdin);
        *(KBDKEYINFO *)dataptr = cd;
    }
    return (0);
}

ULONG APIENTRY DosAllocMem(void *base, ULONG size, ULONG flags)
{
    char *p;

    p = malloc(size);
    if (p == NULL) return (1);
    *(char **)base = p;
    return (0);
}

ULONG APIENTRY DosFreeMem(void *base)
{
    free(base);
    return (0);
}

ULONG APIENTRY DosScanEnv(void *name, void *result)
{
    *(char **)result = getenv(name);
    return (0);
}

ULONG APIENTRY DosSetRelMaxFH(LONG *req, ULONG *max)
{
    return (0);
}

ULONG APIENTRY DosGetInfoBlocks(TIB **tib, PIB **pib)
{
    /* OS/2 appears to have a space before the qqq, but
       PDPCLIB doesn't seem to care either way */
    /* static char cmd[] = "ppp\0qqq rrr"; */

    static char cmd[310]; /* will have NUL terminator */
    static PIB mypib = { 0, 0, 0, cmd };

    if (pib != NULL)
    {
        char *p;

        strncpy(cmd, PosGetCommandLine(), sizeof cmd - 1);

        /* this simple algorithm prevents the program name from
           having a space in it, so may want to fix */
        p = strchr(cmd, ' ');
        if (p != NULL)
        {
            *p = '\0';
        }
        *pib = &mypib;
    }
    return (0);
}
