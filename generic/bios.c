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
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <locale.h>
#include <assert.h>

#include "__os.h"
#include "exeload.h"

extern int __genstart;
extern int (*__genmain)(int argc, char **argv);

/* A BIOS is meant to be machine-specific, so this is not a big deal */
#if 1
#define PATH ""
/* #define PATH "./" */
#else
#define PATH "/storage/emulated/0/Download/"
#endif

#define MEMAMT 24*1000*1000

#if defined(__gnu_linux__) || defined(__ARM__)
extern int __start(int argc, char **argv);
#else
extern int __start(char *p);
#endif

int their_start(char *parm);

static int getmainargs(int *_Argc,
                       char ***_Argv);

void *PosGetDTA(void);

#if defined(__gnu_linux__) || defined(__ARM__)
#include <pos.h>

static int dirfile;

static DTA origdta;

int PosFindFirst(char *pat, int attrib);
int PosFindNext(void);
#endif

static OS bios = { their_start, 0, 0, NULL, NULL, printf, 0, malloc, NULL, NULL,
  fopen, fseek, fread, fclose, fwrite, fgets, strchr,
  strcmp, strncmp, strcpy, strlen, fgetc, fputc,
  fflush, setvbuf,
  PosGetDTA,
#if defined(__gnu_linux__) || defined(__ARM__)
  PosFindFirst, PosFindNext,
#else
  0, 0,
#endif
  0, 0,
  ctime, time,
#if defined(__gnu_linux__) || defined(__ARM__)
  PosChangeDir, PosMakeDir, PosRemoveDir,
#else
  0, 0, 0,
#endif
  remove,
  memcpy, strncpy, strcat, 0 /* stderr */, free, abort, memset, fputs, fprintf,
  getenv, memmove, exit, memcmp, _errno, tmpnam, vfprintf, ungetc, vsprintf,
  sprintf, signal, raise, calloc, realloc, atoi, strtol, strtoul, qsort,
  bsearch, localtime, clock, strerror, strrchr, strstr, strpbrk, strspn,
  strcspn, memchr, ftell, abs, setlocale, perror, rewind, strncat, sscanf,
  isalnum, isxdigit, rename, clearerr, _assert, atof,
};

static char buf[400];
static char cmd[300];

static int (*genstart)(OS *bios);

