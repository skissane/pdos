/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  start.c - startup/termination code                               */
/*                                                                   */
/*********************************************************************/

/* We have the following independent concepts:

1. What processor is used, e.g. __ARM__ or __386__ or __8086__
This is important to setjmp as it needs an appropriate sized
buffer for the registers that need to be saved. Note that the
8086 has different memory models, and in e.g. tiny and small
memory models, the segment registers shouldn't be saved/
restored, as the OS could theoretically use this knowledge to
move the application to some other location. Although the
executable would need to be specially marked that it can be
dynamically relocated, as otherwise it could directly obtain
memory from the OS at runtime and change the segment registers
(via far pointers). The important thing is that the concept exists.

2. What compiler is used, e.g. __GCC__ (and version number within that,
and forks within that, and targets within that - not just processor
as the target but also the calling convention).

3. Whether the OS exports the C library (PDOS-generic does, PDOS/386 doesn't)

4. What API is used, e.g. Pos* vs Win32 vs Posix. Note that it is
possible to get one API to be implemented via translation into another
API.

5. The size of ints, e.g. __32BIT__ vs __16BIT__

6. The API (e.g. Pos) versus any operating system that actually
implements that API (e.g. PDOS/386). Or Win32 API implemented by
PDOS/386.

7. Whether the OS itself relies on a generic pseudo-BIOS/HAL

8. Whether the OS/BIOS provides data in blocks (e.g. MVS or a PC BIOS) or
whether you can read/write an arbitrary number of bytes.

9. The fact that the C library, including PDPCLIB, reconciles block
versus character mode if necessary, and the application always gets
a character mode interface.

10. Whether the OS does the conversion of blocks (ie blocks on a
mainframe CKD disk, or sectors on a PC) into characters within the
OS itself, or outsources that job to the C library that it relies on.
Currently PDOS/386 does Bos* calls in the OS itself to read disk
sectors, but PDPCLIB could have done those Bos* calls itself, in the
same flavor of code that MVS/CMS/VSE use, which would likely have
been neater, and a better abstraction. PDOS/386 may be restructured
to do this in the future, but it may simply become PDOS-generic if
that is done. That's still unclear to me. Something like pcomm
calling PosMonitor() to implement the "monitor" command is something
beyond what can be considered generic, ie dealing with 80386 registers,
and might possibly need to be shifted up to a higher layer (the HAL/
pseudo-BIOS) - but that HAL needs to do printfs which are back in
the OS to do, I think. You can't expect a BIOS to provide a full C
library, even though that is the case when running under some
other OS.

Historically these concepts have all been jumbled up in PDOS/PDPCLIB
and never been reconciled. Which is why there are anomalies such as
a PDOS-generic application for ARM is built needing PDOS386 defined,
when it has nothing whatsoever to do with an 80386. It works, but is
confusing and not properly abstracted/defined.

So let's try to reconcile at least some of them now.

And apologies for Pos* being a bit confusing since the name is similar
to POSIX, but they are completely unrelated. Pos* was actually inspired
by Dos* from OS/2 which I thought was a neat naming convention for
the MSDOS INT 21H functions, even though I hated the actual parameter types.

__POSGEN__ - this define means that both the C library and the Pos*
interface have been exported by the operating system (not necessarily
PDOS-generic) via the well-defined, set in stone (ha ha) __os structure.
Applications can define this. It's not for use when compiling the
actual OS. Or should this be split up into __POS__ and __GENERIC__?
Or __POS__ to specify the API and __CEXPORT__ to say the C library has
been exported (which could be done independently of exporting the
rest of the Pos* API). Note that there is a difference between using
the C library and building the C library.

__POSGENOS__ - I give up.

*/


/* New requirement.

   When running the PDOS-generic OS, we want to run both PDOS-generic
   apps and Win 32/64 apps.

   PDOS-generic apps need the OS structure as a parameter on entry
   and will call __start themselves, as one of the callback functions.

   Win 32/64 apps don't need any parameters, and they also won't call
   __start, so file handles aren't marked for auto-closure.

   To cope with this, the new proposal is a WINNEWMOD to define a new
   behavior. exeload will signal whether msvcrt.dll was invoked. And
   then PDOS-generic will set the genmain itself and call __start itself.
   This will lead to mainCRTStartup in w32start.c being called with
   argc and argv, but that is harmless. We could have another flag to
   avoid that if required for some reason. __start will need to mark
   argc and argv as already computed, so that when mainCRTStartup
   calls getmainargs, it can return quickly because the work has
   already been done. The w32exit/getmainargs hacks can probably be
   removed from both bios.c and exeload.c after this new model has
   been implemented.
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stddef.h"
#include "setjmp.h"

#if USE_MEMMGR
#include "__memmgr.h"
#endif

#if defined(__ARM__) && !defined(__EFI__) && !defined(__WIN32__)
#define __gnu_linux__ 1
#endif

extern FILE *__userFiles[__NFILE];

extern void (*__userExit[__NATEXIT])(void);

#define MAXPARMS 50 /* maximum number of arguments we can handle */

#ifdef __OS2__
#ifdef __16BIT__
#undef NULL
#endif
#define INCL_DOS
#include <os2.h>
#endif

#ifdef __ZPDOSGPB__
extern int __consdn;
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

#ifdef __ATARI__
#include <__os.h>
extern long (*__trap1)(short cnt, void *s);
OS *__os;
#endif

int __runnum = 0;

int __G_live = 0;
void *__G_ptr = NULL;
int __G_one = 1;
int __G_zero = 0;

#ifdef __gnu_linux__

struct termios {
    unsigned int c_iflag;
    unsigned int c_oflag;
    unsigned int c_cflag;
    unsigned int c_lflag;
    unsigned int whoknows[50];
};

static struct termios tios_save;
static struct termios tios_new;

extern int __ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
extern int __open(const char *a, int b, int c);
extern int __read(int a, void *b, int c);
extern int __seek(int handle, long offset, int whence);
extern void __close(int handle);
extern int __getpid(void);

#define TCGETS 0x00005401U
#define TCSETS 0x00005402U

#define ISIG 0x1
#define ICANON 0x2
#define ECHO 0x8
#define ECHONL 0x40

#define IXON 0x400

#endif


#if defined(__WATCOMC__) && !defined(WATLIN)
#define CTYP __cdecl
#else
#define CTYP
#endif

#ifdef __SMALLERC__
#define __MSDOS__
#endif

