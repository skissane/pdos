/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pload.c - Load PDOS/370                                          */
/*                                                                   */
/*  When this (main) program is entered, interrupts are disabled,    */
/*  and this program needs to find the PDOS program and load it into */
/*  memory, and transfer control, again with interrupts disabled.    */
/*                                                                   */
/*  The way it does this is that when doing an I/O, it only          */
/*  enables the I/O then and there, and waits on it to complete,     */
/*  at which point it is disabled again.                             */
/*                                                                   */
/*  The operating system itself won't be able to get away with this  */
/*  simplistic logic because it needs to deal with timer interrupts  */
/*  etc.                                                             */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char buf[40000];

#define CHUNKSZ 18452

/* pload is a standalone loader, and thus is normally loaded
   starting at location 0. */
#define PLOAD_START 0x0

/* when pload is directly IPLed, and thus starts from the
   address in location 0, it knows to create its own stack,
   which it does at 0.5 MB in (thus creating a restriction
   of only being 0.5 MB in size unless this is changed) */
#define PLOAD_STACK (PLOAD_START + 0x080000)

/* the heap - for the equivalent of getmains - is located
   another 0.5 MB in, ie at the 1 MB location */
#define PLOAD_HEAP (PLOAD_STACK + 0x080000)

/* PDOS is loaded another 1 MB above the PLOAD heap - ie the 2 MB location */
#define PDOS_CODE (PLOAD_HEAP + 0x100000)

/* standard offset for a standalone PDPCLIB program that doesn't need
   to load itself, since it wasn't directly IPLed */
#define PDOS_ENTRY (PDOS_CODE + 0x800)

/* The heap starts 1 MB after the code (ie the 3 MB location).
   So PDOS can't be more than 1 MB in size unless this is changed */
#define PDOS_HEAP (PDOS_CODE + 0x100000)

int main(int argc, char **argv)
{
    int dev;
    char *load;
    char *start = (char *)PDOS_CODE;
    void (*entry)(void *) = (void (*)(void *))PDOS_ENTRY;
    int i;
    int j;
    /* programs using the standalone version of PDPCLIB need
       to be informed where they can allocate a heap and
       potentially other parameters (that's why there is a
       length indicator as well. dum should be 0 to indicate
       no normal MVS-style parameters */
    struct { int dum;
             int len;
             char *heap; } pblock = { 0, 4, (char *)PDOS_HEAP };
    void (*fun)(void *);

    printf("PDOS should reside on cylinder 1, head 0 of IPL device\n");
    dev = initsys();
    printf("IPL device is %x\n", dev);
    load = start;
    for (i = 0; i < 10; i++)
    {
        for (j = 1; j < 4; j++)
        {
#if 0
            printf("loading to %p from 1, %d, %d\n", load, i, j);
#endif
            rdblock(dev, 1, i, j, load, CHUNKSZ);
            load += CHUNKSZ;
        }
    }
    entry(&pblock);
#if 0
    printf("about to read first block of PDOS\n");
    rdblock(dev, 1, 0, 1, buf, sizeof buf);
    printf("got %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
    printf("and %x %x %x %x\n", buf[4], buf[5], buf[6], buf[7]);
#endif
    return (0);
}
