/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  int80.c - Interrupt 80 handler                                   */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "int80.h"
#include "support.h"
#include "pos.h"

static void int80handler(union REGS *regsin,
                        union REGS *regsout,
                        struct SREGS *sregs);

int int80(unsigned int *regs)
{
    static union REGS regsin;
    static union REGS regsout;
    static struct SREGS sregs;

    regsin.d.eax = regs[0];
    regsin.d.ebx = regs[1];
    regsin.d.ecx = regs[2];
    regsin.d.edx = regs[3];
    regsin.d.esi = regs[4];
    regsin.d.edi = regs[5];
    regsin.d.cflag = 0;
    memcpy(&regsout, &regsin, sizeof regsout);
    int80handler(&regsin, &regsout, &sregs);
    regs[0] = regsout.d.eax;
    regs[1] = regsout.d.ebx;
    regs[2] = regsout.d.ecx;
    regs[3] = regsout.d.edx;
    regs[4] = regsout.d.esi;
    regs[5] = regsout.d.edi;
    regs[6] = regsout.d.cflag;
    return (0);
}

static void int80handler(union REGS *regsin,
                        union REGS *regsout,
                        struct SREGS *sregs)
{
    void *p;
    unsigned int writtenbytes;

    switch (regsin->d.eax)
    {
        /* Terminate (with return code) */
        case 0x1:
            PosTerminate(regsin->d.ebx);
            break;

        /* Write */
        case 0x4:
            p = (void *)(regsin->d.ecx);
            regsout->d.eax = PosWriteFile(regsin->d.ebx,
                                          p,
                                          regsin->d.edx,
                                          &writtenbytes);
            if (regsout->d.eax) regsout->x.cflag = 1;
            else regsout->d.eax = writtenbytes;
            break;

        default:
            printf("got int 80H call %x\n", regsin->d.eax);
            break;
    }
    return;
}