#if defined(__PDOS386__)
/* Used for PDOS itself to avoid API calls when starting. */
int __minstart = 0;
#endif

/* will be set by __exit */
static int globrc = 0;

static jmp_buf jb;

#ifndef __PDPCLIB_DLL

/* Unfortunately Watcom requires us to make main() a watcall,
   even if everything else is compiled with -ecc */
#if defined(__WATCOMC__)
#define MAINTYP __watcall
#else
#define MAINTYP
#endif

int MAINTYP main(int argc, char **argv);

int __genstart = 0;
/* generic main as defined in pgastart does not use watcall */
int /* MAINTYP */ (*__genmain)(int argc, char **argv); /* = main; */

#endif

#ifdef __MSDOS__
/* Must be unsigned as it is used for array index */
#ifndef __SMALLERC__
extern
#endif
char *__envptr;
#ifndef __SMALLERC__
extern
#endif
unsigned short __osver;
static unsigned char *newcmdline = NULL;
#endif

#ifdef __VSE__
#undef __CMS__
#undef __MVS__
#endif

#if defined(__MVS__) || defined(__CMS__) || defined(__VSE__)
int __tso = 0; /* is this a TSO environment? */
extern int __doperm; /* are we doing the permanent datasets? */
int __upsi = 0; /* UPSI switches for VSE */
#endif

void __exit(int status);
void CTYP __exita(int status);

#if !defined(__MVS__) && !defined(__CMS__) && !defined(__VSE__)
static char buffer1[BUFSIZ + 8];
static char buffer2[BUFSIZ + 8];
static char buffer3[BUFSIZ + 8];
#endif

#if defined(__AMIGA__)

#include <clib/dos_protos.h>

struct Node {
    struct Node *ln_Succ;
    char filler[6];
    char *ln_Name;
};

struct Library {
    struct Node lib_Node;
    /* other junk */
};

struct List {
    struct Node *lh_Head;
    char filler[10];
};

struct ExecBase {
    char filler[378];
    struct List LibList;
};

struct ExecBase *SysBase;
void *DOSBase;
#endif

#if defined(__EFI__)
#include "efi.h"
extern EFI_FILE_PROTOCOL *__EfiRoot;
#endif

#if defined(__PDOS386__)
#include <support.h>
#include <pos.h>
char *__envptr;
static unsigned int stdin_dw;
#endif

#if defined(__MSDOS__)
extern int CTYP __devginfo(int handle, unsigned int *info);
extern void CTYP __devsinfo(int handle, unsigned int info);
static unsigned int stdin_dw;
#endif

#if USE_MEMMGR
extern void *__lastsup; /* last thing supplied to memmgr */
#endif

char **__eplist;
char *__plist;


#ifdef WINNEWMOD
static int G_argc;
static char **G_argv;
static char *G_env[] = {NULL};

/* not sure what this type is */
typedef int _startupinfo;

__PDPCLIB_API__ int __getmainargs(int *_Argc,
                                  char ***_Argv,
                                  char ***_Env,
                                  int _DoWildCard,
                                  _startupinfo *_StartInfo)
{
    *_Argc = G_argc;
    *_Argv = G_argv;
    *_Env = G_env;
    return (0);
}
#endif

#ifdef __WIN32__
static DWORD stdin_dw;
static DWORD stdout_dw;

/* Not sure what _startupinfo is. */
typedef int _startupinfo;

__PDPCLIB_API__ int __getmainargs(int *_Argc,
                                  char ***_Argv,
                                  char ***_Env,
                                  int _DoWildCard,
                                  _startupinfo *_StartInfo)
{
    char *p;
    int x;
    int argc;
    static char *argv[MAXPARMS + 1];
    static char *env[] = {NULL};

    p = GetCommandLine();

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
    *_Env = env;
    return (0);
}
#endif

