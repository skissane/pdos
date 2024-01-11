/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  math.h - math header file.                                       */
/*                                                                   */
/*********************************************************************/

#ifndef __MATH_INCLUDED
#define __MATH_INCLUDED

#if defined(__PDPCLIB_DLL) && !defined(__WATCOMC__)
#define __PDPCLIB_HEADFUNC __declspec(dllexport)
#endif

#ifndef __PDPCLIB_HEADFUNC
#define __PDPCLIB_HEADFUNC
#endif

/*
  Some constants - at present these are only defined for IBM
*/
#if defined(__MVS__) || defined (__CMS__) || defined(__VSE__)
/*
 This is about the nearest thing we can get to inf wthout
*/
#define HUGE_VAL 9.999999999999999999999E72

#else

#define HUGE_VAL 9.999999999999999999999E72

#endif

__PDPCLIB_HEADFUNC double ceil(double x);
__PDPCLIB_HEADFUNC double fabs(double x);
__PDPCLIB_HEADFUNC double floor(double x);
__PDPCLIB_HEADFUNC double fmod(double x, double y);

/* unimplemented: */


__PDPCLIB_HEADFUNC double atan2(double y, double x);
__PDPCLIB_HEADFUNC double frexp(double value, int *exp);
__PDPCLIB_HEADFUNC double ldexp(double x, int exp);
__PDPCLIB_HEADFUNC double modf(double value, double *iptr);

/*
  Implemented by d.wade - April 2006
*/

__PDPCLIB_HEADFUNC double pow(double x, double y);
__PDPCLIB_HEADFUNC double sqrt(double x);
__PDPCLIB_HEADFUNC double acos(double x);
__PDPCLIB_HEADFUNC double asin(double x);
__PDPCLIB_HEADFUNC double atan(double x);
__PDPCLIB_HEADFUNC double cos(double x);
__PDPCLIB_HEADFUNC double sin(double x);
__PDPCLIB_HEADFUNC double tan(double x);
__PDPCLIB_HEADFUNC double cosh(double x);
__PDPCLIB_HEADFUNC double sinh(double x);
__PDPCLIB_HEADFUNC double tanh(double x);
__PDPCLIB_HEADFUNC double exp(double x);
__PDPCLIB_HEADFUNC double log(double x);
__PDPCLIB_HEADFUNC double log10(double x);


#if 0 /* def __WATCOMC__ */
#pragma intrinsic(cos, sin, tan, exp, log, log10, sqrt)
#endif

#if defined(__IBMC__) && defined(__OS2__)
double _Builtin __fabs(double x);
#define fabs(x) (__fabs((x)))
double _Builtin __fsin(double x);
#define sin(x) (__fsin((x)))
double _Builtin __fcos(double x);
#define cos(x) (__fcos((x)))
double _Builtin __fptan(double x);
#define tan(x) (__fptan((x)))
double _Builtin __fpatan(double x);
#define atan(x) (__fpatan((x)))
double _Builtin __facos(double x);
#define acos(x) (__facos((x)))
double _Builtin __fasin(double x);
#define asin(x) (__fasin((x)))
double _Builtin __fsqrt(double x);
#define sqrt(x) (__fsqrt((x)))
#endif

#ifdef __BORLANDC__
double __sin__(double x);
#define sin(x) (__sin__((x)))
double __cos__(double x);
#define cos(x) (__cos__((x)))
#endif

#if defined(__PDOSGEN__)
#include <__os.h>

#define ceil __os->Xceil

#endif

#endif

