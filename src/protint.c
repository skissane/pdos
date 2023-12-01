/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  protint - protected mode interface                               */
/*                                                                   */
/*  This code is linked into the real mode executable and allows     */
/*  the execution of a protected mode binary, and also for the       */
/*  protected mode to call a real mode interrupt (runreal).          */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>

#include "protint.h"
#include "lldos.h"
#include "support.h"
#include "a_out.h"
#include "unused.h"

#ifndef __SUBC__
unsigned long rawprota(unsigned long corsubr, 
                       unsigned long codecor,
                       unsigned long newsp,
                       unsigned long parmlist);
#endif

unsigned long runreal(int intno, unsigned short *regs);
void rtop_stage2(void);
unsigned long protget32(void);

extern unsigned long newstack;

#ifdef __SUBC__
extern long interrupts[];
extern int dseg;
#endif

typedef struct {
    unsigned short limit;
    unsigned short base15_0;
    unsigned char base23_16;
    unsigned char access;
    unsigned char gran_limit;
    unsigned char base31_24;
} descriptor;

static struct {
    descriptor null_descriptor;
    descriptor os_code;
    descriptor os_data;
    descriptor small_code;
    descriptor small_data;
    descriptor spawn_code;
    descriptor spawn_data;
#ifdef __SUBC__
} descriptors;
#else
} descriptors = { 
  {0},
  { 0xffff, 0x0, 0xff, 0x9a, 0xcf, 0xff },
  { 0xffff, 0x0, 0xff, 0x92 /* not code, goes up, writable */, 0xcf, 0xff},
  { 0xffff, 0x0, /* scbase */ 0x00, 0x9a, 0x00, 0x00 },
  { 0xffff, 0x0, /* sdbase */ 0x00, 0x92, 0x00, 0x00 },
  { 0xffff, 0x0, 0x00, 0x9a, 0xcf, 0x00 },
  { 0xffff, 0x0, 0xff, 0x92, 0xcf, 0xff }
};
#endif

typedef struct {
    unsigned short len;
    unsigned long absaddr;
#ifdef __SUBC__
    unsigned int dummy;
#endif
} descptr;

descptr gdtinfo = { 0x37 
#ifdef __SUBC__
,0,0
#endif
};

descptr idtinfo = { 0x7ff
#ifdef __SUBC__
,0,0
#endif
};

/* this real mode idt pointer points to the standard real mode
   interrupts.  We allow room for 256 interrupts. */
descptr ridtinfo = { 0x3ff, 0L
#ifdef __SUBC__
,0
#endif
};

/* interrupt number, 6 registers, cflag, flags, 4 segment registers,
   4 high halves of eax, ebx, ecx, edx */
static unsigned short intbuffer[1 + 6 + 1 + 1 + 4 + 4];

static unsigned long mycbase;
static unsigned long mydbase;

static unsigned long dorealint(unsigned long parm);


/* we shouldn't need these intermediate function pointers,
   but I am working around the fact that Watcom generates
   the seg keyword which is unsuitable for the a.out format */
#ifdef __SUBC__
static void (*rs2)(void);
static unsigned long (*rpfunc)(unsigned long corsubr,
                       unsigned long codecor,
                       unsigned long newsp,
                       unsigned long parmlist);
static unsigned long (*rrfunc)(int intno, unsigned short *regs);
static unsigned long (*drifunc)(unsigned long parm);
#else
static void (*rs2)(void) = rtop_stage2;
static unsigned long (*rpfunc)(unsigned long corsubr,
                       unsigned long codecor,
                       unsigned long newsp,
                       unsigned long parmlist) = rawprota;
static unsigned long (*rrfunc)(int intno, unsigned short *regs) = runreal;
static unsigned long (*drifunc)(unsigned long parm) = dorealint;
#endif

#ifdef __SUBC__
static rawprot_parms parmlist;
#endif