#if defined(__CMS__)
int __start(char *plist, char *pgmname, char **eplist)
#elif defined(__VSE__)
int __start(char *p, char *pgmname, char *ep)
#elif defined(__MVS__)
int __start(char *p, char *pgmname, int tso)
#elif defined(__EFI__) || defined(__MACOS__)
int __start(int argc, char **argv)
#elif defined(__AMIGA__)
int __start(unsigned long cmdlen, char *p, void *pdosbase)
#elif defined(__ATARI__)
int __start(void *sp, void *a0, void *a1)
#elif defined(__SMALLERC__)
__PDPCLIB_API__ int CTYP __start(char *p, unsigned short osver)
#else
__PDPCLIB_API__ int CTYP __start(char *p)
#endif
{
#if defined(__CMS__)
    char *p;
#endif
    int x;
    int oldglobrc = globrc;
    jmp_buf oldjb;
    static char *int_argv[MAXPARMS + 1];
#if !defined(__EFI__) && !defined(__MACOS__)
    int argc;
    char **argv = int_argv;
#endif
#if defined(__EFI__) || defined(__MACOS__) || defined(__ATARI__)
    char *p;
#endif
    int rc;
#ifdef __AMIGA__
    struct Library *library;
#endif
#ifdef __OS2__
    ULONG maxFH;
    LONG reqFH;
#endif
#ifdef __MSDOS__
    unsigned char *env;
    unsigned char *nclsave;
#endif
#if defined(__MVS__) || defined(__CMS__) || defined(__VSE__)
    int parmLen;
    int progLen;
    char parmbuf[310]; /* z/VSE can have a PARM up to 300 characters */
#endif
#if defined(__gnu_linux__)
    char parmbuf[410];
#endif
#ifdef __ZPDOSGPB__
    char parmbuf[65];
#endif
#ifdef __ATARI__
    unsigned char *basepage;
#endif

#ifdef __ATARI__
    if (a0 == NULL)
    {
        basepage = *((void **)sp + 1);
    }
    else if (a0 == (void *)0xffffffffUL)
    {
        basepage = *((void **)sp + 1);
        __os = a1;
        __trap1 = __os->Xtrap1;
    }
    else
    {
        basepage = a0;
    }
    p = basepage + 128 + 1; /* I assume Atari NUL-terminates? */
#endif

#if !defined(__MVS__) && !defined(__CMS__) && !defined(__VSE__)

    __runnum++;
    memcpy(&oldjb, &jb, sizeof oldjb);


#ifdef __ZPDOSGPB__
    memcpy(parmbuf, (char *)0x280, 64);
    parmbuf[64] = '\0';
    p = parmbuf;
    argv[0] = "UNKNOWN";
#endif

#ifdef __AMIGA__
    if (cmdlen >= 0x80000000UL)
    {
        cmdlen -= 0x80000000UL;
        SysBase = (struct ExecBase *)pdosbase;
    }
    else
    {
        SysBase = *(struct ExecBase **)4;
    }
    if (cmdlen > 0)
    {
        cmdlen--;
    }
    p[(size_t)cmdlen] = '\0';
    /* structures to retrieve argv[0] are mentioned here:
       https://www.amigans.net/modules/newbb/viewtopic.php?
       post_id=148641#forumpost148641 */
    argv[0] = "UNKNOWN";
    DOSBase = NULL;
    library = (struct Library *)SysBase->LibList.lh_Head;
    while (library != NULL)
    {
        if (strcmp(library->lib_Node.ln_Name, "dos.library") == 0)
        {
            DOSBase = library;
            break;
        }
        library = (struct Library *)library->lib_Node.ln_Succ;
    }
    if (DOSBase == NULL)
    {
        return (-1);
    }
#endif

#ifdef __WIN32__
    if (__runnum == 1)
    {
    __stdin->hfile = GetStdHandle(STD_INPUT_HANDLE);
    {
        DWORD dw;

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif
        if (GetConsoleMode(__stdin->hfile, &stdin_dw))
        {
            dw = stdin_dw;
            dw |= ENABLE_VIRTUAL_TERMINAL_INPUT;
            SetConsoleMode(__stdin->hfile, dw);
        }
    }
    __stdout->hfile = GetStdHandle(STD_OUTPUT_HANDLE);
    {
        DWORD dw;

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
        if (GetConsoleMode(__stdout->hfile, &stdout_dw))
        {
            dw = stdout_dw;
            dw |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            dw |= ENABLE_PROCESSED_OUTPUT;
            SetConsoleMode(__stdout->hfile, dw);
        }
    }
    __stderr->hfile = GetStdHandle(STD_ERROR_HANDLE);
    }
#elif defined(__AMIGA__)
    __stdin->hfile = Input();
    __stdout->hfile = Output();
    __stderr->hfile = Output();
#elif !defined(__EFI__) /* EFI will leave everything as NULL */
    __stdin->hfile = 0;
    __stdout->hfile = 1;
    __stderr->hfile = 2;
#endif

#if defined(__PDOS386__)
    PosGetDeviceInformation(0, &stdin_dw);
    stdin_dw &= 0xff;
#endif
#if defined(__MSDOS__)
    __devginfo(0, &stdin_dw);
    stdin_dw &= 0xff;
#endif

#if defined(__gnu_linux__)
    if (__runnum == 1)
    {
    /* I believe the reason we switch off echo etc and
       do our own processing is so that when the command
       processor (which needs echo off) launches another
       application, we don't need to put it back to the
       echo state before doing so. */
    __ioctl(0, TCGETS, (unsigned long)&tios_save);
    tios_new = tios_save;
    tios_new.c_iflag &= ~IXON;
    tios_new.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG);
    __ioctl(0, TCSETS, (unsigned long)&tios_new);
    }
#endif

    if (__runnum == 1)
    {
    __stdin->quickBin = 0;
    __stdin->quickText = 0;
    __stdin->textMode = 1;
    __stdin->intFno = 0;
    __stdin->bufStartR = 0;
    __stdin->justseeked = 0;
    __stdin->bufTech = _IOLBF;
    __stdin->intBuffer = buffer1;
    __stdin->fbuf = __stdin->intBuffer + 2;
#ifdef __CC64__
    *__stdin->fbuf = '\0';
    __stdin->fbuf++;
    *__stdin->fbuf = '\0';
    __stdin->fbuf++;
#else
    *__stdin->fbuf++ = '\0';
    *__stdin->fbuf++ = '\0';
#endif
    __stdin->szfbuf = BUFSIZ;
    __stdin->endbuf = __stdin->fbuf + __stdin->szfbuf;
    *__stdin->endbuf = '\n';
    __stdin->noNl = 0;
    __stdin->upto = __stdin->endbuf;
    __stdin->bufStartR = -__stdin->szfbuf;
    __stdin->mode = __READ_MODE;
    __stdin->ungetCh = -1;
    __stdin->update = 0;
    __stdin->theirBuffer = 0;
    __stdin->permfile = 1;
    __stdin->isopen = 1;
#ifdef __OS2__
    __stdin->kbdfile = 0;
#endif

    __stdout->quickBin = 0;
    __stdout->quickText = 0;
    __stdout->textMode = 1;
    __stdout->bufTech = _IOLBF;
    __stdout->intBuffer = buffer2;
    __stdout->fbuf = __stdout->intBuffer;
#ifdef __CC64__
    *__stdout->fbuf = '\0';
    __stdout->fbuf++;
    *__stdout->fbuf = '\0';
    __stdout->fbuf++;
#else
    *__stdout->fbuf++ = '\0';
    *__stdout->fbuf++ = '\0';
#endif
    __stdout->szfbuf = BUFSIZ;
    __stdout->endbuf = __stdout->fbuf + __stdout->szfbuf;
    *__stdout->endbuf = '\n';
    __stdout->noNl = 0;
    __stdout->upto = __stdout->fbuf;
    __stdout->bufStartR = 0;
    __stdout->justseeked = 0;
    __stdout->mode = __WRITE_MODE;
    __stdout->update = 0;
    __stdout->theirBuffer = 0;
    __stdout->permfile = 1;
    __stdout->isopen = 1;

    __stderr->quickBin = 0;
    __stderr->quickText = 0;
    __stderr->textMode = 1;
    __stderr->bufTech = _IOLBF;
    __stderr->intBuffer = buffer3;
    __stderr->fbuf = __stderr->intBuffer;
#ifdef __CC64__
    *__stderr->fbuf = '\0';
    __stderr->fbuf++;
    *__stderr->fbuf = '\0';
    __stderr->fbuf++;
#else
    *__stderr->fbuf++ = '\0';
    *__stderr->fbuf++ = '\0';
#endif
    __stderr->szfbuf = BUFSIZ;
    __stderr->endbuf = __stderr->fbuf + __stderr->szfbuf;
    *__stderr->endbuf = '\n';
    __stderr->noNl = 0;
    __stderr->upto = __stderr->fbuf;
    __stderr->bufStartR = 0;
    __stderr->justseeked = 0;
    __stderr->mode = __WRITE_MODE;
    __stderr->update = 0;
    __stderr->theirBuffer = 0;
    __stderr->permfile = 1;
    __stderr->isopen = 1;

#if USE_MEMMGR
    memmgrDefaults(&__memmgr);
    memmgrInit(&__memmgr);
#endif

#ifdef __ZPDOSGPB__
    if (__consdn == 0)
    {
        __consdn = 0x10000;
    }
    else if (__consdn < 0x10000)
    {
        __consdn = __getssid(__consdn);
    }
#if USE_MEMMGR
    memmgrSupply(&__memmgr, (void *)0x04100000, 100 * 1024 * 1024);
#endif
    __map4gb();
#endif

    }

