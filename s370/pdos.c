/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pdos.c - Public Domain Operating System                          */
/*  (version for S/370, S/380 and S/390 IBM mainframes)              */
/*                                                                   */
/*  When this (main) program is entered, interrupts are disabled,    */
/*  and the program should not assume anything about the status of   */
/*  low memory - ie this program needs to set the interrupt vectors, */
/*  initialize timer, and enable interrupts.                         */
/*                                                                   */
/*  To cope with multitasking, and multiple CPUs, from the outset,   */
/*  the following strategy is being used:                            */
/*                                                                   */
/*  Given that most time is spent in an application, rather than     */
/*  doing OS work, we can envision that 4 CPUs are busy doing some   */
/*  calculations, e.g. LA R2,7(R2). They each have their own version */
/*  of the registers, and they have each been loaded in advance      */
/*  with virtual memory settings that only give access to real       */
/*  memory that doesn't clash with any other CPU.                    */
/*                                                                   */
/*  Each of those CPUs can generate an interrupt at any time, by     */
/*  executing an illegal instruction, or having an expired timer,    */
/*  or requesting a service.                                         */
/*                                                                   */
/*  Each time there is an interrupt, interrupts will be switched     */
/*  off. The CPU receiving the interrupt needs to save its           */
/*  registers, and return to the point at which it last dispatched   */
/*  some interruptable work.                                         */
/*                                                                   */
/*  So a function like dispatch_until_interrupt() should be called,  */
/*  and it is expected to return with an interrupt identifier.       */
/*                                                                   */
/*  Since the interrupt may be a program check etc, this task        */
/*  should remain suspended until the interrupt has been             */
/*  analyzed. So what we're after is:                                */
/*                                                                   */
/*  while (!time_to_terminate)                                       */
/*  {                                                                */
/*      dispatch_until_interrupt();                                  */
/*      if (interrupt == ILLEGAL_INSTRUCTION) time_to_terminate = 1; */
/*      else if (interrupt == TIMER)                                 */
/*      {                                                            */
/*          checkElapsedTimeAgainstAllowedTime();                    */
/*          if (over_time_limit) time_to_terminate = 1;              */
/*      }                                                            */
/*  }                                                                */
/*                                                                   */
/*  In order to effect this simplicity, what that                    */
/*  dispatch_until_interrupt function translates into is:            */
/*                                                                   */
/*  LPSW of user's application code, with interrupts enabled.        */
/*  All interrupts point to an address after that LPSW.              */
/*                                                                   */
/*  Registers are provided and loaded prior to execution of the      */
/*  LPSW. The LPSW will refer to an address in low memory so that    */
/*  no base register (ie R0) can be used.                            */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>

#include "pdosutil.h"

#include "__memmgr.h"

#define S370_MAXMB 16 /* maximum MB in a virtual address space for S/370 */
#define NUM_GPR 16 /* number of general purpose registers */
#define NUM_CR 16 /* number of control registers */
#define SEG_BLK 16 /* number of segments in a block */

#define ASPACE_ALIGN 4096 /* DAT tables need to be aligned on 4k boundary */

#define PSA_ORIGIN 0 /* the PSA starts at literally address 0 */

#define CR13_DUMMY 0x00001000 /* disable ATL memory access. This
  doesn't serve any practical purpose and should not be used. CR13
  is ignored when CR0.10 = 1 (XA mode) */

/* CR13_DUMMY is this:
x address................ xxxx x length..
0 000 0000 0000 0000 0001 0000 0 000 0000
0 123 4567 8901 2345 6789 0123 4 567 8901

ie a length of 0 (which is actually 16 MiB,
and an address of "1", which actualy means
binary 1 0000 0000 0000, ie x'1000',
pointing to nowhere special (ie location 4096)
in low memory. Due to the fact that the length
is given as 16 MiB, and the fact that CR13
is only used for memory accesses above 16 MiB,
it means that this dummy address that is
pointed to is never inspected for a genuine
segment table. The address is populated with
a "1" so that CR13 is non-zero so that
Hercules/380 will not activate the free-for-all
option.

Note that the 12 binary 0s mean that the
segment table must begin on a 4k boundary,
compared to 6 binary 0s for a page table
origin requiring a 64 byte boundary. Note
that the segment table always contains
a multiple of 16 entries, which, at 4 bytes
each, means that the end of the segment
table is always on a 64-byte boundary
suitable for a page table to follow
immediately.
*/


/*

MEMORY MAPS

real (S/390)
0-16: PDOS BTL + ATL
16-160: address space 0 private area
160-592: address spaces 1-3 private area (144 MB each)

virt (S/390)
0-5: PDOS BTL
5-15: priv BTL
15-26: PDOS ATL mostly
26-160: priv ATL


real (S/370)
0-5: PDOS BTL
5-16: unused
16-26: address space 0 private area
26-56: address spaces 1-3 private area (10 MB each)

virt (S/370)
0-5: PDOS BTL
5-15: priv BTL
15-16: PDOS reserved


real (S/380)
0-5: PDOS BTL
5-16: unused
16-26: address space 0 BTL private area
26-56: address spaces 1-3 private area (10 MB each)
64-192: address space 0 ATL private area (128 MB each)
192-576: address spaces 1-3 private area (128 MB each)

virt (S/380)
0-5: PDOS BTL
5-15: priv BTL
15-16: PDOS reserved
16-144: priv ATL (128 MB)



old real memory map:

0 = PLOAD executable
0.5 = PLOAD stack (also used by PDOS, and can extend into heap area)
1 = PLOAD heap
2 = PDOS code
3 = PDOS heap
5 = PCOMM/application code (except S/380 - where it is reserved)
9 = PCOMM/application dynamic storage (stack and heap) - except S/380
15 = system use (reserved)
16 = used to support extended addressing in S/380, else ATL memory
64 = used to support multiple ATL address spaces in S/380
80 = top of memory for non-380
320 = top of memory in S/380



old virtual memory map:

0 = system use
5 = application start
9 = dynamic storage
15 = system use
17 = application ATL memory use (start point may change)

*/


/* pload is a standalone loader, and thus is normally loaded
   starting at location 0. */
#define PLOAD_START 0x0 /* 0 MB */

/* when pload is directly IPLed, and thus starts from the
   address in location 0, it knows to create its own stack,
   which it does at 0.5 MB in (thus creating a restriction
   of only being 0.5 MB in size unless this is changed) */
/* Note that these definitions need to match corresponding
   defines in sapstart - or else make them variables and
   get the infrastructure to inform PDOS of whatever it
   needs to know. */
#define PLOAD_STACK (PLOAD_START + 0x080000) /* 0.5 MB */

/* the heap - for the equivalent of getmains - is located
   another 0.5 MB in, ie at the 1 MB location */
#define PLOAD_HEAP (PLOAD_STACK + 0x080000) /* 1 MB */

/* PDOS is loaded another 1 MB above the PLOAD heap - ie the 2 MB location */
#define PDOS_CODE (PLOAD_HEAP + 0x100000) /* 2 MB */

/* The heap starts 1 MB after the code (ie the 3 MB location).
   So PDOS can't be more than 1 MB in size unless this is changed.
   Note that PDOS doesn't bother to create its own stack, and instead
   relies on the one that pload is mandated to provide being 
   big enough, which is a fairly safe bet given that it is
   effectively 1.5 MB in size, since it can eat into the old heap
   if required. */
#define PDOS_HEAP (PDOS_CODE + 0x100000)  /* 3 MB */

/* approximately where pcomm executable is loaded to */
#define PCOMM_LOAD (PDOS_HEAP + 0x200000) /* 5 MB */

/* entry point for MVS executables can be at known location 8
   into mvsstart, so long as that is linked first. In the future,
   this information will be obtained from the entry point as
   recorded in the IEBCOPY unload RDW executable */
/* we will round pcomm to be on a 64k boundary */
#define PCOMM_ENTRY (PCOMM_LOAD + 0x10000 + 8)

/* where to get getmains from - allow for 4 MB executables */
#define PCOMM_HEAP (PCOMM_LOAD + 0x400000) /* 9 MB */

#define PDOS_STORINC 0x080000 /* storage increment */

#define PCOMM_ATL_START 0x1100000

#define CONFIG_STARTCYL 2 /* cylinder where config.sys can be found -
  this assumes that PLOAD and PDOS are less than 1 cylinder in
  size and that they have been allocated on cylinder boundaries */

/* mandate maximum address space for z/Arch */
#if defined(ZARCH)
#ifndef ZAM31
#define MAXASIZE 4080 /* 16 less than theoretical maximum of 4096 */
#else
/* #define MAXASIZE 2032 */ /* 16 less than theoretical maximum of 2048 */
#define MAXASIZE 128
#endif
#define MAXANUM 1
#define PAGES_PER_SEGMENT (1024U * 1024 / 4096)
#endif

/* trim down storage requirements for S/390 */
#if defined(S390)
#define MAXASIZE 32
#define MAXANUM 1
#endif

/* trim down storage requirements for S/370 */
#if defined(S370)
#define MAXASIZE 16
#define MAXANUM 1
#define EA_ON 0
#endif

#ifndef EA_ON
#if !defined(S390) && !defined(ZARCH)
#define EA_ON 1 /* use extended addressing in 370 and 380 mode */
#else
#define EA_ON 0 /* only supported in 380 mode */
#endif
#endif

#define EA_CR 10 /* hopefully we don't need this control register */
#define EA_OFFSET 0x01000000 /* where extended addressing starts */
#define EA_END 0x04000000 /* where extended addressing ends */

#ifdef ZARCH
#define BTL_PRIVSTART 0xc00000 /* private region starts at 12 MB */
#define BTL_PRIVLEN 3 /* how many MB to give to private region */
#else
#define BTL_PRIVSTART PCOMM_LOAD /* private region starts at 5 MB */
#define BTL_PRIVLEN 10 /* how many MB to give to private region */
#endif

#ifndef MAXASIZE
#if defined(S390)
#define MAXASIZE 160
#elif defined(S370)
#define MAXASIZE 16
#else
#define MAXASIZE 144 /* maximum of 144 MB for address space */
                     /* note that this needs to be a multiple of 16
                        in order for the current logic (MAXASIZE/SEG_BLK)
                        to work */
#endif
#endif

#ifndef MAXANUM
#define MAXANUM  4   /* maximum of 4 address spaces */
#endif

/* do you want to debug all SVC calls? */
#ifndef SVCDEBUG
#define SVCDEBUG 0
#endif

/* do you want to debug all memory requests */
#ifndef MEMDEBUG
#define MEMDEBUG 0
#endif

/* if you want to detect memory leaks */
#ifndef REPORT_MEMLEAK
#define REPORT_MEMLEAK 0
#endif

/* do you want to debug all disk requests */
#ifndef DSKDEBUG
#define DSKDEBUG 0
#endif


#define MAXPAGE 256 /* maximum number of pages in a segment */


/* The S/380 architecture provides a lot of options, and they
   can be selectively enabled here.
   
   First, here is a description of S/380.
   
   If CR0.10 (XA) is off and CR13 = 0, then ATL memory accesses are 
   done to real storage, ignoring storage keys (for ATL accesses only),
   even though (S/370) DAT is switched on. This provides instant 
   access to ATL memory for applications, before having to make 
   any OS changes at all. This is not designed for serious use 
   though, due to the lack of integrity in ATL memory for multiple 
   running programs. It does provide very quick proof-of-concept though.
   
   If in S/370 mode (CR0.10 off), and CR13 is non-zero, e.g. even 
   setting a dummy value of 0x00001000, then ATL memory accesses 
   are done via a DAT mechanism instead of using real memory.
   
   With the above dummy value, the size of the table is only
   sufficient to address BTL memory (ie 16 MB), so that value of 
   CR13 (note that CR13 is of the same format as CR1) effectively 
   prevents access to ATL memory completely.
   
   When CR13 contains a length signifying more than 16 MB of
   memory, then that is used to access ATL memory, and CR1 is
   ignored. Also CR0 is ignored as far as 64K vs 1 MB segments
   is concerned. 1 MB is used unconditionally (for ATL memory).

   However, even if CR13 is non-zero, it is ignored if CR0.10
   is set to 1. Anyone who sets CR0.10 to signify XA DAT is
   expected to provide the sole DAT used, and not be required
   to mess around with CR13. This provides the flexibility of
   some ("legacy") tasks being able to use split DAT, while
   others can be made to use a proper XA DAT.
   
   
   PDOS/380 has options to exercise a lot of the above.
   
   BTL_XA will set CR13 to the dummy value and set up a pure
   XA DAT. Otherwise 370 mode will be used below, and CR13
   will point to a proper sized XA DAT.
   
   SEG_64K will force either the BTL XA DAT, or the 370 DAT,
   to use 64K segments instead of the normal 1 MB.
   
*/

/* The S/370 architecture offers 1 MB and 64K segments, and
   either may be selected here. It is also possible to
   enable extended addressing, which will allow multiple BTL
   address spaces.
*/

/* The S/390 normally only provides 1 MB segments, but with
   Hercules/380 you can also have 64K segments should that
   serve some purpose.
*/



#if !defined(SEG_64K)
/* this can be used to set 64K segments as MVS 3.8j does */
#define SEG_64K 0
#endif

#if !defined(BTL_XA)
/* S/380 allows a split DAT. The most usual reason for using
   this is to allow S/370 addressing below the line, and
   XA addressing above the line. However, there is nothing in
   the architecture that prevents XA DAT from being used below
   the line, and this could theoretically be of some use in
   some circumstances. So setting BTL_XA will set up the page
   tables for XA use */
#define BTL_XA 0
#endif

#if SEG_64K
#undef MAXPAGE
#define MAXPAGE 16
#endif


#if EA_ON
#if (MAXANUM * BTL_PRIVLEN) > (EA_END - S370_MAXMB)
#error too many address spaces defined of given length
#endif
#endif

#if MAXASIZE % 16 != 0
#error maximum address size must be multiple of 16
#endif


/* event control block */
typedef int ECB;


typedef struct {
    int abend;
} TASK;

typedef struct cdentry {
    char filler1[8];
    char cdname[8]; /* name of module */
    void *cdentpt; /* entry point of module */
} CDENTRY;

/* note that RB has another 64 bytes in a prefix area */
typedef struct rb {
    char filler1[12];
    CDENTRY *rbcde1; /* this is actually a 3-byte address */
    /* the following stuff is not in correct order/place */
    int regs[NUM_GPR];
    unsigned int psw1;
    unsigned int psw2;
    struct rb *rblinkb; /* this is really 3 bytes */
    ECB *postecb;
    char *next_exe; /* next executable's location */
    int savearea[20]; /* needs to be in user space */
    char *pptrs[1];
} RB;

typedef struct {
    char tioelngh;
} TIOENTRY;

typedef struct {
    char unused[24];
    TIOENTRY tioentry;
} TIOT;

/* note that the TCB has another 32 bytes in a prefix area */
typedef struct {
    RB *tcbrbp;
    char unused1[8];
    TIOT *tcbtio;
    int tcbcmp;
} TCB;

typedef struct {
    char unused1[232];
    int trkcalc; /* TRKCALC function */
} CVT;

typedef struct {
    char filler1[108];
    void *ascbasxb;
} ASCB;

typedef struct {
    char unused1a[16];
    CVT *cvt;
    char unused1b[12];
    unsigned int svcopsw[2];
    char unused2[96];
    unsigned int svc_code;
    char unused[244];
    int flcgrsav[NUM_GPR];
    char unused3[92];
    TCB *psatold;
    ASCB *psaanew;
    ASCB *psaaold;
} PSA;

typedef struct {
    char unused1a[5];
    char dcbfdad[8];
    char unused1b[3];
    /* this vtape is non-standard (ie different from MVS) */
    /* MVS appears to use it for TRBAL, but mvssupa.asm doesn't */
    int  vtape; /* device number if this a RECFM=V tape to be auto-converted */
    /* looks like this is dcbbufno - set to 1 buffer */
    /* I don't see the code in mvssupa.asm */
    int  used3; /* set to 1 by something */
    /* dcbdsorg - 0x4000 is PS - physical sequential */
    int  used2; /* set to 0x4000 by something */
    /* address of old deb is 1??? offset x'1c' */
    int  used1; /* set to 1 by something */
    int  eodad;
    /* offset x'24' has both dcbrecfm in first byte, and exit list
       as an address */
    union
    {
        char dcbrecfm;
        int dcbexlsa;
    } u2;
    char dcbddnam[8];
    union {
        char dcboflgs;
        int dcbgetput; /* offset 48+1 */
    } u1;
    /* what these unusual (+1) offsets mean is that the value is an
       AL3 so you need to ignore the first byte - the members are
       not misaligned - no packing is required because they are
       naturally aligned */
    int dcbcheck; /* offset 52+1 */
    /* x'38' has synad, x'3c' has TCAM flags, x'3e' is blocksize */
    char unused3[6];
    short dcbblksi;
    /* x'40' seems to be internal use. also x'44', x'48', x'4c' */
    /* so we will try using x'40' for generic device */
    int gendev;
    char unused4[14];
    /* x'52' is lrecl */
    short dcblrecl;
    int dcbnotepoint; /* offset 84+1 */
} DCB;

typedef struct {
    char filler1[30-16]; /* this 16 comes from IOBSTDRD +++ - fix */
    short residual;
} IOB;

typedef struct {
    char unused1[20];
    short ds4dstrk;
    char unused2[74];
} DSCB4;

typedef struct {
    char ds1dsnam[44]; /* dataset name */
    char ds1fmtid; /* must be set to '1' */
    char ds1dssn[6]; /* volser */
    char ds1volsq[2]; /* volume sequence number */
    char ds1credt[3]; /* creation date */
    char ds1expdt[3]; /* expiry date */
    char ds1noepv; /* number of extents */
        /* The count does not include the label track, if any. If the
           first extent type is x'40', then the data set has LABEL=SUL, and
           in (MVS) limited to 15 extents. */
        
    char ds1nobdb; /* number of free bytes in the last block of a
                      PDS. Hercules currently using random values
                      like 0x78 */
    char ds1flag1; 
    char ds1syscd[13]; /* system code - presumably name of operating system */
    char ds1refd[3]; /* reference date - ie last time accessed */
    char ds1smsfg;
    char ds1scxtf;
    short ds1scxtv;
    short ds1dsorg; /* dataset organization, e.g. PS or PO */
    char ds1recfm; /* record format, e.g. F/V/U */
    char ds1optcd; /* mainly used for BDAM and ISAM */
    short ds1blkl; /* block size */
    short ds1lrecl; /* logical record length */
    char ds1keyl;
    char ds1rkp[2];
    char ds1dsind; /* flags, e.g. is this the last volume for this dataset? */
    char ds1scal1; /* is this a cylinder request? */
    char ds1scal3[3]; /* size of secondary allocation */
    char ds1lstar[3]; /* last TTR actually used - ie the EOF record */
    char ds1trbal[2]; /* TRKCALC??? */
    char resv1; /* reserved */
    char ds1ttthi; /* a extra byte for TTR number, ie make lstart TTTR */
    char ds1ext1[2]; /* first extent - actualy 10 bytes, but we use 
                        different names currently. First byte is an
                        extent indicator, second byte is the extent
                        sequence number. Then we have start CCHH and
                        end CCHH (4 bytes each) */
    char startcchh[4];
    char endcchh[4];
    char ds1ext2[10]; /* second extent */
    char ds1ext3[10]; /* third extent */
    char ds1ptrds[5]; /* CCHHR pointing to a format-3
                         DSCB which allows unlimited chaining so your
                         dataset can grow to fill the disk */
    /* format 3 layout can be found here:
https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.idas300/s3022.htm
    */
} DSCB1;



