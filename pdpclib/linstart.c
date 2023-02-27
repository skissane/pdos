/* Startup code for Linux */
/* written by Paul Edwards */
/* released to the public domain */

#include "errno.h"
#include "stddef.h"

/* malloc calls get this */
static char membuf[31000000];
static char *newmembuf = membuf;

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
    /* but some certainly don't */
    /* there doesn't appear to be a syscall to get the page size to
       ensure page alignment (as required), and I read that some
       environments have 4k page sizes but mprotect requires 16k
       alignment. So for now we'll just go with 16k */
    size_t blksize = 16 * 1024;
    size_t numblks;

    newmembuf = membuf + blksize; /* could waste memory here */
    newmembuf = newmembuf - (unsigned int)newmembuf % blksize;
    numblks = sizeof membuf / blksize;
    numblks -= 2; /* if already aligned, we wasted an extra block */
    rc = __mprotect(newmembuf,
                    numblks * blksize,
                    PROT_READ | PROT_WRITE | PROT_EXEC);
    if (rc != 0) return (rc);
#endif

    /* I don't know what the official rules for ARM are, but
       looking at the stack on entry showed that this code
       would work */
#ifdef __ARM__

#if defined(__UNOPT__)
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
    return (newmembuf);
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