#else
    int dyna_sysprint = 0;
    int dyna_systerm = 0;
    int dyna_sysin = 0;
#if defined(__CMS__)
/*
  This code checks to see if DDs exist for SYSIN, SYSPRINT & SYSTERM
  if not it issues FD to the terminal
*/
    char s202parm [800]; /* svc 202 buffer */
    int code;
    int parm;
    int ret;
    int have_sysparm;

    __runnum++;
    memcpy(&oldjb, &jb, sizeof oldjb);
/*
 Now build the SVC 202 string for sysprint
*/
    memcpy ( &s202parm[0] ,  "FILEDEF ", 8);
    memcpy ( &s202parm[8] ,  "SYSPRINT", 8);
    memcpy ( &s202parm[16] , "(       ", 8);
    memcpy ( &s202parm[24] , "NOCHANGE", 8);
    s202parm[32]=s202parm[33]=s202parm[34]=s202parm[35]=
        s202parm[36]=s202parm[37]=s202parm[38]=s202parm[39]=0xff;
/*
  and issue the SVC
*/
    ret = __SVC202 ( s202parm, &code, &parm );
    if (ret == 24)
    { /* we need to issue filedef */
        memcpy ( &s202parm[16] , "TERM    ", 8);
        memcpy ( &s202parm[24] , "(       ", 8);
        memcpy ( &s202parm[32] , "LRECL   ", 8);
        memcpy ( &s202parm[40] , "80      ", 8);
        memcpy ( &s202parm[48] , "RECFM   ", 8);
        memcpy ( &s202parm[56] , "F       ", 8);
        s202parm[64]=s202parm[65]=s202parm[66]=s202parm[67]=
            s202parm[68]=s202parm[69]=s202parm[70]=s202parm[71]=0xff;

        ret = __SVC202 ( s202parm, &code, &parm );
        dyna_sysprint = 1;
    }

/*
 Now build the SVC 202 string for systerm
*/
    memcpy ( &s202parm[0] ,  "FILEDEF ", 8);
    memcpy ( &s202parm[8] ,  "SYSTERM ", 8);
    memcpy ( &s202parm[16] , "(       ", 8);
    memcpy ( &s202parm[24] , "NOCHANGE", 8);
    s202parm[32]=s202parm[33]=s202parm[34]=s202parm[35]=
        s202parm[36]=s202parm[37]=s202parm[38]=s202parm[39]=0xff;
/*
  and issue the SVC
*/
    ret = __SVC202 ( s202parm, &code, &parm );
    if (ret == 24)
    { /* we need to issue filedef */
        memcpy ( &s202parm[16] , "TERM    ", 8);
        memcpy ( &s202parm[24] , "(       ", 8);
        memcpy ( &s202parm[32] , "LRECL   ", 8);
        memcpy ( &s202parm[40] , "80      ", 8);
        memcpy ( &s202parm[48] , "RECFM   ", 8);
        memcpy ( &s202parm[56] , "F       ", 8);
        s202parm[64]=s202parm[65]=s202parm[66]=s202parm[67]=
            s202parm[68]=s202parm[69]=s202parm[70]=s202parm[71]=0xff;

        ret = __SVC202 ( s202parm, &code, &parm );
        dyna_systerm = 1;
    }

/*
 Now build the SVC 202 string for sysin
*/
    memcpy ( &s202parm[0] ,  "FILEDEF ", 8);
    memcpy ( &s202parm[8] ,  "SYSIN   ", 8);
    memcpy ( &s202parm[16] , "(       ", 8);
    memcpy ( &s202parm[24] , "NOCHANGE", 8);
    s202parm[32]=s202parm[33]=s202parm[34]=s202parm[35]=
        s202parm[36]=s202parm[37]=s202parm[38]=s202parm[39]=0xff;
/*
  and issue the SVC
*/
    ret = __SVC202 ( s202parm, &code, &parm );

    if (ret == 24)
    { /* we need to issue filedef */
        memcpy ( &s202parm[16] , "TERM    ", 8);
        memcpy ( &s202parm[24] , "(       ", 8);
        memcpy ( &s202parm[32] , "LRECL   ", 8);
        memcpy ( &s202parm[40] , "80      ", 8);
        memcpy ( &s202parm[48] , "RECFM   ", 8);
        memcpy ( &s202parm[56] , "F       ", 8);
        s202parm[64]=s202parm[65]=s202parm[66]=s202parm[67]=
            s202parm[68]=s202parm[69]=s202parm[70]=s202parm[71]=0xff;

        ret = __SVC202 ( s202parm, &code, &parm );
        dyna_sysin = 1;
    }

#else /* not CMS */
    __runnum++;
    memcpy(&oldjb, &jb, sizeof oldjb);
#endif

#if USE_MEMMGR
    memmgrDefaults(&__memmgr);
    memmgrInit(&__memmgr);
#endif
#if 0 /* MUSIC */
    /* switch on lowercasing of input */
    /* normal MUSIC default is to uppercase, and it's probably
       better to let the user control that with the /TEXT LC
       command instead */
    __textlc();
#endif
#if defined(__MVS__)
    /* need to know if this is a TSO environment straight away
       because it determines how the permanent files will be
       opened */
    parmLen = ((unsigned int)p[0] << 8) | (unsigned int)p[1];
#if 1 /* traditional way of checking to see if it is TSO */
    if ((parmLen > 0) && (p[2] == 0))     /* assume TSO */
    {
        __tso = 1;
    }
#else
    __tso = (tso != 0); /* even "CALL" is considered to be TSO */
#endif

