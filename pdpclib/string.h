/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  string.h - string header file.                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __STRING_INCLUDED
#define __STRING_INCLUDED

#if defined(__PDPCLIB_DLL) && !defined(__WATCOMC__)
#define __PDPCLIB_HEADFUNC __declspec(dllexport)
#endif

#ifndef __PDPCLIB_HEADFUNC
#define __PDPCLIB_HEADFUNC
#endif

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
#if defined(__64BIT__)
typedef unsigned long long size_t;
#elif (defined(__OS2__) || defined(__MVS__) \
    || defined(__CMS__) || defined(__VSE__) || defined(__SMALLERC__) \
    || defined(__ARM__) || defined(__gnu_linux__) \
    || defined(__SZ4__))
typedef unsigned long size_t;
#elif defined(__SUBC__)
/* typedef isn't working in string.c for some reason */
#define size_t int
/* typedef unsigned int size_t; */
#elif (defined(__MSDOS__) || defined(__DOS__) || defined(__POWERC) \
    || defined(__WIN32__) || defined(__AMIGA__) || defined(__EFI__) \
    || defined(__32BIT__) || defined(__PDOS386__))
typedef unsigned int size_t;
#endif
#endif

#define NULL ((void *)0)
__PDPCLIB_HEADFUNC void *memcpy(void *s1, const void *s2, size_t n);
__PDPCLIB_HEADFUNC void *memmove(void *s1, const void *s2, size_t n);
__PDPCLIB_HEADFUNC char *strcpy(char *s1, const char *s2);
__PDPCLIB_HEADFUNC char *strncpy(char *s1, const char *s2, size_t n);
__PDPCLIB_HEADFUNC char *strcat(char *s1, const char *s2);
__PDPCLIB_HEADFUNC char *strncat(char *s1, const char *s2, size_t n);
__PDPCLIB_HEADFUNC int memcmp(const void *s1, const void *s2, size_t n);
__PDPCLIB_HEADFUNC int strcmp(const char *s1, const char *s2);
__PDPCLIB_HEADFUNC int strcoll(const char *s1, const char *s2);
__PDPCLIB_HEADFUNC int strncmp(const char *s1, const char *s2, size_t n);
__PDPCLIB_HEADFUNC size_t strxfrm(char *s1, const char *s2, size_t n);
__PDPCLIB_HEADFUNC void *memchr(const void *s, int c, size_t n);
__PDPCLIB_HEADFUNC char *strchr(const char *s, int c);
__PDPCLIB_HEADFUNC size_t strcspn(const char *s1, const char *s2);
__PDPCLIB_HEADFUNC char *strpbrk(const char *s1, const char *s2);
__PDPCLIB_HEADFUNC char *strrchr(const char *s, int c);
__PDPCLIB_HEADFUNC size_t strspn(const char *s1, const char *s2);
__PDPCLIB_HEADFUNC char *strstr(const char *s1, const char *s2);
__PDPCLIB_HEADFUNC char *strtok(char *s1, const char *s2);
__PDPCLIB_HEADFUNC void *memset(void *s, int c, size_t n);
__PDPCLIB_HEADFUNC char *strerror(int errnum);
__PDPCLIB_HEADFUNC size_t strlen(const char *s);

#if defined(__WATCOMC__) && !defined(__SZ4__)
#ifdef __INLINE_FUNCTIONS__
#pragma intrinsic(memchr, memcmp, memcpy, strcat, strcpy, strlen, strchr)
#endif
#endif

#if defined(__IBMC__) && defined(__OS2__)
char * _Builtin __strcat(char *s1, const char *s2);
#define strcat(s1,s2) (__strcat((s1),(s2)))
char * _Builtin __strchr(const char *s, int c);
#define strchr(s,c) (__strchr((s),(c)))
int _Builtin __strcmp(const char *s1, const char *s2);
#define strcmp(s1,s2) (__strcmp((s1),(s2)))
int _Builtin __strcpy(char *s1, const char *s2);
#define strcpy(s1,s2) (__strcpy((s1),(s2)))
size_t _Builtin __strlen(const char *s);
#define strlen(s) (__strlen((s)))
char * _Builtin __strncat(char *s1, const char *s2, size_t n);
#define strncat(s1,s2,n) (__strncat((s1),(s2),(n)))
int _Builtin __strncmp(const char *s1, const char *s2, size_t n);
#define strncmp(s1,s2,n) (__strncmp((s1),(s2),(n)))
char * _Builtin __strncpy(char *s1, const char *s2, size_t n);
#define strncpy(s1,s2,n) (__strncpy((s1),(s2),(n)))
char * _Builtin __strrchr(const char *s, int c);
#define strrchr(s,c) (__strrchr((s),(c)))
void * _Builtin __memcpy(void *s1, const void *s2, size_t n);
#define memcpy(s1,s2,n) (__memcpy((s1),(s2),(n)))
void * _Builtin __memchr(const void *s, int c, size_t n);
#define memchr(s,c,n) (__memchr((s),(c),(n)))
int _Builtin __memcmp(const void *s1, const void *s2, size_t n);
#define memcmp(s1,s2,n) (__memcmp((s1),(s2),(n)))
void * _Builtin __memset(void *s, int c, size_t n);
#define memset(s,c,n) (__memset((s),(c),(n)))
void * _Builtin __memmove(void *s1, const void *s2, size_t n);
#define memmove(s1,s2,n) (__memmove((s1),(s2),(n)))
#endif