unsigned long rawprot(unsigned long csbase,
                      unsigned long ip,
                      unsigned long dsbase,
                      unsigned long prot_sp,
                      unsigned long userparm,
                      unsigned long intloc)
{
    unsigned long absgdt;
    unsigned long codecor;
    unsigned long parmlist_p;
#ifndef __SUBC__
    rawprot_parms parmlist;
#endif
    unsigned long myc32base;

#ifdef __SUBC__

  /* {0}, */
    descriptors.null_descriptor.limit = 0;
    descriptors.null_descriptor.base15_0 = 0;
    descriptors.null_descriptor.base23_16 = 0;
    descriptors.null_descriptor.access = 0;
    descriptors.null_descriptor.gran_limit = 0;
    descriptors.null_descriptor.base31_24 = 0;

  /* { 0xffff, 0x0, 0xff, 0x9a, 0xcf, 0xff }, */
    descriptors.os_code.limit = 0xffff;
    descriptors.os_code.base15_0 = 0;
    descriptors.os_code.base23_16 = 0xff;
    descriptors.os_code.access = 0x9a;
    descriptors.os_code.gran_limit = 0xcf;
    descriptors.os_code.base31_24 = 0xff;

  /* { 0xffff, 0x0, 0xff, 0x92, 0xcf, 0xff}, */
    descriptors.os_data.limit = 0xffff;
    descriptors.os_data.base15_0 = 0;
    descriptors.os_data.base23_16 = 0xff;
    descriptors.os_data.access = 0x92;
    descriptors.os_data.gran_limit = 0xcf;
    descriptors.os_data.base31_24 = 0xff;

  /* { 0xffff, 0x0, 0x00, 0x9a, 0x00, 0x00 }, */
    descriptors.small_code.limit = 0xffff;
    descriptors.small_code.base15_0 = 0;
    descriptors.small_code.base23_16 = 0;
    descriptors.small_code.access = 0x9a;
    descriptors.small_code.gran_limit = 0;
    descriptors.small_code.base31_24 = 0;

  /* { 0xffff, 0x0, 0x00, 0x92, 0x00, 0x00 }, */
    descriptors.small_data.limit = 0xffff;
    descriptors.small_data.base15_0 = 0;
    descriptors.small_data.base23_16 = 0;
    descriptors.small_data.access = 0x92;
    descriptors.small_data.gran_limit = 0;
    descriptors.small_data.base31_24 = 0;

  /* { 0xffff, 0x0, 0x00, 0x9a, 0xcf, 0x00 }, */
    descriptors.spawn_code.limit = 0xffff;
    descriptors.spawn_code.base15_0 = 0;
    descriptors.spawn_code.base23_16 = 0;
    descriptors.spawn_code.access = 0x9a;
    descriptors.spawn_code.gran_limit = 0xcf;
    descriptors.spawn_code.base31_24 = 0;

  /* { 0xffff, 0x0, 0xff, 0x92, 0xcf, 0xff } */
    descriptors.spawn_data.limit = 0xffff;
    descriptors.spawn_data.base15_0 = 0;
    descriptors.spawn_data.base23_16 = 0xff;
    descriptors.spawn_data.access = 0x92;
    descriptors.spawn_data.gran_limit = 0xcf;
    descriptors.spawn_data.base31_24 = 0xff;

#endif

    myc32base = protget32();
    if (myc32base == 0)
    {
#ifdef __SUBC__
        myc32base = (unsigned long)(rs2);
#else
        myc32base = (unsigned long)(void (far *)())(rs2);
#endif
    }
    myc32base = ADDR2ABS(myc32base);
    /* this masking is because the offsets in protinta already
       have the offset correct, so we don't want to add that
       twice. The offset of the TEXT32 segment will always be
       less than 16 I think */
    myc32base = myc32base & 0xfffffff0UL;
#if defined(__WATCOMC__) && defined(OLDMODEL)
    {
        /* for Watcom, the TEXT32 segment is not merged in
        with the TEXT segment, so protget32 is different, in
        that it returns a non-zero value that gives us much
        of the information we need. We still need to add the
        code base though. */
        unsigned long extra;
        extra = (unsigned long)(void (far *)())(rs2);
        extra = ((extra >> 16) << 4);
        dumpbuf("", 0); /* +++ why is this no-op required to make Watcom
            work? Without it, the first extra (0x600) is not added.
            Looks like a bug in Open Watcom 1.6.
            Good version is generating mov ax,cs
            Bad version is generating mov ax,offset _rtop_stage2 */

        myc32base += extra;
        myc32base += 0x100; /* psp */
    }
#endif

#ifdef __SUBC__
    mycbase = (unsigned long)rpfunc;
#else
    mycbase = (unsigned long)(void (far *)())rpfunc;
#endif
    mycbase = (mycbase >> 16) << 4;

#ifdef __SUBC__
    mydbase = (unsigned long)((void far *)&newstack);
#else
    mydbase = (unsigned long)(void far *)&newstack;
#endif
    mydbase = (mydbase >> 16) << 4;

#ifdef __SUBC__
    mycbase = 0x0;
    mydbase = dseg;
#endif

    descriptors.os_code.base15_0 = (unsigned short)csbase;
    descriptors.os_code.base23_16 = (unsigned char)(csbase >> 16);
    descriptors.os_code.base31_24 = (unsigned char)(csbase >> 24);
    descriptors.os_data.base15_0 = (unsigned short)dsbase;
    descriptors.os_data.base23_16 = (unsigned char)(dsbase >> 16);
    descriptors.os_data.base31_24 = (unsigned char)(dsbase >> 24);
    
    descriptors.small_code.base15_0 = (unsigned short)mycbase;
    descriptors.small_code.base23_16 = (unsigned char)(mycbase >> 16);
    descriptors.small_code.base31_24 = (unsigned char)(mycbase >> 24);
#ifdef __SUBC__
    descriptors.small_data.base15_0 = (unsigned short)(mydbase << 4);
    descriptors.small_data.base23_16 = (unsigned char)(mydbase >> 12);
    descriptors.small_data.base31_24 = 0;
#else
    descriptors.small_data.base15_0 = (unsigned short)mydbase;
    descriptors.small_data.base23_16 = (unsigned char)(mydbase >> 16);
    descriptors.small_data.base31_24 = (unsigned char)(mydbase >> 24);
#endif
    
#ifdef __SUBC__
    absgdt = (int)&descriptors;
    /* this will actually go negative, but the rules of
       maths apparently don't care */
    absgdt += (dseg << 4);
#else
    absgdt = ADDR2ABS(&descriptors);    
#endif

    gdtinfo.absaddr = absgdt;

    idtinfo.absaddr = intloc;
    
    codecor = myc32base - csbase;

#ifdef __SUBC__
    memset(&parmlist, 0x00, sizeof parmlist);
#endif

    parmlist.dsbase = dsbase;
    parmlist.gdt = absgdt;
    parmlist.freem_start = prot_sp; /* free memory in 1 MiB range starts
                                       after the stack */
    parmlist.intloc = intloc;
    parmlist.userparm = userparm;
    parmlist_p = ADDR2ABS(&parmlist);
#ifdef OLDMODEL
    parmlist_p -= dsbase;
#endif

#ifdef __SUBC__
    parmlist_p = (dseg << 4) + (int)&parmlist;

    /* the 9 puts the stack at 0x90000, leaving plenty of
       room for pdos.sys to load */
    return (rawprota(0, 0x2, 0, 0, 0, 0x9, parmlist_p, 0));

    /* return (rawprota(ip, codecor, prot_sp, parmlist_p)); */
#else
    return (rpfunc(ip, codecor, prot_sp, parmlist_p));
#endif
}

