/* mainframe support routines written in C */
/* written by Paul Edwards */
/* released to the public domain */

/* routines have double underscore so that they can be
   used as part of a C library. Do a #define if not
   part of a C library */

#include <string.h>

#include <__os.h>

#include "__mvs.h"

extern OS *__pgparm;


int __wto(int len, int flags, char *msg)
{
    /* not sure what alignment is required - fullword should be enough */
    union {
        int dummy;
        char buf[84];
    } ubuf;
    regs regsin;
    regs regsout;

    if (len < 0) return (0);
    if (len > 80) len = 80;
    len += 4;
    *(short *)ubuf.buf = len;
    *(short *)(ubuf.buf + 2) = flags;
    memcpy(ubuf.buf + 4, msg, len - 4);
    regsin.r[1] = (int)ubuf.buf;
    __svc(35, &regsin, &regsout);
    return (0);
}


/* return number 6 for testing purposes */

int __ret6(void)
{
    return (6);
}


#ifdef __MVS__

/* C startup code for MVS */

int __mvsrun(void)
{
    __wto(11, 0, "ABC DEF GHI");
    return (__ret6());
}

#endif


#ifdef __VSE__

/* C startup code for VSE */

int __vserun(void)
{
    /* issue a WTO for now */
    /* not sure what alignment a CCW needs */
    union {
        double dummy;
        struct {
            unsigned int addr;
            unsigned short flags;
            unsigned short len;
        } actual;
    } ccw;
    char msg[80];
    /* not sure what alignment a CCB needs */
    union {
        double dummy;
        struct {
            unsigned short residual_count;
            unsigned short communication_bytes;
            unsigned short csw_status_bytes;
            unsigned char logical_unit_class;
            unsigned char logical_unit;
            void *ccw_address;
            unsigned int status_and_csw_ccw;
        } actual;
    } ccb;
    regs regsin;
    regs regsout;

    memcpy(msg, "MADE IT!", 8);
    ccw.actual.addr = (0x09 << 24) | (unsigned int)msg;
    ccw.actual.flags = 0;
    ccw.actual.len = 8;
    
    ccb.actual.residual_count = 0;
    ccb.actual.communication_bytes = 0;
    ccb.actual.csw_status_bytes = 0;
    ccb.actual.logical_unit_class = 0;
    /* logical units are:
       0 = sysrdr
       1 = sysipt
       2 = syspch
       3 = syslst
       4 = syslog
       5 = syslnk
       6 = sysres
       7 = sysslb
       8 = sysrlb
       9 = sysuse
       10 = sysrec
       11 = sysclb
       12 = sysvis
       13 = syscat
       and also sys000 is 0, sys001 is 1 etc, I think */
    ccb.actual.logical_unit = 4; /* SYSLOG */
    ccb.actual.ccw_address = &ccw;
    ccb.actual.status_and_csw_ccw = 0;

    /* excp ccb */
    regsin.r[1] = (int)&ccb;
    __svc(0, &regsin, &regsout);

    /* wait ccb */
    /* so long as the top bit is 0, the EXCP was accepted and
       thus it is OK to wait on it - I think */
    if ((ccb.actual.csw_status_bytes & 0x8000U) == 0)
    {
        regsin.r[1] = (int)&ccb;
        __svc(7, &regsin, &regsout);
    }

    return (0);
}

#endif


#ifdef __BIGFOOT__

/* C startup code for MVS */

int __bigrun(void)
{
    regs regsin;
    regs regsout;
    int fh;

    /* standard handles are not available for some reason */
    /* we need to open the TTY instead */
    regsin.r[1] = 5; /* open syscall */
    regsin.r[5] = (int)"/dev/tty0";
    regsin.r[6] = 2; /* O_WRONLY */
    regsin.r[7] = 0; /* not sure if this is required */
    fh = __svc(0, &regsin, &regsout);

    regsin.r[1] = 4; /* write syscall */
    regsin.r[5] = fh; /* 1; */
    regsin.r[6] = (int)"HI BIG\n";
    regsin.r[7] = 7;
    __svc(fh, &regsin, &regsout);

    regsin.r[1] = 1; /* exit syscall */
    regsin.r[5] = 6;
    __svc(0, &regsin, &regsout);
    return (__ret6());
}

#endif


int __svc(int svcnum, void *regsin, void *regsout)
{
    if (__pgparm == 0)
    {
        return (__svcrl(svcnum, regsin, regsout));
    }
    else
    {
        return (__pgparm->Xservice(svcnum, regsin, regsout));
    }
}
