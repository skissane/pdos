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

static int their_start(char *parm);

static OS os = { their_start };

static unsigned char *bios_os;

/* give them something to zap */
int (*genmain)(int argc, char **argv);

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
    bios_os = (unsigned char *)pblk[15];

#if 0
    z = (char *)pblk[13];
    z[0] = 'G';
    z[70000UL] = 'K';
#endif

#if 1
    ret = printf("hi there %s\n", "paul");
    ret = sprintf(buf, "%06lX", 0x1234UL);
    ret = printf("should have 2 leading zeros %s\n", buf);
#endif
    os.Xprintf = printf;
    os.Xsprintf = sprintf;
    os.main = &genmain;
    sprintf(buf, "%X", (int)genmain);
    printf("genmain currently %s\n", buf);
    ret = subprog((void *)&os);
    sprintf(buf, "%d", ret);
    printf("subprog returned %s\n", buf);
    return (ret);
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


static int their_start(char *parm)
{
    int rc;
    char buf[20];

    printf("about to call __start\n", "");
    sprintf(buf, "%X", (int)genmain);
    printf("genmain currently %s\n", buf);
    pblk[12] = offsetof(OS, __start);
    *(char **)&pblk[13] = "";
    rc = (int)callb(0, pblk);
    sprintf(buf, "%d", rc);
    printf("rc from callback is %s\n", buf);
    genmain = 0;
#if 0
    __start("" /* cmd */);
#endif
    return (rc);
}
