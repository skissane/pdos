/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  w32start - startup code for WIN32 using MSVCRT.DLL               */
/*                                                                   */
/*********************************************************************/

/* This file is not part of the DLL, so should either be
   compiled without this defined, or we just undefine it now */
#undef __PDPCLIB_DLL

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef __SUBC__
int __getmainargs(int *_Argc, char ***_Argv, char ***_Env, int _DoWildCard,
                  int *_StartInfo);
#else
int __getmainargs(int *_Argc, void *_Argv, void *_Env, int _DoWildCard,
                  int *_StartInfo);
#endif

#ifdef __WATCOMC__
int __watcall main(int argc, char **argv);
int __cdmain(int argc, char **argv);
#else
int main(int argc, char **argv);
#endif

/* This is the main entry point of a console mode executable */

/* My understanding is that prior to this code being executed, the
   dependency on msvcrt.dll will have caused it to be loaded first.
   And it was built with PDPCLIB_DLL and included dllcrt.c, and the
   entry point DllMainCRTStartup will be called, which does a call
   to __start, and that call (part of the DLL) exits early - before
   calling main(). So then, when the main executable is loaded,
   mainCRTStartup is then called, and the only thing that hasn't
   been done (for whatever reason, presumably design choice) is
   parsing the parameters. The DLL has a function (getmainargs)
   to do that too, so that is now called */

/* consider adding this line so that relocatables are generated
   for some/most/all versions of "ld". The reason for this is
   that, at least with binutils 2.14a, there is code in there
   that will only generate relocatables if there are some
   exported symbols (or some other conditions) */
/*__declspec(dllexport)*/
void mainCRTStartup(void)
{
#if defined(__SUBC__) && defined(__64BIT__)
    /* int is 64-bits in SubC, but the functions being called
       are expecting both long and int to be 32-bits. We can
       get away with this with care. In thise case, we need
       to ensure the upper 32 bits are initialized to 0 by
       doing an explicit initialization. This should probably
       be moved to the subcglue.asm code */
    int argc = 0;
#else
    int argc;
#endif

#ifndef __SUBC__
    char **argv;
    char **environ;
#else
    char *argv;
    char *environ;
#endif
    int startinfo = 0;
    int status;

    /* Default Windows msvcrt.dll wrongly assumes
     * that pipes do not point to an interactive device
     * and makes stdout and stderr fully buffered.
     * PDPCLIB always uses line buffering for stdout and stderr,
     * so this forces the same behavior on Windows msvcrt.dll.
     *
     * But as _IOLBF does not work properly in Windows msvcrt.dll,
     * _IONBF must be used but it should not be used for PDPCLIB.
     * That is why _IOLBF is intentionally set to incompatible value
     * (2 instead of 64) what makes Windows msvcrt.dll return error
     * and identify itself.
     */
    if (setvbuf (stdout, NULL, _IOLBF, BUFSIZ)) {
        setvbuf (stdout, NULL, _IONBF, BUFSIZ);
        setvbuf (stderr, NULL, _IONBF, BUFSIZ);
    }

/* 0 = don't expand wildcards, 1 = expand */
#ifndef __SUBC__
    __getmainargs(&argc, &argv, &environ, 0, &startinfo);
#else
    __getmainargs(&argc, &argv, &environ, 0, &startinfo);
#endif

#ifdef __WATCOMC__
    status = __cdmain(argc, argv);
#elif defined(__SUBC__)
    status = main(argc, (void *)argv);
#else
    status = main(argc, argv);
#endif

    exit(status);
}

#ifndef NOUNDMAIN
void __main(void)
{
    return;
}
#endif

#if defined(__MSC__)
int _fltused;
int _ftol2_sse;
#endif
