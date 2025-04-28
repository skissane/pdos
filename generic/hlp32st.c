/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  hlp32st - helper32 start of C code                               */
/*                                                                   */
/*********************************************************************/

#include <__os.h>

static unsigned long *pblk;
static long (*callb)(int x, void *y);
static int (*subprog)(void *);

static void hlp32callback(char *str);

/* with the 16-bit version we had another parameter first,
   an we should standardize this one way or the other.
   And be consistent with the callback function */
unsigned long hlp32st(void *parms)
{
    int ret;
    char buf[20];
    char *z;

    pblk = parms;
    callb = (long (*)(int, void *))pblk[5];
    subprog = (int (*)(void *))pblk[14];

#if 0
    z = (char *)pblk[13];
    z[0] = 'G';
    z[70000UL] = 'K';
#endif

/* this doesn't work yet */
#if 1
    ret = printf("hi there %s\n", "paul");
    return (ret);

    ret = printf("hi there %s\n", "john");
    buf[0] = '\0';
    ret = sprintf(buf, "%06lX", 0x1234UL);
    ret = printf("should have 2 leading zeros %s\n", buf);
#endif
    ret = subprog((void *)hlp32callback);
    return (ret);
}

static void hlp32callback(char *str)
{
    printf("in helper32 callback - yippee!\n");
    printf("%s\n", str);
    return;
}

int printf(const char *format, ...)
{
    va_list arg;
    int ret;
    const char *vcptr;

    va_start(arg, format);
    vcptr = va_arg(arg, const char *);
    pblk[12] = offsetof(OS, Xprintf);
    *(const char **)&pblk[13] = format;
    *(const char **)&pblk[14] = vcptr;
    va_end(arg);
    ret = (int)callb(0, pblk);
    return (ret);
}



int sprintf(char *s, const char *format, ...)
{
    va_list arg;
    int ret;
    unsigned long x;

    va_start(arg, format);
    x = va_arg(arg, unsigned long);
    pblk[12] = offsetof(OS, Xsprintf);
    *(char **)&pblk[13] = s;
    *(const char **)&pblk[14] = format;
    pblk[15] = x;
    va_end(arg);
    ret = (int)callb(0, pblk);
    return (ret);
}
