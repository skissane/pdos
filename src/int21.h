/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  int21.h - header for int21.c                                     */
/*                                                                   */
/*********************************************************************/

#ifndef INT21_INCLUDED
#define INT21_INCLUDED

#ifndef __32BIT__
#ifdef __WATCOMC__
#define CTYP __cdecl
#else
#define CTYP
#endif
#endif

#ifdef __32BIT__
int int21(unsigned int *regs);
#else
void CTYP int21(unsigned int *regptrs);
#endif

#endif /* INT21_INCLUDED */
