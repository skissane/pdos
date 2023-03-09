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

int main(int argc, char **argv);

/* This name is known to certain versions of "ld" as the entry
point of an application and there is no particular reason to not
use it. */

int __crt0(OS *os)
{
    __os = os;
    *__os->main = main;
    return (__os->__start(0));
}

#if defined(__AMIGA__) || defined(NEED_UNDMAIN)
/* needed for Amiga (gccami) */
void __main(void)
{
    return;
}
#endif