#endif /* MVS */
    if (__runnum == 1)
    {
    __doperm = 1;
    __stdout = fopen("dd:SYSPRINT", "w");
    if (__stdout == NULL)
    {
        __exita(EXIT_FAILURE);
    }
    __stdout->dynal = dyna_sysprint;

    __stderr = fopen("dd:SYSTERM", "w");
    if (__stderr == NULL)
    {
        printf("SYSTERM DD not defined\n");
        fclose(__stdout);
        __exita(EXIT_FAILURE);
    }
    __stderr->dynal = dyna_systerm;

    __stdin = fopen("dd:SYSIN", "r");
    if (__stdin == NULL)
    {
        fprintf(__stderr, "SYSIN DD not defined\n");
        fclose(__stdout);
        fclose(__stderr);
        __exita(EXIT_FAILURE);
    }
    __stdin->dynal = dyna_sysin;
    __doperm = 0;
    }
#if defined(__CMS__)
    __eplist = eplist;
    __plist = plist;

    if (plist[0] == '\xff')  /* are we at the fence already? */
    {
        p = plist;   /* yes, this is also the start of the plist */
    }
    else
    {
        p = plist + 8; /* no, so jump past the command name */
    }

    /* Now build the SVC 202 string for sysparm */
    memcpy ( &s202parm[0] ,  "FILEDEF ", 8);
    memcpy ( &s202parm[8] ,  "SYSPARM ", 8);
    memcpy ( &s202parm[16] , "(       ", 8);
    memcpy ( &s202parm[24] , "NOCHANGE", 8);
    s202parm[32]=s202parm[33]=s202parm[34]=s202parm[35]=
        s202parm[36]=s202parm[37]=s202parm[38]=s202parm[39]=0xff;
    /* and issue the SVC */
    ret = __SVC202 ( s202parm, &code, &parm );

    have_sysparm = (ret != 24);


    /* if no parameters are provided, the tokenized
       plist will start with x'ff'. However, if they
       have provided a SYSPARM, then we'll use that
       as the parameter. But only if they haven't
       provided any parameters! If they have provided
       parameters then we instead lowercase everything
       and go to special processing (useful when in
       an EXEC with CONTROL MSG etc). */

    /* No parameters */
    if (p[0] == 0xff)
    {
        parmLen = 0;

        if (have_sysparm)
        {
            FILE *pf;

            /* have a parameter file - let's use it */
            pf = fopen("dd:SYSPARM", "r");
            if (pf != NULL)
            {
                fgets(parmbuf + 2, sizeof parmbuf - 2, pf);
                fclose(pf);
                p = strchr(parmbuf + 2, '\n');
                if (p != NULL)
                {
                    *p = '\0';
                }
                parmLen = strlen(parmbuf + 2);
            }
        }
    }
    /* If there is no EPLIST, or there is a SYSPARM so
       they are invoking special processing, then we
       will be using the PLIST only. */
    else if ((eplist == NULL) || have_sysparm)
    {
        /* copy across the tokenized plist, which
           consists of 8 character chunks, space-padded,
           and terminated by x'ff'. Note that the first
           2 characters of parmbuf are reserved for an
           (unused) length, so we must skip them */
        for (x = 0; x < sizeof parmbuf / 9 - 1; x++)
        {
            if (p[x * 8] == 0xff) break;
            memcpy(parmbuf + 2 + x * 9, p + x * 8, 8);
            parmbuf[2 + x * 9 + 8] = ' ';
        }
        parmbuf[2 + x * 9] = '\0';
        parmLen = strlen(parmbuf + 2);

        /* even though we have a SYSPARM, we don't use it,
           we just use it as a signal to do some serious
           underscore searching! */
        if (have_sysparm)
        {
            char *q;
            char *r;
            char *lock;
            int cnt = 0;
            int c;
            int shift = 0;
            int rev = 0; /* reverse logic */

            q = parmbuf + 2;
            r = q;
            lock = q;

            /* reverse the case switching when _+ is specified
               as the first parameter */
            if (memcmp(r, "_+", 2) == 0)
            {
                rev = 1;
                cnt += 2;
                r += 2;
            }
            while (*r != '\0')
            {
                cnt++;
                if (rev)
                {
                    c = toupper((unsigned char)*r);
                }
                else
                {
                    c = tolower((unsigned char)*r);
                }
                if (shift && (c != ' '))
                {
                    if (rev)
                    {
                        c = tolower((unsigned char)*r);
                    }
                    else
                    {
                        c = toupper((unsigned char)*r);
                    }
                    shift = 0;
                }
                if (c == '_')
                {
                    shift = 1;
                }
                /* if we've reached the inter-parameter space, then
                   collapse it - a space requires a shift */
                else if (cnt == 9)
                {
                    while (q > lock)
                    {
                        q--;
                        if (*q != ' ')
                        {
                            q++;
                            lock = q;
                            break;
                        }
                    }
                    cnt = 0;
                    if (shift)
                    {
                        *q++ = ' ';
                        shift = 0;
                    }
                }
                else if (c != ' ')
                {
                    *q++ = c;
                }
                r++;
            }
            *q = '\0';
            parmLen = strlen(parmbuf + 2);
        }
    }
    /* else, we have an eplist, and no sysparm, so use that */
    else
    {
        parmLen = eplist[2] - eplist[1];
        /* 2 bytes reserved for an unused length, 1 byte for NUL */
        if (parmLen >= sizeof parmbuf - 2)
        {
            parmLen = sizeof parmbuf - 1 - 2;
        }
        memcpy(parmbuf + 2, eplist[1], parmLen);
    }
#elif defined(__VSE__)
    __upsi = pgmname[9]; /* we shouldn't really clump this */

    if (ep != NULL)
    {
        ep = *(char **)ep;
    }
    /* The ep only has a genuine value if the top bit is set */
    if (((unsigned int)ep & 0x80000000) != 0)
    {
        /* it is a 24-bit address */
        ep = (char *)((unsigned int)ep & 0x00ffffff);
        parmLen = *(short *)ep;
        memcpy(parmbuf + 2, ep + 2, parmLen);
    }
    /* if no parm, use SYSPARM instead */
    else if (p[0] != 0)
    {
        /* in the special case of a "?", inspect the UPSI switches */
        if ((p[0] == 1) && (p[1] == '?'))
        {
            /* user is required to set all switches to 0. All
               are reserved, except for the first one, which
               says that the parameter will be read from SYSINPT */
            if (__upsi & 0x80)
            {
                fgets(parmbuf + 2, sizeof parmbuf - 2, __stdin);
                p = strchr(parmbuf + 2, '\n');
                if (p != NULL)
                {
                    *p = '\0';
                }
                parmLen = strlen(parmbuf + 2);
            }
            else
            {
                parmLen = 0;
            }
        }
        /* for all other parameter values, just use as-is */
        else
        {
            parmLen = p[0];
            memcpy(parmbuf + 2, p + 1, parmLen);
        }

    }
    /* otherwise there is no parm */
    else
    {
        parmLen = 0;
    }
