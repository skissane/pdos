/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  p32start - startup code for non-MSVCRT WIN32                     */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

__PDPCLIB_API__ int __start(char *p);

/* This is the main entry point of a console mode executable */

/* consider adding this line so that relocatables are generated
   for some/most/all versions of "ld". The reason for this is
   that, at least with binutils 2.14a, there is code in there
   that will only generate relocatables if there are some
   exported symbols (or some other conditions) */
/*__declspec(dllexport)*/
void mainCRTStartup(void)
{
    __start(0);
    return;
}

#ifndef NOUNDMAIN
void __main(void)
{
    return;
}
#endif

/* we may be compiling this with cc64, but compiling some
   other application with Visual C, so we need this to exist */
#if 1 /* defined(__MSC__) */
int _fltused;
/* Use QIfist option if necessary so that this function is not required */
/* int _ftol2_sse(void) { return 0; } */
#endif