int main(int argc, char **argv)
{
    unsigned char *p;
    unsigned char *entry_point;
    int rc;
    char *prog_name;
    int need_usage = 0;
    int valid = 0;
    int shell = 0;
    FILE *scr = NULL;

    printf("bios starting\n");
    bios.mem_amt = MEMAMT;
    bios.Xstdin = stdin;
    bios.Xstdout = stdout;
    bios.Xstderr = stderr;
    __genstart = 1;
    bios.main = &__genmain;

    /* parameters override everything */
    if (argc > 1)
    {
        /* if they've typed in --help or anything, give them usage */
        if (argv[1][0] == '-')
        {
            need_usage = 1;
        }
        else if (argc == 2)
        {
            bios.prog_name = argv[1];
            bios.prog_parm = "";
            valid = 1;
        }
        else if (argc == 3)
        {
            bios.prog_name = argv[1];
            bios.prog_parm = argv[2];
            valid = 1;
        }
        else
        {
            need_usage = 1;
        }
    }
    if (!valid && !need_usage)
    {
        /* an individual command(s) overrides a shell */
        scr = fopen("biosauto.cmd", "r");
        if (scr != NULL)
        {
            valid = 1;
        }
        else
        {
            scr = fopen("biosauto.shl", "r");
            if (scr != NULL)
            {
                valid = 1;
                shell = 1;
            }
        }
    }
    if (!valid && !need_usage)
    {
        scr = stdin;
        printf("enter commands, press enter to exit\n");
    }
    do
    {
        if (need_usage) break; /* should put this before do */
        if (scr != NULL)
        {
            if (fgets(buf, sizeof buf, scr) == NULL)
            {
                break;
            }
            p = strchr(buf, '\n');
            if (p != NULL)
            {
                *p = '\0';
            }
            if (buf[0] == '\0')
            {
                if (scr == stdin)
                {
                    break;
                }
                continue;
            }
            if (buf[0] == '#')
            {
                continue;
            }
            bios.prog_name = buf;
            p = strchr(buf, ' ');
            if (p != NULL)
            {
                *p = '\0';
                bios.prog_parm = p + 1;
            }
            else
            {
                bios.prog_parm = "";
            }
        }

    p = calloc(1, 5000000);
    if (p == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    if (exeloadDoload(&entry_point, bios.prog_name, &p) != 0)
    {
        printf("failed to load executable\n");
        return (EXIT_FAILURE);
    }
    genstart = (void *)entry_point;
    /* printf("first byte of code is %02X\n", *(unsigned char *)entry_point); */

#ifdef NEED_DELAY
    for (rc = 0; rc < 500; rc++)
    {
        printf("please accept a delay before we execute program "
               "in BSS memory\n");
    }
#endif

#if 1
    rc = genstart(&bios);
#else
    rc = 0;
#endif
    printf("return from called program is %d\n", rc);
    free(p);

        if (scr == NULL)
        {
            break;
        }
    } while (1);

    if (need_usage)
    {
        printf("usage: bios <prog> [single parm]\n");
        printf("allows execution of non-standard executables\n");
        printf("if no parameters are given and biosauto.cmd is given,\n");
        printf("commands are read, executed and there will be a pause\n");
        printf("otherwise, biosauto.shl is looked for, and there will be\n");
        printf("no pause, because it is assumed to be a shell\n");
        return (EXIT_FAILURE);
    }
    if (scr == stdin)
    {
        /* pause has already been done, effectively */
    }
    else if (!shell || (scr == NULL))
    {
        printf("press enter to exit\n");
        fgets(buf, sizeof buf, stdin);
    }
    if ((scr != NULL) && (scr != stdin))
    {
        fclose(scr);
    }
    printf("bios exiting\n");
    return (0);
}

int their_start(char *parm)
{
#if defined(__gnu_linux__) || defined(__ARM__)
    int argc;
    char **argv;
    getmainargs(&argc, &argv);
    __start(argc, argv);
#else
    __start(parm);
#endif
}


#define MAXPARMS 50

static int getmainargs(int *_Argc,
                       char ***_Argv)
{
    char *p;
    int x;
    int argc;
    static char *argv[MAXPARMS + 1];
    static char *env[] = {NULL};

    p = cmd;

    argv[0] = p;
    p = strchr(p, ' ');
    if (p == NULL)
    {
        p = "";
    }
    else
    {
        *p = '\0';
        p++;
    }

    while (*p == ' ')
    {
        p++;
    }
    if (*p == '\0')
    {
        argv[1] = NULL;
        argc = 1;
    }
    else
    {
        for (x = 1; x < MAXPARMS; )
        {
            char srch = ' ';

            if (*p == '"')
            {
                p++;
                srch = '"';
            }
            argv[x] = p;
            x++;
            p = strchr(p, srch);
            if (p == NULL)
            {
                break;
            }
            else
            {
                *p = '\0';
                p++;
                while (*p == ' ') p++;
                if (*p == '\0') break; /* strip trailing blanks */
            }
        }
        argv[x] = NULL;
        argc = x;
    }

    *_Argc = argc;
    *_Argv = argv;
    return (0);
}

#if defined(__gnu_linux__) || defined(__ARM__)

void *PosGetDTA(void)
{
    return (&origdta);
}

static int ff_search(void)
{
    static unsigned char buf[500];
    static size_t upto = 0;
    static size_t avail = 0;

    if (avail <= 0)
    {
        avail = __getdents(dirfile, buf, 500);
        if (avail <= 0)
        {
            __close(dirfile);
            return (1);
        }
    }
    strncpy(origdta.file_name, buf + upto + 10, sizeof origdta.file_name);
    origdta.file_name[sizeof origdta.file_name - 1] = '\0';
    strncpy(origdta.lfn, buf + upto + 10, sizeof origdta.lfn);
    origdta.lfn[sizeof origdta.lfn - 1] = '\0';
    upto += *(short *)(buf + upto + 8);
    if (upto >= avail)
    {
        upto = avail = 0;
    }
    return (0);
}

int PosFindFirst(char *pat, int attrib)
{
    dirfile = __open(".", 0, 0);
    if (dirfile < 0) return (1);
    return (ff_search());
}

int PosFindNext(void)
{
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

#else

void *PosGetDTA(void)
{
    return (NULL);
}

#endif