#else /* MVS etc */
    parmLen = ((unsigned int)p[0] << 8) | (unsigned int)p[1];
    if (parmLen >= sizeof parmbuf - 2)
    {
        parmLen = sizeof parmbuf - 1 - 2;
    }
    /* We copy the parameter into our own area because
       the caller hasn't necessarily allocated room for
       a terminating NUL, nor is it necessarily correct
       to clobber the caller's area with NULs. */
    memcpy(parmbuf, p, parmLen + 2);
#endif
    p = parmbuf;
#ifdef __MVS__
    if (__tso)
#else
    if (0)
#endif
    {
        progLen = ((unsigned int)p[2] << 8) | (unsigned int)p[3];
        parmLen -= (progLen + 4);
        argv[0] = p + 4;
        p += (progLen + 4);
        if (parmLen > 0)
        {
            *(p - 1) = '\0';
        }
        else
        {
            *p = '\0';
        }
        p[parmLen] = '\0';
    }
    else         /* batch or tso "call" */
    {
        progLen = 0;
        p += 2;
        argv[0] = pgmname;
        pgmname[8] = '\0';
        pgmname = strchr(pgmname, ' ');
        if (pgmname != NULL)
        {
            *pgmname = '\0';
        }
        if (parmLen > 0)
        {
            p[parmLen] = '\0';
        }
        else
        {
            p = "";
        }
    }
#endif /* defined(__MVS__) || defined(__CMS__) || defined(__VSE__) */

    for (x=0; x < __NFILE; x++)
    {
        __userFiles[x] = NULL;
    }

#ifdef __PDPCLIB_DLL
    return (0);
#endif

#if defined(__PDOS386__)
    /* PDOS-32 uses an API call returning the full command line string. */
    if (!__minstart)
    {
        p = PosGetCommandLine();
        __envptr = PosGetEnvBlock();
    }
    else
    {
        /* PDOS itself is starting so no API calls should be used. */
        p = "";
        __envptr = NULL;
    }
#endif

/* override argc/argv (which depend on a wacky stack which PDOS/386
   does not provide) with what can be obtained from /proc, so that
   "properly-built" Linux executables work under PDOS/386. The
   "file" has fields NUL-separated already, with the length as the
   terminator */

/* Note that this doesn't seem to work for MacOS, so we use the
   argc/argv provided on entry instead */

#if defined(__gnu_linux__) && !defined(__MACOS__)
    if (__runnum == 1)
    {
        char fnm[FILENAME_MAX];
        int pf;
        int tot;

        sprintf(fnm, "/proc/%d/cmdline", __getpid());
        pf = __open(fnm, 0, 0);
        /* note that the open syscall can return numbers other than
           -1, e.g. -2, but most documentation is for open() which
           is a wrapper that will return -1 and set errno based on
           which negative value was returned */
        if (pf < 0)
        {
            /* could be early Linux userspace or chroot() jail - let's
               take our chances with the stack */
#if !defined(LOGSHELL) /* we don't have parameters yet */
            argc = *(int *)p;
            p += sizeof(char *); /* I think next parm will be on pointer
                                    boundary - especially 64-bit */
            argv = (char **)p;
#else
            argc = 1;
            argv[0] = "";
#endif
        }
        else
        {
            tot = __read(pf, parmbuf, sizeof parmbuf - 1);
            __close(pf);
            parmbuf[sizeof parmbuf - 1] = '\0';
            p = parmbuf;
            argc = 1;
            argv[0] = p;
            p += strlen(p) + 1;
            while (p < parmbuf + tot)
            {
                argv[argc] = p;
                p += strlen(p) + 1;
                argc++;
            }
        }
    }
    else
    {
        /* we are expecting PDOS-generic format here
           rather than Linux */
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
    }
#endif

#ifdef __WIN32__
    if (__runnum == 1)
    {
        p = GetCommandLine();
    }

#endif

#if defined(__EFI__)
    if (__runnum != 1)
    {
        /* we are expecting PDOS-generic format here
           rather than EFI */
        /* Note that we shouldn't be extracting the pointer from argc
           like this. We can use a global variable or standardize on
           the interface to start */
        argv = int_argv;
        p = (char *)argc;
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
    }
#endif

#if defined(__OS2__) && !defined(__16BIT__)
    reqFH = 0;
    DosSetRelMaxFH(&reqFH, &maxFH);
    if (maxFH < (FOPEN_MAX + 10))
    {
        reqFH = FOPEN_MAX - maxFH + 10;
        DosSetRelMaxFH(&reqFH, &maxFH);
    }
#endif
#ifdef __OS2__
    if (__runnum == 1)
    {
#ifdef __16BIT__
        {
            USHORT seg, offs;

            if (DosGetEnv(&seg, &offs) != 0)
            {
                p = "\0"; /* two NUL characters */
            }
            else
            {
                p = (char *)(((unsigned long)seg << 16)
                             | offs);
            }
        }
#else
        {
            PIB *pib;

            if (DosGetInfoBlocks(NULL, &pib) != 0)
            {
                p = "\0"; /* two NUL characters */
            }
            else
            {
                /* skip three 32-bit values and then you get a
                   pointer to the command line */
                p = pib->pib_pchcmd;
            }
        }
#endif
        argv[0] = p;
        p += strlen(p) + 1;
    }
    else
    {
        /* we are exporting PDOS-generic format here
           rather than OS/2 */
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
    }
#endif
#if defined(__WIN32__) || defined(__PDOS386__) || defined(__ATARI__)
    /* Windows and PDOS-32 get the full command line string. */
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
#elif defined(__MSDOS__)
    argv[0] = "";

#ifdef __SMALLERC__
    __envptr = (char *)
               ((unsigned long)(*(unsigned short *)(p + 0x2c)) << 4);
    __osver = osver;