#ifdef __SUBC__
    /* keep variables in the first 64k of RAM */
static runprot_parms runparm;
#endif

unsigned long runprot(unsigned long csbase,
                      unsigned long ip,
                      unsigned long dsbase,
                      unsigned long prot_sp,
                      unsigned long userparm)
{
    unsigned long intloc;
#ifndef __SUBC__
    runprot_parms runparm;
#endif
    unsigned long runparm_p;
    unsigned long myc32base;
#ifndef __SUBC__
    unsigned long interrupts[256*2]; /* allow room for 256 interrupts */
#endif
    
#ifdef __SUBC__
    rs2 = rtop_stage2;
    rpfunc = rawprota;
    rrfunc = runreal;
    /* because runreal is in TEXT32, we apparently need
       to cope with the relocation correction ourselves */
    rrfunc += 0x700;
    drifunc = dorealint;
#endif

    intloc = ADDR2ABS(interrupts);
#ifdef __SUBC__
    /* it doesn't seem to matter that this becomes negative */
    intloc += (dseg << 4);
#endif

#ifdef __SUBC__
    memset(&runparm, 0x00, sizeof runparm);
#endif
    runparm.userparm = userparm;    
    runparm.intbuffer = ADDR2ABS(intbuffer);

    myc32base = protget32();
#ifdef __SUBC__
    runparm.runreal = (int)rrfunc;
#else
    if (myc32base == 0)
    {
        runparm.runreal = CADDR2ABS(rrfunc);
    }
    else
    {
        runparm.runreal = ((myc32base >> 16) << 4) + (unsigned short)rrfunc;
#if defined(__WATCOMC__) && defined(OLDMODEL)
        {
            unsigned long extra;
            extra = (unsigned long)(void (far *)())(rrfunc);
            extra = ((extra >> 16) << 4);
            runparm.runreal += extra;
            runparm.runreal += 0x100; /* psp */
        }
#endif
    }
#endif


#ifdef __SUBC__
    runparm.dorealint = (unsigned long)drifunc;
    runparm_p = (dseg << 4) + (int)&runparm;
#else
    runparm.dorealint = (unsigned long)(void (far *)())drifunc;
    runparm_p = ADDR2ABS(&runparm);
#endif
    
#ifdef OLDMODEL
    runparm_p -= dsbase;
#endif
    
    return (rawprot(csbase, ip, dsbase, prot_sp, runparm_p, intloc));
}

