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

#if defined(__MACOS__) || defined(__CC64__)
int __close(int fd);
int __open(char *path, int flags, int x);
int __chdir(const char *filename);
int __mkdir(const char *filename, int x);
int __rmdir(const char *filename);
#endif

#if defined(__64BIT__) && !defined(__LONG64__)
int __getdents(int dirfile, void *buf, int n);
#endif

#if defined(__gnu_linux__) && defined(__M68K__)
static int callami(char *buf);
#endif


#ifdef FREEMEM
#include <__memmgr.h>
extern int __mmgid;
#endif

static int globrc = 0;

#if defined(W64HACK) || defined(__EFI__) || defined(W64DLL)
#include <efi.h>
static EFI_STATUS dir_list (EFI_FILE_PROTOCOL *dir);
static EFI_STATUS directory_test (void);
static EFI_STATUS check_path(unsigned char *);
#endif

#include "__os.h"
#include "exeload.h"

#define EXITMSG "enter command, exit to exit\n"

#if defined(W64HACK) || defined(W32HACK) || defined(W64DLL)
void w64exit(int status);
#endif

#if defined(W32EMUL)
void w32exit(int status);
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
#elif defined(__ZPDOSGPB__)
#define MEMAMT 70*1000*1000
#else
#define MEMAMT 24*1000*1000
#endif

extern int __start(char *p);

#ifdef __CC64OS__
extern long long __ncallbacks;
#endif

#if defined(__EFI__) || defined(W64HACK) || defined(W32EMUL) \
    || defined(GENSHELL)
extern char __cwd[FILENAME_MAX];
#endif

int their_start(char *parm);

#ifndef __SUBC__
int getmainargs(int *_Argc, char ***_Argv);
#endif

void *PosGetDTA(void);

#if defined(__gnu_linux__) || defined(__ARM__) \
   && !defined(__M68K__)
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


int PosFindFirst(char *pat, int attrib);
int PosFindNext(void);
#endif

extern int salone; /* is this a standalone program that
    will not invoke start of its own free will? */

static char buf[600];
static char cmd[600];

static jmp_buf jb;

static OS bios = { their_start, 0, 0, cmd, printf, 0, malloc, NULL, NULL,
  fopen, fseek, fread, fclose, fwrite, fgets, strchr,
  strcmp, strncmp, strcpy, strlen, fgetc, fputc,
  fflush, setvbuf,
  PosGetDTA,
#if defined(__gnu_linux__) \
   && !defined(__M68K__) /* || defined(__ARM__) */
  PosFindFirst, PosFindNext,
#else
  0, 0,
#endif
  0, 0,
  ctime, time,
#if defined(__gnu_linux__) \
  && !defined(__M68K__) /* || defined(__ARM__) */
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
  isdigit, isalpha, isprint, isspace, tolower, system,
  islower, isupper, atexit, ceil, toupper, iscntrl,
  sin, cos, tan, floor, asin, acos, atan, sinh, cosh, tanh,
  rand, srand, strftime, puts,
  pow, modf, log, log10, atan2, fabs, exp, sqrt,
  strtok, atol, mktime, vprintf, ferror, putc, feof, getc,
  getchar, putchar, 0 /* PosExec */, longjmp,
  0, /* service call */
  0, /* PosGetCommandLine */
  0, /* PosGetReturnCode */
  strtod, tmpfile,
};

static int (*genstart)(OS *bios);

#if SHIMCM32
static void shimcm32_start(void);
static void shimcm32_run(void);
static void shimcm32_end(void);
#endif

static struct {
    char *name;
    int to_handle_num;
    FILE *saved_handle;
    enum {
        MODE_NONE = 0,
        MODE_TO_FILE,
        MODE_TO_HANDLE,
        MODE_APPEND,
        MODE_FROM_FILE,
        MODE_FROM_HANDLE
    } mode;
} redirects[10] = {{NULL}};
static const char *error_msg = NULL;

