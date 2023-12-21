/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  fpfuncsw.c - some support routines for floating point work       */
/*  for Watcom C compiler for OS/2                                   */
/*                                                                   */
/*********************************************************************/

int _fltused_ = 0;
int __real87 = 0;
int _argc;

void __init_387_emulator(void)
{
    return;
}

void _8087(void)
{
    return;
}

void __STOSD(void)
{
    return;
}

#if defined(__OS2__) && defined(__16BIT__)
/* do nothing */
#else
void _CHP(void)
{
    return;
}
#endif

void cstart_(void)
{
    return;
}


#ifdef __WATCOMC__
int __watcall main(int argc, char **argv);

int __cdmain(int argc, char **argv)
{
    return (main(argc, argv));
}
#endif


#ifdef __OS2__
#include <os2.h>

/* Attempting to use the function that MSC 6.0
   uses results in either a link error or a bind
   error, so this MSC code is currently disabled
   and issue is worked around in the makefile */
#ifdef __XMSC__
void APIENTRY __AHSHIFT(void);
void APIENTRY __AHINCR(void);
#else
void APIENTRY DosHugeShift(void);
void APIENTRY DosHugeIncr(void);
#endif

#ifndef __PDOSGEN__
void __myDosExit(int one, int two)
{
    DosExit(one, two);
}
#endif

#ifdef __16BIT__
void *__myDosHugeShift(void)
{
#ifdef __XMSC__
    return (void *)__AHSHIFT;
#else
    return (void *)DosHugeShift;
#endif
}

void *__myDosHugeIncr(void)
{
#ifdef __XMSC__
    return (void *)__AHINCR;
#else
    return (void *)DosHugeIncr;
#endif
}
#endif

int argc;

#ifdef __16BIT__
extern unsigned int __shift;
extern unsigned int __incr;

void __fixhuge(void)
{
    __shift = (unsigned int)(unsigned long)__myDosHugeShift();
    __incr = (unsigned int)(unsigned long)__myDosHugeIncr();
}
#endif

#endif
