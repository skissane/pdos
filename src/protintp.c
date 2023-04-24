/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  protintp - part of protint                                       */
/*                                                                   */
/*  This is linked in to the protected mode executable. Interrupts   */
/*  are channeled into this code, and either get handled by PDOS     */
/*  or redirected to real mode                                       */
/*                                                                   */
/*********************************************************************/

#include "support.h"
#include "protint.h"
#include "unused.h"

unsigned long dorealint;
unsigned long (*runreal_p)(unsigned long func, unsigned short *regs);
rawprot_parms *rp_parms;
int *G_intloc;

void inthdlr(void);
void inthdlr_0(void);
void inthdlr_1(void);
void inthdlr_3(void);
void inthdlr_8(void);
void inthdlr_9(void);
void inthdlr_E(void);
void inthdlr_10(void);
void inthdlr_13(void);
void inthdlr_14(void);
void inthdlr_15(void);
void inthdlr_16(void);
void inthdlr_1A(void);
void inthdlr_20(void);
void inthdlr_21(void);
void inthdlr_25(void);
void inthdlr_26(void);
void inthdlr_80(void);
void inthdlr_A0(void);
void inthdlr_A3(void);
void inthdlr_A4(void);
void inthdlr_A5(void);
void inthdlr_A6(void);
void inthdlr_AA(void);
void inthdlr_B0(void);
void inthdlr_B1(void);
void inthdlr_BE(void);
void int_enable(void);

static unsigned short *intbuffer;
static int numUserInts;
static struct {
    int intno;
    int (*func)(unsigned int *);
} userInt[256];


/* registers come in as eax, ebx, ecx, edx, esi, edi,
   cflag, flags, ebp */
/* When translating to a real mode routine we need to set up
   ax, bx, cx, dx, si, di, cflag, flags, bp, es, blank, blank, ds */
/* also note that since it is inappropriate to be using segment
   registers in protected mode, we use the upper 16 bits of esi
   for ds, and the upper 16 bits of edi for es, when translating
   to real mode */
/* also note that we provide extra room for the high halves of eax,
   ebx, ecx and edx, to both set and obtain the high 16-bits. */
void gotint(int intno, unsigned int *save)
{
    unsigned short newregs[13+4]; /* 4 for upper 16-bits of eax to edx */
    unsigned short *ssave;
    int x;

    /* clear all flags by default */
    save[6] = 0; /* cflag */
    save[7] = 0; /* flags */
    for (x = 0; x < numUserInts; x++)
    {
        if (userInt[x].intno == intno)
        {
            /* Protected mode handler of IRQ 0 is called after the BIOS one
             * so the BIOS handler is not affected by task switch. */
            if (userInt[x].intno == 0xB0) break;
            /* An int 21h e.g. will be handled by PDOS and will
               return 0 indicating that no further action is
               required. If it returns non-zero, then it will fall
               through and call the real-mode interrupt. */
            if (userInt[x].func(save) == 0)
            {
                if (save[6])
                {
                    save[7] |= 1;
                }
                return;
            }
            break;
        }
    }

    /* If the interrupt number is 0xff, it signifies that this is
       just the default handler, and we are not required to take
       any action. */
    if (intno == 0xff)
    {
        return;
    }
    
    /* The default behaviour is to convert any protected mode
       interrupt into a real mode interrupt. */
    newregs[0] = (unsigned short)save[0]; /* ax */    
    newregs[1] = (unsigned short)save[1]; /* bx */
    newregs[2] = (unsigned short)save[2]; /* cx */
    newregs[3] = (unsigned short)save[3]; /* dx */
    newregs[4] = (unsigned short)save[4]; /* si */
    newregs[5] = (unsigned short)save[5]; /* di */
    newregs[6] = 0;                       /* cflag */
    newregs[7] = 0;                       /* flags */
    newregs[8] = (unsigned short)save[8]; /* bp */
    newregs[9] = (unsigned short)(save[5] >> 16); /* es = top edi */
    newregs[10] = 0;                       /*  */
    newregs[11] = 0;                       /*  */
    newregs[12] = (unsigned short)(save[4] >> 16); /* ds = top esi */
    newregs[13 + 0] = (unsigned short)(save[0] >> 16); /* top eax */
    newregs[13 + 1] = (unsigned short)(save[1] >> 16); /* top ebx */
    newregs[13 + 2] = (unsigned short)(save[2] >> 16); /* top ecx */
    newregs[13 + 3] = (unsigned short)(save[3] >> 16); /* top edx */
    *intbuffer = intno;
    memcpy(intbuffer + 1, newregs, sizeof newregs);
    runreal_p(dorealint, 0);
    memcpy(newregs, intbuffer + 1, sizeof newregs);
    ssave = (unsigned short *)save;
    ssave[0*2] = newregs[0]; /* ax */
    ssave[0*2 + 1] = newregs[12 + 0]; /* high eax */
    ssave[1*2] = newregs[1]; /* bx */
    ssave[1*2 + 1] = newregs[12 + 1]; /* high ebx */
    ssave[2*2] = newregs[2]; /* cx */
    ssave[2*2 + 1] = newregs[12 + 2]; /* high ecx */
    ssave[3*2] = newregs[3]; /* dx */
    ssave[3*2 + 1] = newregs[12 + 3]; /* high edx */
    ssave[4*2] = newregs[4]; /* si */
    ssave[4*2 + 1] = newregs[12]; /* ds */
    ssave[5*2] = newregs[5]; /* di */
    ssave[5*2 + 1] = newregs[9]; /* es */
    save[6] = newregs[6]; /* cflag */
    save[7] = newregs[7]; /* flags */
    ssave[8*2] = newregs[8]; /* bp */
    /* Protected mode handler of IRQ 0 is called after the BIOS one
     * so the BIOS handler is not affected by task switch. */
    if (userInt[x].intno == 0xB0) userInt[x].func(save);
    return;
}