/* A S/370 logical address consists of a segment index, which is
   bits 8-11 (for 1MB index), for the 16 possible values, then
   a page index, bits 12-19, ie 8 bits, ie 256 entries
   and then a byte index, bits 20-31, ie 12 bits, ie up to 4095 */

/* For S/370XA it is bits 1-11, ie 11 bits, ie 2048 segments,
   with other values same as S/370. */

/* There are 2 main control registers of interest.

   CR0 determines things like the segment size and whether XA 
   mode is on or not.
   
   CR1 points to a set of DAT translation tables.
   
   In S/380, there is also a CR13 which is similar to CR1,
   but is only used for ATL memory access.
*/

typedef unsigned short UINT2;
typedef unsigned int UINT4;
typedef unsigned int UINT8[2];


/* for CR0: */
/* bits 8-9 = 10 (4K pages), bits 11-12 = 10 (1 MB segments) */
/* for S/370XA, bit 10 also needs to be 1 */
/* 
              0123 4567 89AB CDEF
   00800000 = 0000 0000 1000 0000  4k pages
   00900000 = 0000 0000 1001 0000  1M segments
   00A00000 = 0000 0000 1010 0000  XA dat (CR0.10 = 1)
   00B00000 = 0000 0000 1011 0000  4k, 1M and XA
*/

#if defined(S390)
static int cr0 = 0x00B00000;
#elif BTL_XA && SEG_64K
static int cr0 = 0x00A00000; /* 64K, S/390 */
#elif BTL_XA && !SEG_64K
static int cr0 = 0x00B00000; /* 1MB, S/390 */
#elif !BTL_XA && SEG_64K
static int cr0 = 0x00800000; /* 64K, S/370 */
#elif !BTL_XA && !SEG_64K
static int cr0 = 0x00900000; /* 1MB, S/370 */
#endif


/* for CR1: */
/* bits 0-7 = number of blocks (of 16) segment table entries */
/* specify 1 less than what you want, because there's an
   automatic + 1 */
/* bits 8-25, plus 6 binary zeros = 24-bit address of segment table */
/* for 16 MB, using 1 MB segments, we only need 1 block */
/* note that MVS 3.8j uses 64k segments rather than 1 MB segments,
   but we don't care about that here */
/* for S/370XA, it is bits 1-19, plus 12 binary zeros, to give
   a 31-bit address. Also bits 25-31 have the length as before */
/*static int cr1 = 0x01000000;*/ /* need to fill in at runtime */



/* the S/370 and S/390 hardware requires a 4-byte integer for
   segment table entries. */

/* for S/370, bits 0-3 have length, with an amount 1 meaning 1/16 of the 
   maximum size of a page table, so just need to specify 1111 here */
/* bits 8-28, plus 3 binary zeros = address of page table */
/* bit 31 needs to be 0 for valid segments */
/* so this whole table is only 128 bytes (per address space) */

/* for S/370XA this changes to bits 1-25 having the page table
   origin, with 6 binary zeros on the end, giving a 31-bit address.
   Bit 26 is in the invalid segment bit.
   Also bits 28-31 have the length, according to the same length
   rules as CR1 - ie blocks of 16, with an implied + 1,
   giving a maximum of 16*16 = 256
   page entries, sufficient (256 * 4096) to map the 1 MB segment,
   so just set to 1111 */
/* the segment table will thus need 2048 + 16 = approx 8K in size */

typedef UINT4 SEG_ENT370;
#ifdef ZARCH
typedef UINT8 SEG_ENTRY;
#else
typedef UINT4 SEG_ENTRY;
#endif

/* the S/370 hardware requires a 2-byte integer for page
   table entries. S/370XA requires 4-byte */

/* for S/370, bits 0-11, plus 12 binary zeros = real memory address */
/* bit 12 = 0 for valid pages. All other bits to be 0 */
/* unless extended addressing for 370 is in place. Then bits 13
   and 14 become bits 6 and 7 of a 26-bit address */
/* so array dimensions are 16 = 16 segments */
/* and with each page entry addressing 4096 bytes, we need
   1024*1024/4096 = 256 to address the full 1 MB */
/* this whole table is only 8K (per address space) */

/* with S/370XA this becomes a 4-byte integer, with bits
   1-19 containing an address (when you add 12 binary zeros on
   the end to give a total of 19+12=31 bits) */
/* the page table will thus become 2048 instead of 16, and
   thus be 2 MB in size. Since this is so large, a #define
   is required to give a more reasonable maximum address
   space at compile time (or else run time). */
/* for example, 4 address spaces, each 128 MB in size, is
   equal to 256K */

typedef UINT2 PAGE_ENT370;
#ifdef ZARCH
typedef UINT8 PAGE_ENTRY;
#else
typedef UINT4 PAGE_ENTRY;
#endif




/* for S/380, we have a mixed/split DAT. CR1 continues normal
   S/370 behaviour, but if CR13 is non-zero, it uses an XA dat
   for any storage references above 16 MB. Note that the first
   16 MB should still be included in the XA storage table, but
   they will be ignored (unless CR0.10 is set to 1 - with DAT
   still on, which switches off 370 completely) */

/* one address space */

typedef struct {

#if !defined(S370) && !defined(ZARCH)
#if SEG_64K
    SEG_ENTRY segtable[MAXASIZE*16]; /* needs 4096-byte alignment */
    PAGE_ENTRY pagetable[MAXASIZE*16][MAXPAGE]; /* needs 64-byte alignment */
#else
    SEG_ENTRY segtable[MAXASIZE]; /* needs 4096-byte alignment */
      /* segments are in blocks of 64, so will remain 64-byte aligned */
    PAGE_ENTRY pagetable[MAXASIZE][MAXPAGE]; /* needs 64-byte alignment */
#endif
#endif

#if !defined(S390) && !defined(ZARCH)
#if SEG_64K
    SEG_ENT370 seg370[S370_MAXMB*16]; /* needs 64-byte alignment */
    PAGE_ENT370 page370[S370_MAXMB*16][MAXPAGE]; /* needs 8-byte alignment */
#else
    SEG_ENT370 seg370[S370_MAXMB]; /* needs 64-byte alignment */
    PAGE_ENT370 page370[S370_MAXMB][MAXPAGE]; /* needs 8-byte alignment */
#endif
#endif

#if defined(ZARCH)
    /* this requires 4k alignment */
    UINT8 region1[512]; /* 1 entry plus padding */
    /* this requires 4k alignment */
    UINT8 region2[512]; /* 1 entry plus padding */
    /* this requires 4k alignment */
    UINT8 region3[512]; /* 4 lots of 2 GB plus padding */
    /* this requires 4k alignment */
    SEG_ENTRY segtable[4096 * 2]; /* we need double the mappings so that
                                         4-8 GiB gets mapped to 0-4 GiB */
    /* I think this requires 2k alignment, but will be 4k regardless */
    PAGE_ENTRY pagetable[4096 * PAGES_PER_SEGMENT];
#endif

    int cregs[NUM_CR];

    MEMMGR btlmem; /* manage memory below the line */
    MEMMGR atlmem; /* manage memory above the line */

    RB first_rb; /* first request block */
    RB *curr_rb; /* current request block */
    TCB tcb;
} ONESPACE;


/* each address space structure starts with the required
   DAT, which needs to be 4k-aligned, so we make sure there
   is a filler */

typedef struct {
    ONESPACE o;
    
    /* because the DAT tables need to be 4k-byte aligned, we
       need to split the structure in two and add a filler here */
    char filler[ASPACE_ALIGN + ASPACE_ALIGN 
                - (sizeof(ONESPACE) % ASPACE_ALIGN)];
} ASPACE;


#define DCBOFOPN 0x10
#define DCBRECU  0xC0
#define DCBRECF  0x80
#define DCBRECV  0x40
#define DCBRECBR 0x10

#define INTERRUPT_SVC 1

#if 0
TASK *task;
#endif

#define CHUNKSZ 18452 /* typical block size for files */

#define MAXBLKSZ 32767 /* maximum size a disk block can be */

typedef struct {
    ASPACE aspaces[MAXANUM]; /* needs to be 8-byte aligned because
         the segment points to a page table, using an address
         with 3 binary 0s appended. Due to the implied 6 binary 0s
         in XA segment tables, 64-byte alignment is required */
    RB *context; /* current thread's context */
    PSA *psa;
    int exitcode;
    int shutdown;
    int ipldev;
    int iplregs[17]; /* 16 registers plus an extra for NUL-terminator */
    int curdev;
    int curr_aspace; /* current address space */
} PDOS;

/*static PDOS pdos;*/

static DCB *gendcb = NULL; /* +++ need to eliminate this state info */
static IOB geniob; /* +++ move this somewhere */
static char lastds[FILENAME_MAX]; /* needs to be in TIOT */
static int memid = 256; /* this really belongs in the address space */
static int cons_type = 3270; /* do we have a 1052 or 3215? */
static unsigned char *ramdisk = NULL;

void gotret(void);
int adisp(void);
void dread(void);
void dwrite(void);
void dcheck(void);
void dnotpnt(void);
void dexit(int oneexit, DCB *dcb);
void trkclc(void);
void datoff(void);
void daton(void);
extern int __consdn;
extern int __istape;
extern int __iscard;
extern int __ismem;
int wrblock(int dev, int cyl, int head, int rec, void *buf, int len, int cmd);
int rdblock(int dev, int cyl, int head, int rec, void *buf, int len, int cmd);

int pdosRun(PDOS *pdos);
void pdosDefaults(PDOS *pdos);
int pdosInit(PDOS *pdos);
void pdosTerm(PDOS *pdos);
static int pdosDispatchUntilInterrupt(PDOS *pdos);
static void pdosDumpregs(PDOS *pdos);
static void pdosInitAspaces(PDOS *pdos);
static void pdosProcessSVC(PDOS *pdos);
static void pdosSVC99(PDOS *pdos);
static int pdosDoDIR(PDOS *pdos, char *parm);
static void brkyd(int *year, int *month, int *day);
static int pdosDiskInit(PDOS *pdos, char *parm);
static int pdosFil2Dsk(PDOS *pdos, char *parm);
static int pdosDsk2Fil(PDOS *pdos, char *parm);
static int pdosNewF(PDOS *pdos, char *parm);
static int pdosGetMaxima(PDOS *pdos, int *dircyl, int *dirhead,
                         int *dirrec, int *datacyl);
static int pdosDumpBlk(PDOS *pdos, char *parm);
static int pdosZapBlk(PDOS *pdos, char *parm);
static int pdosNewBlk(PDOS *pdos, char *parm);
static int pdosRamDisk(PDOS *pdos, char *parm);
static int pdosLoadExe(PDOS *pdos, char *prog, char *parm);
static int pdosDumpMem(PDOS *pdos, void *buf, int cnt);
#if 0
static int pdosLoadPE(PDOS *pdos, char *prog, char *parm);
#endif
static int pdosLoadPcomm(PDOS *pdos);

static void split_cchhr(char *cchhr, int *cyl, int *head, int *rec);
static void join_cchhr(char *cchhr, int cyl, int head, int rec);

static void write3270(char *buf, size_t lenbuf, int cr);
static int cprintf(char *format, ...);
#define printf cprintf

int __consrd(int len, char *buf);
int __c3270r(int len, char *buf);
int __conswr(int len, char *buf, int cr);

static int getssid(int devnum);

static int int_wrblock(int dev, int cyl, int head, int rec,
                       void *buf, int len, int cmd);
int int_rdblock(int dev, int cyl, int head, int rec,
                void *buf, int len, int cmd);

static int ins_strncmp(char *one, char *two, size_t len);

#define wrblock(dev, cyl, head, rec, buf, len, cmd) \
    int_wrblock(dev, cyl, head, rec, buf, len, cmd)

#define rdblock(dev, cyl, head, rec, buf, len, cmd) \
    int_rdblock(dev, cyl, head, rec, buf, len, cmd)

int main(int argc, char **argv)
{
    PDOS *pdos;
    char *p;
    int remain;
    int ret = EXIT_FAILURE;

    /* we need to align the PDOS structure on a 4k boundary */
    p = malloc(sizeof(PDOS) + ASPACE_ALIGN);
    if (p != NULL)
    {
        remain = (unsigned int)p % ASPACE_ALIGN;
        p += (ASPACE_ALIGN - remain);
        pdos = (PDOS *)p;

        pdosDefaults(pdos);
        if (pdosInit(pdos)) /* initialize address spaces etc */
        {
            ret = pdosRun(pdos); /* dispatch tasks */
            pdosTerm(pdos);
        }
    }
    return (ret);
}


/* pdosRun - the heart of PDOS */

int pdosRun(PDOS *pdos)
{   
    int intrupt;

    while (!pdos->shutdown)
    {
        intrupt = pdosDispatchUntilInterrupt(pdos);
        if (intrupt == INTERRUPT_SVC)
        {
            pdosProcessSVC(pdos);
        }
    }

    return (pdos->exitcode);
}


/* initialize any once-off variables */
/* cannot fail */

void pdosDefaults(PDOS *pdos)
{
    pdos->psa = PSA_ORIGIN;    
    return;
}


/* initialize PDOS */
/* zero return = error */

int pdosInit(PDOS *pdos)
{
    int cyl;
    int head;
    int rec;

    pdos->ipldev = initsys();
    /* The registers as they were at IPL time are stored at FLCGRSAV
       by sapstart */
    memcpy(pdos->iplregs, (int *)0x180, 16 * 4);
    pdos->iplregs[16] = 0; /* effective NUL-terminator */
    if (__consdn == 0)
    {
        sscanf((char *)pdos->iplregs, "%x %d", &__consdn, &cons_type);
    }

/* cover the case where they have zapped a device number
   and convert it to SSID */
#if defined(S390) || defined(ZARCH)
    if ((__consdn != 0) && (__consdn < 0x10000))
    {
        __consdn = getssid(__consdn);
    }
#endif
    if (cons_type == 0)
    {
        cons_type = 3270;
    }

    if (__ismem)
    {
        /* IPL device will be ramdisk */
        pdos->ipldev = 0x20000;
        /* which should already be loaded at the 3 GiB location */
        ramdisk = (char *)(3UL * 1024 * 1024 * 1024);
    }
    pdos->curdev = pdos->ipldev;
#ifndef ZARCH
    lcreg0(cr0);
#endif
    if (__istape || __iscard || __ismem)
    {
        /* __consdn = 0x10000;
        cons_type = 3215; */
    }
    else if (findFile(pdos->ipldev, "CONFIG.SYS", &cyl, &head, &rec) != 0)
    {
        printf("config.sys missing\n");
        return (0);
    }
    if (__consdn == 0 || __istape || __iscard || __ismem)
    {
        char tbuf[MAXBLKSZ + 2];
        int cnt;

#if 0 /* DSKDEBUG */
        printf("loading to %p from %d, %d, %d\n", tbuf,
               cyl, head, rec);
#endif
        /* the chances of anyone having a herc config file more
           than a block in size are like a million gazillion to one */
        if (__istape) cnt = rdtape(pdos->ipldev, tbuf, MAXBLKSZ);
        else if (__iscard)
        {
            cnt = 0;
            while (1)
            {
                rdtape(pdos->ipldev, tbuf + cnt, 80);
                if (tbuf[cnt] == '\0')
                {
                    cnt = strlen(tbuf);
                    break;
                }
                cnt += 72;
            }
        }
        else if (__ismem)
        {
            /* config.sys should be loaded at 1 GiB location */
            char *configloc = (char *)(1 * 1024 * 1024 * 1024);

            memcpy(tbuf, configloc, strlen(configloc) + 1);
        }
        else
        cnt = rdblock(pdos->ipldev, cyl, head, rec, tbuf, MAXBLKSZ, 0x0e);
#if DSKDEBUG
        printf("cnt is %d\n", cnt);
#endif
        /* find out which device console really is */
        if ((__consdn == 0) && (cnt > 0))
        {
            char *p;
            char *q;
            char *fin;
            int ctr = 0;

            strcpy(tbuf + cnt, "\n"); /* ensure sentinel */
            fin = tbuf + cnt;
            p = tbuf;
            while (p < fin)
            {
                q = strchr(p, '\n');
                *q = '\0';
                if (isdigit((unsigned char)p[0]))
                {
                    if ((strstr(p, "3215") != NULL)
                        || (strstr(p, "3270") != NULL)
                        || (strstr(p, "1052") != NULL))
                    {
                        sscanf(p, "%x", &__consdn);
#if defined(S390) || defined(ZARCH)
                        __consdn = getssid(__consdn);
                        if (__consdn == 0)
                        {
                            __consdn = 0x10000;
                        }
#endif
                        if (strstr(p, "3215") != NULL)
                        {
                            cons_type = 3215;
                        }
                        else if (strstr(p, "3270") != NULL)
                        {
                            cons_type = 3270;
                        }
                        else
                        {
                            cons_type = 1052;
                        }
                        break;
                    }
                    ctr++;
                }
                p = q + 1;
            }
        }
    }
    printf("Welcome to PDOS!!!\n");
#if 0
    printf("IPL string is %s\n", pdos->iplregs);
    printf("IPL device is %x\n", pdos->ipldev);
    printf("IPL device is %x\n", getdevn(pdos->ipldev));
    printf("terminal 9 is %x\n", getssid(0x9));
    printf("card 1d1 is %x\n", getssid(0x1d1));
    printf("non-exists 333 is %x\n", getssid(0x333));
#endif
#if 0
    printf("PDOS structure is %d bytes\n", sizeof(PDOS));
    printf("free memory maybe starts at %p\n", malloc(500));
#endif
#if 0
    printf("CR0 is %08X\n", cr0);
    printf("PDOS structure is %d bytes\n", sizeof(PDOS));
    printf("aspace padding is %d bytes\n", sizeof pdos->aspaces[0].filler);

    printf("IPL device is %x\n", pdos->ipldev);
#endif
    pdos->shutdown = 0;
    pdos->psa->cvt = calloc(sizeof(CVT), 1);
    pdos->psa->psaaold = calloc(sizeof(ASCB), 1);
    pdos->psa->cvt->trkcalc = (int)trkclc;
    pdosInitAspaces(pdos);
    pdos->curr_aspace = 0;
    pdos->psa->psatold = &pdos->aspaces[pdos->curr_aspace].o.tcb;
    pdos->aspaces[pdos->curr_aspace].o.tcb.tcbtio = calloc(sizeof(TIOT), 1);
    pdos->aspaces[pdos->curr_aspace].o.tcb.tcbrbp = calloc(sizeof(RB), 1);
    pdos->aspaces[pdos->curr_aspace].o.tcb.tcbrbp->rbcde1
        = calloc(sizeof(CDENTRY), 1);
    memcpy(pdos->aspaces[pdos->curr_aspace].o.tcb.tcbrbp->rbcde1->cdname,
           "PROGNAME", 8);
    pdos->context =
        pdos->aspaces[pdos->curr_aspace].o.curr_rb =
        &pdos->aspaces[pdos->curr_aspace].o.first_rb;

    /* Now we set the DAT pointers for our first address space,
       in preparation for switching DAT on. */
    lcreg1(pdos->aspaces[pdos->curr_aspace].o.cregs[1]);
#if defined(S380)
    lcreg13(pdos->aspaces[pdos->curr_aspace].o.cregs[13]);
#endif
    daton();

    if (pdosLoadPcomm(pdos) != 0)
    {
        datoff();
        return (0);
    }
    return (1);
}


