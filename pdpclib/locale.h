/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  locale.h - locale header file.                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __LOCALE_INCLUDED
#define __LOCALE_INCLUDED

#if defined(__PDPCLIB_DLL) && !defined(__WATCOMC__) \
    && !defined(__NODECLSPEC__)
#define __PDPCLIB_HEADFUNC __declspec(dllexport)
#endif

#ifndef __PDPCLIB_HEADFUNC
#define __PDPCLIB_HEADFUNC
#endif

struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *int_curr_symbol;
    char *currency_symbol;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char int_frac_digits;
    char frac_digits;
    char p_cs_precedes;
    char p_sep_by_space;
    char n_cs_precedes;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
};

#define NULL ((void *)0)
#define LC_ALL 1
#define LC_COLLATE 2
#define LC_CTYPE 3
#define LC_MONETARY 4
#define LC_NUMERIC 5
#define LC_TIME 6

__PDPCLIB_HEADFUNC char *setlocale(int category, const char *locale);
__PDPCLIB_HEADFUNC struct lconv *localeconv(void);

#if defined(__PDOSGEN__)
#include <__os.h>

#define setlocale __os->setlocale

#endif


#endif
