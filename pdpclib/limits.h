/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  limits.h - limits header file.                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __LIMITS_INCLUDED
#define __LIMITS_INCLUDED

#define CHAR_BIT 8
#define SCHAR_MIN -128
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#if ('\x80' < 0)
#define CHAR_MIN -128
#define CHAR_MAX 127
#else
#define CHAR_MIN 0
#define CHAR_MAX 255
#endif
#define MB_LEN_MAX 1
#define SHRT_MIN (-SHRT_MAX-1)
#if defined(__FORCE_ALL_64__)
#define SHRT_MAX 0x7fffffffffffffffL
#define USHRT_MAX ((unsigned short)0xffffffffffffffffUL)
#else
#define SHRT_MAX 32767
#define USHRT_MAX 65535U
#endif

#if (defined(__OS2__) || defined(__32BIT__) || defined(__MVS__) \
    || defined(__MF32__) \
    || defined(__VSE__) || defined(__PDOS386__) \
    || defined(__CMS__) || defined(__WIN32__) || defined(__gnu_linux__) \
    || defined(__AMIGA__) ||defined(__SMALLERC__) || defined(__ARM__) \
    || defined(__EFI__) || defined(__ATARI__))
#define INT_MIN (-INT_MAX-1)

#if defined(__FORCE_ALL_64__)
#define INT_MAX 0x7fffffffffffffffL
#define UINT_MAX ((unsigned short)0xffffffffffffffffUL)
#else
#define INT_MAX 2147483647
#define UINT_MAX 4294967295U
#endif

#endif

#if (defined(__MSDOS__) || defined(__DOS__) || defined(__POWERC))
#define INT_MIN (-INT_MAX-1)
#define INT_MAX 32767
#define UINT_MAX 65535U
#endif

#define LONG_MIN (-LONG_MAX-1)

#if (defined(__64BIT__) && defined(__gnu_linux__)) || defined(__LONG64__)
#define LONG_MAX 0x7fffffffffffffffL
#define ULONG_MAX ((unsigned short)0xffffffffffffffffUL)
#else
#define LONG_MAX 2147483647L
#define ULONG_MAX 4294967295UL
#endif

#endif