/* pdosTerm - any cleanup routines required */

void pdosTerm(PDOS *pdos)
{
    /* switch DAT off so that we return to the caller (probably
       pload) in the same state we were called. */
    datoff();
    return;
}


static int pdosDispatchUntilInterrupt(PDOS *pdos)
{
    int ret;

    while (1)
    {
        /* store registers and PSW in low memory to be loaded */
        memcpy(pdos->psa->flcgrsav,
               pdos->context->regs,
               sizeof pdos->context->regs);

        pdos->psa->svcopsw[0] = pdos->context->psw1;
        pdos->psa->svcopsw[1] = pdos->context->psw2;

        ret = adisp();  /* dispatch */
        
        /* restore registers and PSW from low memory */
        memcpy(pdos->context->regs,
               pdos->psa->flcgrsav,               
               sizeof pdos->context->regs);
#ifdef ZARCH
        pdos->context->psw1 = 0x040C0000U;
        pdos->context->psw1 |= (*(unsigned int *)0x140 & 1);
        pdos->context->psw2 = *(unsigned int *)0x14C;
        pdos->context->psw2 |= (*(unsigned int *)0x144 & 0x80000000U);
#else
        pdos->context->psw1 = pdos->psa->svcopsw[0];
        pdos->context->psw2 = pdos->psa->svcopsw[1];
#endif
        
        if (ret == 0) break;

        /* non-zero returns are requests for services */
        /* these "DLLs" don't belong in the operating system proper,
           but for now, we'll action them here. Ideally we want to
           get rid of that while loop, and run the DLL as normal
           user code. Only when it becomes time for WRITE to run
           SVC 0 (EXCP) or CHECK to run SVC 1 (WAIT) should it be
           reaching this bit of code. It's OK to keep the code
           here, but it needs to execut in user space. */
        if (ret == 2) /* got a WRITE request */
        {
            int len;
            char *buf;
            int cr = 0;
            char tbuf[MAXBLKSZ + 8];
            char *fin;
            int l;
            char *p;
            char *q;
            char *decb;
            DCB *dcb;
            int *pptr;

            /* A write request has R1 pointing to a parameter list
               (of fullwords etc), known as the DECB.
               first is the ECB (fullword)
               then 8 bits of read flags
               then 8 bits of write flags
               then halfword length of buffer to write.
               then DCB address (fullword)
               then buffer address (fullword)
               then optional record address (fullword) */

            decb = (char *)pptr;
            pptr = (int *)pdos->context->regs[1];
            len = pptr[1] & 0xffff;
            buf = (char *)pptr[3];
            dcb = (DCB *)pptr[2];

#if 0
            printf("got a request to write block %p, len %d\n", buf, len);
#endif
            if (dcb->vtape != 0)
            {
                int cnt;

                if (len > 8)
                {
                    cnt = wrtape(dcb->vtape, buf + 8, len - 8);
                    printf("wrote %d bytes to %x\n", cnt, dcb->vtape);
                }
            }
            else if (dcb->gendev != 0)
            {
                int cnt;

                cnt = wrtape(dcb->gendev, buf, len);
                printf("wrote %d bytes to %x\n", cnt, dcb->gendev);
            }
            /* not a disk, must be terminal */
            else if (memcmp(dcb->dcbfdad,
                            "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
            {
                /* assume RECFM=U, with caller providing NL */            
                /* although this was specified to be a unit record
                   device, some applications ignore that, so be prepared
                   for more than one NL */
                printf("%.*s", len, buf);
            }
            else
            {
                int cyl;
                int head;
                int rec;
                int cnt;
                
                split_cchhr(dcb->dcbfdad + 3, &cyl, &head, &rec);
                rec++;
#if DSKDEBUG
                printf("writing from %p to %d, %d, %d\n",
                       buf,
                       cyl, head, rec);
#endif
                memcpy(tbuf, "\x00\x01\x00\x00\x0d\x2c\x00\x60", 8);
                *(short *)tbuf = cyl;
                *(short *)(tbuf + 2) = head;
                tbuf[4] = rec;
                tbuf[5] = 0;
                *(short *)(tbuf + 6) = len;
                memcpy(tbuf + 8, buf, len);

                /* record number must be one less when using 0x1d
                   destructive (of track) write (to set block size) */
                cnt = wrblock(pdos->ipldev, cyl, head, rec - 1,
                              tbuf, len + 8, 0x1d);
#if DSKDEBUG
                printf("cnt is %d\n", cnt);
#endif
                /* we get a count of 0 when full, so need to retry */
                if (cnt <= 0)
                {
                    rec = 1;
                    head++;
                    *(short *)(tbuf + 2) = head;
                    tbuf[4] = rec;
                    cnt = wrblock(pdos->ipldev, cyl, head, rec - 1,
                                  tbuf, len + 8, 0x1d);
#if DSKDEBUG
                    printf("cnt is %d\n", cnt);
#endif
                    if (cnt <= 0)
                    {
                        head = 0;
                        cyl++;
                        *(short *)tbuf = cyl;
                        *(short *)(tbuf + 2) = head;
                        cnt = wrblock(pdos->ipldev, cyl, head, rec - 1,
                                      tbuf, len + 8, 0x1d);
                    }
                }
#if DSKDEBUG
                printf("cnt is %d\n", cnt);
#endif
                join_cchhr(dcb->dcbfdad + 3, cyl, head, rec);
            }
        }
        else if (ret == 3) /* got a READ request */
        {
            int len;
            char *buf;
            char tbuf[MAXBLKSZ];
            int cnt;
            char *decb;
            DCB *dcb;
            int *pptr;

            /* A read request has R1 pointing to a parameter list
               (of fullwords etc), known as the DECB.
               first is the ECB (fullword)
               then 8 bits of read flags
               then 8 bits of write flags
               then halfword length of buffer to read.
               then DCB address (fullword)
               then buffer address (fullword)
               then optional record address (fullword) */

            pptr = (int *)pdos->context->regs[1];
            decb = (char *)pptr;
            len = pptr[1] & 0xffff;
            buf = (char *)pptr[3];
            dcb = (DCB *)pptr[2];
#if 0
            printf("dcb read is for lrecl %d\n", dcb->dcblrecl);
#endif
            if (dcb->gendev != 0)
            {
                cnt = rdtape(dcb->gendev, buf, len);
                printf("read %d bytes from %x\n", cnt, dcb->gendev);
            }
            else if (memcmp(dcb->dcbfdad,
                            "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
            {
                if (cons_type == 3270)
                {
                    cnt = __c3270r(300, tbuf);
                    if (cnt >= 6)
                    {
                        memmove(tbuf, tbuf + 6, cnt - 6);
                        cnt -= 6;
                    }
                }
                else
                {
                    cnt = __consrd(300, tbuf);
                    /* hack - PDPCLIB MVS routines cannot currently handle
                       EOF being reached (a count of 0 seems to do that,
                       which happens when the remote disconnects) and cleared,
                       so we pretend a NUL was entered by the user instead. */
                    /* also if there is a delay put into Hercules we get -1
                       instead, so cover that too */
                    if ((cnt == 0) || (cnt == -1))
                    {
                        cnt = 0;
                        tbuf[0] = 0;
                        cnt++;
                    }
                    if (cnt == 300)
                    {
                        cnt = -1;
                    }
                }
                if (cnt == -1)
                {
                    printf("unable to read from keyboard - system halted\n");
                    for (;;) ;
                }
                if ((cnt >= 0) && ((cons_type == 3215) || (cons_type == 3270)))
                {
                    tbuf[cnt++] = '\n';
                }
            }
            else
            {
                int cyl;
                int head;
                int rec;
                
                split_cchhr(dcb->dcbfdad + 3, &cyl, &head, &rec);
                rec++;
#if DSKDEBUG
                printf("reading into %p from %d, %d, %d\n",
                       buf,
                       cyl, head, rec);
#endif
                cnt = rdblock(pdos->ipldev, cyl, head, rec, 
                              tbuf, len, 0x0e);
#if DSKDEBUG
                printf("cnt is %d\n", cnt);
#endif
                if (cnt < 0)
                {
                    rec = 1;
                    head++;
                    cnt = rdblock(pdos->ipldev, cyl, head, rec, 
                                  tbuf, len, 0x0e);
#if DSKDEBUG
                    printf("cnt is %d\n", cnt);
#endif
                    if (cnt < 0)
                    {
                        head = 0;
                        cyl++;
                        cnt = rdblock(pdos->ipldev, cyl, head, rec, 
                                      tbuf, len, 0x0e);
                    }
                }
#if DSKDEBUG
                printf("cnt is %d\n", cnt);
#endif
                join_cchhr(dcb->dcbfdad + 3, cyl, head, rec);
            }
            if (cnt <= 0)
            {
                /* need to return to EOF routine - don't need to
                   return here */
                pdos->context->psw2 = ((dcb->eodad & 0x7fffffff)
                                       | (pdos->context->psw2 & 0x80000000));
            }
            else
            {
                memcpy(buf, tbuf, cnt);
                geniob.residual = (short)(len - cnt);
                *(IOB **)(decb + 16) = &geniob;
            }
        }
    }
    return (INTERRUPT_SVC); /* only doing SVCs at the moment */
}


static void pdosDumpregs(PDOS *pdos)
{
    int x;
    int alt1;
    int p;
    int alt2;
    
    p = pdos->context->regs[1];
    p &= 0x7fffffff;
    if ((p >= 0) && (p < MAXASIZE * 1024U * 1024))
    {
        /* nothing */
    }
    else
    {
        p = 0;
    }
    for (x = 0; x < 16; x++)
    {
        alt1 = pdos->context->regs[x];
        alt1 &= 0x7fffffff;
        if ((alt1 >= 0) && (alt1 < MAXASIZE * 1024U * 1024))
        {
            alt1 = *(int *)alt1;
        }
        else
        {
            alt1 = -1;
        }        
        alt2 = ((int *)p)[x];
        alt2 &= 0x7fffffff;
        if ((alt2 >= 0) && (alt2 < MAXASIZE * 1024U * 1024))
        {
            alt2 = *(int *)alt2;
        }
        else
        {
            alt2 = -1;
        }        
        printf("register %2d is %08X, alt %08X, parm %08X %08X\n",
               x, pdos->context->regs[x], alt1, ((int *)p)[x], alt2);
    }
    return;
}


/* we initialize all address spaces upfront, with fixed locations
   for all segments. The whole approach should be changed though,
   with address spaces created dynamically and the virtual space
   not being allocated to real memory until it is actually
   attempted to be used. */

static void pdosInitAspaces(PDOS *pdos)
{
    int s;
    int a;
    int p;

#ifndef ZARCH
    /* initialize 370-DAT tables */
#if (defined(S370) || defined(S380)) && !BTL_XA
    for (a = 0; a < MAXANUM; a++)
    {
#if SEG_64K
        for (s = 0; s < (S370_MAXMB * 16); s++)
#else
        for (s = 0; s < S370_MAXMB; s++)
#endif
        {
            /* we only store the upper 21 bits of the page
               table address. therefore, the lower 3 bits
               will need to be truncated (or in this case,
               assumed to be 0), and no shifting or masking
               is required in this case. The assumption of
               0 means that the page table must be 8-byte aligned */
            pdos->aspaces[a].o.seg370[s] = (0xfU << 28)
                                 | (unsigned int)pdos->aspaces[a].o.page370[s];
            for (p = 0; p < MAXPAGE; p++)
            {
                int real = 0; /* real memory location */
                int extrabits = 0;

                /* because the address starts in bit 0, it is
                   8 bits too far to be a normal 24-bit address,
                   otherwise we could shift 20 bits to get 1 MB.
                /* and 12 bits to move the 4K page into position */
                /* However, since we only have a 16-bit target,
                   need to subtract 16 bits in the shifting.
                   But after allowing for the 8 bits, it's only
                   8 that needs to be subtracted */
                /* Also if we're using extended addressing, place
                   private region up there */
                real =
#if SEG_64K
                       (s << (20-4))
#else
                       (s << 20)
#endif
                       | (p << 12);

                /* the private areas are all put into the EA
                   region which starts at 16 MB */
#if EA_ON
#if SEG_64K
                if ((s * 1024 * 1024/16 >= BTL_PRIVSTART) 
                    && (s * 1024 * 1024/16 < 
                        (BTL_PRIVSTART + BTL_PRIVLEN * 1024 * 1024)))
#else
                if ((s * 1024 * 1024 >= BTL_PRIVSTART) 
                    && (s * 1024 * 1024 < 
                        (BTL_PRIVSTART + BTL_PRIVLEN * 1024 * 1024)))
#endif
                {
                    real += EA_OFFSET + a * BTL_PRIVLEN * 1024 * 1024;
                }

                if (real >= (S370_MAXMB * 1024 * 1024))
                {
                    /* get bits 6 and 7 of 26-bit address */
                    /* the - 1 is to position rightmost bit on bit 14 */
                    extrabits = (real & 0x03000000) >> (24 - 1);
                    /* trim to 24-bit address */
                    real = real & 0x00FFFFFF;
                }
#endif
                pdos->aspaces[a].o.page370[s][p] = 
                    ((real >> 8) | extrabits) & 0xFFFF;
            }
        }

        /* 1 - 1 is for 1 block of 16, minus 1 implied */
        pdos->aspaces[a].o.cregs[1] = 
#if SEG_64K
                               ((S370_MAXMB - 1) << 24)
#else
                               ((1 - 1) << 24)
#endif
                               | (unsigned int)pdos->aspaces[a].o.seg370;
        /* note that the CR1 needs to be 64-byte aligned, to give
           6 low zeros */
#if 0
        printf("aspace %d, seg370 %p, cr1 %08X\n",
               a, pdos->aspaces[a].o.seg370, pdos->aspaces[a].o.cregs[1]);
#endif
        /* now set up the memory manager for BTL storage */
        lcreg1(pdos->aspaces[a].o.cregs[1]);
        daton();
        memmgrDefaults(&pdos->aspaces[a].o.btlmem);
        memmgrInit(&pdos->aspaces[a].o.btlmem);
        memmgrSupply(&pdos->aspaces[a].o.btlmem,
                     (char *)BTL_PRIVSTART,
                     BTL_PRIVLEN * 1024 * 1024);
        datoff();
    }
#endif


    /* initialize XA-DAT tables */
#if defined(S380)
    for (a = 0; a < MAXANUM; a++)
    {
/* normally even if it is SEG_64K, we ignore that for the ATL DAT
   tables, because ATL is still 1 MB segments. However, if we're
   using XA BTL, then we accept it ATL too. */
#if SEG_64K && BTL_XA
        for (s = 0; s < (MAXASIZE * 16); s++)
#else
        for (s = 0; s < MAXASIZE; s++)
#endif
        {
            int adjust = 0;

/* If EA is on, it is mixed DAT, and thus implied 1M segment size */
/* If we're not using EA, then just map all address spaces onto
   the same one, implying that a single address space should be
   used. */
#if EA_ON
            if (s >= S370_MAXMB)
            {
                /* steer clear of the bottom 64 MB */
                adjust = EA_END / (1024 * 1024);
                /* for the ATL portion of the virtual address, adjust
                   appropriately */
                adjust += a * (MAXASIZE - S370_MAXMB) - S370_MAXMB;
            }
#endif

            /* no shifting of page table address is required,
               but the low order 12 bits must be 0, ie address must
               be aligned on 64-byte boundary */
            pdos->aspaces[a].o.segtable[s] = 
#if SEG_64K && BTL_XA
                  0x0U
#else
                  0xfU
#endif
                  | (unsigned int)pdos->aspaces[a].o.pagetable[s];
            for (p = 0; p < MAXPAGE; p++)
            {
                /* because the address begins in bit 1, just like
                   a 31-bit bit address, we just need to shift
                   20 bits (1 MB) to get the right address. Plus
                   add in the page number, by shifting 12 bits
                   for the 4K multiple */
                pdos->aspaces[a].o.pagetable[s][p] = 
#if SEG_64K && BTL_XA
                                  (s << 16)
#else
                                  ((s + adjust) << 20)
#endif
                                  | (p << 12);
            }
        }
#if defined(S380) && !BTL_XA
        /* S/380 references XA-DAT memory via CR13, not CR1 */
        pdos->aspaces[a].o.cregs[13] =
#else
        pdos->aspaces[a].o.cregs[1] = 
#endif
            /* - 1 because architecture implies 1 extra block of 16 */
#if SEG_64K && BTL_XA
            (MAXASIZE/SEG_BLK * 16 - 1)
#else
            (MAXASIZE/SEG_BLK - 1)
#endif
            | (unsigned int)pdos->aspaces[a].o.segtable;
            /* note that the CR1 needs to be 4096-byte aligned, to give
               12 low zeros */
#if defined(S380) && BTL_XA
        /* set a dummy CR13 since we're using XA below the line,
           and don't want the simple version of S/380 to take effect */
        pdos->aspaces[a].o.cregs[13] = CR13_DUMMY;
#endif
#if 0
        printf("aspace %d, seg %p, cr13 %08X\n",
               a, pdos->aspaces[a].o.segtable, pdos->aspaces[a].o.cregs[13]);
#endif
        /* now set up the memory manager for BTL and ATL storage */
        lcreg13(pdos->aspaces[a].o.cregs[13]);
        lcreg1(pdos->aspaces[a].o.cregs[1]);
        daton();
/* if BTL is XA, we won't have defined memmgr yet */
#if BTL_XA
        memmgrDefaults(&pdos->aspaces[a].o.btlmem);
        memmgrInit(&pdos->aspaces[a].o.btlmem);
        memmgrSupply(&pdos->aspaces[a].o.btlmem,
                     (char *)BTL_PRIVSTART,
                     BTL_PRIVLEN * 1024 * 1024);
#endif
        memmgrDefaults(&pdos->aspaces[a].o.atlmem);
        memmgrInit(&pdos->aspaces[a].o.atlmem);
        memmgrSupply(&pdos->aspaces[a].o.atlmem,
                     (char *)(S370_MAXMB * 1024 * 1024),
                     (MAXASIZE-S370_MAXMB) * 1024 * 1024);
        datoff();
    }
#endif


/* For S/390 DAT there are no EA considerations. So we set
   aside the first 5 MB virtual for the OS, then 10 MB for 
   BTL address space, then 11 MB for the OS, then the rest
   for the ATL address space. This requires 16 MB real for
   PDOS, plus (10 + ATL) * # address spaces. */

#if defined(S390)
    for (a = 0; a < MAXANUM; a++)
    {
#if SEG_64K
        for (s = 0; s < (MAXASIZE * 16); s++)
#else
        for (s = 0; s < MAXASIZE; s++)
#endif
        {
            int r = 0;
            static int atl = MAXASIZE - S370_MAXMB - BTL_PRIVLEN;
            static int btl = BTL_PRIVLEN;
            static int pbtl = BTL_PRIVSTART / (1024 * 1024);
            static int patl = S370_MAXMB - BTL_PRIVSTART / (1024 * 1024);

            r = s;
            /* if less than 5 MB, it is PDOS, so no adjustment required */
            if (s < pbtl)
            {
                /* no adjustment required, V=R */
            }
            
            /* if we're in the BTL private region, that requires us to
               skip previous address spaces plus all PDOS memory
               (PDOS gets the entire 16 MB BTL real), then zero base it */
            else if (s < (pbtl + btl))
            {
                r += a * (btl + atl);
                r += pbtl + patl;
                r -= pbtl;
            }
            
            /* if we're in the PDOS atl area, map to BTL real (PDOS
               gets the lot) */
            else if (s < (pbtl + btl + patl))
            {
                r -= (pbtl + btl);
                r += pbtl;
            }
            
            /* if we're in the ATL private area, need to skip previous
               address spaces, plus pdos, then zero-base */
            else if (s < (pbtl + btl + patl))
            {
                r += a * (btl + atl);
                r += (pbtl + patl);
                r -= (pbtl + btl + patl);
            }

            /* no shifting of page table address is required,
               but the low order 12 bits must be 0, ie address must
               be aligned on 64-byte boundary */
            pdos->aspaces[a].o.segtable[s] = 
#if SEG_64K
                  0x0U
#else
                  0xfU
#endif
                  | (unsigned int)pdos->aspaces[a].o.pagetable[s];
            for (p = 0; p < MAXPAGE; p++)
            {
                /* because the address begins in bit 1, just like
                   a 31-bit bit address, we just need to shift
                   20 bits (1 MB) to get the right address. Plus
                   add in the page number, by shifting 12 bits
                   for the 4K multiple */
                pdos->aspaces[a].o.pagetable[s][p] = 
#if SEG_64K
                                  (s << 16)
#else
                                  (r << 20)
#endif
                                  | (p << 12);
            }
        }
        pdos->aspaces[a].o.cregs[1] = 
            /* - 1 because architecture implies 1 extra block of 16 */
#if SEG_64K
            (MAXASIZE/SEG_BLK * 16 - 1)
#else
            (MAXASIZE/SEG_BLK - 1)
#endif
            | (unsigned int)pdos->aspaces[a].o.segtable;
            /* note that the CR1 needs to be 4096-byte aligned, to give
               12 low zeros */
#if 0
        printf("aspace %d, seg %p, cr13 %08X\n",
               a, pdos->aspaces[a].o.segtable, pdos->aspaces[a].o.cregs[13]);
#endif

        /* now set up the memory manager for BTL and ATL storage */
        lcreg1(pdos->aspaces[a].o.cregs[1]);
        daton();
        memmgrDefaults(&pdos->aspaces[a].o.btlmem);
        memmgrInit(&pdos->aspaces[a].o.btlmem);
        memmgrSupply(&pdos->aspaces[a].o.btlmem,
                     (char *)BTL_PRIVSTART,
                     BTL_PRIVLEN * 1024 * 1024);

        memmgrDefaults(&pdos->aspaces[a].o.atlmem);
        memmgrInit(&pdos->aspaces[a].o.atlmem);
        memmgrSupply(&pdos->aspaces[a].o.atlmem,
                     (char *)(S370_MAXMB * 1024 * 1024),
                     (MAXASIZE - S370_MAXMB) * 1024 * 1024);
        datoff();
    }
    
#endif

#else /* ZARCH */

    /* MAXANUM must be 1 for z/Arch */
    /* 8196 MB needs to be mapped - the upper 4 GB to the lower 4 GB */
    for (a = 0; a < MAXANUM; a++)
    {
        for (s = 0; s < 4096; s++)
        {
            pdos->aspaces[a].o.segtable[s][0] = 0;
            pdos->aspaces[a].o.segtable[s][1] =
                  (unsigned int)
                  &pdos->aspaces[a].o.pagetable[s * PAGES_PER_SEGMENT][0];
            pdos->aspaces[a].o.segtable[4096 + s][0] = 0;
            pdos->aspaces[a].o.segtable[4096 + s][1] = 
                  (unsigned int)
                  &pdos->aspaces[a].o.pagetable[s * PAGES_PER_SEGMENT][0];
            for (p = 0; p < PAGES_PER_SEGMENT; p++)
            {
                pdos->aspaces[a].o.pagetable[s
                                             * PAGES_PER_SEGMENT
                                             + p][0] = 0;
                pdos->aspaces[a].o.pagetable[s
                                             * PAGES_PER_SEGMENT
                                             + p][1] =
                    (s * PAGES_PER_SEGMENT + p) << 12;
            }
        }
        pdos->aspaces[a].o.region3[0][0] = 0;
        pdos->aspaces[a].o.region3[0][1] =
            (unsigned int)&pdos->aspaces[a].o.segtable[0]
            | (0x1 << 2) /* region 3 indicator */
            | 0x03; /* segment table is max size */
        pdos->aspaces[a].o.region3[1][0] = 0;
        pdos->aspaces[a].o.region3[1][1] =
            (unsigned int)&pdos->aspaces[a].o.segtable[2048]
            | (0x1 << 2) /* region 3 indicator */
            | 0x03; /* segment table is max size */
        pdos->aspaces[a].o.region3[2][0] = 0;
        pdos->aspaces[a].o.region3[2][1] =
            (unsigned int)&pdos->aspaces[a].o.segtable[0]
            | (0x1 << 2) /* region 3 indicator */
            | 0x03; /* segment table is max size */
        pdos->aspaces[a].o.region3[3][0] = 0;
        pdos->aspaces[a].o.region3[3][1] =
            (unsigned int)&pdos->aspaces[a].o.segtable[2048]
            | (0x1 << 2) /* region 3 indicator */
            | 0x03; /* segment table is max size */
        /* pad to minimum size */
        for (p = 4; p < 512; p++)
        {
            pdos->aspaces[a].o.region3[p][0] = 0;
            pdos->aspaces[a].o.region3[p][1] = 1 << 5; /* region invalid bit */
        }

        pdos->aspaces[a].o.region2[0][0] = 0;
        pdos->aspaces[a].o.region2[0][1] =
            (unsigned int)&pdos->aspaces[a].o.region3[0]
            | (0x2 << 2); /* region 2 indicator */
        /* pad to minimum size */
        for (p = 1; p < 512; p++)
        {
            pdos->aspaces[a].o.region2[p][0] = 0;
            pdos->aspaces[a].o.region2[p][1] = 1 << 5; /* region invalid bit */
        }

        pdos->aspaces[a].o.region1[0][0] = 0;
        pdos->aspaces[a].o.region1[0][1] =
            (unsigned int)&pdos->aspaces[a].o.region2[0]
            | (0x3 << 2); /* region 1 indicator */
        /* pad to minimum size */
        for (p = 1; p < 512; p++)
        {
            pdos->aspaces[a].o.region2[p][0] = 0;
            pdos->aspaces[a].o.region2[p][1] = 1 << 5; /* region invalid bit */
        }

        pdos->aspaces[a].o.cregs[1] = 
            0
            | (unsigned int)&pdos->aspaces[a].o.region1
            | (0x03 << 2); /* region 1 indicator */

        memmgrDefaults(&pdos->aspaces[a].o.btlmem);
        memmgrInit(&pdos->aspaces[a].o.btlmem);
        memmgrSupply(&pdos->aspaces[a].o.btlmem,
                     (char *)BTL_PRIVSTART,
                     BTL_PRIVLEN * 1024U * 1024);

        memmgrDefaults(&pdos->aspaces[a].o.atlmem);
        memmgrInit(&pdos->aspaces[a].o.atlmem);
        if (__ismem)
        {
            /* 1024 MB less memory available because of the
               ramdisk at the 3 GiB location */
            memmgrSupply(&pdos->aspaces[a].o.atlmem,
                         (char *)(S370_MAXMB * 1024U * 1024),
                         (MAXASIZE - S370_MAXMB - 1024) * 1024U * 1024);
        }
        else
        {
            memmgrSupply(&pdos->aspaces[a].o.atlmem,
                         (char *)(S370_MAXMB * 1024U * 1024),
                         (MAXASIZE - S370_MAXMB) * 1024U * 1024);
        }
    }
#endif /* ZARCH */

    return;
}



static void pdosProcessSVC(PDOS *pdos)
{
    int svc;
    int getmain;
       /* should move to PDOS and use memmgr - but virtual memory
          will obsolete anyway */

    svc = pdos->psa->svc_code & 0xffff;
#if SVCDEBUG
    printf("SVC code is %d\n", svc);
#endif
    if (svc == 3)
    {
        /* if this is a LINKed program, then restore old context
           rather than shutting down */
        if (pdos->context->rblinkb == 
            &pdos->aspaces[pdos->curr_aspace].o.first_rb)
        {
            /* normally the OS would not exit on program end */
            printf("return from PCOMM is %d\n", pdos->context->regs[15]);
            pdos->shutdown = 1;
            pdos->exitcode = pdos->context->regs[15];
        }
        else
        {
            /* set ECB of person waiting */
            /* +++ needs to run at user priviledge */
            *pdos->context->rblinkb->postecb =
                pdos->aspaces[pdos->curr_aspace].o.tcb.tcbcmp =
                pdos->context->regs[15];
            pdos->aspaces[pdos->curr_aspace].o.curr_rb = 
                pdos->context->rblinkb;

            /* free old context */
#if defined(ZARCH)
            memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.atlmem,
                       pdos->context);
#else
            memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.btlmem,
                       pdos->context);
#endif

            pdos->context = pdos->aspaces[pdos->curr_aspace].o.curr_rb;
            pdos->context->regs[15] = 0; /* signal success to caller */
            /* R1 they are expecting the TCB of the (now-completed)
               task - ie, yes, this is fudged, as we only support LINK */
            pdos->context->regs[1] = 
                (int)&pdos->aspaces[pdos->curr_aspace].o.tcb;

#if defined(ZARCH)
            /* free the memory that was allocated to the executable */
            memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.atlmem,
                       pdos->context->next_exe);
#else
            /* free the memory that was allocated to the executable */
            memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.btlmem,
                       pdos->context->next_exe);
#endif
#if REPORT_MEMLEAK
            printf("exited program, total free space is %d\n",
                   memmgrTotSize(&pdos->aspaces[pdos->curr_aspace].o.btlmem));
#endif
        }
    }
    else if ((svc == 120) || (svc == 10))
    {
        /* if really getmain */
        if (((svc == 10) && (pdos->context->regs[1] < 0))
            || ((svc == 120) && (pdos->context->regs[1] == 0)))
        {
            int len;
            int above = 0;

#ifndef S370
            if ((svc == 120) && ((pdos->context->regs[15] & 0x30) == 0x30))
            {
                above = 1;
            }
#endif

/* we should support RMODE 24 programs instead of assuming
   they are RMODE ANY */
/* #ifdef S390
            if ((svc == 120) && ((pdos->context->regs[15] & 0x30) != 0x10))
            {
                above = 1;
            }
#endif */

            pdos->context->regs[15] = 0;
            len = pdos->context->regs[0];
            if (svc == 10)
            {
                len &= 0xffffff;
            }
/* If PCOMM is using MEMMGR it will request a size
   that is way too large, so we kludge it here */
#if 1 /* #ifdef S370 */
            if (len >= (16 * 1024 * 1024))
            {
                len = PDOS_STORINC;
            }
#endif

            if (above)
            {
                getmain = (int)memmgrAllocate(
                    &pdos->aspaces[pdos->curr_aspace].o.atlmem,
                    len,
                    memid);
            }
            if (!above || (getmain == 0))
            {
                getmain = (int)memmgrAllocate(
                    &pdos->aspaces[pdos->curr_aspace].o.btlmem,
                    len,
                    memid);
            }
#if MEMDEBUG
            printf("allocated %x for r0 %x, r1 %x, r15 %x\n", getmain,
                len, pdos->context->regs[1],
                pdos->context->regs[15]);
#endif

#if 0
            if (getmain == 0)
            {
                printf("out of memory, above is %d, len is %d - looping\n",
                       above, len);
                for (;;) ;
            }
#endif

            pdos->context->regs[1] = getmain;
            if (getmain == 0)
            {
                pdos->context->regs[15] = 4;
            }
        }
        /* freemain */
        else
        {
#if MEMDEBUG
            printf("freeing r0 %x, r1 %x\n",
                    pdos->context->regs[0], pdos->context->regs[1]);
#endif
            if (pdos->context->regs[1] < (16 * 1024 * 1024))
            {
                memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.btlmem,
                           (char *)pdos->context->regs[1]);
            }
            else
            {
                memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.atlmem,
                           (char *)pdos->context->regs[1]);
            }
        }
    }
    else if (svc == 99)
    {
        pdosSVC99(pdos);
    }
    else if (svc == 24) /* devtype */
    {
        /* hardcoded constants that fell off the back of a truck */
        memcpy((void *)pdos->context->regs[0], 
               "\x30\x50\x08\x0B" /* 08 = unit record, but note
                 that the C part of PDPCLIB no longer relies on
                 the unit record bit. */
               "\x00\x00\x7f\xff", /* device size = 32767 byte blocks */
               8);
        if (memcmp((void *)pdos->context->regs[1], "SYS", 3) != 0)
        {
            ((char *)pdos->context->regs[0])[2] = 0;
        }
        pdos->context->regs[15] = 0;
    }
    else if (svc == 64) /* rdjfcb */
    {
        int *p;

        pdos->context->regs[15] = 0;
        p = (int *)pdos->context->regs[1];
        /* DCB is pointed to by first parameter */
        /* needs validation */
        gendcb = (DCB *)(p[0] & 0xffffff);
#if 0
        printf("got rdjfcb for %.8s\n", gendcb->dcbddnam);
#endif
        /* we assume that any SYS files are terminal input/output,
           which by default they certainly are. Anything else, we
           assume that it is a reference to the most recent dynamic
           allocation, since there is normally a dynamic allocation,
           rdjfcb then open sequence. We should really have a TIOT
           table and look that up though. */
        if (memcmp(gendcb->dcbddnam, "SYS", 3) != 0)
        {
            int cyl;            
            int head;
            int rec;
#if 0
            printf("must be dataset name %s\n", lastds);
#endif
            if ((strchr(lastds, ':') != NULL)
                && (ins_strncmp(lastds, "tav", 3) == 0))
            {
                gendcb->vtape = strtoul(lastds + 3, NULL, 16);
#if defined(S390) || defined(ZARCH)
                if ((gendcb->vtape != 0) && (gendcb->vtape < 0x10000))
                {
                    gendcb->vtape = getssid(gendcb->vtape);
                }
#endif
            }
            else if ((strchr(lastds, ':') != NULL)
                && (ins_strncmp(lastds, "dev", 3) == 0))
            {
                gendcb->gendev = strtoul(lastds + 3, NULL, 16);
            }
            else if ((strchr(lastds, ':') != NULL)
                && (ins_strncmp(lastds, "crd", 3) == 0))
            {
                gendcb->gendev = strtoul(lastds + 3, NULL, 16);
            }
            else if ((strchr(lastds, ':') != NULL)
                && (ins_strncmp(lastds, "tap", 3) == 0))
            {
                gendcb->gendev = strtoul(lastds + 3, NULL, 16);
            }
            else if (findFile(pdos->ipldev, lastds, &cyl, &head, &rec) == 0)
            {
                rec = 0; /* so that we can do increments */
                join_cchhr(gendcb->dcbfdad + 3, cyl, head, rec);
#if 0
                printf("located at %d %d %d\n", cyl, head, rec);
#endif
            }
            else
            {
                pdos->context->regs[15] = 12; /* +++ find out
                    something sensible, and then go no further */
            }
#if defined(S390) || defined(ZARCH)
            if ((gendcb->gendev != 0) && (gendcb->gendev < 0x10000))
            {
                gendcb->gendev = getssid(gendcb->gendev);
            }
#endif
        }
        /* we only support RECFM=U files currently */
        /* for non-SYS files we should really get this info from
           the VTOC */
        /* except for tav files, which are RECFM=V */
        if (gendcb->vtape != 0)
        {
            gendcb->u2.dcbrecfm |= DCBRECV;
            gendcb->dcblrecl = 18452 + 4;
            gendcb->dcbblksi = 18452 + 4 * 2;
        }
        /* and cards are F80 */
        else if (ins_strncmp(lastds, "crd", 3) == 0)
        {
            gendcb->u2.dcbrecfm |= DCBRECF;
            gendcb->dcblrecl = 80;
            gendcb->dcbblksi = 80;
        }
        else
        {
            gendcb->u2.dcbrecfm |= DCBRECU;
            gendcb->dcblrecl = 0;
            gendcb->dcbblksi = 18452;
        }
    }
    else if (svc == 22) /* open */
    {
        int oneexit;
        unsigned int ocode;
        int *p;

        p = (int *)pdos->context->regs[1];
        /* DCB is pointed to by first parameter */
        /* needs validation */
        gendcb = (DCB *)(p[0] & 0xffffff);
#if 0
        printf("in open, DCB is %p\n", gendcb);
#endif

        ocode = p[0];
        ocode >>= 24;
        pdos->context->regs[15] = 0;
        gendcb->dcbcheck = (int)dcheck;
        gendcb->dcbnotepoint = (int)dnotpnt;
        oneexit = gendcb->u2.dcbexlsa & 0xffffff;
        if (oneexit != 0)
        {
            oneexit = *(int *)oneexit & 0xffffff;
            if (oneexit != 0)
            {
                dexit(oneexit, gendcb);
            }
        }
        if (ocode == 0x80) /* in */
        {
#if 0
            printf("opening for input\n");
#endif
            gendcb->u1.dcbgetput = (int)dread;
        }
        else if (ocode == 0x8f) /* out */
        {
#if 0
            printf("opening for output\n");
#endif
            gendcb->u1.dcbgetput = (int)dwrite;
        }
        else /* don't understand - refuse to open */
        {
            pdos->context->regs[15] = 12; /* is this correct? */
        }
        if (pdos->context->regs[15] == 0)
        {
            gendcb->u1.dcboflgs |= DCBOFOPN;
        }
    }
    else if (svc == 35) /* WTO */
    {
        char *p;
        int len;
        int flags;

        p = (char *)pdos->context->regs[1];
        len = *(short *)p;
        if (len >= (2 * sizeof(short)))
        {
            len -= (2 * sizeof(short));
        }
        p += sizeof(short);
        flags = *(short *)p;
        p += sizeof(short);
        printf("%.*s\n", len, p);
    }
    else if (svc == 42) /* attach */
    {
        char *prog;
        char *parm;
        int newcont = -1;
        struct {
            int eploc;
            int unused1;
            int ecb;
        } *sysatlst;
        
        memid += 256;
        sysatlst = (void *)pdos->context->regs[15];
        prog = (char *)sysatlst->eploc;
#if 0
        printf("got request to run %.8s\n", prog);
#endif
        parm = *(char **)pdos->context->regs[1];
#if 0
        printf("parameter string is %d bytes\n", *(short *)parm);
#endif
        
        /* We save the ECB (which specifies where the return code
           is meant to go) in our current RB. But does it logically
           belong in this one or the next? Or somewhere else entirely */
        pdos->context->postecb = (ECB *)(sysatlst->ecb & 0xffffff);

        /* special exception for special program to look at
           disk blocks */
        if (memcmp(prog, "DUMPBLK", 7) == 0)
        {
            parm = (char *)((unsigned int)parm & 0x7FFFFFFFUL);
            *pdos->context->postecb = 
                pdos->aspaces[pdos->curr_aspace].o.tcb.tcbcmp =
                pdosDumpBlk(pdos, parm);
            pdos->context->regs[15] = 0;
            pdos->context->regs[1] = 
                (int)&pdos->aspaces[pdos->curr_aspace].o.tcb;
        }
        else if (memcmp(prog, "ZAPBLK", 6) == 0)
        {
            parm = (char *)((unsigned int)parm & 0x7FFFFFFFUL);
            *pdos->context->postecb = 
                pdos->aspaces[pdos->curr_aspace].o.tcb.tcbcmp =
                pdosZapBlk(pdos, parm);
            pdos->context->regs[15] = 0;
            pdos->context->regs[1] = 
                (int)&pdos->aspaces[pdos->curr_aspace].o.tcb;
        }
        else if (memcmp(prog, "NEWBLK", 6) == 0)
        {
            parm = (char *)((unsigned int)parm & 0x7FFFFFFFUL);
            *pdos->context->postecb =
                pdos->aspaces[pdos->curr_aspace].o.tcb.tcbcmp =
                pdosNewBlk(pdos, parm);
            pdos->context->regs[15] = 0;
            pdos->context->regs[1] =
                (int)&pdos->aspaces[pdos->curr_aspace].o.tcb;
        }
        else if (memcmp(prog, "RAMDISK", 7) == 0)
        {
            parm = (char *)((unsigned int)parm & 0x7FFFFFFFUL);
            *pdos->context->postecb =
                pdos->aspaces[pdos->curr_aspace].o.tcb.tcbcmp =
                pdosRamDisk(pdos, parm);
            pdos->context->regs[15] = 0;
            pdos->context->regs[1] =
                (int)&pdos->aspaces[pdos->curr_aspace].o.tcb;
        }
        else if (memcmp(prog, "DIR", 3) == 0)
        {
            parm = (char *)((unsigned int)parm & 0x7FFFFFFFUL);
            pdosDoDIR(pdos, parm);
            *pdos->context->postecb = 0;
            pdos->context->regs[15] = 0;
        }
        else if (memcmp(prog, "DISKINIT", 8) == 0)
        {
            parm = (char *)((unsigned int)parm & 0x7FFFFFFFUL);
            pdosDiskInit(pdos, parm);
            *pdos->context->postecb = 0;
            pdos->context->regs[15] = 0;
        }
        else if (memcmp(prog, "FIL2DSK", 7) == 0)
        {
            parm = (char *)((unsigned int)parm & 0x7FFFFFFFUL);
            pdosFil2Dsk(pdos, parm);
            *pdos->context->postecb = 0;
            pdos->context->regs[15] = 0;
        }
        else if (memcmp(prog, "DSK2FIL", 7) == 0)
        {
            parm = (char *)((unsigned int)parm & 0x7FFFFFFFUL);
            pdosDsk2Fil(pdos, parm);
            *pdos->context->postecb = 0;
            pdos->context->regs[15] = 0;
        }
        else if (memcmp(prog, "MEMTEST", 7) == 0)
        {
            printf("writing 4 bytes to address X'7FFFFFFE'\n");
            memcpy((char *)0x7ffffffe, "\x01\x02\x03\x04", 4);
            printf("done!\n");
            *pdos->context->postecb = 0;
            pdos->context->regs[15] = 0;
        }
        else if ((newcont = pdosLoadExe(pdos, prog, parm)) != 0)
        {
            /* +++ not sure what proper return code is */
            pdos->context->regs[15] = 4;
        }
        /* we usually have a new context loaded, so don't
           mess with R15 */
        if (newcont != 0)
        {
            /* ECB is no longer applicable */
            pdos->context->postecb = NULL;
        }
    }
    else if (svc == 1) /* wait */
    {
        /* do nothing */
    }
    else if (svc == 62) /* detach */
    {
        /* should switch back context here */

        /* we free any junk (memory leak) left behind by the program */
        /* we should also set a flag to say if a subpool other than 0
           was used by this program, and free all 256 possible memids
           instead of just subpool 0 */
        memmgrFreeId(&pdos->aspaces[pdos->curr_aspace].o.btlmem, memid);
#ifndef S370
        memmgrFreeId(&pdos->aspaces[pdos->curr_aspace].o.atlmem, memid);
#endif
        memid -= 256;
    }
    else if (svc == 20) /* close */
    {
        DCB *dcb;
        int *p;
        unsigned int ocode;

        p = (int *)pdos->context->regs[1];
        /* DCB is pointed to by first parameter */
        /* needs validation */
        dcb = (DCB *)(p[0] & 0xffffff);
#if 0
        printf("in close, DCB is %p\n", dcb);
        printf("ddname to close is %.8s\n", dcb->dcbddnam);
#endif

        ocode = p[0];
        ocode >>= 24;

        /* if this is a disk file, and, it was open for write, need to 
           write an EOF record */
        if ((memcmp(dcb->dcbfdad,
                   "\x00\x00\x00\x00\x00\x00\x00\x00", 8) != 0)
            && (ocode == 0x8f))
        {
            int cyl;
            int head;
            int rec;
            int cnt;
            int len = 0;
            char tbuf[8]; /* +++ needs to be short-aligned */
            
            split_cchhr(dcb->dcbfdad + 3, &cyl, &head, &rec);
            rec++;
#if DSKDEBUG
            printf("writing from %p to %d, %d, %d\n",
                   tbuf,
                   cyl, head, rec);
#endif
            memcpy(tbuf, "\x00\x01\x00\x00\x0d\x2c\x00\x60", 8);
            *(short *)tbuf = cyl;
            *(short *)(tbuf + 2) = head;
            tbuf[4] = rec;
            tbuf[5] = 0;
            *(short *)(tbuf + 6) = len;

            /* record number must be one less when using 0x1d
               destructive (of track) write (to set block size) */
            cnt = wrblock(pdos->ipldev, cyl, head, rec - 1,
                          tbuf, len + 8, 0x1d);
#if 0
            printf("destructing write %d %d %d gave %d\n",
                cyl, head, rec - 1, cnt);
#endif
            /* if track is full, count is 0 */
            if (cnt <= 0)
            {
                rec = 1;
                head++;
                *(short *)(tbuf + 2) = head;
                tbuf[4] = rec;
                cnt = wrblock(pdos->ipldev, cyl, head, rec - 1,
                              tbuf, len + 8, 0x1d);
                if (cnt <= 0)
                {
                    head = 0;
                    cyl++;
                    *(short *)tbuf = cyl;
                    *(short *)(tbuf + 2) = head;
                    cnt = wrblock(pdos->ipldev, cyl, head, rec - 1,
                                  tbuf, len + 8, 0x1d);
                }
            }
#if DSKDEBUG
            printf("cnt is %d\n", cnt);
#endif
            join_cchhr(dcb->dcbfdad + 3, cyl, head, rec);
        }
    }
    return;
}



