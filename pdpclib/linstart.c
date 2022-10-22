/* Startup code for Linux */
/* written by Paul Edwards */
/* released to the public domain */

#include "errno.h"
#include "stddef.h"

/* malloc calls get this */
static char membuf[31000000];

extern int __start(int argc, char **argv);
extern int __exita(int rc);

#ifdef NEED_MPROTECT
extern int __mprotect(void *buf, size_t len, int prot);

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#endif

/* We can get away with a minimal startup code, plus make it
   a C program. There is no return address. Instead, on the
   stack is a count, followed by all the parameters as pointers */

int _start(char *p)
{
    int rc;

#ifdef NEED_MPROTECT
    /* make malloced memory executable */
    /* most environments already make the memory executable */
    __mprotect(membuf, sizeof membuf, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif

    /* I don't know what the official rules for ARM are, but
       looking at the stack on entry showed that this code
       would work */
#ifdef __ARM__

#ifdef __NOARGS__
    char *progname = "progname";
    rc = __start(1, &progname);
#elif defined(__UNOPT__)
    rc = __start(*(int *)(&p + 5), &p + 6);
#else
    rc = __start(*(int *)(&p + 6), &p + 7);
#endif

#else
    rc = __start(*(int *)(&p - 1), &p);
#endif
    __exita(rc);
    return (rc);
}


void *__allocmem(size_t size)
{
    return (membuf);
}


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