static int parse_redirection (char *src, char *dst)
{    
#define SKIP_SPACES(a) do { while (*(a) == ' ') (a)++; } while (0)
    SKIP_SPACES (src);
    
    while (*src) {
        int handle_num = -1;

        if (*src == '^') {
            src++;
            goto copy;
        }

        if (isdigit (*src)
            && (src[1] == '<' || src[1] == '>')) {
            handle_num = *src - '0';
            src++;
        }

        if (*src == '<' || *src == '>') {
            char *p;
            
            if (*src == '<') {
                if (handle_num == -1) handle_num = 0;
                redirects[handle_num].mode = MODE_FROM_FILE;
                if (src[1] == '&') {
                    redirects[handle_num].mode = MODE_FROM_HANDLE;
                    *src = '\0';
                    src++;
                }
            } else {
                if (handle_num == -1) handle_num = 1;
                redirects[handle_num].mode = MODE_TO_FILE;
                if (src[1] == '&') {
                    redirects[handle_num].mode = MODE_TO_HANDLE;
                    *src = '\0';
                    src++;
                } else if (src[1] == '>') {
                    redirects[handle_num].mode = MODE_APPEND;
                    *src = '\0';
                    src++;
                }
            }

            *src = '\0';
            src++;

            SKIP_SPACES (src);

            if (redirects[handle_num].mode == MODE_TO_HANDLE
                || redirects[handle_num].mode == MODE_FROM_HANDLE) {
                long lhandle;

                lhandle = strtol (src, &p, 10);
                if (lhandle == 0 && src == p) {
                    fprintf (stderr,
                         "The syntax of the command is incorrect.\n");
                    return 1;
                }

                redirects[handle_num].to_handle_num = (int)lhandle;
            } else {
                for (p = src;
                     *p && *p != '>' && *p != '<' && *p != ' ';
                     p++) {}
                if (p == src) {
                    fprintf (stderr,
                             "The syntax of the command is incorrect.\n");
                    return 1;
                }

                redirects[handle_num].name = src;
            }

            src = p;
            if (*src == ' ') {
                *src = '\0';
                src++;
                SKIP_SPACES (src);
            }
        } else {
    copy:
            *dst = *src;
            src++;
            dst++;
        }
    }
    *dst = '\0';

    return 0;
}

