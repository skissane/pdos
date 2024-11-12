/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  setjmp.h - setjmp header file.                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __SETJMP_INCLUDED
#define __SETJMP_INCLUDED

#if defined(__PDPCLIB_DLL) && !defined(__WATCOMC__)
#define __PDPCLIB_HEADFUNC __declspec(dllexport)
#endif

#ifndef __PDPCLIB_HEADFUNC
#define __PDPCLIB_HEADFUNC
#endif

typedef struct {
#if defined(__64BIT__)
    long long retval;
    long long retaddr;

#if defined(__ARM__) || defined(__ARMGEN__)
    long long regs[32-2+1];
#else
    long long regs[16-2+1];
#endif

#elif defined(__MVS__) || defined(__CMS__) || defined(__VSE__) \
    || defined(__MF32__)
    int regs[15];
#elif defined(__AMIGA__)
    long a0;
    long a1;
    long a2;
    long a3;
    long a4;
    long a5;
    long a6;
    long a7;
    long d0;
    long d1;
    long d2;
    long d3;
    long d4;
    long d5;
    long d6;
    long d7;
    long retaddr;
#elif defined(__ARM__) || defined(__ARMGEN__)
    /* I think we need to match msvcrt use, which I think
       is fp, r4, r5, r6, r7, r8, r9, r10, r11, sp,
          new pc (r15) = old lr (r14) I think,
          fpscr (floating point status and control register),
          then 8 * 64-bit floating point */
    /* ip is scratch so doesn't need to be saved */
    void *sp;
    void *fp;
    int lr; /* was void *ip; */
    int r4;
    int r5;
    int r6;
    int r7;
    int r8;
    int r9;
    int r10;
    int r2;
    int r3;
    int r12; /* ip */
    int r1;
    int extra[13]; /* MSVCRT requires 28 total - retaddr below is extra 1 */
#elif defined(__WIN32__) || defined(__32BIT__) \
    || (defined(__OS2__) && !defined(__16BIT__)) \
    || defined(__PDOS386__) || defined(__gnu_linux__) || defined(__EFI__)
    /* not sure how we're getting away with not matching MSVCRT,
       which I think is ebp, ebx, edi, esi, esp, eip, and then
       10 * 32-bit crap for a total of 16 32-bit values. We only
       have 15 here, because the 16th is the retval, which
       doesn't need to be in this buffer as a separate entry I think */
    int ebx;
    int ecx;
    int edx;
    int edi;
    int esi;
    int esp;
    int ebp;
    int retaddr;
    int extra[7]; /* for compatibility with MSVCRT */
#elif defined(__OS2__) || defined(__MSDOS__) || defined(__DOS__) \
      || defined(__POWERC)
    int bx;
    int cx;
    int dx;
    int di;
    int si;
    int sp;
    int bp;
    int retoff;
    int retseg;
    int ds;
    int es;
#else
#error unknown system in setjmp.h
#endif
#ifndef __64BIT__
    int retval;
#endif
} jmp_buf[1];

__PDPCLIB_HEADFUNC void longjmp(jmp_buf env, int val);

#if defined(__MSC__) && !defined(__ARM__)
#define setjmp(x) __setj(x)
int __setj(jmp_buf env);

#elif defined(__ARM__) || defined(__ARMGEN__)
/* it appears that gcc has _setjmp as a known keyword which
   is causing issues on ARM, so we change the name */
#define setjmp(x) __Ysetjmp(x)
int __Ysetjmp(jmp_buf env);

/* we need to standardize on setj I think */
#elif defined(__EFI__) || defined(__AMIGA__) \
    || defined(__64BIT__)
#define setjmp(x) __setj(x)
int __setj(jmp_buf env);

#elif defined(__MSDOS__) || (defined(__OS2__) && defined(__16BIT__))
#define setjmp(x) __setj(x)
#if defined(__WATCOMC__)
int __cdecl __setj(jmp_buf env);
#else
int __setj(jmp_buf env);
#endif

#elif defined(__MVS__) || defined(__CMS__) || defined(__VSE__) \
    || defined(__MF32__)
#define setjmp(x) __setj(x)

#else
#define setjmp(x) _setjmp(x)
int _setjmp(jmp_buf env);
#endif

#if defined(__PDOSGEN__)
#include <__os.h>

#define longjmp __os->Xlongjmp
#endif


#endif
