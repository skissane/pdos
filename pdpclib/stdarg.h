/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  stdarg.h - stdarg header file.                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __STDARG_INCLUDED
#define __STDARG_INCLUDED

/* don't use builtins on MVS until they have been implemented */
/* don't use on EMX either */
/* and they haven't been implemented in PDOS/386 either */
#if defined(__GNUC__) && !defined(__MVS__) && !defined(__CMS__) \
    && !defined(__VSE__) && !defined(__EMX__) && !defined(__PDOS386__) \
    && !defined(__gnu_linux__) && !defined(__NOBIVA__) \
    && !defined(__ARM__)

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
#endif

typedef __gnuc_va_list va_list;

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L
#define va_copy(d,s)    __builtin_va_copy(d,s)
#endif
#define __va_copy(d,s)  __builtin_va_copy(d,s)

#else /* __GNUC__ */

#ifdef __XWATCOMC__
/* The arguments are on the stack, but references are normally
   to ds (DGROUP). Small data is a fundamentally flawed model
   for PDPCLIB, as functions don't know if they are receiving
   a stack parameter or a data parameter, or even a code
   parameter if literals are in the code section. As such, memory
   models small and medium are not supported. Only tiny, compact,
   large and huge are supported. However, making this a far
   pointer at least allows you to printf a %d (but not %ld
   because that goes through a different code path). Also note
   that tiny memory model doesn't support malloc, so you won't
   be able to open any files, so you won't be able to do much. */
typedef char far *va_list;
#else
typedef char * va_list;
#endif

#ifdef __XWATCOMC__
#define va_start(ap, parmN) ap = (char far *)&parmN + sizeof(parmN)
#define va_arg(ap, type) *(type far *)(ap += sizeof(type), ap - sizeof(type))
#else
#define va_start(ap, parmN) ap = (char *)&parmN + sizeof(parmN)
#define va_arg(ap, type) *(type *)(ap += sizeof(type), ap - sizeof(type))
#endif
#define va_end(ap) ap = (char *)0

#endif

#endif
