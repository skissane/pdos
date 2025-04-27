/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  hlp16st - helper16 start of C code                               */
/*                                                                   */
/*********************************************************************/

#include <__os.h>

static unsigned long *pblk;
static long (*callb)(int x, void *y);

unsigned long hlp16st(int val, void *parms)
{
    int ret;

    pblk = parms;
    callb = (long (*)(int, void *))pblk[5];
    /* *(char **)&pblk[12] = offsetof(OS, Xprintf); */
    ret = printf("hi there %s\n", "paul");
    ret = printf("hi there %s\n", "john");
    return (ret);
    /* callb(0, parms)); */
}

int printf(const char *format, ...)
{
    va_list arg;
    int ret;
    char *firstparm;
    const char *vcptr;

    va_start(arg, format);
    vcptr = va_arg(arg, const char *);
    pblk[12] = offsetof(OS, Xprintf);
    *(char **)&pblk[13] = format;
    *(char **)&pblk[14] = vcptr;
    ret = (int)callb(0, pblk);
    va_end(arg);
    return (ret);
}
