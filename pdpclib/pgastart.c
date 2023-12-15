/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pgastart - startup routine for PDOS generic applications         */
/*                                                                   */
/*********************************************************************/

#include <__os.h>

OS *__os;

#if defined(__WATCOMC__)
int __dummain(int argc, char **argv);
#define MAINTYP __watcall
#else
#define MAINTYP
#endif

int MAINTYP main(int argc, char **argv);

/* This name is known to certain versions of "ld" as the entry
point of an application and there is no particular reason to not
use it. */

int __crt0(OS *os)
{
    __os = os;
#ifdef __WATCOMC__
    *__os->main = __dummain;
#else
    *__os->main = main;
#endif
    return (__os->__start(0));
}

#if defined(__AMIGA__) || defined(NEED_UNDMAIN)
/* needed for Amiga (gccami) */
void __main(void)
{
    return;
}
#endif

#if defined(__WATCOMC__)
int __dummain(int argc, char **argv)
{
    return (main(argc, argv));
}
#endif

#if defined(NEED_EXIT)
void exit(int x)
{
    __os->Xexit(x);
    return;
}
#endif