typedef struct {
  char len; /* length of request block, always 20 */
  char verb; /* dynamic allocation function requested */
  char flag1;
  char flag2;
  short error_reason; /* returned */
  short info_reason; /* returned */
  void *tu_list; /* list of pointers to text units */
  int reserved;
  char moreflags[4]; /* extraflags */
} SVC99RB;

typedef struct {
  short key; /* key defining what this text unit is */
  short numparms; /* number of parms that follow */
  short parm1_len;
  char parm1[98];
  /* parm2_len etc would theoretically follow, but we
  /* can't define them, because the length of 98 is probably
     not correct in the first place */
} SVC99TU;


static void pdosSVC99(PDOS *pdos)
{
    int cyl;
    int head;
    int rec;
    SVC99RB *svc99rb;
    SVC99TU **svc99tu;
    int new = -1;

    svc99rb = *(SVC99RB **)pdos->context->regs[1];
    /* the rb can have the high bit set, so it needs to be cleared */
    svc99rb = (SVC99RB *)((unsigned int)svc99rb & 0x7fffffffU);
    if (svc99rb->verb == 2) /* unallocate */
    {
        /* unconditional success */
        pdos->context->regs[15] = 0;
        pdos->context->regs[0] = 0;
        return;
    }
    else if (svc99rb->verb == 1) /* allocate */
    {
        svc99tu = svc99rb->tu_list;
        while (1)
        {
            if (((int)*svc99tu & 0x7fffffff) != 0)
            {
                if ((*svc99tu)->key == 1) /* DDNAME */
                {
#if 0
                    printf("dynalloc ddname is %.8s\n", (*svc99tu)->parm1);
#endif
                }
                else if ((*svc99tu)->key == 2) /* DSNAME */
                {
#if 0
                    printf("dynalloc dsname is %.44s\n", (*svc99tu)->parm1);
#endif
                    /* save dataset away */
                    sprintf(lastds, "%.44s", (*svc99tu)->parm1);
                }
                else if ((*svc99tu)->key == 4) /* DISP */
                {
                    int disp;

                    disp = (*svc99tu)->parm1[0];
                    if (disp == 4) /* NEW */
                    {
                        new = 1;
#if 0
                        printf("dynalloc disp is NEW\n");
#endif
                    }
                    else if (disp == 8) /* SHR */
                    {
                        new = 0;
#if 0
                        printf("dynalloc disp is SHR\n");
#endif
                    }
                }
                else if ((*svc99tu)->key == 0x49) /* RECFM */
                {
#if 0
                    int recfm;
                    
                    recfm = (*svc99tu)->parm1[0];
                    if (recfm == 0x40) /* V */
                    {
                        printf("file is RECFM V\n");
                    }
#endif
                }
                else if ((*svc99tu)->key == 0x42) /* LRECL */
                {
#if 0
                    int lrecl;
                    
                    lrecl = *(short *)((*svc99tu)->parm1);
                    printf("lrecl is %d\n", lrecl);
#endif
                }
            }
            if (((int)*svc99tu & 0x80000000) != 0) break;
            svc99tu++;
        }

        /* we will auto-handle tav tapes */
        if ((strchr(lastds, ':') != NULL)
            && (ins_strncmp(lastds, "tav", 3) == 0))
        {
            pdos->context->regs[15] = 0;
            pdos->context->regs[0] = 0;
        }
        /* and normal tapes */
        else if ((strchr(lastds, ':') != NULL)
                 && (ins_strncmp(lastds, "tap", 3) == 0))
        {
            pdos->context->regs[15] = 0;
            pdos->context->regs[0] = 0;
        }
        /* and cards */
        else if ((strchr(lastds, ':') != NULL)
                 && (ins_strncmp(lastds, "crd", 3) == 0))
        {
            pdos->context->regs[15] = 0;
            pdos->context->regs[0] = 0;
        }
        /* and generic devices */
        else if ((strchr(lastds, ':') != NULL)
                 && (ins_strncmp(lastds, "dev", 3) == 0))
        {
            pdos->context->regs[15] = 0;
            pdos->context->regs[0] = 0;
        }
        else if (new == -1)
        {
            pdos->context->regs[15] = 12;
            pdos->context->regs[0] = 12;
            svc99rb->error_reason = 12;
            strcpy(lastds, "");
        }
        else
        {
            if (findFile(pdos->ipldev, lastds, &cyl, &head, &rec) != 0)
            {
                if (new == 1)
                {
                    pdosNewF(pdos, lastds);
                }
                else
                {
                    strcpy(lastds, "");
                    pdos->context->regs[15] = 12;
                    pdos->context->regs[0] = 12;
                    svc99rb->error_reason = 12;
                }
            }
            else
            {
                pdos->context->regs[15] = 0;
                pdos->context->regs[0] = 0;
            }
        }
        return;
    }
    /* fail anything else */
    pdos->context->regs[15] = 12;
    pdos->context->regs[0] = 12;
    svc99rb->error_reason = 12;
    return;
}



