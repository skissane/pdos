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

typedef struct {
#if defined(__MVS__) || defined(__CMS__) || defined(__VSE__)
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
    void *sp;
    void *fp;
    void *ip;
    int r4;
    int r5;
    int r6;
    int r7;
    int r8;
    int r9;
    int r10;
#elif defined(__WIN32__) || defined(__32BIT__) || defined(__OS2__) \
    || defined(__PDOS386__) || defined(__gnu_linux__)
    int ebx;
    int ecx;
    int edx;
    int edi;
    int esi;
    int esp;
    int ebp;
    int retaddr;
    int extra[7]; /* for compatibility with MSVCRT */
#elif defined(__MSDOS__) || defined(__DOS__) || defined(__POWERC)
    int bx;
    int cx;
    int dx;
    int di;
    int si;
    int sp;
    int bp;
    int cs;
    int ss;
    int ds;
    int es;
    int retoff;
    int retseg;
#else
#error unknown system in setjmp.h
#endif
    int retval;
} jmp_buf[1];

void longjmp(jmp_buf env, int val);

#ifdef __MSC__
int setjmp(jmp_buf env);

#elif defined(__ARM__) || defined(__ARMGEN__)
/* it appears that gcc has _setjmp as a known keyword which
   is causing issues on ARM, so we change the name */
#define setjmp(x) __Ysetjmp(x)
int __Ysetjmp(jmp_buf env);

#elif defined(__MSDOS__)
#define setjmp(x) __setj(x)
#if defined(__WATCOMC__)
int __cdecl __setj(jmp_buf env);
#else
int __setj(jmp_buf env);
#endif

#else
#define setjmp(x) _setjmp(x)
int _setjmp(jmp_buf env);
#endif

#endif