#endif


    if(__osver > 0x300)
    {
        env=__envptr;
        if (env != NULL)
        {
            while (1)
            {
                if (*env++ == '\0' && *env++ == '\0')
                {
                    if (*(unsigned short *)env != 0)
                    {
                        argv[0] = (char *)env + 2;
                    }
                    break;
                }
            }
        }
    }

    nclsave = newcmdline;
    newcmdline = NULL;
    if (p == NULL)
    {
        p = "";
    }
    else
    {
        p = p + 0x80;
        if (*p == 0x7f)
        {
            unsigned char *q;

             /* this is above the limits of the PSP, so CMDLINE
                potentially has the whole command line, including
                the command */
            q = getenv("CMDLINE");
            if (q != NULL)
            {
                q = strchr(q, ' ');
                if (q != NULL)
                {
                    q++;
                    newcmdline = malloc(strlen(q) + 1);
                    if (newcmdline != NULL)
                    {
                        strcpy(newcmdline, q);
                        p = newcmdline;
                    }
                }
            }
            if (newcmdline == NULL)
            {
                fprintf(stderr, "command line too long to handle\n");
                newcmdline = nclsave;
                if (__runnum == 1)
                {
                    __exit(EXIT_FAILURE);
                }
                __runnum--;
                return(EXIT_FAILURE);
            }
        }
        else
        {
            p[*p + 1] = '\0';
            p++;
        }
    }
#endif
#if !defined(__MACOS__)

#if defined(__gnu_linux__) || defined(__EFI__)

    if (__runnum > 1)
    {
#endif

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

#if defined(__gnu_linux__) || defined(__EFI__)
    }
#endif

#endif
#ifdef PDOS_MAIN_ENTRY
    *i1 = argc;
    *i2 = (int)argv;
    return (0);
#elif defined(__PDPCLIB_DLL)
    return (0);
#else
    /* note that any executable (mainly one being used as a
       pseudo-BIOS/HAL) can choose to export it's C library,
       and in that case, the called program should not directly
       call the OS's exit function, but should return here so
       that this executable regains control. But exit() can be
       called from anywhere. So we need to use setjmp and longjmp.
       Note that the runnum increments for every invocation */

    if (!__genstart)
    {
        rc = setjmp(jb);
        if (rc != 0)
        {
#if defined(__64BIT__) && defined(__gnu_linux__)
	    /* need this for some reason */
	    /* but only if optimization is switched on */
	    /* return 0; */
#endif
            /* we're here because of longjmp */
            /* the invoker needs to set globrc first, otherwise
               we can't distinguish a genuine return code */
            rc = globrc;
        }
        else
        {
            /* I'm not sure if we can eliminate this call to main
               and always use genmain instead */
            /* We can't under current design where genmain is not
               being initialized so that it can work with Watcom
               where we have a dummy main as cdecl in pgastart.c */
            rc = main(argc, argv);
            exit(rc); /* this will return to the above setjmp */
        }
    }
    else
    {
#if defined(__64BIT__) && defined(__gnu_linux__)
        /* need this for some reason */
	/* but only if optimization is switched on */
        /* return 0; */
#endif
        rc = setjmp(jb);
        if (rc != 0)
        {
            rc = globrc;
        }
        else
        {

#ifdef WINNEWMOD
            G_argc = argc;
            G_argv = argv;
#endif

#ifdef __CC64__
            rc = (*__genmain)(argc, argv);
#else
            rc = __genmain(argc, argv);
#endif
            exit(rc); /* this will return to the above setjmp */
        }
    }
    globrc = oldglobrc;
    memcpy(&jb, &oldjb, sizeof jb);

#ifdef __MSDOS__
    if (newcmdline != NULL)
    {
        free(newcmdline);
    }
    newcmdline = nclsave;
#endif
    if (__runnum == 1)
    {
/* I think this only applies to EFI and should be changed */
#if !defined(__64BIT__) || defined(__WIN32__)
    __exit(rc);
#endif
    }
    __runnum--;
    return (rc);
#endif
}

__PDPCLIB_API__ void _exit(int status);
__PDPCLIB_API__ void _cexit(void);
__PDPCLIB_API__ void _c_exit(void);


/* Exiting is very complicated.

   Some systems such as *nix cannot be exited by doing a return
   from the entry point, because they have an unusual stack, with
   no return address. The proper solution to this is possibly to
   have some assembler code as the entry point, and we return
   cleanly to that, and then it can be responsible for doing the
   exit syscall to return to the OS. But historically we have
   done other semi-random things.

   Other systems may REQUIRE you to exit the same way you were
   entered, with a standard return. AmigaDOS may or may not be
   one of those systems, but we are not set up to exit any other
   way. A PDOS-generic app running under a pseudo-bios with the
   C library being reused would be an example of a system requiring
   (I think) clean exit.

   All OSes could be burdened with the requirement to provide an
   exit call - in which case they would be burdened with requiring
   a working setjmp/longjmp.

   Taking the burden off the OS means putting a burden on the app
   to have a working setjmp/longjmp. Traditionally we have implemented
   systems as proof of concept and setjmp/longjmp (more complicated
   assembler) is deferred. At time of writing, we don't have it for
   the 68000 and even on the x86 and ARM - although it exists and
   works, it doesn't match msvcrt, so you may get a crash if setjmp
   from PDPCLIB is used, but longjmp from real msvcrt.dll is used.

   But the ideal is probably to get setjmp/longjmp working in both
   the app and the OS so that a new OS, or a new app/C library, can
   get a quick and clean result.

   But until then, we probably need some defines that represent the
   current situation. This rationalization has not yet been done. It
   has always been adhoc, and not even been understood. Especially
   when I was still experimenting with using the same C library to
   run two different types of binaries (e.g. the mini replacement
   for WINE).
*/

