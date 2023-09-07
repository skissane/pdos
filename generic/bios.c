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

/*

As originally conceived, the pseudo-bios would have minimal
functionality. It would be a replacement for the PC BIOS, and only
be responsible for loading a single sector, and that code would need
to make callbacks to the provided callbacks to read more sectors
(via an odd fread, possibly of an already-open disk file, but more
likely requiring an fopen of "!BOOT" - it remains to be proven what
can fit into a sector, and for that matter, a ROM).

However, the pseudo-bios began life running under some existing OS,
which meant that individual file access existed, allowing you to
make use of the existing OS, including eventually UEFI as that OS.

As such, it became convenient to turn the pseudo-bios into some sort
of "OS converter", and included shell capabilities too.

It is unclear if one day a new version needs to be created to create
the original pseudo-bios concept, since this one has morphed into
some sort of (useful) Frankenstein.

*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <locale.h>
#include <assert.h>
#include <setjmp.h>

#ifdef W64HACK
#include <efi.h>
static EFI_STATUS dir_list (EFI_FILE_PROTOCOL *dir);
static EFI_STATUS directory_test (void);
#endif

#include "__os.h"
#include "exeload.h"

#if defined(W64HACK) || defined(W32HACK)
void w64exit(int status);
#endif

extern int __genstart;
extern int (*__genmain)(int argc, char **argv);

/* A BIOS is meant to be machine-specific, so this is not a big deal */
#if 1
#define PATH ""
/* #define PATH "./" */
#else
#define PATH "/storage/emulated/0/Download/"
#endif

#ifdef __64BIT__
#define MEMAMT 100*1000*1000
#else
#define MEMAMT 24*1000*1000
#endif

#if defined(__gnu_linux__) || defined(__ARM__) || defined(__EFI__)
extern int __start(int argc, char **argv);
#else
extern int __start(char *p);
#endif

#ifdef __CC64OS__
extern long long __ncallbacks;
#endif

int their_start(char *parm);

#ifndef __SUBC__
int getmainargs(int *_Argc, char ***_Argv);
#endif

void *PosGetDTA(void);

#if defined(__gnu_linux__) || defined(__ARM__)
#include <pos.h>

static int dirfile;

static DTA origdta;

int PosFindFirst(char *pat, int attrib);
int PosFindNext(void);
#endif

static char buf[400];
static char cmd[400];

static jmp_buf jb;

static OS bios = { their_start, 0, 0, cmd, printf, 0, malloc, NULL, NULL,
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
    int quiet = 0;

    bios.mem_amt = MEMAMT;
    bios.Xstdin = stdin;
    bios.Xstdout = stdout;
    bios.Xstderr = stderr;
    __genstart = 1;
    bios.main = &__genmain;

    /* parameters override everything */
    if (argc > 1)
    {
        if (strcmp(argv[1], "-quiet") == 0)
        {
            quiet = 1;
            argc--;
            argv++;
        }
        if (argc > 1)
        {
            if (strcmp(argv[1], "-shell") == 0)
            {
                shell = 1;
                argc--;
                argv++;
            }
        }
        if (argc > 1)
        {
            if (strcmp(argv[1], "-quiet") == 0)
            {
                quiet = 1;
                argc--;
                argv++;
            }
        }
        /* if they've typed in --help or anything, give them usage */
        if (argv[1][0] == '-')
        {
            need_usage = 1;
        }
        else if (argc == 2)
        {
            prog_name = argv[1];
            strcpy(cmd, argv[1]);
            valid = 1;
        }
        else if (argc == 3)
        {
            prog_name = argv[1];
            strcpy(cmd, argv[1]);
            strcat(cmd, " ");
            strcat(cmd, argv[2]);
            valid = 1;
        }
        else
        {
            need_usage = 1;
        }
    }
    if (!quiet && !need_usage)
    {
        printf("bios starting\n");
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
            p = (unsigned char *)strchr(buf, '\n');
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
            strcpy(cmd, buf);
            prog_name = buf;
            p = (unsigned char *)strchr(buf, ' ');
            if (p != NULL)
            {
                *p = '\0';
            }
        }

#ifdef W64HACK
    if (strcmp(prog_name, "dir") == 0)
    {
        directory_test();
        printf("enter another command, enter to exit\n");
        continue;
    }
#endif

#ifdef __64BIT__
    p = calloc(1, 50000000);
#else
    p = calloc(1, 5000000);
#endif
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

#ifdef NEED_DELAY
    for (rc = 0; rc < 500; rc++)
    {
        printf("please accept a delay before we execute program "
               "in BSS memory\n");
    }
#endif

#if defined(W64HACK) || defined(W32HACK)
#ifdef __CC64OS__
    __ncallbacks = 0;
#endif
    rc = setjmp(jb);
    if (rc != 0)
    {
        /* we got exit via longjmp */
        /* no action required */
    }
    else
    {
#endif

    printf("about to execute program\n");
#if 1
#ifdef __CC64__
    rc = (*genstart)(&bios);
#else
    rc = genstart(&bios);
#endif
#else
    rc = 0;
#endif

#if defined(W64HACK) || defined(W32HACK)
    }
#endif

    if (!quiet)
    {
        printf("return from called program is %d\n", rc);
    }
    free(p);

        if (scr == NULL)
        {
            break;
        }
    printf("enter another command, enter to exit\n");
    } while (1);

    if (need_usage)
    {
        printf("usage: bios [options] <prog> [single parm]\n");
        printf("allows execution of non-standard executables\n");
        printf("if no parameters are given and biosauto.cmd is given,\n");
        printf("commands are read, executed and there will be a pause\n");
        printf("otherwise, biosauto.shl is looked for, and there will be\n");
        printf("no pause, because it is assumed to be a shell\n");
        printf("valid options are -quiet and -shell\n");
        printf("e.g. bios -shell pdos.exe uc8086.vhd\n");
        printf("e.g. bios pcomm.exe\n");
        return (EXIT_FAILURE);
    }
    if (scr == stdin)
    {
        /* pause has already been done, effectively */
    }
    else if (!shell)
    {
        printf("press enter to exit\n");
        fgets(buf, sizeof buf, stdin);
    }
    if ((scr != NULL) && (scr != stdin))
    {
        fclose(scr);
    }
    if (!quiet)
    {
        printf("bios exiting\n");
    }
    return (0);
}