#ifdef ZARCH
#define PSW_ENABLE_INT 0x000C0000 /* actually disable interrupts for now */
#else
#define PSW_ENABLE_INT 0x040C0000 /* actually disable interrupts for now */
#endif


#define DS1RECFF 0x80
#define DS1RECFV 0x40
#define DS1RECFU 0xc0
#define DS1RECFB 0x10

/* do DIR command */

static int pdosDoDIR(PDOS *pdos, char *parm)
{
    int cyl;
    int head;
    int rec;
    char tbuf[MAXBLKSZ];
    DSCB1 dscb1;
    int cnt;
    char *p;
    int year;
    int month;
    int day;
    int recfm;
    char *blk;
    int errcnt = 0;
    int dev = pdos->curdev;
    
    /* read VOL1 record which starts on cylinder 0, head 0, record 3 */
    cnt = rdblock(dev, 0, 0, 3, tbuf, MAXBLKSZ, 0x0e);
    if (cnt >= 20)
    {
        split_cchhr(tbuf + 15, &cyl, &head, &rec);
        rec += 2; /* first 2 blocks are of no interest */
        
        while (errcnt < 4)
        {
            int c, h, r;
            
            cnt = rdblock(dev, cyl, head, rec, &dscb1, sizeof dscb1, 0x0e);
            if (cnt < 0)
            {
                errcnt++;
                if (errcnt == 1)
                {
                    rec++;
                }
                else if (errcnt == 2)
                {
                    rec = 1;
                    head++;
                }
                else if (errcnt == 3)
                {
                    rec = 1;
                    head = 0;
                    cyl++;
                }
                continue;
            }
            errcnt = 0;
            if (dscb1.ds1dsnam[0] == '\0') break;
            dscb1.ds1dsnam[44] = '\0';
            p = strchr(dscb1.ds1dsnam, ' ');
            if (p != NULL)
            {
                *p = '\0';
            }
            split_cchhr(dscb1.startcchh, &c, &h, &r);
            r = 1;
            year = dscb1.ds1credt[0];
            day = dscb1.ds1credt[1] << 8 | dscb1.ds1credt[2];
            brkyd(&year, &month, &day);
            recfm = dscb1.ds1recfm;
            recfm &= 0xc0;
            if (recfm == DS1RECFU)
            {
                recfm = 'U';
            }
            else if (recfm == DS1RECFV)
            {
                recfm = 'V';
            }
            else if (recfm == DS1RECFF)
            {
                recfm = 'F';
            }
            else
            {
                recfm = 'X';
            }
            blk = ((dscb1.ds1recfm & DS1RECFB) != 0) ? "B" : "";
            
            printf("%-44s %04d-%02d-%02d %c%s %d %d (%d %d %d)\n",
                   dscb1.ds1dsnam, year, month, day,
                   recfm, blk, dscb1.ds1lrecl, dscb1.ds1blkl,
                   c, h, r);
            rec++;
        }
    }
    
    return (0);
}


static void brkyd(int *year, int *month, int *day)
{
    time_t tt;
    struct tm tms = { 0 };
    
    
    tms.tm_year = *year;
    tms.tm_mon = 0;
    tms.tm_mday = *day;
    tt = mktime(&tms);
    *year = 1900 + tms.tm_year;
    *month = tms.tm_mon + 1;
    *day = tms.tm_mday;
}


/* do DISKINIT command */

static int pdosDiskInit(PDOS *pdos, char *parm)
{
    int cyl;
    int head;
    int rec;
    int len;
    char tbuf[MAXBLKSZ];
    long cnt = -1;
    int lastcnt = 0;
    int ret;
    int c, pos1, pos2;
    long x = 0L;
    char prtln[100];
    long i;
    int dev;
    int n;
    int skip = 0;
    int count = INT_MAX;

    tbuf[0] = '\0';
    i = *(short *)parm;
    parm += sizeof(short);
    if (i < (sizeof tbuf - 1))
    {
        memcpy(tbuf, parm, i);
        tbuf[i] = '\0';
    }
    n = sscanf(tbuf, "%x", &dev);
    if (n < 1)
    {
        printf("usage: diskinit dev(x)\n");
        printf("e.g. diskinit 1b1\n");
        return (0);
    }

#if defined(S390) || defined(ZARCH)
    if ((dev != 0) && (dev < 0x10000))
    {
        dev = getssid(dev);
    }
#endif

    cyl = 0;
    head = 0;
    rec = 1;
    len = 0;
    memcpy(tbuf, "\x00\x01\x00\x00\x0d\x2c\x00\x60", 8);
    *(short *)(tbuf + 2) = head;
    tbuf[4] = rec;
    tbuf[5] = 0;
    *(short *)(tbuf + 6) = len;
    while (cyl < 1113)
    {
        *(short *)tbuf = cyl;

        for (head = 0; head < 15; head++)
        {
            *(short *)(tbuf + 2) = head;
            /* record number must be one less when using 0x11 erase */
            cnt = wrblock(dev, cyl, head, rec - 1,
                          tbuf, len + 8, 0x11);
        }
        cyl++;
    }
    return (0);
}


/* do FIL2DSK command */
/* The file is meant to be any generic filename, but at the
   moment we only support special files */
/* When reading from a disk, if we get any data on a track, then
   we continue to the next track. If we get an empty track, we
   skip to the next cylinder. Also if we get an EOF record we skip
   to the next cylinder. An EOF record has the additional property
   that it causes the output cylinder to sync to the input
   cylinder, which will hopefully come in useful when restoring a
   3390 image to a 3350 - it may just work, if knowledge is embedded
   of cylinder 0 head 0 and the VTOC and there are appropriate empty
   cylinders after the VTOC. */

