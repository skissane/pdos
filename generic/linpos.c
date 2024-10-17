/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  linpos - linux implementation of pos routines (partial)          */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h>

#include <pos.h>

static DTA origdta;

static int dirfile;

struct dirsave {
    int dirfilesave;
    unsigned char buf[500];
    int upto;
    int avail;
};

struct dirsave *curdirsave = NULL;

void *PosGetDTA(void)
{
    return (&origdta);
}

static int ff_search(void)
{
#ifdef __MACOS__
    return (1);
#else

    /* note that Linux is expected to return multiple entries,
       but they will always be complete entries */

#if defined(__64BIT__) && defined(__ARM__)
    /* this is actually getdents64 so structure is different */
#define EXTRAPAD 1
#define __getdents __getdents64
#else
#define EXTRAPAD 0
#endif

    if (curdirsave->avail <= 0)
    {
        curdirsave->avail = __getdents(dirfile, curdirsave->buf, 500);
        if (curdirsave->avail <= 0)
        {
            __close(dirfile);
            free(curdirsave);
            curdirsave = NULL;
            return (1);
        }
    }
    strncpy((char *)origdta.file_name,
#ifdef __CC64__
            (char *)curdirsave->buf + curdirsave->upto + sizeof(long) * 4
#else
            (char *)curdirsave->buf + curdirsave->upto + sizeof(long) * 2
#endif
                    + sizeof(short) + EXTRAPAD,
            sizeof origdta.file_name);
    origdta.file_name[sizeof origdta.file_name - 1] = '\0';

    strncpy((char *)origdta.lfn,
#ifdef __CC64__
            (char *)curdirsave->buf + curdirsave->upto + sizeof(long) * 4
#else
            (char *)curdirsave->buf + curdirsave->upto + sizeof(long) * 2
#endif
                    + sizeof(short) + EXTRAPAD,
            sizeof origdta.lfn);
    origdta.lfn[sizeof origdta.lfn - 1] = '\0';

#if EXTRAPAD == 0

    /* DT_DIR = 4 - directory
       DT_REG = 8 - normal file
       DT_UNKNOWN = 0
     */
    if (*(curdirsave->buf + curdirsave->upto
          + *(unsigned short *)(curdirsave->buf
                                + curdirsave->upto
#ifdef __CC64__
                                + sizeof(long) * 4)
#else
                                + sizeof(long) * 2)
#endif
          -1
         )
        == 4)
    {
        origdta.attrib = FILE_ATTR_DIRECTORY;
    }
    else
    {
        origdta.attrib = 0;
    }

#endif

    curdirsave->upto += *(short *)(curdirsave->buf
                                   + curdirsave->upto
#ifdef __CC64__
                                   + sizeof(long) * 4);
#else
                                   + sizeof(long) * 2);
#endif

    if (curdirsave->upto >= curdirsave->avail)
    {
        curdirsave->upto = curdirsave->avail = 0;
    }
    return (0);
#endif
}

int PosFindFirst(char *pat, int attrib)
{
#if defined(W32EMUL) /* || defined(GENSHELL) */
    char cwd2[sizeof __cwd + 2];
    char *p;

    strcpy(cwd2, "./");
    strcat(cwd2, __cwd);
    p = cwd2;
    while ((p = strchr(p, '\\')) != NULL)
    {
        *p++ = '/';
    }
    dirfile = __open(cwd2, 0, 0);
#else
    if (strcmp(pat, "") == 0)
    {
        pat = ".";
    }
    else if (strcmp(pat, "*.*") == 0)
    {
        pat = ".";
    }
    dirfile = __open(pat, 0, 0);
#endif
    if (dirfile < 0) return (1);
    {
        curdirsave = malloc(sizeof(struct dirsave));
        if (curdirsave == NULL)
        {
            __close(dirfile);
            return (1);
        }
        *(struct dirsave **)&origdta = curdirsave;
        curdirsave->upto = 0;
        curdirsave->avail = 0;
        curdirsave->dirfilesave = dirfile;
    }
    return (ff_search());
}

int PosFindNext(void)
{
    curdirsave = *(struct dirsave **)&origdta;
    dirfile = curdirsave->dirfilesave;
    return (ff_search());
}

int PosChangeDir(const char *to)
{
    return (__chdir(to));
}

int PosMakeDir(const char *dname)
{
    return (__mkdir(dname, 0777));
}

int PosRemoveDir(const char *dname)
{
    return (__rmdir(dname));
}

int PosExec(char *prog, POSEXEC_PARMBLOCK *parmblock)
{
    return (system(prog));
}

char *PosGetCommandLine(void)
{
    return ("");
}

int PosGetReturnCode(void)
{
    return 0;
}