void __exit(int status)
{
    /* Complete C library termination and exit with error code. */
    if (1) /* __runnum == 1) */
    {
    _cexit();
    }

    if (__runnum == 1)
    {
/* The Amiga doesn't have a good exita - and all environments should
   probably do the same - ie rely on longjmp working */
#if !defined(__AMIGA__) && !defined(__ATARI__)
    __exita(status);
#endif

    /* and i think we now need this for all environments */
    globrc = status;

    /* in case exita returns here, which is the case for PDOS-generic
       running under the pseudo-BIOS currently, decrement the run number
       so that we don't get called again */
    __runnum--;

    /* and we can't go through another longjmp either on
       some systems - supposedly/historically - hasn't
       yet been rationalized */
    /* 64 bit (64-bit what? EFI? Currently we are
       not doing it for Win64 until proven necessary) needs a longjmp. */
    /* PDOS-generic under the pseudo-bios needs a longjmp currently,
       so adding that too. It might be better to make this (1)
       until specific systems are identified as failing */
    /* ARM PE UEFI needs it too - biting the bullet and making it 1 */
    /* 64-bit ARM UEFI was found to have a problem when longjmp wasn't
       properly implemented and it was found to be that exit() was
       defining __exit as a "noreturn" function and not generating
       code that would allow a return. The "noreturn" was dropped for
       this situation */
    /* Also note that (at least) gccami (68000) - even with -fno-builtin -
       recognizes longjmp as a keyword and generates code (inc eliminates
       code) that assumes that longjmp will not be returned from - so if
       you haven't implemented longjmp yet, it's probably better to make
       it go into an infinite loop as a failsafe */
    /* ok, mystery solved I think. When running an MSVCRT-based executable,
       it doesn't go through the start() logic, so runnum is not increased.
       In that situation, exita needs to actually work - at least
       using current design. May be possible to change w32start.c to have
       a setjmp buffer, and do something in getmainargs. Or maybe
       get the pseudo-bios to force it through another start. I found
       that my setjmp in bios.c wasn't being executed because of
       insufficient defines, and that was causing a crash on exit. bios.c
       was fixed. */
#if 1 /* (defined(__64BIT__) && !defined(__WIN32__)) \
    || defined(__PDOS386__) */
    longjmp(jb, status);
#endif
    return;
    }
    globrc = status;

    longjmp(jb, status);

}

/* Not sure why this function exists */
/* I can't see any user of it. It doesn't seem to be a C90
   requirement. It was presumably added to be compatible with
   msvcrt.dll, but the programs we build shouldn't be requiring that */

__PDPCLIB_API__ void _exit(int status)
{
    /* Quick C library termination and exit with error code.. */
    _c_exit();

    __exita(status);
}

__PDPCLIB_API__ void _cexit(void)
{
    /* Complete C library termination. */
    _c_exit();
}

__PDPCLIB_API__ void _c_exit(void)
{
    /* Quick C library termination. */
    int x;

#if 0
    for (x = __NATEXIT - 1; x >= 0; x--)
    {
        if (__userExit[x] != 0)
        {
            (__userExit[x])();
        }
    }
#endif

    for (x = 0; x < __NFILE; x++)
    {
        if ((__userFiles[x] != NULL) && (__userFiles[x]->runnum == __runnum))
        {
#if defined(__VSE__)
            /* this should be closed after the rest of the user files */
            if (__userFiles[x] != __stdpch)
#endif
            fclose(__userFiles[x]);
            /* note that fclose itself will set the userfiles[x] to NULL */
        }
    }

#if defined(__VSE__)
    if (__stdpch != NULL)
    {
        if (__runnum == 1)
        {
            fclose(__stdpch);
        }
        else
        {
            fflush(__stdpch);
        }
    }
#endif

    if (__stdout != NULL) fflush(__stdout);
    if (__stderr != NULL) fflush(__stderr);

    /* bypass remainder for now unless top level */
    if (__runnum != 1) return;

#if defined(__WIN32__)
    SetConsoleMode(__stdin->hfile, stdin_dw);
    SetConsoleMode(__stdout->hfile, stdout_dw);
#endif

#if defined(__EFI__)
    if (__EfiRoot != NULL)
    {
        __EfiRoot->Close(__EfiRoot);
        __EfiRoot = NULL;
    }
#endif

#if defined(__PDOS386__)
    PosSetDeviceInformation(0, stdin_dw);
#endif
#if defined(__MSDOS__)
    __devsinfo(0, stdin_dw);
#endif

#if defined(__gnu_linux__)
    if (__runnum == 1)
    {
    __ioctl(0, TCSETS, (unsigned long)&tios_save);
    }
#endif

#if defined(__MVS__) || defined(__CMS__) || defined(__VSE__)
    if (__runnum == 1)
    {
    if (__stdin != NULL) fclose(__stdin);
    if (__stdout != NULL) fclose(__stdout);
    if (__stderr != NULL) fclose(__stderr);
    }
#endif


    if (__runnum == 1)
    {
#if USE_MEMMGR
    memmgrTerm(&__memmgr);

/* release memory for most circumstances, although a
   better solution will be required eventually */
#if defined(__MVS__) || defined(__CMS__) || defined(__VSE__)
    if (__lastsup != NULL)
    {
        __freem(__lastsup);
    }
#endif
#endif /* USE_MEMMGR */
    }
}

#ifdef __WIN32__
/* Windows extensions. */
static int _fmode;
__PDPCLIB_API__ int *__p__fmode(void)
{
    /* Not sure what should this function do. */
    return (&_fmode);
}

static char *_environ[] = {NULL};
static char **_environ_ptr = _environ;
__PDPCLIB_API__ char ***__p__environ(void)
{
    /* Not sure what should this function do. */
    return (&_environ_ptr);
}

__PDPCLIB_API__ void __set_app_type(int at)
{
    /* Not sure what should this function do. */
    ;
}

__PDPCLIB_API__ int _setmode(int fd, int mode)
{
    /* Should change mode of file descriptor (fd)
     * to mode (binary, text, Unicode...)
     * and return the previous mode.
     * We do not have _fileno() to convert FILE *
     * to int and _fileno() can be implemented
     * as macro accesing FILE internals...,
     * so this function is just a dummy. */
    return (0);
}

__PDPCLIB_API__ void (*_onexit(void (*func)(void)))(void)
{
    if (atexit(func)) return (NULL);
    return (func);
}
#endif

void __G_debug(void *ptr)
{
    int x;
    unsigned char *p;

    if (__G_ptr == NULL)
    {
        __G_live = 1;
#if 0
        __G_ptr = (void *)0x2cee948;
#else
        __G_ptr = (void *)0x33334444;
#endif
    }
    if (__G_live)
    {
#if 1
        p = __G_ptr;
        printf("%p ", p);
        for (x = 0; x < 16; x++)
        {
            printf(" %02X", p[x]);
        }
        printf("\n");
#endif
        printf("parm is %p\n", ptr);
    }
    return;
}

/* probably more than just WIN32 should define their
   exita here */
#ifdef __WIN32__
void __exita(int status)
{
    ExitProcess(status);
    return;
}
#endif