static unsigned long getlong(unsigned long loc)
{
    unsigned long retval;

    retval = ((unsigned long)getabs(loc + 3) << 24)
             | ((unsigned long)getabs(loc + 2) << 16)
             | ((unsigned long)getabs(loc + 1) << 8)
             | (unsigned long)getabs(loc);
    return (retval);
}

static void putlong(unsigned long loc, unsigned long val)
{
    putabs(loc, (unsigned int)(val & 0xff)); val >>= 8;
    putabs(loc + 1, (unsigned int)(val & 0xff)); val >>= 8;
    putabs(loc + 2, (unsigned int)(val & 0xff)); val >>= 8;
    putabs(loc + 3, (unsigned int)(val & 0xff));
    return;
}

unsigned long runaout(char *fnm, unsigned long absaddr, unsigned long userparm)
{
    FILE *fp;
    int ret;
    unsigned long curraddr;
    char buf[512];
    int x;
    int y;
    struct exec firstbit;
    int numreads;
    unsigned long sp;
    unsigned long z;
    unsigned long offs;
    unsigned long type;
    unsigned long zaploc;
    unsigned long oldval;
    unsigned long newval;
    unsigned long corrloc;
    unsigned long progret;

    fp = fopen(fnm, "rb");
    if (fp == NULL) return (-1);
#if 1
    /* This version accepts a relocatable executable,
       and relocates it */
    /* at time of writing, fread() is only known to work on
       512-byte buffers */
    curraddr = absaddr;
    ret = fread(buf, 1, sizeof buf, fp);
    memcpy(&firstbit, buf, sizeof firstbit);
    while (1)
    {
#ifndef OLDMODEL
        memcpy(FP_NORM(ABS2ADDR(curraddr)), buf, ret);
#else
        for (y = 0; y < ret; y++)
        {
            putabs(curraddr + y, buf[y]);
        }
#endif
        curraddr += ret;
        if (ret < sizeof buf) break;
        ret = fread(buf, 1, sizeof buf, fp);
    }

    fclose(fp);

    absaddr += sizeof firstbit; /* skip header */

    corrloc = absaddr + firstbit.a_text + firstbit.a_data;
    for (z = 0; z < firstbit.a_trsize; z += 8)
    {
        offs = getlong(corrloc + z);
        type = getlong(corrloc + z + 4);
        if (((type >> 24) & 0xff) != 0x04) continue;
        zaploc = absaddr + offs;
        oldval = getlong(zaploc);
        newval = oldval + absaddr;
        putlong(zaploc, newval);
    }

    corrloc += firstbit.a_trsize;
    for (z = 0; z < firstbit.a_drsize; z += 8)
    {
        offs = getlong(corrloc + z);
        type = getlong(corrloc + z + 4);
        if (((type >> 24) & 0xff) != 0x04) continue;
        zaploc = absaddr + firstbit.a_text + offs;
        oldval = getlong(zaploc);
        newval = oldval + absaddr;
        putlong(zaploc, newval);
    }

    curraddr = absaddr + firstbit.a_text + firstbit.a_data;
    /* initialise BSS */
    for (z = 0; z < firstbit.a_bss; z++)
    {
        putabs(curraddr + z, '\0');
    }
    curraddr += firstbit.a_bss;

    sp = curraddr + 0x8000;
    /* absaddr -= 0x10000UL; */
    progret = runprot(0, absaddr + firstbit.a_entry, 0, sp, userparm);
    /* dumplong(progret); */
    return (progret);
#endif
#if 0
    /* This version accepts a relocatable executable,
       but doesn't relocate it */
    /* at time of writing, fread() is only known to work on
       512-byte buffers */
    ret = fread(buf, 1, sizeof buf, fp);
    memcpy(&firstbit, buf, sizeof firstbit);
    curraddr = absaddr;
    for (y = 0; y < ret; y++)
    {
        putabs(curraddr + y, buf[y]);
    }
    curraddr += ret;

    /* read code and data, maybe a little beyond */
    numreads = (int)((firstbit.a_text + firstbit.a_data) / sizeof buf);
    numreads++;
    for (x = 0; x < numreads; x++)
    {
        ret = fread(buf, 1, sizeof buf, fp);
        for (y = 0; y < ret; y++)
        {
            putabs(curraddr + y, buf[y]);
        }
        curraddr += ret;
    }

    fclose(fp);

    absaddr += 0x20;
    curraddr = absaddr + firstbit.a_text + firstbit.a_data;
    /* initialise BSS */
    for (z = 0; z < firstbit.a_bss; z++)
    {
        putabs(curraddr + z, '\0');
    }
    curraddr += firstbit.a_bss;

    sp = curraddr - absaddr + 0x8000;
    /* absaddr -= 0x10000UL; */
    return (runprot(absaddr, firstbit.a_entry, absaddr, sp, userparm));
#endif
#if 0
    /* this version expects a ZMAGIC file */
    fread(buf, 1, sizeof buf, fp);
    memcpy(&firstbit, buf, sizeof firstbit);

    /* skip rest of header info */
    numreads = N_TXTOFF(firstbit) / sizeof buf;
    numreads--;
    for (x = 0; x < numreads; x++)
    {
        fread(buf, 1, sizeof buf, fp);
    }
    
    /* read code */
    numreads = (int)(firstbit.a_text / sizeof buf);
    curraddr = absaddr;
    for (x = 0; x < numreads; x++)
    {
        ret = fread(buf, 1, sizeof buf, fp);
        for (y = 0; y < ret; y++)
        {
            putabs(curraddr + y, buf[y]);
        }
        curraddr += ret;
    }

    /* read data */    
    numreads = (int)(firstbit.a_data / sizeof buf);
    curraddr = absaddr + N_DATADDR(firstbit) - N_TXTADDR(firstbit);
    for (x = 0; x < numreads; x++)
    {
        ret = fread(buf, 1, sizeof buf, fp);
        for (y = 0; y < ret; y++)
        {
            putabs(curraddr + y, buf[y]);
        }
        curraddr += ret;
    }
    
    fclose(fp);
    
    /* initialise BSS */
    curraddr = absaddr + N_BSSADDR(firstbit) - N_TXTADDR(firstbit);
    for (y = 0; y < firstbit.a_bss; y++)
    {
        putabs(curraddr + y, '\0');
    }

    sp = N_BSSADDR(firstbit) + firstbit.a_bss + 0x8000;
    absaddr -= 0x10000UL;
    return (runprot(absaddr, 0x10000UL, absaddr, sp, userparm));
#endif
}

#ifdef __SUBC__
unsigned long realsub(unsigned long func, int one, unsigned long parm, int two)
#else
unsigned long realsub(unsigned long func, unsigned long parm)
#endif
{
#ifdef __SUBC__
    unsigned long (*functocall)(unsigned long parm, unsigned long two);
    unsigned long xxx;

    functocall = (unsigned long (*)())func;
    xxx = (unsigned long)functocall(parm, two);
    return (xxx);
#else
    unsigned long (*functocall)(unsigned long parm);

    functocall = (unsigned long (*)(unsigned long))func;
    return (functocall(parm));
#endif
}

static unsigned long dorealint(unsigned long parm)
{
    unsigned short *genshort;
    unsigned short savehigh[4];
    
    unused(parm);
    genshort = intbuffer;
    gethigh(savehigh);
    /* 9 * 16-bit registers/flags + 4 segment registers = 13 */
    sethigh(genshort + 1 + 13);
    int86x(*genshort, 
           (union REGS *)(genshort + 1),
           (union REGS *)(genshort + 1),
           (struct SREGS *)(genshort + 1 + 9));
    gethigh(genshort + 1 + 13);
    sethigh(savehigh);
    return (0);
}
