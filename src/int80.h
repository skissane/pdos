/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  int80.h - header for int80.c                                     */
/*                                                                   */
/*********************************************************************/

#ifndef INT80_INCLUDED
#define INT80_INCLUDED

int int80(unsigned int *regs);
void initlinux(void);

#endif /* INT80_INCLUDED */