#ifdef __WATMUS__
#define __GNUC__ 0
#endif

#if defined (__GNUC__) && __GNUC__ >= 3 && !defined(__ARM__)
#define memcpy(s1,s2,n) (__builtin_memcpy((s1),(s2),(n)))
#define memcmp(s1,s2,n) (__builtin_memcmp((s1),(s2),(n)))
#endif

/* We don't activate these GCC builtins, because they
   normally resort to a call to the normal function,
   and when they do, they generate slightly worse
   code! Also, they appear to be buggy on MVS. */

#if 0
#define strcat(s1,s2) (__builtin_strcat((s1),(s2)))
#define strchr(s,c) (__builtin_strchr((s),(c)))
#define strcmp(s1,s2) (__builtin_strcmp((s1),(s2)))
#define strcpy(s1,s2) (__builtin_strcpy((s1),(s2)))
#define strlen(s) (__builtin_strlen((s)))
#define strncat(s1,s2,n) (__builtin_strncat((s1),(s2),(n)))
#define strncmp(s1,s2,n) (__builtin_strncmp((s1),(s2),(n)))
#define strncpy(s1,s2,n) (__builtin_strncpy((s1),(s2),(n)))
#define strrchr(s,c) (__builtin_strrchr((s),(c)))
#define memset(s,c,n) (__builtin_memset((s),(c),(n)))
#define strstr(s1,s2) (__builtin_strstr((s1),(s2)))
#define strpbrk(s1,s2) (__builtin_strpbrk((s1),(s2)))
#define strspn(s1,s2) (__builtin_strspn((s1),(s2)))
#define strcspn(s1,s2) (__builtin_strcspn((s1),(s2)))
#endif

#ifdef __BORLANDC__
#ifdef __INLINE_FUNCTIONS__
void *__memcpy__(void *s1, const void *s2, size_t n);
#define memcpy(s1,s2,n) (__memcpy__((s1),(s2),(n)))
void *__memchr__(const void *s, int c, size_t n);
#define memchr(s,c,n) (__memchr__((s),(c),(n)))
int __memcmp__(const void *s1, const void *s2, size_t n);
#define memcmp(s1,s2,n) (__memcmp__((s1),(s2),(n)))
void *__memset__(void *s, int c, size_t n);
#define memset(s,c,n) (__memset__((s),(c),(n)))
char *__strcat__(char *s1, const char *s2);
#define strcat(s1,s2) (__strcat__((s1),(s2)))
char *__strchr__(const char *s, int c);
#define strchr(s,c) (__strchr__((s),(c)))
int __strcmp__(const char *s1, const char *s2);
#define strcmp(s1,s2) (__strcmp__((s1),(s2)))
char *__strcpy__(char *s1, const char *s2);
#define strcpy(s1,s2) (__strcpy__((s1),(s2)))
size_t __strlen__(const char *s);
#define strlen(s) (__strlen__((s)))
char *__strncat__(char *s1, const char *s2, size_t n);
#define strncat(s1,s2,n) (__strncat__((s1),(s2),(n)))
int __strncmp__(const char *s1, const char *s2, size_t n);
#define strncmp(s1,s2,n) (__strncmp__((s1),(s2),(n)))
char *__strncpy__(char *s1, const char *s2, size_t n);
#define strncpy(s1,s2,n) (__strncpy__((s1),(s2),(n)))
char *__strrchr__(const char *s, int c);
#define strrchr(s,c) (__strrchr__((s),(c)))
#endif
#endif


#if defined(__PDOSGEN__)
#include <__os.h>

#define strchr __os->strchr
#define strcmp __os->strcmp
#define strncmp __os->strncmp
#define strcpy __os->strcpy
#define strlen __os->strlen
#undef memcpy
#define memcpy __os->memcpy
#define strncpy __os->strncpy
#define strcat __os->strcat
#define memset __os->memset
#define memmove __os->memmove
#undef memcmp
#define memcmp __os->memcmp
#define strerror __os->strerror
#define strrchr __os->strrchr
#define strstr __os->strstr
#define strpbrk __os->strpbrk
#define strspn __os->strspn
#define strcspn __os->strcspn
#define memchr __os->memchr
#define strncat __os->strncat
#endif


#endif
