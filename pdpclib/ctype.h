/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  ctype.h - ctype header file.                                     */
/*                                                                   */
/*********************************************************************/

#ifndef __CTYPE_INCLUDED
#define __CTYPE_INCLUDED

int isalnum(int c);
int isalpha(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);

#if defined(__WIN32__) && !defined(__STATIC__) \
    || (defined(__SUBC__) && !defined(__PDOSGEN__))

/* don't use macros on these environments */

#elif defined(__PDOSGEN__)
#include <__os.h>

#define isalnum __os->isalnum
#define isxdigit __os->isxdigit
#define isdigit __os->isdigit
#define isalpha __os->isalpha
#define isprint __os->isprint
#define isspace __os->isspace
#define tolower __os->tolower
#define islower __os->islower
#define isupper __os->isupper
#define toupper __os->toupper
#define iscntrl __os->iscntrl
#define ispunct __os->ispunct

#else
extern unsigned short *__isbuf;
extern short *__tolow;
extern short *__toup;

#define isalnum(c) (__isbuf[(c)] & 0x0001U)
#define isalpha(c) (__isbuf[(c)] & 0x0002U)
#define iscntrl(c) (__isbuf[(c)] & 0x0004U)
#define isdigit(c) (__isbuf[(c)] & 0x0008U)
#define isgraph(c) (__isbuf[(c)] & 0x0010U)
#define islower(c) (__isbuf[(c)] & 0x0020U)
#define isprint(c) (__isbuf[(c)] & 0x0040U)
#define ispunct(c) (__isbuf[(c)] & 0x0080U)
#define isspace(c) (__isbuf[(c)] & 0x0100U)
#define isupper(c) (__isbuf[(c)] & 0x0200U)
#define isxdigit(c) (__isbuf[(c)] & 0x0400U)
#define tolower(c) (__tolow[(c)])
#define toupper(c) (__toup[(c)])
#endif

#endif
