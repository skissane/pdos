/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  stdlib.h - stdlib header file                                    */
/*                                                                   */
/*********************************************************************/

#ifndef __STDLIB_INCLUDED
#define __STDLIB_INCLUDED

#if defined(__PDPCLIB_DLL) && !defined(__WATCOMC__)
#define __PDPCLIB_HEADFUNC __declspec(dllexport)
#endif

#ifndef __PDPCLIB_HEADFUNC
#define __PDPCLIB_HEADFUNC
#endif

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
#if defined(__64BIT__)

#if defined(__LONG64__)
typedef unsigned long size_t;
#else
typedef unsigned long long size_t;
#endif

#elif (defined(__OS2__) || defined(__MVS__) \
    || defined(__CMS__) || defined(__VSE__) || defined(__SMALLERC__) \
    || defined(__ARM__)|| defined(__gnu_linux__) \
    || defined(__SZ4__))
typedef unsigned long size_t;
#elif (defined(__MSDOS__) || defined(__DOS__) || defined(__POWERC) \
    || defined(__WIN32__) || defined(__AMIGA__) || defined(__EFI__) \
    || defined(__32BIT__) || defined(__PDOS386__))
typedef unsigned int size_t;
#endif
#endif
#ifndef __WCHAR_T_DEFINED
#define __WCHAR_T_DEFINED
#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
#endif
typedef char wchar_t;
#endif
typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;

#define NULL ((void *)0)
#define EXIT_SUCCESS 0
#if defined(__MVS__) || defined(__CMS__) || defined(__VSE__)
#define EXIT_FAILURE 12
#else
#define EXIT_FAILURE 1
#endif
#if defined(__32BIT__) || defined(__WIN32__)
#define RAND_MAX 2147483647
#else
#define RAND_MAX 32767
#endif
#define MB_CUR_MAX 1
#define __NATEXIT 32

__PDPCLIB_HEADFUNC void *malloc(size_t size);
__PDPCLIB_HEADFUNC void *calloc(size_t nmemb, size_t size);
__PDPCLIB_HEADFUNC void *realloc(void *ptr, size_t size);
__PDPCLIB_HEADFUNC void free(void *ptr);
#if (defined(__MVS__) || defined(__CMS__) || defined(__VSE__)) \
    && defined(__GNUC__)
void abort(void) __attribute__((noreturn));
void exit(int status) __attribute__((noreturn));
#else
__PDPCLIB_HEADFUNC void abort(void);
__PDPCLIB_HEADFUNC void exit(int status);
#if defined(__SUBC__) && defined(__64BIT__)
#define exit(x) __scexit(x)
#endif
#endif

#ifdef __SUBC__
void qsort(void *a, size_t b, size_t c,
    int (*f)());
#else
__PDPCLIB_HEADFUNC void qsort(void *a, size_t b, size_t c,
    int (*f)(const void *d, const void *e));
#endif

__PDPCLIB_HEADFUNC void srand(unsigned int seed);
__PDPCLIB_HEADFUNC int rand(void);
__PDPCLIB_HEADFUNC double atof(const char *nptr);
__PDPCLIB_HEADFUNC double strtod(const char *nptr, char **endptr);
__PDPCLIB_HEADFUNC int atoi(const char *nptr);
__PDPCLIB_HEADFUNC long atol(const char *nptr);
__PDPCLIB_HEADFUNC long strtol(const char *nptr, char **endptr, int base);
__PDPCLIB_HEADFUNC unsigned long strtoul(const char *nptr, char **endptr,
    int base);
__PDPCLIB_HEADFUNC int mblen(const char *s, size_t n);
__PDPCLIB_HEADFUNC int mbtowc(wchar_t *pwc, const char *s, size_t n);
__PDPCLIB_HEADFUNC int wctomb(char *s, wchar_t wchar);
__PDPCLIB_HEADFUNC size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
__PDPCLIB_HEADFUNC size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);
__PDPCLIB_HEADFUNC int abs(int j);
__PDPCLIB_HEADFUNC div_t div(int numer, int denom);
__PDPCLIB_HEADFUNC long labs(long j);
__PDPCLIB_HEADFUNC ldiv_t ldiv(long numer, long denom);

#ifdef __SUBC__
int atexit(int (*func)());
#else
__PDPCLIB_HEADFUNC int atexit(void (*func)(void));
#endif

__PDPCLIB_HEADFUNC char *getenv(const char *name);
__PDPCLIB_HEADFUNC int system(const char *string);

#ifdef __SUBC__
void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)());
#else
__PDPCLIB_HEADFUNC void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
#endif

#ifdef __WATCOMC__
#if defined(__OS2__)
/* do nothing */
#else
#pragma intrinsic (abs,labs,div,ldiv)
#endif
#endif

#if defined(__IBMC__) && defined(__OS2__)
int _Builtin __abs(int j);
#define abs(j) (__abs((j)))
long _Builtin __labs(long j);
#define labs(j) (__labs((j)))
#endif

#if defined(__PDOSGEN__)
#include <__os.h>

#define malloc __os->malloc
#define free (*__os->free)
#undef abort
#define abort __os->abort
#define getenv __os->Xgetenv
#define exit __os->Xexit
#define calloc __os->Xcalloc
#define realloc __os->Xrealloc
#define atoi __os->Xatoi
#define strtol __os->Xstrtol
#define strtoul __os->Xstrtoul
#define qsort __os->Xqsort
#define bsearch __os->Xbsearch
#define abs __os->Xabs
#define atof __os->Xatof
#define system __os->Xsystem
#define atexit __os->Xatexit
#define rand __os->Xrand
#define srand __os->Xsrand
#define atol __os->Xatol

#endif

#endif
