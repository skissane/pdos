/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  signal.c - implementation of stuff in signal.h                   */
/*                                                                   */
/*********************************************************************/

#include "signal.h"
#include "stdlib.h"
#include "stddef.h"

static void (*handlers[])(int) = {
    __sigdfl,
    __sigdfl,
    __sigdfl,
    __sigdfl,
    __sigdfl,
    __sigdfl,
    __sigdfl };

void __sigdfl(int sig);
void __sigerr(int sig);
void __sigign(int sig);

#if 0
#define SIG_DFL __sigdfl
#define SIG_ERR __sigerr
#define SIG_IGN __sigign
#endif

__PDPCLIB_API__ void (*signal(int sig, void (*func)(int)))(int)
{
#if defined(__WIN32__) && !defined(__STATIC__)
    if ((int)func == SIG_DFL)
    {
        handlers[sig] = __sigdfl;
    }
    else if ((int)func == SIG_ERR)
    {
        handlers[sig] = __sigerr;
    }
    else if ((int)func == SIG_IGN)
    {
        handlers[sig] = __sigign;
    }
#else
    handlers[sig] = func;
#endif
    return (func);
}


__PDPCLIB_API__ int raise(int sig)
{
    (handlers[sig])(sig);
    return (0);
}

__PDPCLIB_API__ void __sigdfl(int sig)
{
#ifndef SEGHACK
    handlers[sig] = SIG_DFL;
#endif
    if (sig == SIGABRT)
    {
        exit(EXIT_FAILURE);
    }
    return;
}

__PDPCLIB_API__ void __sigerr(int sig)
{
    (void)sig;
    return;
}

__PDPCLIB_API__ void __sigign(int sig)
{
    (void)sig;
    return;
}

