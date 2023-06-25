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

int _fltused;

int _ftol2_sse;


/* I don't know what these are */

#ifdef __MSDOS__

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

void _fmemset(void)
{
    return;
}

void _fmemcpy(void)
{
    return;
}

long _aFNauldiv(void)
{
    return (0);
}

#endif
