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

void _CHP(void)
{
    return;
}

void cstart_(void)
{
    return;
}


int __watcall main(int argc, char **argv);

int __cdmain(int argc, char **argv)
{
    return (main(argc, argv));
}


#ifdef __OS2__
#include <os2.h>

void __myDosExit(int one, int two)
{
    DosExit(one, two);
}

int argc;

#endif
