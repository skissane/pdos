/* Startup code for Linux */
/* written by Paul Edwards */
/* released to the public domain */

#include "stddef.h"

#ifdef __MACOS__
extern int __start(int argc, char **argv);
#else
extern int __start(char *p);
#endif

extern int __exita(int rc);

/* For Linux environments, we (normally) ignore the data on the
   (abnormal) stack
   and get parameters from /proc/<pid>/command instead. This way the
   executable can be run under PDOS/386 which does not create a
   silly stack. Note that the Linux stack has no return address,
   and instead has a count, followed by all the parameters as pointers.

   However, because /proc is not available in early Linux userspace
   or in a chroot() jail, we allow the possibility of an assembler
   stub to be used, that will call this function with the address
   of the stack on entry, and then further code can take its chances
   with interpreting it if /proc/... can't be opened */

#ifdef __MACOS__
/* The dynamic loader is getting first crack so that we
   end up receiving a simple argc and argv */

int _start(int argc, char **argv)
{
    int rc;

    rc = __start(argc, argv);
    __exita(rc);
    return (rc);
}

#else

int _start(char *p)
{
    int rc;

    rc = __start(p);
    __exita(rc);

    /* normal Unix environments don't have a valid return address,
       so just loop rather than doing something random */
    for (;;);
    return (rc);
}

#endif


#if defined(__WATCOMC__)

#define CTYP __cdecl

/* this is invoked by long double manipulations
   in stdio.c and needs to be done properly */

int CTYP _CHP(void)
{
    return (0);
}

/* don't know what these are */

void CTYP cstart_(void) { return; }
void CTYP _argc(void) { return; }
void CTYP argc(void) { return; }
void CTYP _8087(void) { return; }

#endif
