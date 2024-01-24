/* Startup code for Linux */
/* written by Paul Edwards */
/* released to the public domain */

#include "errno.h"
#include "stddef.h"

/* malloc calls get this */
#ifdef __64BIT__
static char membuf[126000000];
#else
static char membuf[31000000];
#endif
static char *newmembuf = membuf;

extern int __start(int argc, char **argv);
extern int __exita(int rc);

#ifdef NEED_MPROTECT
int __mprc;
#endif

#ifdef STACKPARM
#if !defined(__UNOPT__)
#define ADJUST -6
#elif defined(NEED_MPROTECT)
#define ADJUST -7
#else
#define ADJUST -5
#endif
#else
#define ADJUST 0
#endif

#ifdef NEED_MPROTECT
extern int __mprotect(void *buf, size_t len, int prot);

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#endif

/* We can get away with a minimal startup code, plus make it
   a C program. There is no return address. Instead, on the
   stack is a count, followed by all the parameters as pointers */

#ifdef __64BIT__

#ifdef __MACOS__
/* Note that this can be made the entry point if you
   wish, but in the future we may be bypassing the
   dynamic loader and need access to the stack as
   provided by the kernel, so it is best to keep the
   entry point __pdpstart. But currently, the dynamic
   loader appears to have first crack and we end up
   receiving a simple argc and argv */

int _start(int argc, char **argv)

#elif defined(__ARM__)
int _start(char *a, char *b, char *d, char *e,
           char *f, char *g, char *h, char *i,
           char *p)
#else

#ifdef __FORCE_ALL_64__
int _start(char *p)
#else
int _start(char *a, char *b, char *c, char *d, char *e, char *f, char *p)
#endif

#endif

#else
int _start(char *p)
#endif
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
    __mprc = __mprotect(newmembuf,
                        numblks * blksize,
                        PROT_READ | PROT_WRITE | PROT_EXEC);
    /* can't return here - we'll check return code in start */
#endif

    /* I don't know what the official rules for ARM are, but
       looking at the stack on entry showed that this code
       would work */

    /* The official rules for 64-bit are for parameters to
       be passed in x0 to x7. But not sure how that translates
       to the actual stack */

#if defined(__MACOS__)

    /* We pass on the simple argc/argv */
    __start(argc, argv);

#elif defined(__ARM__) && defined(__64BIT__)

    rc = __start(*(int *)(&p), &p);

#elif defined(__ARM__)

#if defined(__UNOPT__)
    /* these hardcoded numbers are dependent on the number of
       stack parameters defined above */
#ifdef NEED_MPROTECT
    rc = __start(*(int *)(&p + 7 + ADJUST), &p + 8 + ADJUST);
#else
    rc = __start(*(int *)(&p + 5 + ADJUST), &p + 6 + ADJUST);
#endif

#else
    /* the optimized version doesn't appear to be dependent on
       the number of stack parameters defined above */
    rc = __start(*(int *)(&p + 6 + ADJUST), &p + 7 + ADJUST);
#endif

/* Note that a problem with this stack manipulation was found when
   built with clang with optimization on. clang correctly determines
   that it is undefined behavior (this sort of stack manipulation is),
   but instead of doing what the programmer clearly intended to do
   (ie the result you get without optimization), it decides to silently
   drop the first parameter and pass whatever rubbish is on the stack
   as argc. And they don't see anything wrong with that!
   https://github.com/llvm/llvm-project/issues/61112
   I don't consider my code to be wrong (although it is obviously not
   portable - I'm writing a C library, not a C program), so if you wish
   to use clang, then switch off optimization. Otherwise use a better
   compiler, not one "supported" by jackasses on the internet who have
   no concept of the spirit of C.
*/

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