static int pdosFil2Dsk(PDOS *pdos, char *parm)
{
    int incyl = 0;
    int outcyl = 0;
    int inhead = 0;
    int outhead = 0;
    int inrec = 1;
    int outrec = 1;
    int len;
    char tbuf[MAXBLKSZ];
    long cnt = -1;
    int lastcnt = 0;
    int ret;
    int c, pos1, pos2;
    long x = 0L;
    char prtln[100];
    long i;
    int dev;
    int n;
    int skip = 0;
    int count = INT_MAX;
    char fnm[FILENAME_MAX];
    int indev;
    int outdev;
    int hiteof;
    int lastkeylen = 0;
    int intape = 0;

    tbuf[0] = '\0';
    i = *(short *)parm;
    parm += sizeof(short);
    if (i < (sizeof tbuf - 1))
    {
        memcpy(tbuf, parm, i);
        tbuf[i] = '\0';
    }
    n = sscanf(tbuf, "%s %x", fnm, &outdev);
    if (n < 2)
    {
        printf("usage: fil2dsk fnm dev(x)\n");
        printf("e.g. fil2dsk drv1b9: 1b1\n");
        printf("(for disk to disk copy)\n");
        printf("e.g. fil2dsk tap1c1: 1b1\n");
        printf("(for tape to disk copy)\n");
        return (0);
    }
    if (strncmp(fnm, "drv", 3) == 0)
    {
        sscanf(fnm + 3, "%x", &indev);
#if defined(S390) || defined(ZARCH)
        if ((indev != 0) && (indev < 0x10000))
        {
            indev = getssid(indev);
        }
#endif
    }
    else if (strncmp(fnm, "tap", 3) == 0)
    {
        sscanf(fnm + 3, "%x", &intape);
#if defined(S390) || defined(ZARCH)
        if ((intape != 0) && (intape < 0x10000))
        {
            intape = getssid(intape);
        }
#endif
    }

    if (intape != 0)
    {
        char onetrack[0xde00];
        char inbuf[MAXBLKSZ];
        int tracks_per_cylinder;
        int fin;
        char *p;
        struct {
            unsigned short cc;
            unsigned short hh;
            unsigned char r;
            unsigned char kl;
            unsigned short dl;
        } cchhr_kl_dl;
        int minilen;

        cnt = rdtape(intape, inbuf, sizeof inbuf);
        if (cnt < 0x200)
        {
            printf("initial block needs to be at least 512 bytes\n");
            printf("because I'm lazy - no technical reason\n");
            return (0);
        }
        tracks_per_cylinder = inbuf[8];
        if (tracks_per_cylinder != 15)
        {
            printf("I doubt that this is a 3390 disk image\n");
            return (0);
        }
        minilen = cnt - 0x200;
        memcpy(onetrack, inbuf + 0x200, minilen);
        fin = 0;
        while (!fin)
        {
            while (1)
            {
                cnt = rdtape(intape, inbuf, sizeof inbuf);
                if ((cnt + minilen) >= sizeof onetrack)
                {
                    memcpy(onetrack + minilen,
                           inbuf,
                           sizeof onetrack - minilen);
                    break;
                }
                if (cnt <= 0)
                {
                    fin = 1;
                    break;
                }
                memcpy(onetrack + minilen, inbuf, cnt);
                minilen += cnt;
            }
            if (fin) break;
            /* don't abandon EOF processing if we have empty cylinders */
            if (hiteof
                && (inhead == 0)
                && (memcmp(onetrack + 21, "\xFF\xFF", 2) != 0))
            {
                hiteof = 0;
                outcyl = incyl;
                outhead = 0;
                outrec = 1;
            }
            if (hiteof)
            {
                inhead++;
                if (inhead == tracks_per_cylinder)
                {
                    inhead = 0;
                    incyl++;
                }
                memcpy(onetrack,
                       inbuf + (sizeof onetrack - minilen),
                       cnt - (sizeof onetrack - minilen));
                minilen = cnt - (sizeof onetrack - minilen);
                continue;
            }
            if (!fin && !hiteof)
            {
                int outcnt;

                /* we have a track. assume record 0 exists, and skip it */
                p = onetrack + 0x15;
                while (1)
                {
                    memcpy(&cchhr_kl_dl, p, sizeof cchhr_kl_dl);
                    if (cchhr_kl_dl.cc == 0xffff)
                    {
                        /* end of track. clean up and continue */
                        memcpy(onetrack,
                               inbuf + (sizeof onetrack - minilen),
                               cnt - (sizeof onetrack - minilen));
                        minilen = cnt - (sizeof onetrack - minilen);
                        if ((incyl == 0) && (inhead == 0))
                        {
                            /* exception - move onto next track */
                            outhead++;
                            outrec = 1;
                        }
                        break;
                    }
                    hiteof = 0;
                    outcnt = sizeof(cchhr_kl_dl)
                             + cchhr_kl_dl.kl
                             + cchhr_kl_dl.dl;
                    if (outcnt == 8)
                    {
                        /* hit EOF - write out record then skip to next
                           cylinder */
                        /* output cylinder needs to synch to input */
                        hiteof = 1;
                    }
                    /* if we are transitioning from the VTOC, force a synch */
                    if ((cchhr_kl_dl.kl != lastkeylen) && (lastkeylen == 44))
                    {
                        outcyl = incyl;
                        outhead = inhead;
                        outrec = 1;
                    }
                    /* remember if we were writing a VTOC block */
                    lastkeylen = cchhr_kl_dl.kl;

                    *(short *)tbuf = outcyl;
                    *(short *)(tbuf + 2) = outhead;
                    tbuf[4] = outrec;
                    tbuf[5] = cchhr_kl_dl.kl;
                    *(short *)(tbuf + 6) = cchhr_kl_dl.dl;
                    memcpy(tbuf + 8,
                           p + sizeof(cchhr_kl_dl),
                           cchhr_kl_dl.kl + cchhr_kl_dl.dl);
                    len = outcnt;
            /* record number must be one less when using 0x1d write */
            printf("attempting to write to %d %d %d\n", outcyl, outhead, outrec);

                    /* we certainly can't exceed 255, probably not 254 either,
                       but existing practice seems to be to not exceed 50 (for
                       3390 - other disks are different, but similar) */
                    if (outrec > 50)
                    {
                        outcnt = -1;
                    }
                    else
                    {
                        outcnt = wrblock(outdev, outcyl, outhead, outrec - 1,
                                         tbuf, len, 0x1d);
                    }
                    printf("outcnt is %d\n", outcnt);
                    /* sometimes we get a count of -1 on error, sometimes
                       we get 0 */
                    /* The -1 is probably when the seek fails to find the
                       previous record, and the 0 is when it was correctly
                       positioned, but not able to write any data */
                    /* seems more bulletproof to check for the exact count */
                    if (outcnt != len)
                    {
                        outrec = 1;
                        tbuf[4] = outrec;
                        outhead++;
                        *(short *)(tbuf + 2) = outhead;
        printf("new attempt to write to %d %d %d\n", outcyl, outhead, outrec);
                        outcnt = wrblock(outdev, outcyl, outhead, outrec - 1,
                                         tbuf, len, 0x1d);
                        if (outcnt != len)
                        {
                            outhead = 0;
                            *(short *)(tbuf + 2) = outhead;
                            outcyl++;
                            *(short *)tbuf = outcyl;
            printf("new new attempt to write to %d %d %d\n", outcyl, outhead, outrec);
                            outcnt = wrblock(outdev, outcyl, outhead,
                                             outrec - 1, tbuf, len, 0x1d);
                            if (outcnt != len)
                            {
                                printf("write error\n");
                                fin = 1;
                                break;
                            }
                        }
                    }
                    if (hiteof)
                    {
                        break;
                    }
                    /* we can't skip tracks here when EOF */
                    if (!hiteof)
                    {
                        inrec++; /* this isn't used */
                        outrec++;
                    }
                    p += sizeof cchhr_kl_dl + cchhr_kl_dl.kl + cchhr_kl_dl.dl;
                }
                if (hiteof)
                {
                    /* end of track. clean up and continue */
                    memcpy(onetrack,
                           inbuf + (sizeof onetrack - minilen),
                           cnt - (sizeof onetrack - minilen));
                    minilen = cnt - (sizeof onetrack - minilen);
                }
                inhead++;
                if (inhead == tracks_per_cylinder)
                {
                    inhead = 0;
                    incyl++;
                }
            }
        }
        return (0);
    }
    len = 0;
    while (incyl < 1113)
    {
        printf("reading %x %d %d %d\n", indev, incyl, inhead, inrec);
        /* I think we need to specify one less record */
        cnt = rdblock(indev, incyl, inhead, inrec - 1, tbuf, sizeof tbuf, 0x1e);
        /* I think if we are positioned on the last record, it cycles back
           to the first record rather than giving an error. So if we don't
           get the record number we wanted, we force it to -1 */
        /* I think -1 is for a failed seek to the previous record, and 0 is
           what we get if the actual record we want is not found, so we need
           to check for 0 or negative later. Actually we should probably
           be checking to make sure we got at least 8 bytes */
        if (cnt != -1)
        {
            if (tbuf[4] != inrec)
            {
                cnt = -1;
            }
        }
        printf("cnt is %d\n", cnt);

        if (cnt <= 0)
        {
            if ((incyl == 0) && (inhead == 0))
            {
                /* exception - move onto next track */
                inhead++;
                outhead++;
                inrec = 1;
                outrec = 1;
            }
            else
            {
                /* if the beginning of the cylinder is empty, don't attempt
                   to do another read. we just need to advance and synch */
                if ((inhead == 0) && (inrec == 1))
                {
                    incyl++;
                    outcyl = incyl;
                    inhead = outhead = 0;
                }
                else
                {
                    inhead++;
                    inrec = 1;
                    cnt = rdblock(indev, incyl, inhead, inrec, tbuf,
                                  sizeof tbuf, 0x1e);
                    if (cnt == -1)
                    {
                        /* reached end of a cylinder, but could still be
                           processing a file - haven't hit EOF yet */
                        inhead = 0;
                        incyl++;
                    }
                }
            }
        }
        else
        {
            hiteof = 0;
            if (cnt == 8)
            {
                /* hit EOF - write out record then skip to next cylinder */
                /* output cylinder needs to synch to input */
                hiteof = 1;
            }
            /* if we are transitioning from the VTOC, force a synch */
            if ((tbuf[5] != lastkeylen) && (lastkeylen == 44))
            {
                outcyl = incyl;
                outhead = inhead;
                outrec = inrec;
            }
            /* remember if we were writing a VTOC block */
            lastkeylen = tbuf[5];

            *(short *)tbuf = outcyl;
            *(short *)(tbuf + 2) = outhead;
            tbuf[4] = outrec;
            /* key and length should be preserved */
            /* tbuf[5] = 0; */
            /* len = *(short *)(tbuf + 6); */
            /* *(short *)(tbuf + 6) = len; */
            len = cnt;
            /* record number must be one less when using 0x1d write */
            printf("attempting to write to %d %d %d\n", outcyl, outhead, outrec);

            /* we certainly can't exceed 255, probably not 254 either, but
               existing practice seems to be to not exceed 50 */
            if (outrec > 50)
            {
                cnt = -1;
            }
            else
            {
                cnt = wrblock(outdev, outcyl, outhead, outrec - 1,
                              tbuf, len, 0x1d);
            }
            lastkeylen = tbuf[5];
            printf("cnt is %d\n", cnt);
            /* sometimes we get a count of -1 on error, sometimes we get 0 */
            /* The -1 is probably when the seek fails to find the previous
               record, and the 0 is when it was correctly positioned, but
               not able to write any data */
            /* seems more bulletproof to check for the exact count */
            if (cnt != len)
            {
                outrec = 1;
                tbuf[4] = outrec;
                outhead++;
                *(short *)(tbuf + 2) = outhead;
            printf("new attempt to write to %d %d %d\n", outcyl, outhead, outrec);
                cnt = wrblock(outdev, outcyl, outhead, outrec - 1,
                              tbuf, len, 0x1d);
                if (cnt != len)
                {
                    outhead = 0;
                    *(short *)(tbuf + 2) = outhead;
                    outcyl++;
                    *(short *)tbuf = outcyl;
            printf("new new attempt to write to %d %d %d\n", outcyl, outhead, outrec);
                    cnt = wrblock(outdev, outcyl, outhead, outrec - 1,
                                  tbuf, len, 0x1d);
                    if (cnt != len)
                    {
                        printf("write error\n");
                        break;
                    }
                }
            }
            if (hiteof)
            {
               incyl++;
               outcyl = incyl;
               inhead = outhead = 0;
               inrec = outrec = 1;
            }
            else
            {
                inrec++;
                outrec++;
            }
        }
    }
    return (0);
}


/* do DSK2FIL command */
/* The file is meant to be any generic filename, but at the
   moment we only support special files */

static int pdosDsk2Fil(PDOS *pdos, char *parm)
{
    int incyl = 0;
    int outcyl = 0;
    int inhead = 0;
    int outhead = 0;
    int inrec = 1;
    int outrec = 1;
    int len;
    char tbuf[MAXBLKSZ];
    long cnt = -1;
    int lastcnt = 0;
    int ret;
    int c, pos1, pos2;
    long x = 0L;
    char prtln[100];
    long i;
    int dev;
    int n;
    int skip = 0;
    int count = INT_MAX;
    char fnm[FILENAME_MAX];
    int indev;
    int outdev;
    int hiteof;
    int lastkeylen = 0;
    int intape = 0;
    char onetrack[0xde00];
    char header[0x200];
    struct {
        unsigned short cc;
        unsigned short hh;
        unsigned char r;
        unsigned char kl;
        unsigned short dl;
    } cchhr_kl_dl;
    char *p;

    tbuf[0] = '\0';
    i = *(short *)parm;
    parm += sizeof(short);
    if (i < (sizeof tbuf - 1))
    {
        memcpy(tbuf, parm, i);
        tbuf[i] = '\0';
    }
    n = sscanf(tbuf, "%x %s", &indev, fnm);
    if (n < 2)
    {
        printf("usage: dsk2fil dev(x) fnm\n");
        printf("e.g. dsk2fil 1b1 tap1c2:\n");
        return (0);
    }
#if defined(S390) || defined(ZARCH)
    if ((indev != 0) && (indev < 0x10000))
    {
        indev = getssid(indev);
    }
#endif
    if (strncmp(fnm, "tap", 3) == 0)
    {
        sscanf(fnm + 3, "%x", &outdev);
#if defined(S390) || defined(ZARCH)
        if ((outdev != 0) && (outdev < 0x10000))
        {
            outdev = getssid(outdev);
        }
#endif
    }

    memset(header, '\0', sizeof header);

    memcpy(header, "\x43\x4B\x44\x5F\x50\x33\x37\x30"
                   "\x0F\x00\x00\x00\x00\xDE\x00\x00\x90", 0x11);

#if 0
   000000  434B445F 50333730 0F000000 00DE0000  CKD_P370........
   000010  90000000 00000000 00000000 00000000  ................
#endif

    cnt = wrtape(outdev, header, sizeof header);
    printf("outcnt is %d\n", cnt);

    len = 0;
    for (incyl = 0; incyl < 1113; incyl++)
    {
        cchhr_kl_dl.cc = incyl;
        for (inhead = 0; inhead < 15; inhead++)
        {
            memset(onetrack, '\0', sizeof onetrack);
#if 0
388CD600 00045800 0E045800 0E000000 08000000
388CD610 00000000 00FFFFFF FFFFFFFF FF000000
#endif
            p = onetrack + 1;
            cchhr_kl_dl.hh = inhead;
            memcpy(p, &cchhr_kl_dl, 4);
            p += 4;
            cchhr_kl_dl.r = 0;
            cchhr_kl_dl.kl = 0;
            cchhr_kl_dl.dl = 8;
            memcpy(p, &cchhr_kl_dl, sizeof cchhr_kl_dl);
            p += sizeof cchhr_kl_dl;
            p += 8;
            inrec = 1;
            while (1)
            {
                printf("reading %x %d %d %d\n", indev, incyl, inhead, inrec);
                /* I think we need to specify one less record */
                cnt = rdblock(indev, incyl, inhead, inrec - 1, tbuf,
                              sizeof tbuf, 0x1e);
                /* I think if we are positioned on the last record, it cycles
                   back to the first record rather than giving an error. So if
                   we don't get the record number we wanted, we force it to
                   -1 */
                /* Or maybe that is a misdiagnosis. We are getting a count of
                   0 on the first record not found, so need to check for 0 or
                   negative a bit later */
                printf("cnt is %d\n", cnt);
                if (cnt != -1)
                {
                    if (tbuf[4] != inrec)
                    {
                        cnt = -1;
                    }
                }
                if (cnt <= 0)
                {
                    int x;

                    memcpy(p, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8);
                    for (x = 0; x < 4; x++)
                    {
                        cnt = wrtape(outdev, onetrack + x * 14208, 14208);
                        printf("outcnt is %d\n", cnt);
                    }
                    break;
                }
                /* This should already contain the CCHHR, keylen, datalen */
                memcpy(p, tbuf, cnt);
                p += cnt;
                inrec++;
            }
        }
    }
    return (0);
}


