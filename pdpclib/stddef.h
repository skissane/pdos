/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  stddef.h - assert header file.                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __STDDEF_INCLUDED
#define __STDDEF_INCLUDED

#if defined(__64BIT__)
#ifdef __SUBC__
typedef int ptrdiff_t;
#else
typedef long long ptrdiff_t;
#endif
#else
typedef int ptrdiff_t;
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
    || defined(__ARM__) || defined(__gnu_linux__) \
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

#define NULL ((void *)0)
#define offsetof(x, y) (size_t)&(((x *)0)->y)

#if defined(__PDPCLIB_DLL) && !defined(__WATCOMC__)
#define __PDPCLIB_API__ __declspec(dllexport)
#elif defined(__CC64OS__)
#define __PDPCLIB_API__ $callback
#else
#define __PDPCLIB_API__
#endif

#endif