/* we don't need to do anything much up here currently for a raw call */

unsigned long rawprot_p(rawprot_parms *parmlist)
{
    rp_parms = parmlist;
    return (parmlist->userparm);
}


/* we need to reestablish the interrupts here */

unsigned long runprot_p(rawprot_parms *parmlist)
{
    char *intloc;
    int x;
    unsigned long intdesc1;
    unsigned long intdesc2;
    unsigned long intaddr;
    runprot_parms *runparm;
    static struct {
        int number;
        void (*handler)(void);
    } handlerlist[] = { 
        { 0x0, inthdlr_0 },
        { 0x1, inthdlr_1 },
        { 0x3, inthdlr_3 },
        { 0x8, inthdlr_8 },
        { 0x9, inthdlr_9 },
        { 0xE, inthdlr_E },
        { 0x10, inthdlr_10 },
        { 0x13, inthdlr_13 },
        { 0x14, inthdlr_14 },
        { 0x15, inthdlr_15 },
        { 0x16, inthdlr_16 },
        { 0x1A, inthdlr_1A },
        { 0x20, inthdlr_20 },
        { 0x21, inthdlr_21 },
        { 0x25, inthdlr_25 },
        { 0x26, inthdlr_26 },
        { 0x80, inthdlr_80 },
        { 0xA0, inthdlr_A0 },
        { 0xA3, inthdlr_A3 },
        { 0xA4, inthdlr_A4 },
        { 0xA5, inthdlr_A5 },
        { 0xA6, inthdlr_A6 },
        { 0xAA, inthdlr_AA },
        { 0xB0, inthdlr_B0 },
        { 0xB1, inthdlr_B1 },
        { 0xFF, 0 } };

    intloc = (void *)(parmlist->intloc);
    G_intloc = (void *)intloc;
    runparm = (runprot_parms *)parmlist->userparm;
    intbuffer = (void *)(runparm->intbuffer);
    dorealint = runparm->dorealint;
    runreal_p = ((unsigned long (*)(unsigned long func, unsigned short *regs))
                 (runparm->runreal));

    /* Configures the Interrupt Descriptor Table (IDT). */
    intaddr = (unsigned long)(&inthdlr);
    intdesc1 = (0x8 << 16) | (intaddr & 0xffff);
    intdesc2 = (intaddr & 0xffff0000)
               | (1 << 15)
               | (0 << 13)
               | (0x0e << 8);
               
    /* install the generic interrupt handler */
    for (x = 0; x < 256; x++)
    {
        *((unsigned long *)intloc + x * 2) = intdesc1;
        *((unsigned long *)intloc + x * 2 + 1) = intdesc2;
    }

    /* Installs the handlers from handlerlist. */
    for (x = 0; handlerlist[x].number != 0xFF; x++)
    {    
        intaddr = (unsigned long)handlerlist[x].handler;
        intdesc1 = (0x8 << 16) | (intaddr & 0xffff);
        intdesc2 = (intaddr & 0xffff0000)
                   | (1 << 15)
                   | (0 << 13)
                   | (0x0e << 8);
        *((unsigned long *)intloc + handlerlist[x].number * 2) = intdesc1;
        *((unsigned long *)intloc + handlerlist[x].number * 2 + 1) = intdesc2;
    }
    /* End of IDT configuration. */
    rawprot_p(parmlist);
    numUserInts = 0;
    int_enable();
    return (runparm->userparm);
}


/* we don't need to do anything special for an a.out executable
   currently.  In the past we used to have to relocate the data
   section, but that's now done in the real mode code as part of
   the load procedure */

unsigned long runaout_p(rawprot_parms *parmlist)
{
    return (runprot_p(parmlist));
}


/* Allow PDOS to get access to an interrupt. */
void protintHandler(int intno, int (*func)(unsigned int *))
{
    int x;
    
    for (x = 0; x < numUserInts; x++)
    {
        if (userInt[x].intno == intno)
        {
            userInt[x].func = func;
            return;
        }
    }
    userInt[x].intno = intno;
    userInt[x].func = func;
    numUserInts++;
    return;
}