static int pdosNewF(PDOS *pdos, char *parm)
{
    char tbuf[MAXBLKSZ];
    DSCB1 *dscb1;
    int len;
    char *dsn;
    int datacyl;
    int dircyl;
    int dirhead;
    int dirrec;
    int cnt;

    dsn = parm;
#if 0
    printf("got request to create file %s\n", dsn);
#endif
    pdosGetMaxima(pdos, &dircyl, &dirhead, &dirrec, &datacyl);
#if 0
    printf("maxima is %d %d %d and cyl %d\n", dircyl, dirhead, dirrec, datacyl);
#endif

    dscb1 = (DSCB1 *)tbuf;

    len = strlen(dsn);
    if (len > sizeof dscb1->ds1dsnam)
    {
        len = sizeof dscb1->ds1dsnam;
    }
    memset(dscb1, '\0', sizeof(DSCB1));
    memset(dscb1->ds1dsnam, ' ', sizeof dscb1->ds1dsnam);
    memcpy(dscb1->ds1dsnam, dsn, len);
    dscb1->ds1fmtid = '1'; /* format 1 DSCB */
    memcpy(dscb1->ds1dssn, "PDOS00", 6);
    dscb1->ds1volsq[1] = 1; /* volume sequence number */
    memcpy(dscb1->ds1credt, "\x6e\x00\x01", 3); /* 2010-01-01 */
    memcpy(dscb1->ds1expdt, "\x00\x00\x00", 3); /* 1900-01-00 ? */
    dscb1->ds1noepv = 1; /* number of extents */
    strcpy(dscb1->ds1syscd, "PDOS         "); /* beware NUL */
    dscb1->ds1dsorg = 0x4000; /* DS1DSGPS */
    dscb1->ds1recfm = 0xc0; /* DS1RECFU */
    dscb1->ds1blkl = 18452;
    dscb1->ds1lrecl = 0;
    dscb1->ds1dsind = 0x80; /* DS1IND80 = last volume for this dataset */
    dscb1->ds1scal1 = 0xc0; /* DS1CYL = cylinder request */
    dscb1->ds1scal3[2] = 1; /* secondary allocation */
    /* this is meant to be the last used track and block for the dataset
       in TTR format */
    /* since we don't know what that will be, just set it to relative
       track 14 (since this is 0-based, and record 1 */
    memcpy(dscb1->ds1lstar, "\x00\x0e\x01", 3);
    /* this is some value apparently from TRKCALC, whatever that is. */
    /* lower would seem to be more prudent */
    memcpy(dscb1->ds1trbal, "\x01\x01", 2);
    dscb1->ds1ext1[0] = 0x81; /* on a cylinder boundary */
    
    /* space starts on specified cylinder */
    memcpy(dscb1->startcchh, (char *)&datacyl + sizeof datacyl - 2, 2);
    
    /* if you want 2 cylinders per file, uncomment below. Otherwise
       we assume the file ends on the same cylinder. */
    /* datacyl++; */

    memcpy(dscb1->endcchh, (char *)&datacyl + sizeof datacyl - 2, 2);
    /* last head on that cylinder too */
    memcpy(dscb1->endcchh + 2, "\x00\x0e", 2);
    
    cnt = wrblock(pdos->ipldev, dircyl, dirhead, dirrec, tbuf, 140, 0x0d);
#if 0
    printf("cnt from wrblock is %d\n", cnt);
#endif
    
#if 0
    memset(tbuf, '\0', sizeof tbuf);
    memcpy(tbuf, "\x00\x01\x00\x00\x0d\x2c\x00\x60", 8);
    strcpy(tbuf + 8, "AAAA");
    /* free directory space starts at 1 0 12 */    
    wrblock(pdos->ipldev, 1, 0, 12, tbuf, 140, 0x1d);
#endif

#if 0

1c1
< 000000  C3D6D4D4 C1D5C44B C5E7C540 40404040  COMMAND.EXE
---
> 000000  C1E4E3D6 C5E7C5C3 4BC2C1E3 40404040  AUTOEXEC.BAT
7,8c7,8
< 000060  00010001 03754600 00810000 04000000  .........a......
< 000070  04000E00 00000000 00000000 00000000  ................
---
> 000060  00010000 02DFEC00 00810000 05000000  .........a......
> 000070  05000E00 00000000 00000000 00000000  ................

000000  C3D6D4D4 C1D5C44B C5E7C540 40404040  COMMAND.EXE
000010  40404040 40404040 40404040 40404040
000020  40404040 40404040 40404040 F1D7C4D6              1PDO
000030  E2F0F000 016E012F 00000001 7800C8C5  S00..>........HE
000040  D9C3E4D3 C5E24040 40404000 00000000  RCULES     .....
000050  00004000 C0004814 00000000 0080C000  .. .{.........{.
000060  00010001 03754600 00810000 04000000  .........a......
000070  04000E00 00000000 00000000 00000000  ................
000080  00000000 00000000 00000000           ............

#endif


    return (0);
}


/* the directory will point to the first empty directory.
   The data cylinder will point to the first empty space */

static int pdosGetMaxima(PDOS *pdos, int *dircyl, int *dirhead,
                         int *dirrec, int *datacyl)
{
    int cyl;
    int head;
    int rec;
    char tbuf[MAXBLKSZ];
    DSCB1 dscb1;
    int cnt;
    char *p;
    int year;
    int month;
    int day;
    int recfm;
    char *blk;
    int errcnt = 0;
    
    *dirrec = *dirhead = *dircyl = *datacyl = 0;
    /* read VOL1 record which starts on cylinder 0, head 0, record 3 */
    cnt = rdblock(pdos->ipldev, 0, 0, 3, tbuf, MAXBLKSZ, 0x0e);
    if (cnt >= 20)
    {
        split_cchhr(tbuf + 15, &cyl, &head, &rec);
        rec += 2; /* first 2 blocks are of no interest */
        
        while (errcnt < 4)
        {
            cnt = rdblock(pdos->ipldev, cyl, head, rec, &dscb1,
                          sizeof dscb1, 0x0e);
            if (cnt < 0)
            {
                errcnt++;
                if (errcnt == 1)
                {
                    rec++;
                }
                else if (errcnt == 2)
                {
                    rec = 1;
                    head++;
                }
                else if (errcnt == 3)
                {
                    rec = 1;
                    head = 0;
                    cyl++;
                }
                continue;
            }
            else
            {
                int c, h, r;

                errcnt = 0;
                if (dscb1.ds1dsnam[0] == '\0') break;
                split_cchhr(dscb1.endcchh, &c, &h, &r);
                if (c >= *datacyl)
                {
                    *datacyl = c + 1;
                }
                rec++;
            }
        }
        *dirrec = rec;
        *dirhead = head;
        *dircyl = cyl;
    }
    
    return (0);
}


/* dump block */

static int pdosDumpBlk(PDOS *pdos, char *parm)
{
    int cyl;
    int head;
    int rec;
    char tbuf[MAXBLKSZ];
    long cnt = -1;
    int lastcnt = 0;
    int ret = 0;
    int c, pos1, pos2;
    long x = 0L;
    char prtln[100];
    long i;
    int dev;
    int n;
    int skip = 0;
    int count = INT_MAX;

    tbuf[0] = '\0';
    i = *(short *)parm;
    parm += sizeof(short);
    if (i < (sizeof tbuf - 1))
    {
        memcpy(tbuf, parm, i);
        tbuf[i] = '\0';
    }
    n = sscanf(tbuf, "%x %d %d %d %i %i",
               &dev, &cyl, &head, &rec, &skip, &count);
    if (n < 4)
    {
        printf("usage: dumpblk dev(x) cyl(d) head(d) rec(d) [start] [len]\n");
        printf("dev of 0 will use IPL device\n");
        return (0);
    }
    if (dev == 0)
    {
        dev = pdos->ipldev;
    }
#if defined(S390) || defined(ZARCH)
    if ((dev != 0) && (dev < 0x10000))
    {
        dev = getssid(dev);
    }
#endif
    printf("dumping cylinder %d, head %d, record %d of device %x\n",
           cyl, head, rec, dev);
    cnt = rdblock(dev, cyl, head, rec, tbuf, MAXBLKSZ, 0x0e);
    if (cnt > 0)
    {
        if (cnt <= skip)
        {
            cnt = 0;
        }
        else if ((skip + count) > cnt)
        {
            cnt = cnt - skip;
        }
        else
        {
            cnt = count;
        }
        for (i = skip; i < skip + cnt; i++)
        {
            c = tbuf[i];
            if (x % 16 == 0)
            {
                memset(prtln, ' ', sizeof prtln);
                sprintf(prtln, "%0.6lX   ", skip + x);
                pos1 = 8;
                pos2 = 45;
            }
            sprintf(prtln + pos1, "%0.2X", c);
            if (isprint((unsigned char)c))
            {
                sprintf(prtln + pos2, "%c", c);
            }
            else
            {
                sprintf(prtln + pos2, ".");
            }
            pos1 += 2;
            *(prtln + pos1) = ' ';
            pos2++;
            if (x % 4 == 3)
            {
                *(prtln + pos1++) = ' ';
            }
            if (x % 16 == 15)
            {
                printf("%s\n", prtln);
            }
            x++;
        }
        if (x % 16 != 0)
        {
            printf("%s\n", prtln);
        }
    }
    
    return (cnt);
}


/* zap block */

static int pdosZapBlk(PDOS *pdos, char *parm)
{
    int cyl;
    int head;
    int rec;
    char tbuf[MAXBLKSZ];
    long cnt = -1;
    int lastcnt = 0;
    int ret = 0;
    int c, pos1, pos2;
    long x = 0L;
    char prtln[100];
    long i;
    long start = 0;
    int off;
    int nval;
    int dev;
    int n;

    tbuf[0] = '\0';
    i = *(short *)parm;
    parm += sizeof(short);
    if (i < (sizeof tbuf - 1))
    {
        memcpy(tbuf, parm, i);
        tbuf[i] = '\0';
    }
    n = sscanf(tbuf, "%x %d %d %d %i %i", &dev, &cyl, &head, &rec, &off, &nval);
    if (n != 6)
    {
        printf("usage: zapblk dev(x) cyl(d) head(d) rec(d) off(i) nval(i)\n");
        printf("dev of 0 will use IPL device\n");
        return (0);
    }
    if (dev == 0)
    {
        dev = pdos->ipldev;
    }
#if defined(S390) || defined(ZARCH)
    if ((dev != 0) && (dev < 0x10000))
    {
        dev = getssid(dev);
    }
#endif
    printf("zapping cylinder %d, head %d, record %d, "
           "byte %X of device %x to %0.2X\n",
           cyl, head, rec, off, dev, nval);
    cnt = rdblock(dev, cyl, head, rec, tbuf, MAXBLKSZ, 0x0e);
    if (cnt > 0)
    {
        if (off >= cnt)
        {
            printf("byte to zap is out of range\n");
        }
        else
        {
            tbuf[off] = nval;
            wrblock(dev, cyl, head, rec, tbuf, cnt, 0x0d);
        }
    }
    
    return (cnt);
}


/* new block */

static int pdosNewBlk(PDOS *pdos, char *parm)
{
    int cyl;
    int head;
    int rec;
    char tbuf[MAXBLKSZ];
    long cnt = -1;
    int lastcnt = 0;
    int ret = 0;
    int c, pos1, pos2;
    long x = 0L;
    char prtln[100];
    long i;
    long start = 0;
    int keylen;
    int datalen;
    int dev;
    int n;

    tbuf[0] = '\0';
    i = *(short *)parm;
    parm += sizeof(short);
    if (i < (sizeof tbuf - 1))
    {
        memcpy(tbuf, parm, i);
        tbuf[i] = '\0';
    }
    n = sscanf(tbuf, "%x %d %d %d %i %i",
               &dev, &cyl, &head, &rec, &keylen, &datalen);
    if (n != 6)
    {
        printf("usage: newblk dev(x) cyl(d) head(d) rec(d)"
               " keylen(i) datalen(i)\n");
        printf("dev of 0 will use IPL device\n");
        return (0);
    }
    if (dev == 0)
    {
        dev = pdos->ipldev;
    }
#if defined(S390) || defined(ZARCH)
    if ((dev != 0) && (dev < 0x10000))
    {
        dev = getssid(dev);
    }
#endif
    printf("creating on cylinder %d, head %d, record %d, "
           "of device %x empty block of key length %d and data length %d\n",
           cyl, head, rec, dev, keylen, datalen);
    cnt = keylen + datalen + 8;
    if (cnt > sizeof tbuf)
    {
        printf("too big - %d vs %d\n", cnt, sizeof tbuf);
        return (0);
    }
    memset(tbuf, '\0', cnt);
    *(short *)tbuf = cyl;
    *(short *)(tbuf + 2) = head;
    tbuf[4] = rec;
    tbuf[5] = keylen;
    *(short *)(tbuf + 6) = datalen;
    /* rec must be one less when doing destructive write */
    cnt = wrblock(dev, cyl, head, rec - 1, tbuf, cnt, 0x1d);
    printf("cnt is %d, includes 8 bytes of markup\n", cnt);

    return (cnt);
}


/* Create a ramdisk */

static int pdosRamDisk(PDOS *pdos, char *parm)
{
    int cyl;
    int head;
    char *p;

#if defined(ZARCH)
    ramdisk = memmgrAllocate(&pdos->aspaces[pdos->curr_aspace].o.atlmem,
                             948810752L, 0);
#endif
    if (ramdisk == NULL)
    {
        printf("insufficient memory\n");
        return (-1);
    }
    p = ramdisk;
    memcpy(p, "\x43\x4B\x44\x5F\x50\x33\x37\x30"
              "\x0F\x00\x00\x00\x00\xDE\x00\x00\x90", 0x11);

    p = ramdisk + 0x200;
    for (cyl = 0; cyl < 1113; cyl++)
    {
        for (head = 0; head < 15; head++)
        {
            *(short *)(p + 1) = cyl;
            *(short *)(p + 3) = head;
            *(short *)(p + 5) = cyl;
            *(short *)(p + 7) = head;
            *(char *)(p + 0xc) = 8;
            memcpy(p + 0x15, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8);
            p += 0xde00;
        }
    }
    pdos->ipldev = pdos->curdev = 0x20000;
    printf("ramdisk initialized and known as device 20000\n");
    return (0);
}


/* load executable into memory, on a predictable 1 MB boundary,
   by requesting 5 MB */

static int pdosLoadExe(PDOS *pdos, char *prog, char *parm)
{
    char *raw;
    char *initial;
    char *load;
    /* Standard C programs can start at a predictable offset */
    int entry;
    int cyl;
    int head;
    int rec;
    int i;
    int j;
    char tbuf[MAXBLKSZ];
    char srchprog[FILENAME_MAX+10]; /* give an extra space */
    int cnt = -1;
    int lastcnt = 0;
    int ret = 0;
    DSCB1 dscb1;
    int pe = 0;
    int exeLen;
    int imgsize;
    static int first = 1; /* first executable, ie command.exe */

    if (!first || (!__istape && !__iscard && !__ismem))
    {
    /* try to find the load module's location */
    
    /* +++ replace this 8 with some constant */
    memcpy(srchprog, prog, 8);
    srchprog[8] = ' ';
    *strchr(srchprog, ' ') = '\0';
    strcat(srchprog, ".EXE "); /* extra space deliberate */
    
    /* read VOL1 record */
    cnt = rdblock(pdos->ipldev, 0, 0, 3, tbuf, MAXBLKSZ, 0x0e);
    if (cnt >= 20)
    {
        cyl = head = rec = 0;
        /* +++ probably time to create some macros for this */
        memcpy((char *)&cyl + sizeof(int) - 2, tbuf + 15, 2);
        memcpy((char *)&head + sizeof(int) - 2, tbuf + 17, 2);
        memcpy((char *)&rec + sizeof(int) - 1, tbuf + 19, 1);
        
        while ((cnt =
               rdblock(pdos->ipldev, cyl, head, rec, &dscb1, sizeof dscb1,
                       0x0e))
               > 0)
        {
            if (cnt >= sizeof dscb1)
            {
                if (dscb1.ds1fmtid == '1')
                {
                    dscb1.ds1fmtid = ' '; /* for easy comparison */
                    if (memcmp(dscb1.ds1dsnam,
                               srchprog,
                               strlen(srchprog)) == 0)
                    {
                        cyl = head = 0;
                        rec = 1;
                        /* +++ more macros needed here */
                        memcpy((char *)&cyl + sizeof(int) - 2, 
                               dscb1.startcchh, 2);
                        memcpy((char *)&head + sizeof(int) - 2,
                               dscb1.startcchh + 2, 2);
                        break;
                    }
                }
            }
            rec++;
        }        
    }
    
    if (cnt <= 0)
    {
        *strchr(srchprog, ' ') = '\0';
        printf("executable %s not found!\n", srchprog);
        return (-1);
    }
    cnt = rdblock(pdos->ipldev, cyl, head, rec, tbuf, MAXBLKSZ, 0x0e);
    }
    else if (!__ismem)
    {
        cnt = rdtape(pdos->ipldev, tbuf, MAXBLKSZ);
    }
    if (__iscard || __ismem)
    {
        pe = 1;
    }
    else if ((cnt > 8) && (*((int *)tbuf + 1) == 0xca6d0f))
    {
        pe = 1;
    }
    else
    {
        printf("only MVS PE executables are supported\n");
        return (-1);
    }

    /* assume 5 MB max */
#if defined(ZARCH)
    raw = memmgrAllocate(&pdos->aspaces[pdos->curr_aspace].o.atlmem,
                         5 * 1024 * 1024, 0);
#else
    raw = memmgrAllocate(&pdos->aspaces[pdos->curr_aspace].o.btlmem,
                         5 * 1024 * 1024, 0);
#endif
    if (raw == NULL)
    {
        printf("insufficient memory to load program\n");
        return (-1);
    }

#if 0
    /* should be no need for specific alignment */
    /* round to 16 byte boundary */
    load = (char *)(((int)raw & ~0xf) + 0x10);
#endif
    load = raw;
    initial = load;
#if 0
    printf("load point designated as %p\n", load);
#endif
    /* should store old context first */
    i = head;
    j = rec;
    if (first)
    {
    if (__istape)
    {
        memcpy(load, tbuf, cnt);
        load += cnt;
    }
    else if (__iscard)
    {
        imgsize = *(int *)tbuf;
    }
    else if (__ismem)
    {
        /* command.exe should be at the 2 GiB location */
        char *memloc = (char *)(2UL * 1024 * 1024 * 1024);

        /* assume 1 MiB in size */
        memcpy(load, memloc, 1 * 1024 * 1024);
        load += 1 * 1024 * 1024;
        cnt = 0;
    }
    }
    /* Note that we read until we get EOF (a zero-length block). */
    /* +++ note that we need a security check in here to ensure
       that people don't leave out an EOF to read the next guy's
       data - use the endcchh */
    while (cnt != 0)
    {
#if DSKDEBUG
        printf("loading to %p from %d, %d, %d\n", load,
               cyl, i, j);
#endif
        if (!first || (!__istape && !__iscard))
        {
        cnt = rdblock(pdos->ipldev, cyl, i, j, tbuf, MAXBLKSZ, 0x0e);
        }
        else
        {
            cnt = rdtape(pdos->ipldev, tbuf, MAXBLKSZ);
            if (__iscard && (cnt >= 80))
            {
                cnt = 72;
            }
        }
#if DSKDEBUG
        printf("cnt is %d\n", cnt);
#endif
        if (cnt == -1)
        {
            if (lastcnt == -2)
            {
                printf("three I/O errors in a row - terminating load\n");
                break;
            }
            else if (lastcnt == -1)
            {
#if DSKDEBUG
                printf("probably last track on cylinder\n");
#endif
                /* probably reached last track on cylinder */
                lastcnt = -2;
                cyl++;
                i = 0;
                j = 1;
                continue;
            }
            /* probably reached last record on track */
            lastcnt = -1;
            j = 1;
            i++;
            continue;
        }
        lastcnt = cnt;
        memcpy(load, tbuf, cnt);
        load += cnt;
        j++;
        if (first)
        {
        if (__istape && (cnt != 18452)) break;
        if (__iscard && ((load - initial) >= imgsize)) break;
        }
    }

    exeLen = load - initial;    
    if (pe)
    {
        if (fixPE(initial, &exeLen, &entry, (int)initial) != 0)
        {
#if defined(ZARCH)
            memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.atlmem, raw);
#else
            memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.btlmem, raw);
#endif
            return (-1);
        }
        exeLen += 0x1000; /* give some buffer */
        /* unconditionally reduce the 5 MB to something reasonable */
#if defined(ZARCH)
        memmgrRealloc(&pdos->aspaces[pdos->curr_aspace].o.atlmem,
                      raw, exeLen);
#else
        memmgrRealloc(&pdos->aspaces[pdos->curr_aspace].o.btlmem,
                      raw, exeLen);
#endif
        /* if we can't adjust down, don't care */
    }

    pdos->context->next_exe = raw;

    /* get a new RB */
#if defined(ZARCH)
    pdos->context = memmgrAllocate(
        &pdos->aspaces[pdos->curr_aspace].o.atlmem,
        sizeof *pdos->context, 0);
#else
    pdos->context = memmgrAllocate(
        &pdos->aspaces[pdos->curr_aspace].o.btlmem,
        sizeof *pdos->context, 0);
#endif
    if (pdos->context == NULL)
    {
        /* free the memory that was allocated to the executable */
#if defined(ZARCH)
        memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.atlmem, raw);
#else
        memmgrFree(&pdos->aspaces[pdos->curr_aspace].o.btlmem, raw);
#endif
        ret = -1;
    }
    else
    {
        /* switch to new context */
        memset(pdos->context, 0x00, sizeof *pdos->context);
        pdos->context->rblinkb =
            pdos->aspaces[pdos->curr_aspace].o.curr_rb;
        pdos->aspaces[pdos->curr_aspace].o.curr_rb = pdos->context;

        /* fill in details of new context */
        pdos->context->regs[13] = (int)pdos->context->savearea;
        pdos->context->regs[14] = (int)gotret;
        pdos->context->regs[15] = entry;
        pdos->context->psw1 = PSW_ENABLE_INT; /* need to enable interrupts */
#if defined(ZARCH) && !defined(ZAM31)
        pdos->context->psw1 |= 1; /* dispatch in 64-bit mode */
#endif
        pdos->context->psw2 = entry; /* 24-bit mode for now */
#if defined(S380) || defined(S390) || defined(ZARCH)
        pdos->context->psw2 |= 0x80000000; /* dispatch in 31-bit mode */
#endif

        pdos->context->pptrs[0] = parm;
    
        pdos->context->regs[1] = (int)pdos->context->pptrs;

#if 0
        printf("finished loading executable\n");
#endif
        first = 0;
    }
    return (ret);
}