int their_start(char *parm)
{
    int rc;

#if defined(__gnu_linux__) || defined(__ARM__) || defined(__EFI__)
    int argc;
    char **argv;

    getmainargs(&argc, &argv);
    rc = __start(argc, argv);
#else
    rc = __start(cmd);
#endif
    return (rc);
}


#define MAXPARMS 50

#ifndef __SUBC__
#ifdef __CC64__
$callback
#endif
int getmainargs(int *_Argc,
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
#endif

#if defined(W64HACK) || defined(W32HACK)
#ifdef __CC64__
$callback
#endif
void w64exit(int status)
{
    longjmp(jb, status);
    return;
}
#endif

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


#ifdef W64HACK

#define STATUS_IS_ERROR(a) (!!((a) >> (sizeof (a) * 8 - 1)))
#define STATUS_GET_CODE(a) (((a) << 1) >> 1)

#define return_Status_if_fail(func) do { if ((Status = (func))) { return Status; }} while (0)

static EFI_STATUS dir_list (EFI_FILE_PROTOCOL *dir)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN Read, buf_size;
    EFI_FILE_INFO *Buffer;

    buf_size = sizeof (*Buffer) + 8 /* Just enough space for 5 CHAR16 FileName for demonstration. */;

    return_Status_if_fail (__gBS->AllocatePool (EfiLoaderData, buf_size, (void **)&Buffer));

    while (1) {
        Read = buf_size;
        Status = dir->Read (dir, &Read, Buffer);
        if (STATUS_IS_ERROR (Status) && STATUS_GET_CODE (Status) == EFI_BUFFER_TOO_SMALL) {
            __gBS->FreePool (Buffer);
            buf_size = Read;
            /* printf("HAD to increase size of buffer\n"); */
            return_Status_if_fail (__gBS->AllocatePool (EfiLoaderData, buf_size, (void **)&Buffer));
            return_Status_if_fail (dir->Read (dir, &buf_size, Buffer));
        }

        if (!Read) break;

        return_Status_if_fail (__gST->ConOut->OutputString (__gST->ConOut, Buffer->FileName));
        printf("\n");
    };

    __gBS->FreePool (Buffer);

    return Status;
}

static EFI_STATUS directory_test (void)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID li_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *li_protocol;
    EFI_GUID sfs_protocol_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs_protocol;
    
    EFI_FILE_PROTOCOL *EfiRoot;

    return_Status_if_fail (__gBS->HandleProtocol (__gIH, &li_guid, (void **)&li_protocol));
    return_Status_if_fail (__gBS->HandleProtocol (li_protocol->DeviceHandle, &sfs_protocol_guid, (void **)&sfs_protocol));
    return_Status_if_fail (sfs_protocol->OpenVolume (sfs_protocol, &EfiRoot));

    return_Status_if_fail (dir_list (EfiRoot));
    
    EfiRoot->Close (EfiRoot);

    return Status;
}

#endif
