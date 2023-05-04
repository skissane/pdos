/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  signal.h - signal header file.                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __SIGNAL_INCLUDED
#define __SIGNAL_INCLUDED

#if defined(__PDPCLIB_DLL) && !defined(__WATCOMC__)
#define __PDPCLIB_HEADFUNC __declspec(dllexport)
#endif

#ifndef __PDPCLIB_HEADFUNC
#define __PDPCLIB_HEADFUNC
#endif

typedef int sig_atomic_t;

__PDPCLIB_HEADFUNC void __sigdfl(int sig);
__PDPCLIB_HEADFUNC void __sigerr(int sig);
__PDPCLIB_HEADFUNC void __sigign(int sig);

#if (defined(__WIN32__) && !defined(__STATIC__)) || defined (__PDOSGEN__)
#define SIG_DFL 0
#define SIG_ERR -1
#define SIG_IGN 1
#else
#define SIG_DFL __sigdfl
#define SIG_ERR __sigerr
#define SIG_IGN __sigign
#endif

#define SIGABRT 1
#define SIGFPE 2
#define SIGILL 3
#define SIGINT 4
#define SIGSEGV 5
#define SIGTERM 6

#ifdef __SUBC__
int signal(int sig, int (*handler)());
#else
__PDPCLIB_HEADFUNC void (*signal(int sig, void (*func)(int)))(int);
#endif

__PDPCLIB_HEADFUNC int raise(int sig);


#if defined(__PDOSGEN__)
#include <__os.h>

#define signal __os->signal
#define raise __os->raise
#endif


#endif
