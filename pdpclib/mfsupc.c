/* mainframe support routines written in C */
/* written by Paul Edwards */
/* released to the public domain */

/* routines have double underscore so that they can be
   used as part of a C library. Do a #define if not
   part of a C library */

#include <string.h>

#include "__mvs.h"

int __wto(int len, int flags, char *msg)
{
    char buf[84];
    regs regsin;
    regs regsout;

    *(short *)buf = len;
    *(short *)(buf + 2) = flags;
    if (len > 84) len = 84;
    if (len < 4) return (0);
    /* memcpy(buf + 4, msg, len - 4); */
    regsin.r[1] = (int)buf;
    __svc(35, &regsin, &regsout);
    return (0);
}


/* return number 6 for testing purposes */

int __ret6(void)
{
    return (6);
}


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
    ccb.actual.logical_unit = 4; /* I guess this means SYSLOG */
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
