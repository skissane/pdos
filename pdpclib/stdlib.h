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

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
#if (defined(__OS2__) || defined(__32BIT__) || defined(__MVS__) \
    || defined(__CMS__) || defined(__VSE__) || defined(__SMALLERC__) \
    || defined(__ARM__))
typedef unsigned long size_t;
#elif (defined(__MSDOS__) || defined(__DOS__) || defined(__POWERC) \
    || defined(__WIN32__) || defined(__gnu_linux__) || defined(__AMIGA__) \
    || defined(__PDOS386__))
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
#define RAND_MAX 2147483648
#else
#define RAND_MAX 32767
#endif
#define MB_CUR_MAX 1
#define __NATEXIT 32

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
#if (defined(__MVS__) || defined(__CMS__) || defined(__VSE__)) \
    && defined(__GNUC__)
void abort(void) __attribute__((noreturn));
void exit(int status) __attribute__((noreturn));
#else
void abort(void);
void exit(int status);
#endif

#ifdef __SUBC__
void qsort(void *a, size_t b, size_t c,
    int (*f)());
#else
void qsort(void *a, size_t b, size_t c,
    int (*f)(const void *d, const void *e));
#endif

void srand(unsigned int seed);
int rand(void);
double atof(const char *nptr);
double strtod(const char *nptr, char **endptr);
int atoi(const char *nptr);
long atol(const char *nptr);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
int mblen(const char *s, size_t n);
int mbtowc(wchar_t *pwc, const char *s, size_t n);
int wctomb(char *s, wchar_t wchar);
size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);
int abs(int j);
div_t div(int numer, int denom);
long labs(long j);
ldiv_t ldiv(long numer, long denom);

#ifdef __SUBC__
int atexit(int (*func)());
#else
int atexit(void (*func)(void));
#endif

char *getenv(const char *name);
int system(const char *string);

#ifdef __SUBC__
void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)());
#else
void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
#endif

#ifdef __WATCOMC__
#pragma intrinsic (abs,labs,div,ldiv)
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
#endif

#endif
