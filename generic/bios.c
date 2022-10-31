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

#define MEMAMT 28*1000*1000

#if defined(__gnu_linux__) || defined(__ARM__)
extern int __start(int argc, char **argv);
#else
extern int __start(char *p);
#endif

int their_start(char *parm);

static int getmainargs(int *_Argc,
                       char ***_Argv);

#if defined(__gnu_linux__) || defined(__ARM__)
#include <pos.h>

static int dirfile;

static DTA origdta;

void *PosGetDTA(void);
int PosFindFirst(char *pat, int attrib);
int PosFindNext(void);
#endif

static OS bios = { their_start, 0, 0, NULL, printf, 0, malloc, NULL, NULL,
  fopen, fseek, fread, fclose, fwrite, fgets, strchr,
  strcmp, strncmp, strcpy, strlen, fgetc, fputc,
  fflush, setvbuf,
#if defined(__gnu_linux__) || defined(__ARM__)
  PosGetDTA, PosFindFirst, PosFindNext,
#else
  0, 0, 0,
#endif
  0, 0 };

static char buf[200];
static char cmd[200];
static char pgm[200];

static int (*genstart)(OS *bios);

int main(int argc, char **argv)
{
    unsigned char *p;
    unsigned char *entry_point;
    int rc;
    char *prog_name;

    printf("bios starting\n");
    bios.mem_amt = MEMAMT;
    bios.Xstdin = stdin;
    bios.Xstdout = stdout;
    __genstart = 1;
    bios.main = &__genmain;

    if (argc < 3)
    {
        /* should really get this from a config file */
        printf("please enter a command\n");
        fgets(cmd, sizeof cmd, stdin);
        p = strchr(cmd, '\n');
        if (p != NULL)
        {
            *p = '\0';
        }
        strcpy(pgm, PATH);
        strncat(pgm, cmd, sizeof pgm);
        pgm[sizeof pgm - 1] = '\0';
        p = strchr(pgm, ' ');
        if (p != NULL)
        {
            *p = '\0';
        }
        prog_name = pgm;
    }
    else
    {
        bios.disk_name = *(argv + 2);
        prog_name = argv[1];
    }
    p = calloc(1, 1000000);
    if (p == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    if (exeloadDoload(&entry_point, prog_name, &p) != 0)
    {
        printf("failed to load executable\n");
        return (EXIT_FAILURE);
    }
    genstart = (void *)entry_point;
    /* printf("first byte of code is %02X\n", *(unsigned char *)entry_point); */

    for (rc = 0; rc < 500; rc++)
    {
        printf("please accept a delay before we execute program "
               "in BSS memory\n");
    }

#if 1
    rc = genstart(&bios);
#else
    rc = 0;
#endif
    printf("return from called program is %d\n", rc);
    /* Some programs will have displayed output, and if you
       terminate immediately, there is no opportunity to see
       the output. Other programs, or OSes, will have already
       had the user type poweroff or exit, and the user doesn't
       need to see another message. But for now we display a
       message unconditionally. But it should really be in a
       config file. */
    printf("press enter to terminate\n");
    fgets(buf, sizeof buf, stdin);
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
    strncpy(origdta.file_name, buf + upto + 10, 11);
    origdta.file_name[11] = '\0';
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

#endif
