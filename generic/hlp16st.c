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
static int (*subprog)(void *);

static void hlp16callback(char *str);

unsigned long hlp16st(int val, void *parms)
{
    int ret;
    char buf[20];
    char *z;

    pblk = parms;
    callb = (long (*)(int, void *))pblk[5];
    subprog = (int (*)(void *))pblk[14];
    z = (char *)pblk[13];
    z[0] = 'G';
    z[70000UL] = 'K';
    ret = printf("hi there %s\n", "paul");
    ret = printf("hi there %s\n", "john");
    buf[0] = '\0';
    ret = sprintf(buf, "%06lX", 0x1234UL);
    ret = printf("should have 2 leading zeros %s\n", buf);
    ret = subprog((void *)hlp16callback);
    return (ret);
}

static void hlp16callback(char *str)
{
    printf("in helper16 callback - yippee!\n");
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
    *(char **)&pblk[13] = format;
    *(char **)&pblk[14] = vcptr;
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
    *(char **)&pblk[14] = format;
    pblk[15] = x;
    va_end(arg);
    ret = (int)callb(0, pblk);
    return (ret);
}
