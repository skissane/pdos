/* mainframe support routines written in C */
/* written by Paul Edwards */
/* released to the public domain */

/* routines have double underscore so that they can be
   used as part of a C library. Do a #define if not
   part of a C library */


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