static int do_redirection (void)
{
    int i;
    
    /* C library does not know about the redirection,
     * so it needs to be told to flush buffers.
     */
    fflush (stdout);
    fflush (stderr);

    /* Errors that occur during redirection cannot be reported immediately
     * as stderr might have been already redirected.
     */
    for (i = 0; i < sizeof (redirects) / sizeof (redirects[0]); i++) {
        if (redirects[i].mode) {
            FILE *file;

            switch (redirects[i].mode) {
                case MODE_TO_FILE:
                    if (!(file = fopen (redirects[i].name, "w"))) {
                        error_msg = "Failed to create file.";
                        goto end;
                    }
                    break;

                case MODE_TO_HANDLE:
                    error_msg = "'>&' is not yet supported.";
                    goto end;

                case MODE_APPEND:
                    error_msg = "'>>' is not yet supported.";
                    goto end;

                case MODE_FROM_FILE:
                    /* '<' does not provide the file directly,
                     * it "type"s it and provides the output.
                     * (So '<' can be done with same file
                     *  multiple times for one command.)
                     * It could be probably implemented using
                     * temporary file.
                     */
                    error_msg = "'<' is not yet supported.";
                    goto end;

                case MODE_FROM_HANDLE:
                    error_msg = "'<&' is not yet supported.";
                    goto end;
            }

            /* Handles 3-9 are undefined, so they should not be redirected
             * but they still should create a file when '>' is used.
             */
            if (i < 3) {
                /* The underscores should be removed
                 * if non-PDPCLIB C library is used. */
#define CHANGE_HANDLE(name, file) \
 do { redirects[i].saved_handle = __##name; \
      __##name = (file); bios.X##name = (file); } while (0)

                switch (i) {
                    case 0: CHANGE_HANDLE (stdin, file); break;
                    case 1: CHANGE_HANDLE (stdout, file); break;
                    case 2: CHANGE_HANDLE (stderr, file); break;
                }

#undef CHANGE_HANDLE
            }
        }
    }

    return 0;

end:
    return 1;
}

static int undo_redirection (void)
{
    int i;
    
    fflush (stdout);
    fflush (stderr);

    for (i = 0; i < sizeof (redirects) / sizeof (redirects[0]); i++) {
        if (redirects[i].mode) {
            if (redirects[i].saved_handle) {
#define RESTORE_HANDLE(name) \
 do { fclose (__##name); \
      __##name = redirects[i].saved_handle; bios.X##name = name; } while (0)

                switch (i) {
                    case 0: RESTORE_HANDLE (stdin); break;
                    case 1: RESTORE_HANDLE (stdout); break;
                    case 2: RESTORE_HANDLE (stderr); break;
                }

#undef RESTORE_HANDLE
            }
        }
    }

    memset (redirects, 0, sizeof (redirects));

    if (error_msg) {
        fprintf (stderr, "%s\n", error_msg);
        error_msg = NULL;
        return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    unsigned char *p;
    unsigned char *entry_point;
    int rc;
    char prog_name_s[FILENAME_MAX];
    char *prog_name;
    int need_usage = 0;
    int valid = 0;
    int shell = 0;
    FILE *scr = NULL;
    int quiet = 0;
    char new_prog_name[FILENAME_MAX];
    int alloc_here = 1; /* we do initial allocation */

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
        printf("\nbios starting\n");
#if defined(W64HACK) || defined(W32EMUL) || defined(GENSHELL)
        printf("type in \"dir\" to get a list of files\n");
        printf("type in \"type\" to show the contents of a file\n");
        printf("type in \"e/me/uemacs file.txt\" to edit a file\n");
        printf("ctrl-x, ctrl-s saves; ctrl-x, ctrl-c quits\n");
#endif
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
        printf(EXITMSG);
    }

#if SHIMCM32
    shimcm32_start();
#endif

    do
    {
        if (need_usage) break; /* should put this before do */
        
        undo_redirection ();
        
        if (scr != NULL)
        {
            if (fgets(buf, sizeof buf, scr) == NULL)
            {
#if defined(W64HACK) || defined(W32EMUL) || defined(GENSHELL)
                if (scr == stdin) break;
                fclose(scr);
                scr = stdin;
                continue;
#else
                break;
#endif
            }
#if defined(W64HACK) || defined(W32EMUL) || defined(GENSHELL)
            if (scr != stdin) printf("%s", buf);
#endif
            p = (unsigned char *)strchr(buf, '\n');
            if (p != NULL)
            {
                *p = '\0';
            }
            if (buf[0] == '\0')
            {
                if (scr == stdin)
                {
#if defined(W64HACK) || defined(W32EMUL) || defined(GENSHELL)
                    continue;
#else
                    break;
#endif
                }
                continue;
            }
            if (buf[0] == '#')
            {
                continue;
            }

            if (parse_redirection (buf, cmd)) continue;
            /* Redirection without a command is not permitted. */
            if (!cmd[0]) {
                fprintf (stderr, "The syntax of the command is incorrect.\n");
                continue;
            }
            if (do_redirection ()) continue;

            strcpy (prog_name_s, cmd);
            prog_name = prog_name_s;
            p = (unsigned char *)strchr(prog_name, ' ');
            if (p != NULL)
            {
                *p = '\0';
            }
        }

#if defined(W64HACK) || defined(W32EMUL) || defined(GENSHELL)
    if (strcmp(prog_name, "dir") == 0)
    {
#if (defined(W32EMUL) || defined(GENSHELL)) \
    && !defined(__EFI__)

#ifndef HAVE_DIR
        printf("sorry - dir not supported yet\n");
#else
        {
            DTA *dta;
            int ret;

            dta = PosGetDTA();
            if (dta == NULL)
            {
                printf("dir unavailable\n");
                continue;
            }
            ret = PosFindFirst("*.*", 0x10);
            while (ret == 0)
            {
                if (dta->lfn[0] != '\0')
                {
                    printf("%s\n", dta->lfn);
                }
                else
                {
                    printf("%s\n", dta->file_name);
                }
                ret = PosFindNext();
            }
        }
#endif

#else
        directory_test();
#endif
        undo_redirection ();
        printf(EXITMSG);
        continue;
    }
    else if (strcmp(prog_name, "exit") == 0)
    {
        break;
    }
    else if (strcmp(prog_name, "cd") == 0)
    {
        p++;
#ifdef LINDIR
        PosChangeDir((char *)p);
#elif defined(__EFI__)
        check_path(p);
#endif
        undo_redirection ();
        printf(EXITMSG);
        continue;
    }
    else if (strcmp(prog_name, "type") == 0)
    {
        FILE *fp;
        fp = fopen((char *)p + 1, "r");
        if (fp != NULL)
        {
            int c;
            while ((c = getc(fp)) != EOF)
            {
                putc(c, stdout);
            }
            fclose(fp);
        }
        undo_redirection ();
        printf(EXITMSG);
        continue;
    }
    else if (strcmp(prog_name, "copy") == 0)
    {
        FILE *fp;
        FILE *fq;
        char *q;
        int c;

        q = strchr((char *)p + 1, ' ');
        if (q == NULL)
        {
            printf("two filenames required\n");
        }
        else
        {
            *q = '\0';
            fp = fopen((char *)p + 1, "rb");
            if (fp == NULL)
            {
                printf("can't open %s\n", p + 1);
            }
            else
            {
                fq = fopen(q + 1, "wb");
                if (fq == NULL)
                {
                    printf("can't open %s\n", q + 1);
                }
                else
                {
                    while ((c = getc(fp)) != EOF)
                    {
                        putc(c, fq);
                    }
                    fclose(fq);
                }
                fclose(fp);
            }
        }
        undo_redirection ();
        printf(EXITMSG);
        continue;
    }
#endif

#if defined(W64HACK) || defined(W32EMUL) || defined(GENSHELL)
    strcpy(new_prog_name, prog_name);
    if (strstr(new_prog_name, ".exe") == NULL)
    {
        strcat(new_prog_name, ".exe");
    }
    prog_name = new_prog_name;
    {
        FILE *fp;

#ifdef LINDIR
        char cwd2[sizeof __cwd + 2];
        char *p;

        /* cwd2 is historical - should go back to
           non-lindir logic or close */
        /* but need a PATH command first */
        strcpy(cwd2, "");
        strcat(cwd2, prog_name);
        fp = fopen(cwd2, "rb");
        if (fp == NULL)
        {
            char *zzz;

            zzz = strstr(cwd2, ".exe");
            if (zzz != NULL)
            {
                strcpy(zzz, ".bat");
                fp = fopen(cwd2, "r");
                if (fp != NULL)
                {
                    scr = fp;
                    strcpy(prog_name, cwd2);
                    continue;
                }
                strcpy(zzz, ".exe");
            }
        }
        if (fp == NULL)
        {
            /* hack for now - off current directory */
            strcpy(cwd2, "./dos/");
            strcat(cwd2, prog_name);
            fp = fopen(cwd2, "rb");
        }
        if (fp == NULL)
        {
            /* hack for now - one level up */
            strcpy(cwd2, "../dos/");
            strcat(cwd2, prog_name);
            fp = fopen(cwd2, "rb");
        }
        if (fp == NULL)
        {
            /* hack for now - one level up */
            strcpy(cwd2, "../");
            strcat(cwd2, prog_name);
            fp = fopen(cwd2, "rb");
        }
        if (fp != NULL)
        {
            strcpy(prog_name, cwd2);
        }
#else
        fp = fopen(prog_name, "rb");
        if (fp == NULL)
        {
            char *zzz;

            zzz = strstr(prog_name, ".exe");
            if (zzz != NULL)
            {
                strcpy(zzz, ".bat");
                fp = fopen(prog_name, "r");
                if (fp != NULL)
                {
                    scr = fp;
                    continue;
                }
                strcpy(zzz, ".exe");
            }
        }
        if (fp == NULL)
        {
            memmove(prog_name + 1, prog_name, strlen(prog_name) + 1);
            prog_name[0] = '\\';
            fp = fopen(prog_name, "rb");
        }
        if (fp == NULL)
        {
            memmove(prog_name + 4, prog_name, strlen(prog_name) + 1);
            memcpy(prog_name, "\\DOS", 4);
            fp = fopen(prog_name, "rb");
        }
#endif
        if (fp == NULL)
        {
#ifdef LINDIR
            printf("no such program %s\n", prog_name);
#else
            printf("no such program %s\n", prog_name + 5);
#endif
            undo_redirection ();
            printf(EXITMSG);
            continue;
        }
        fclose(fp);
    }
#endif

/* cc64 uses lots of BSS - 47 MB - so on any environment that
   it runs on, ie x64, need to bump up allocated memory - but
   may as well just let exeload do that on those environments */
#if defined(__gnu_linux__) && defined(__64BIT__) && defined(WARN2G)
    p = calloc(1, 50000000);
    printf("if %p is above 2 GiB, you will get a crash\n", p);
    printf("(only if called program is using cc64/gccw64_l64/etc)\n");

#elif defined(__OS2__) || defined(__64BIT__)
    p = NULL;
    alloc_here = 0;
#else
    p = calloc(1, 5000000);
#endif

    if ((p == NULL) && alloc_here)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }

    salone = 0;
    if ((strncmp(prog_name, "addr=", 5) == 0)
        || (strncmp(prog_name, "ADDR=", 5) == 0)
       )
    {
        entry_point = (void *)strtoul(prog_name + 5, NULL, 16);
    }
#if SHIMCM32
    else if (1)
    {
        /* don't load anything for now - just run the
           dummy code */
    }
#endif
    else if (exeloadDoload(&entry_point, prog_name, &p) != 0)
    {
        printf("failed to load executable\n");
        return (EXIT_FAILURE);
    }
    genstart = (void *)entry_point;
    /* printf("entry_point is %p\n", entry_point); */
    /* printf("first byte of code is %02X\n", *(unsigned char *)entry_point); */

#ifdef NEED_DELAY
#ifdef __ARM__
    {
        /* 1, 0 works */
        /* 0, 50000000 works */
        /* 0, 5000000 sometimes fails */
        /* 0, 10000000 works, so double to be hopefully safe */
        /* Although that is apparently enough for a Pinebook Pro,
           it wasn't enough for an armv7 netbook, which continued
           to randomly fail. So 1 second was added */
        static unsigned int val1[2] = {1, 20000000UL};
        unsigned int val2[2];
        printf("sleeping before executing BSS memory\n");
        __nanosleep(val1, val2);
    }
#else
    for (rc = 0; rc < 500; rc++)
    {
        printf("please accept a delay before we execute program "
               "in BSS memory\n");
    }
#endif

#endif


#ifdef NEED_FLUSH
#if defined(__gnu_linux__) && defined(__64BIT__)
        __cacheflush(p, p + 50000000, 0);
#else
        __cacheflush(p, p + 5000000, 0);
#endif
#endif


#if defined(W64HACK) || defined(W32HACK) || defined(W32EMUL) \
    || defined(GENSHELL) || defined(W64DLL)
#ifdef __CC64OS__
    __ncallbacks = 0;
#endif
    rc = setjmp(jb);
    if (rc != 0)
    {
        /* we got exit via longjmp */
        /* true return code is in global */
        rc = globrc;
        undo_redirection ();
    }
    else
    {
#endif

#ifdef FREEMEM
        __mmgid += 256;
#endif

    if (!quiet)
    {
        printf("about to execute program with entry point %p\n",
               entry_point);
    }
#if 1
#ifdef __CC64__
    rc = (*genstart)(&bios);
#elif SHIMCM32
    shimcm32_run();
#elif defined(__gnu_linux__) && defined(__M68K__)
    rc = callami(cmd);
#else
    if (salone)
    {
        __genmain = (void *)genstart;
        rc = __start(cmd);
        salone = 0;
        __genmain = 0;
    }
    else
    {
        rc = genstart(&bios);
    }
#endif
#else
    rc = 0;
#endif
    undo_redirection ();
    
#if defined(W64HACK) || defined(W32HACK) || defined(W32EMUL) \
    || defined(GENSHELL) || defined(W64DLL)
    }
#endif

    if (!quiet)
    {
        printf("return from called program is hex %x\n", rc);
    }
    free(p);

#ifdef FREEMEM
        memmgrFreeId(&__memmgr, __mmgid);
        __mmgid -= 256;
#endif

        if (scr == NULL)
        {
            break;
        }
#if defined(W64HACK) || defined(W32EMUL) || defined(GENSHELL)
    printf(EXITMSG);
#else
    printf(EXITMSG);
#endif
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
        printf("e.g. bios addr=0x1000000\n");
        return (EXIT_FAILURE);
    }
    if (scr == stdin)
    {
        /* pause has already been done, effectively */
    }
    else if (quiet)
    {
        /* for now, assume they want immediate exit */
        /* we can add another parameter to force a pause if desired */
    }
    else if (!shell)
    {
        printf(EXITMSG);
        fgets(buf, sizeof buf, stdin);
    }
    if ((scr != NULL) && (scr != stdin))
    {
        fclose(scr);
    }
#if SHIMCM32
    shimcm32_end();
#endif
    if (!quiet)
    {
        printf("bios exiting\n");
    }
    return (0);
}

int their_start(char *parm)
{
    int rc;

    rc = __start(cmd);
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

#if defined(W64HACK) || defined(W32HACK) || defined(W64DLL)
#ifdef __CC64__
$callback
#endif
void w64exit(int status)
{
    globrc = status;
    longjmp(jb, status);
    return;
}
#endif

/* this should be combined with the w64 version */
#if defined(W32EMUL) || defined(GENSHELL)
void w32exit(int status)
{
    globrc = status;
    longjmp(jb, status);
    return;
}
#endif


#if defined(__gnu_linux__) \
    && !defined(__M68K__) /* || defined(__ARM__) */

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
#if defined(__64BIT__) && !defined(__LONG64__)
            (char *)curdirsave->buf + curdirsave->upto + sizeof(long) * 4
#else
            (char *)curdirsave->buf + curdirsave->upto + sizeof(long) * 2
#endif
                    + sizeof(short) + EXTRAPAD,
            sizeof origdta.file_name);
    origdta.file_name[sizeof origdta.file_name - 1] = '\0';

    strncpy((char *)origdta.lfn,
#if defined(__64BIT__) && !defined(__LONG64__)
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
#if defined(__64BIT__) && !defined(__LONG64__)
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
#if defined(__64BIT__) && !defined(__LONG64__)
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

#else

void *PosGetDTA(void)
{
    return (NULL);
}

#endif


char *PosGetCommandLine(void)
{
    return (cmd);
}

void PosTerminate(int rc)
{
    exit(rc);
    return;
}

#if defined(W64HACK) || defined(__EFI__)

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
    static CHAR16 path[FILENAME_MAX];
    size_t len = strlen(__cwd);

#ifndef __NO_LONGLONG_AND_LONG_IS_ONLY_32BIT__
    static UINT64 OpenModeDirReadWrite = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    static UINT64 Attributes = 0;
#else
    static UINT64 OpenModeDirReadWrite = {0x3, 0};
    static UINT64 Attributes = {0, 0};
#endif

    return_Status_if_fail (__gBS->HandleProtocol (__gIH, &li_guid, (void **)&li_protocol));
    return_Status_if_fail (__gBS->HandleProtocol (li_protocol->DeviceHandle, &sfs_protocol_guid, (void **)&sfs_protocol));
    return_Status_if_fail (sfs_protocol->OpenVolume (sfs_protocol, &EfiRoot));

    if (len != 0) 
    {
        EFI_FILE_PROTOCOL *dir;
        int i = 0;
        for (i = 0; i < len; ++i)
        {
            path[i] = __cwd[i];
        }
        path[i] = '\0';

        Status = EfiRoot->Open (EfiRoot, &dir, path, OpenModeDirReadWrite, Attributes);
        if (Status) return Status;

        return_Status_if_fail (dir_list (dir));
        dir->Close (dir);
    } else 
    {
        return_Status_if_fail (dir_list (EfiRoot));
    }
 
    EfiRoot->Close (EfiRoot);
    return Status;
}

static EFI_STATUS check_path(unsigned char *path)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID li_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *li_protocol;
    EFI_GUID sfs_protocol_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs_protocol;
    EFI_FILE_PROTOCOL *EfiRoot, *dir;
    size_t len;

#ifndef __NO_LONGLONG_AND_LONG_IS_ONLY_32BIT__
    static UINT64 OpenModeDirReadWrite = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    static UINT64 Attributes = 0;
#else
    static UINT64 OpenModeDirReadWrite = {0x3, 0};
    static UINT64 Attributes = {0, 0};
#endif

    static CHAR16 path_check[FILENAME_MAX];
    static char cur_path[FILENAME_MAX];
    strcpy(cur_path, __cwd);

    if (path[0] == '\\')
    {
        strcpy(cur_path, (char *)path + 1);
    }
    else
    { 
        if (cur_path[0] == '\0')
        {
            strcpy(cur_path, (char *)path);
        }
        else
        {
            /* +++ check overflow here */
            strcat(cur_path, "\\");
            strcat(cur_path, (char *)path);
        }
    }
    
    len = strlen(cur_path);
    if (len == 0) {
        path_check[0] = '\\';
        path_check[1] = '\0';
    } else 
    {
        int i = 0;
        for (i = 0; i < len; ++i)
        {
            path_check[i] = cur_path[i];
        }
        path_check[i] = '\0';
    }
    
    return_Status_if_fail (__gBS->HandleProtocol (__gIH, &li_guid, (void **)&li_protocol));
    return_Status_if_fail (__gBS->HandleProtocol (li_protocol->DeviceHandle, &sfs_protocol_guid, (void **)&sfs_protocol));
    return_Status_if_fail (sfs_protocol->OpenVolume (sfs_protocol, &EfiRoot));

    Status = EfiRoot->Open (EfiRoot, &dir, path_check, OpenModeDirReadWrite, Attributes);
    if (STATUS_IS_ERROR (Status) && STATUS_GET_CODE (Status) == EFI_NOT_FOUND) {
        return_Status_if_fail (printf("%s does not exist\n", cur_path));
    }
    if (Status) return Status;

    /*update __cwd*/
    strcpy(__cwd, cur_path);
    
    dir->Close (dir);
    EfiRoot->Close (EfiRoot);

    return Status;
}

#endif



#if defined(__gnu_linux__) && defined(__M68K__)

#include <clib/dos_protos.h>

struct Node {
    struct Node *ln_Succ;
    char filler[6];
    char *ln_Name;
};

/* static struct Node dosnode = {
    NULL,
    {},
    "dos.library" }; */

struct Library {
    struct Node lib_Node;
    /* other junk */
};

/* static struct Library doslib = {
    dosnode }; */

static struct {
    void *(*Input)(void);
    char filler[50];
    struct Node node;
} dosfuncs = {
  Input,
  {},
  {
    NULL,
    {},
    "dos.library"
  }
  };

static void *DOSBase = &dosfuncs.node;

struct List {
    struct Node *lh_Head;
    char filler[10];
};

/* static struct List doslist = {
    &dosfuncs.node }; */

struct ExecBase {
    char filler[378];
    struct List LibList;
};

extern struct Node newdosbase;

extern struct ExecBase SysBase;

#if 0
static struct ExecBase SysBase = {
    {},
    { &newdosbase /* &dosfuncs.node */, {} },
    };
#endif

void *Input(void);
void *c_Input(void)
{
    return (stdin);
}

void *Output(void);
void *c_Output(void)
{
    return (stdout);
}

int c_Write(void *handle, void *buf, int len)
{
    int rc;
    rc = fwrite(buf, 1, len, handle);
    return (rc);
}

void *c_AllocMem(unsigned int amount, void *ptr)
{
    return (malloc(amount));
}

int callami2(unsigned int len, char *buf, void *sysbase, void *ep);

static int callami(char *buf)
{
    char *p;

    /* Amiga programs are only expecting to receive the parameters,
       not the program name. */
    p = strchr(buf, ' ');
    if (p == NULL)
    {
        p = buf;
    }
    else
    {
        p++;
    }

    /* seems we need to include NUL terminator in length */
    /* it could be a newline character that Amiga programs
       are expecting - but PDPCLIB-based programs tolerate
       either */
    return (callami2(strlen(p) + 1 + 0x80000000UL,
                     p,
                     &SysBase,
                     genstart));
}

#endif



#if SHIMCM32

/* x64 to CM32 shim support */

struct gdescriptor {
    unsigned char limit[2];
    unsigned char base[3];
    unsigned char access_byte;
    unsigned char limit_flags;
    unsigned char base2;
};

int get_cs (void);

void disable_interrupts (void);
void enable_interrupts (void);

void save_gdt (void *gdtr);
void load_gdt (void *gdt, int size);

void call_cm32 (int cm32_cs, void (*test32)(void));
void call_cm16 (int cm32_cs, int cm16_ss, void (*test16)(void));
void test32 (void);
void test16 (void);

static unsigned char gdtr[10];
static size_t original_gdt_size;
static void *original_gdt;
static size_t gdt_size;
static struct gdescriptor *gdt;
static int cs;
static int cm32_cs;

#ifdef CM16
static int cm16_ss;
static char *cm16_sbuf;
static unsigned long cm16_sul;
#endif



static void shimcm32_start(void)
{
    printf ("start\n");
    save_gdt (gdtr);
    original_gdt_size = (*(unsigned short *)gdtr) + 1;
    original_gdt = *(void **)(gdtr + 2);
    cs = get_cs ();
    printf ("gdt size: %u ptr: %p cs: %i\n", original_gdt_size, original_gdt, cs);

    gdt_size = original_gdt_size + sizeof (*gdt);
#ifdef CM16
    gdt_size += sizeof (*gdt);
#endif
    gdt = malloc (gdt_size);
    cm32_cs = original_gdt_size;
#ifdef CM16
    cm16_ss = original_gdt_size + sizeof (*gdt);
    /* enough room for stack to grow in either direction
       for now */
    cm16_sbuf = malloc(64*1024*4);
    cm16_sul = (unsigned long)cm16_sbuf;
    cm16_sul += 64*1024*2;
    cm16_sul &= ~0xffffUL;
#endif
    memcpy (gdt, original_gdt, original_gdt_size);
    gdt[cm32_cs / sizeof (*gdt)] = gdt[cs / sizeof (*gdt)];
    /* Converts the duplicated code segment descriptor to 32 bit code
     * by disabling long mode flag and setting size flag.
     */
    gdt[cm32_cs / sizeof (*gdt)].limit_flags &= ~0x20;

#ifdef CM16
    /* I am guessing this will set byte granularity */
    gdt[cm32_cs / sizeof (*gdt)].limit_flags &= 0xf0;
    /* I am guessing that this sets 0xffff as the maximum offset */
    gdt[cm32_cs / sizeof (*gdt)].limit[0] = 0xff;
    gdt[cm32_cs / sizeof (*gdt)].limit[1] = 0xff;
    /* I am hoping that this sets 05b1 0000 as the base address */
    gdt[cm32_cs / sizeof (*gdt)].base2 = 0x05;
    gdt[cm32_cs / sizeof (*gdt)].base[2] = 0xb1;
    gdt[cm32_cs / sizeof (*gdt)].base[1] = 0x0;
    gdt[cm32_cs / sizeof (*gdt)].base[0] = 0x0;

    gdt[cm16_ss / sizeof (*gdt)].base2 = (cm16_sul >> 24) & 0xff;
    gdt[cm16_ss / sizeof (*gdt)].base[2] = (cm16_sul >> 16) & 0xff;
    gdt[cm16_ss / sizeof (*gdt)].base[1] = 0x0;
    gdt[cm16_ss / sizeof (*gdt)].base[0] = 0x0;
    /* I am guessing that this sets 0xffff as the maximum offset */
    gdt[cm16_ss / sizeof (*gdt)].limit[0] = 0xff;
    gdt[cm16_ss / sizeof (*gdt)].limit[1] = 0xff;
    /* I am hoping this sets it to 16-bit stack */
    /* I need ERW set and also S */
    gdt[cm16_ss / sizeof (*gdt)].access_byte = 0x16;
    gdt[cm16_ss / sizeof (*gdt)].limit_flags = 0;
#else
    gdt[cm32_cs / sizeof (*gdt)].limit_flags |= 0x40;
#endif

    disable_interrupts ();
    load_gdt (gdt, gdt_size);
    enable_interrupts ();

    printf ("running with new gdt\n");
}

static void shimcm32_run(void)
{
    printf ("trying cm32 (cm32_cs: %i)\n", cm32_cs);
    printf("test32 is at %p\n", test32);
    printf("test16 is at %p\n", test16);
#ifdef CM16
    printf("this will only succeed if the test16 address is 05B1 xxxx\n");
    printf("note that this is not a real mode address - it is basically flat\n");
    call_cm16 (cm32_cs, cm16_ss, ((unsigned int)&test16 & 0xffffU));
#else
    call_cm32 (cm32_cs, &test32);
#endif
    printf ("success\n");
}

static void shimcm32_end(void)
{
    printf ("restoring old gdt\n");
    disable_interrupts ();
    load_gdt (original_gdt, original_gdt_size);
    enable_interrupts ();

    free (gdt);
    
    printf ("end, press enter\n");
    getc (stdin);
}

#endif
