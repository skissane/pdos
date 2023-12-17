/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  fpfuncsv.c - some support routines for floating point work       */
/*  for Visual Studio                                                */
/*                                                                   */
/*********************************************************************/

#include <string.h>

int _fltused;

int _ftol2_sse;


/* I don't know what these are */

#if defined(__MSDOS__) || defined(__OS2__)

void _aFftol(void)
{
    return;
}

void _fac(void)
{
    return;
}

void _aFfcompp(void)
{
    return;
}

#ifndef __PDOSGEN__
void *_fmemset(void *s, int c, unsigned int n)
{
    return (memset(s,c,n));
}

void *_fmemcpy(void *s1, const void *s2, unsigned int n)
{
    return (memcpy(s1,s2,n));
}
#endif

#endif