static int pdosDumpMem(PDOS *pdos, void *buf, int cnt)
{
    int ret = 0;
    int c, pos1, pos2;
    long x = 0L;
    char prtln[100];
    long i;
    long start = 0;
    char *tbuf = (char *)buf;

    if (cnt > 0)
    {
        for (i = 0; i < cnt; i++)
        {
            c = tbuf[i];
            if (x % 16 == 0)
            {
                memset(prtln, ' ', sizeof prtln);
                sprintf(prtln, "%0.6lX   ", start + x);
                pos1 = 8;
                pos2 = 45;
            }
            sprintf(prtln + pos1, "%0.2X", c);
            if (isprint((unsigned char)c))
            {
                sprintf(prtln + pos2, "%c", c);
            }
            else
            {
                sprintf(prtln + pos2, ".");
            }
            pos1 += 2;
            *(prtln + pos1) = ' ';
            pos2++;
            if (x % 4 == 3)
            {
                *(prtln + pos1++) = ' ';
            }
            if (x % 16 == 15)
            {
                printf("%s\n", prtln);
            }
            x++;
        }
        if (x % 16 != 0)
        {
            printf("%s\n", prtln);
        }
    }
    
    return (0);
}


/* load the PCOMM executable. Note that this should
   eventually be changed to call a more generic
   loadExe() routine */

static int pdosLoadPcomm(PDOS *pdos)
{
    static char mvsparm[] = { "\x00" "\x02" "/P" };

    if (pdosLoadExe(pdos, "COMMAND", mvsparm) != 0)
    {
        printf("can't load COMMAND.EXE\n");
        printf("PDOS terminating\n");
        return (-1);
    }
    return (0);
}


static void split_cchhr(char *cchhr, int *cyl, int *head, int *rec)
{
    *cyl = *head = *rec = 0;
    memcpy((char *)cyl + sizeof *cyl - 2, cchhr, 2);
    memcpy((char *)head + sizeof *head - 2, cchhr + 2, 2);
    *rec = cchhr[4];
    return;
}


static void join_cchhr(char *cchhr, int cyl, int head, int rec)
{
    memcpy(cchhr, (char *)&cyl + sizeof cyl - 2, 2);
    memcpy(cchhr + 2, (char *)&head + sizeof head - 2, 2);
    cchhr[4] = rec;
    return;
}

static void write3270(char *buf, size_t lenbuf, int cr)
{
    static char intbuf[6+22*80+7];
    static int first = 1;
    static int lineupto = 0;

    if (first)
    {
        first = 0;
        memset(intbuf, ' ', sizeof intbuf);
        memcpy(intbuf, "\xc3\x11\x5d\x7f\x1d\xf0", 6);
        memcpy(intbuf + 6 + 22 * 80, "\x1d\x00\x13\x3c\x5d\x7f\x00", 7);
    }
    memset(intbuf + 6 + lineupto * 80, ' ', 80);
    memcpy(intbuf + 6 + lineupto * 80, buf, lenbuf);
    __conswr(sizeof intbuf, intbuf, 0);
    lineupto++;
    if (lineupto == 22)
    {
        lineupto--;
        memmove(intbuf + 6, intbuf + 6 + 80, lineupto * 80);
    }
}

static int cprintf(char *format, ...)
{
    static char buf[3000];
    char tbuf[300];
    va_list args;
    int len;
    char *fin;
    char *p;
    char *q;
    int cr;

    va_start(args, format);
    len = vsprintf(buf, format, args);
    va_end(args);
    fin = buf + len;
    p = buf;
    while (p < fin)
    {
        q = memchr(p, '\n', fin - p);
        if (q == NULL)
        {
            cr = 0;
            q = fin;
        }
        else
        {
            cr = 1; /* assembler needs to do with CR */
        }
        if (cons_type == 3270)
        {
            write3270(p, q - p, cr);
        }
        else
        {
            memcpy(tbuf, p, q - p);
            __conswr(q - p, tbuf, cr);
        }
        p = q;
        if (cr)
        {
            p++;
        }
    }
    return (len);
}

static int getssid(int devnum)
{
    int ssid = 0x10000;
    int tempdn;

    while (1)
    {
        tempdn = getdevn(ssid);
        if (tempdn == 0)
        {
            ssid = 0;
            break;
        }
        if (tempdn == devnum) break;
        ssid++;
    }
    return (ssid);
}

static int int_wrblock(int dev, int cyl, int head, int rec,
                       void *buf, int len, int cmd)
{
    unsigned char *p;
    int curr;
    unsigned char *start;

    if (dev != 0x20000)
    {
        return ((wrblock)(dev, cyl, head, rec, buf, len, cmd));
    }
    if (head >= 15) return (-1);
    p = ramdisk + 0x200 + 0xde00 * head + 0xde00 * 15 * cyl + 0x15;
    start = p;
    curr = 0; /* currently positioned after record 0 */
    while (curr < rec)
    {
        /* if we've reached the end of the track, it's an error */
        if (*(unsigned short *)p == 0xffff)
        {
            return (-1);
        }
        p += p[5] + *(unsigned short *)(p + 6) + 8;
        curr++;
    }
    /* this algorithm is probably wrong, but probably good enough */
    if ((p + len) > (start + (18452 + 8) * 3))
    {
        return (-1);
    }
    memcpy(p, buf, len);
    memcpy(p + len, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8);
    return (len);
}

/* instead of defining this internal rdblock in two places
   (the other place pload.c) we could instead rename the
   assembler routine to say ARDBLOCK and then define rdblock
   as a C function in pdosutil. But at the moment that will
   lose the ability to do printf. Even at the moment, you
   can only do printf if you override the console device
   number and type with an IPL parameter */

int int_rdblock(int dev, int cyl, int head, int rec,
                void *buf, int len, int cmd)
{
    unsigned char *p;
    int curr;
    unsigned char *start;
    int len2;

    if (dev != 0x20000)
    {
        return ((rdblock)(dev, cyl, head, rec, buf, len, cmd));
    }
    p = ramdisk + 0x200 + 0xde00 * head + 0xde00 * 15 * cyl + 0x15;
    start = p;
    curr = 0; /* currently positioned after record 0 */
    if (cmd == 0x1e) rec++; /* bit of a kludge */
    while (curr < (rec - 1))
    {
        /* if we've reached the end of the track, it's an error */
        if (*(unsigned short *)p == 0xffff)
        {
            return (-1);
        }
        p += p[5] + *(unsigned short *)(p + 6) + 8;
        curr++;
    }
    /* we should be positioned correctly. If not, it's an error */
    if (*(short *)p != cyl) return (-1);
    if (*(short *)(p + 2) != head) return (-1);
    if (*(unsigned char *)(p + 4) != rec) return (-1);
    len2 = p[5] + *(unsigned short *)(p + 6);
    p += 8;
    if (cmd == 0x1e)
    {
        len2 += 8;
        p -= 8;
    }
    if (len2 > len)
    {
        len2 = len;
    }
    memcpy(buf, p, len2);
    return (len2);
}

static int ins_strncmp(char *one, char *two, size_t len)
{
    size_t x = 0;

    if (len == 0) return (0);
    while ((x < len) && (toupper(*one) == toupper(*two)))
    {
        if (*one == '\0')
        {
            return (0);
        }
        one++;
        two++;
        x++;
    }
    if (x == len) return (0);
    return (toupper(*one) - toupper(*two));
}


#if 0
/* start an independent process (with its own virtual address space) */

static int pdosStart(PDOS *pdos, char *pgm, char *parm)
{
    
}
#endif












#if 0

/* This is a high-level conceptual version of what PDOS
   needs to do, but this code is just a big comment. The
   heart of PDOS is in pdosRun which is called from the
   real main. */

int main(int argc, char **argv)
{
    /* Note that the loader may pass on a parameter of some sort */
    /* Also under other (test) environments, may have options. */

    /* Note that the bulk of the operating system is set up in
       this initialization, but this main routing focuses on
       the heart of the OS - the actual millisecond by millisecond
       execution, where people (ie programs) are expecting to get
       some CPU time. */
    init();
    
    task = NULL;
    
    /* The OS is not designed to exit */
    while (1)
    {
        /* all this will be done with interrupts disabled,
           and in supervisor status. This function will take
           into account the last executing task, to decide
           which task to schedule next. As part of that, it
           will also mark the last task as "no longer executing" */
        task = getNextAvailTask();
        if (task == NULL)
        {
            /* suspend the computer until an interrupt occurs */
            intrupt = goWaitState();
            if (intrupt == INTERRUPT_TIMER)
            {
                /* may eventually do some housekeeping in this
                   idle time */
            }
            else if (intrupt == INTERRUPT_IO)
            {
                processIO();
            }
            /* we're not expecting any other interrupts. If we get
               any, put it in the too-hard basket! */
            
            continue; /* go back and check for a task */
        }
        
        /* we have a task, and now it is time to run it, in user
           mode (usually) until an interrupt of some sort occurs */
        sliceOver = 0;
        while (!sliceOver)
        {
            sliceOver = 1; /* most interrupts will cause this timeslice
                              to end, so set that expectation by
                              default. */
        
            /* run the user's code (stock-standard general instructions,
               e.g. LA, LR, AR) until some interrupt occurs where the 
               OS gets involved. On return from this function, interrupts
               will be redisabled. In addition, the task will be in a
               saved state, ie all registers saved. The handling of
               the interrupt may alter the state of the task, to make
               it e.g. shift to an SVC routine. */
               
            intrupt = dispatch_until_interrupt();
            if (intrupt == INTERRUPT_TIMER)
            {
                /* this will check how long the program has been
                   running for, and if the limit has been reached,
                   the context will be switched to an abend routine,
                   which will be dispatched at this task's next
                   timeslice. */
                checkElapsedTime();
            }
        
            else if (intrupt == INTERRUPT_ILLEGAL)
            {
                /* This is called if the user program has executed
                   an invalid instruction, or accessed memory that
                   doesn't belong to it, etc */
                /* this function will switch the context to an abend
                   handler to do the dump, timesliced. */
                /* If the task has abended, the equivalent of a
                   branch to user-space code is done, to perform
                   the cleanup, as this is an expensive operation,
                   so should be timesliced. */
                processIllegal();
            }
        
            else if (intrupt == INTERRUPT_SVC)
            {
                /* This function switches context to an SVC handler,
                   but it won't actually execute it until this task's
                   next timeslice, to avoid someone doing excessive
                   SVCs that hog the CPU. It will execute in
                   supervisor status when eventually dispatched though. */
                /* We can add in some smarts later to allow someone
                   to do lots of quick SVCs, so long as it fits into
                   their normal timeslice, instead of being constrained
                   to one per timeslice. But for now, avoid the
                   complication of having an interruptable SVC that
                   could be long-running. */
                startSVC();
            }
        
            else if (intrupt == INTERRUPT_IO)
            {
                /* This running app shouldn't have to pay the price
                   for someone else's I/O completing. */
                
                saveTimer();
                
                processIO();
                
                /* If the timeslice has less than 10% remaining, it
                   isn't worth dispatching again. Otherwise, set
                   this slice to be run again. */
                if (resetTimer())
                {
                    sliceOver = 0;
                }
            }
        }
    }
    return (0); /* the OS may exit at some point */
}
#endif





#if 0

/*
Suggestions for a general-purpose OS:


1. Real-time - within a general-purpose system, I
should be able to define a task that will be
certain to be dispatched as soon as the I/O it is
waiting on has been completed. But I don't expect
such tasks to require 100% of the CPU. 10% of the
CPU seems more reasonable. If anyone has complaints
about that, they should buy a bigger CPU. But within
that 10% powerful CPU, I do expect immediate dispatch
of real-time tasks, as soon as the device they are
waiting on is complete. So a link from the I/O
interrupt handler to real time tasks associated with
that I/O device.


2. If I am an end user, doing light activity, I
expect near-immediate responses, like 0.1 seconds.
ie my light activity is high priority by default.
So the time to dispatch for a new task should be
low. I guess that means the terminal controller.
But since there will be users abusing that, by
running ind$file, when those repeat (ab)users are
detected, they are put on a "slow dispatch queue",
and treated like batch jobs (infrequently
dispatched - but they can be dispatched for longer
periods of time when they do get dispatched).

But the principle is to find out where the humans
are, and ensure that so long as they are just
editting a file in fullscreen mode, that the
light CPU activity of reading a block from disk
and forming it into a 3270 screen, is done within
0.1 seconds, from the time they hit enter after
spending 5 seconds to find the "e" on their
keyboard.

I'll assume the general case where there's no
deliberate Denial of Service attack, or users on
a production system risking their jobs by
deliberately running software masquerading as
light users.

So it's not real-time (as in a device needs a
response within 0.01 seconds, otherwise the
nuclear plant goes into Two Mile Island mode).
But it is human-time, as in failure to respond
as quickly as a monkey can put his finger on
and off the PF8 button, means you're making a
C programmer cranky, especially when you know
that after doing this 10 times, he's going to
spend the next 10 minutes doing nothing that
the computer sees, so he's hardly the anti-social 
prick he's made out to be.

The same goes for CICS users. I don't see why
CICS is required. Why can't the OS do what CICS
does? Once again, after displaying the results
of a balance inquiry, a human needs to move
their jawbone to say "have a nice day", so in
the brief period of time that they are actually
requesting services, response should be fast.


3. People running a long-running batch job don't
need frequent dispatching if the overhead of
dispatching is going to be great. So infrequent
dispatching, but within that, prioritize I/O-bound
tasks up to the point that they're getting the
same amount of CPU as their CPU-bound cousins.


So, a strategy for real-time, a strategy for humans
and a strategy for batch.

As each I/O interrupt comes in, it should (a
priori) be associated with one of those things,
which will determine what to do next.


In the general case of a mixed workload, and
trying to minimize the overhead of gratuitous
task switching, we need the following:

A linked list of currently dispatchable tasks.
Taking the general case of them all being
CPU-bound, and with the say 2 real-time tasks 
only taking 10% of the CPU, and the other (say
40) dispatchable human tasks taking 80%, and 
the (say 20) batch jobs taking 10%, then in
0.1 seconds we would want to see the 0.1 seconds
divided up amongst the 2 real-time, 40 human, 
and say 1 batch job (meaning the batch jobs are
only dispatched once every 2 seconds).

So let's say we're on a 100 MIPS CPU.

That means we have 10 million instructions for 
the next 0.1 seconds.

each real time task will get 50% * 10% * 10 = 0.5 million

each human task will get 1/40 * 80% * 10 = 0.2 million

the one batch job will get 10% * 10 = 1 million instructions.

And it means 43 task switches.

Because these numbers will vary, we want to 
make the task switching as efficient as possible.

So a single function which scans through the 52
tasks. Since 9 of the batch jobs aren't meant to
be dispatched, they instead have countdown timers.
They've all been seeded with numbers from 1-10,
and when it gets down to 0, they get dispatched,
and the countdown is reset to 10.

Any new tasks joining the queue will fit in with
this schema in an appropriate position, starting
off as human, downgrading to batch.

That process of downgrading and initial setup will
be a logically separate task that is nominally
timesliced itself.

As well as a countdown, they also have a timer
associated with them. So each realtime task would
get 0.5 million in its timer. This number would
vary as appropriate as new tasks are added and
removed.

So, it's a case of

for each dispatchable task (in an infinite loop) repeat
  if (counter == 0) or (--counter == 0)
     set timer to value specified
     copy registers and PSW into low memory
     LM
     LPSW
     (wait for timer)
     LM
     copy registers and PSW back
  end
end

Keep it simple and fast. Perhaps eventually written
in assembler. It may be possible to complicate it a
bit in order to avoid the movement into low memory.
ie do the LM directly from the task's context, and
then load one register from low memory, instead of 16.
If the total number of dispatchable tasks is under
say 200, one register, plus the two PSW words, could
be kept below the 4096 mark.

When an I/O interrupt comes in, and it's a realtime
task, that is added into the linked list, and
immediately dispatched for its 0.5 million alloted
instructions.

Human I/O is added at the end of the queue, to avoid
possible thrashing.

*/
#endif




#if 0

Suggestions for MVS 3.8j:

A normal MVS 3.8j address space has a private
region of say 2MB-10MB, and uses 64k segments.

So.

For a particular job class, say CLASS=X (X=extended),
and this will only work for about 6 or thereabout
address spaces before you run out of EA memory,
what you do at address space creation time is:

1. Make MVS 3.8j allocate the 2 MB-10MB region
to 16MB-24MB in extended real memory, and be
page fixed.

2. The 370 DAT points to BTL real memory for the
0-2MB and 10-16MB areas, and that gets paged.

3. The 370 DAT points to EA for the 2-10MB region.

4. An XA DAT also points to the same areas of
memory as 370 for the 0-16 MB region, but then
starts pointing to say 256-368 MB for the
next 112 MB (giving a total address space of
128 MB) - all via 64k segments.

5. Not sure what happens in an interrupt, ie
each interrupt may need to have CR0.10 saved,
set to 0 for MVS 3.8j use, then restored.

MVS 3.8j page tables can remain as 2-byte,
370 mode, yet 6 or whatever address spaces
can get up to 128 MB (it may be possible to steal
more bits to get up to 2 GB if required).

#endif
