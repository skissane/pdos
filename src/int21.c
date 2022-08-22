/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  int21.c - Interrupt 21 handler                                   */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "int21.h"
#include "support.h"
#include "pos.h"
#include "log.h"

#ifdef __32BIT__
/* SBRK support for EMX. */
static char *sbrk_start = NULL;
static char *sbrk_end;
#endif

static void int21handler(union REGS *regsin,
                         union REGS *regsout,
                         struct SREGS *sregs);

#ifdef __32BIT__
int int21(unsigned int *regs)
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
    int21handler(&regsin, &regsout, &sregs);
    regs[0] = regsout.d.eax;
    regs[1] = regsout.d.ebx;
    regs[2] = regsout.d.ecx;
    regs[3] = regsout.d.edx;
    regs[4] = regsout.d.esi;
    regs[5] = regsout.d.edi;
    regs[6] = regsout.d.cflag;
    return (0);
}
#else
void int21(unsigned int *regptrs)
{
    static union REGS regsin;
    static union REGS regsout;
    static struct SREGS sregs;

    sregs.es = regptrs[0];
    sregs.ds = regptrs[1];
    regsin.x.di = regptrs[2];
    regsin.x.si = regptrs[3];
    regsin.x.dx = regptrs[4];
    regsin.x.cx = regptrs[5];
    regsin.x.bx = regptrs[6];
    regsin.x.cflag = 0;
    regsin.x.ax = regptrs[8];
    memcpy(&regsout, &regsin, sizeof regsout);
    int21handler(&regsin, &regsout, &sregs);
    regptrs[0] = sregs.es;
    regptrs[1] = sregs.ds;
    regptrs[2] = regsout.x.di;
    regptrs[3] = regsout.x.si;
    regptrs[4] = regsout.x.dx;
    regptrs[5] = regsout.x.cx;
    regptrs[6] = regsout.x.bx;
    regptrs[7] = regsout.x.cflag;
    regptrs[8] = regsout.x.ax;
    return;
}
#endif

static void int21handler(union REGS *regsin,
                         union REGS *regsout,
                         struct SREGS *sregs)
{
    void *p;
    void *q;
    int handle;
    size_t readbytes;
    size_t writtenbytes;
    long newpos;
    int ret;
    int attr;
    void *tempdta;
    unsigned long pid;
    size_t sz;

    switch (regsin->h.ah)
    {
        /* INT 21,00 - Program terminate (no return code) */
        case 0x00:
            PosTerminate(0);
            break;

        /* INT 21,01 - Keyboard input (with echo) */
        /* INT 21,02 - Display output */
        case 0x02:
            regsout->h.al = PosDisplayOutput(regsin->h.dl);
            break;

        /* INT 21,03 - Wait for STDAUX input */
        /* INT 21,04 - Output character to STDAUX */
        /* INT 21,05 - Output character to STDPRN */
        /* INT 21,06 - Direct console IO */
        case 0x06:
            regsout->h.al = PosDirectConsoleOutput(regsin->h.dl);
            break;

        /* INT 21,07 - Direct console input without echo */
        case 0x07:
            regsout->h.al = PosDirectCharInputNoEcho();
            break;

        /* INT 21,08 - Console input without echo */
        case 0x08:
            regsout->h.al = PosGetCharInputNoEcho();
            break;

        /* INT 21,09 - Print $-terminated string */
        case 0x09:
            regsout->h.al = PosDisplayString(MK_FP(sregs->ds, regsin->x.dx));
            break;

        /* INT 21,0A - Buffered Input */
        case 0x0a:
#ifdef __32BIT__
            PosReadBufferedInput((void *)(regsin->d.edx));
#else
            PosReadBufferedInput(MK_FP(sregs->ds, regsin->x.dx));
#endif
            break;

        /* INT 21,0B - Check STDIN status */
        /* INT 21,0C - Clear Keyboard Buffer then Invoke Function */
        /* INT 21,0D - Disk Reset */
        /* INT 21,0E - Select Disk */
        case 0x0e:
            regsout->h.al = PosSelectDisk(regsin->h.dl);
            break;

        /* INT 21,0F - FCB Open File */
        /* INT 21,10 - FCB Close File */
        /* INT 21,11 - FCB Find First */
        /* INT 21,12 - FCB Find Next */
        /* INT 21,13 - FCB Delete File */
        /* INT 21,14 - FCB Sequential Read */
        /* INT 21,15 - FCB Sequential Write */
        /* INT 21,16 - FCB Create File */
        /* INT 21,17 - FCB Rename File */
        /* INT 21,18 - DOS NULL FUNCTION
         * CP/M BDOS call DRV_LOGINVEC - Return bitmap of logged-in drives */

        /* INT 21,19 - Get current default drive */
        case 0x19:
            regsout->h.al = PosGetDefaultDrive();
            break;

        /* INT 21,1A - Set Disk Transfer Address (DTA) */
        case 0x1a:
#ifdef __32BIT__
            tempdta = (void *)(regsin->d.edx);
#else
            tempdta = MK_FP(sregs->ds, regsin->x.dx);
#endif
            PosSetDTA(tempdta);
            break;

        /* INT 21,1A - Set disk transfer address */
        /* INT 21,1B - Get allocation table info */
        /* INT 21,1C - Get allocation table info  */
        /* INT 21,1D - DOS NULL FUNCTION
         * CP/M BDOS call DRV_ROVEC - Return bitmap of read-only drives */
        /* INT 21,1E - DOS NULL FUNCTION
         * CP/M BDOS call F_ATTRIB - Set CP/M file attributes */
        /* INT 21,1F - Get Drive Parameter Table (DPT) (DOS 1.0+) */
        /* INT 21,20 - DOS NULL FUNCTION
         * CP/M BDOS call F_USERNUM - Get/Set CP/M user number */
        /* INT 21,21 - FCB Random Read */
        /* INT 21,22 - FCB Random Write */
        /* INT 21,23 - FCB Get File Size */
        /* INT 21,24 - FCB Set Relative Record Field */
        /* INT 21,25 - Set Interrupt Vector */
        case 0x25:
#ifdef __32BIT__
            p = (void *)(regsin->d.edx);
#else
            p = MK_FP(sregs->ds, regsin->x.dx);
#endif
            PosSetInterruptVector(regsin->h.al, p);
            break;

        /* INT 21,26 - Create New PSP */
        /* INT 21,27 - FCB Random Block Read */
        /* INT 21,28 - FCB Random Block Write */
        /* INT 21,29 - FCB Parse Filename */
        /* INT 21,2A - Get System Date */
        case 0x2a:
            {
                int year, month, day, dw;

                PosGetSystemDate(&year, &month, &day, &dw);
                regsout->x.cx = year;
                regsout->h.dh = month;
                regsout->h.dl = day;
                regsout->h.al = dw;
            }
            break;

        /* INT 21,2B - Set System Date */
        case 0x2b:
            regsout->h.al = PosSetSystemDate(regsin->x.cx,
                                             regsin->h.dh,
                                             regsin->h.dl);
            break;

        /* INT 21,2C - Get System Time */
        case 0x2c:
            {
                unsigned int hour, minute, second, hundredths;

                PosGetSystemTime(&hour, &minute, &second, &hundredths);
                regsout->h.ch = hour;
                regsout->h.cl = minute;
                regsout->h.dh = second;
                regsout->h.dl = hundredths;
            }
            break;

        /* INT 21,2D - Set System Time */

        /* INT 21,2E - Set Verify Flag */
        case 0x2e:
            PosSetVerifyFlag(regsin->h.al);
            break;

        /* INT 21,2F - Get Disk Transfer Address (DTA) */
        case 0x2f:
            tempdta = PosGetDTA();
#ifdef __32BIT__
            regsout->d.ebx = (int)tempdta;
#else
            regsout->x.bx = FP_OFF(tempdta);
            sregs->es = FP_SEG(tempdta);
#endif
            break;

        /* INT 21,30 - Get DOS Version */
        case 0x30:
            regsout->x.ax = PosGetDosVersion();
            regsout->x.bx = 0;
            regsout->x.cx = 0;
            break;

        /* INT 21,31 - Terminate and Stay Resident */
        case 0x31:
            PosTerminateAndStayResident(regsin->h.al, regsin->x.dx);
            break;

        /* INT 21,32 - Get Drive Parameter Table (DPT) (DOS 2.0+) */

        /* INT 21,33 - primary function is managing breakFlag.
         * Also some other subfunctions around DOS version and boot drive.
         */
        case 0x33:
            /* AL=0: Get break flag in DL register */
            if (regsin->h.al == 0)
            {
                regsout->h.dl = PosGetBreakFlag();
            }

            /* AL=1: Set break flag in DL register */
            else if (regsin->h.al == 1)
            {
                PosSetBreakFlag(regsin->h.dl);
            }

            /* AL=2: Set break flag, return old value.
             * This just combines AL=0 and AL=1 in one operation.
             */
            else if (regsin->h.al == 2)
            {
                regsout->h.dl = PosGetBreakFlag();
                PosSetBreakFlag(regsin->h.dl);
            }

            /* AL=3/AL=4: These functions managed CPSW
             * (Code Page SWitching) state. In released versions of MS-DOS,
             * these have always been NO-OPS. The CPSW functions were
             * implemented during the development of DOS 4.0, but removed
             * before it was releaseed.
             */

            /* AL=5: Get DOS boot drive (PosGetBootDrive) */
            else if (regsin->h.al == 5)
            {
                regsout->h.dl = PosGetBootDrive();
            }

            /* AL=6: Get DOS version number.
             * Unlike INT 21,30, which can be faked with SETVER (in MS-DOS 5+),
             * 3306 is supposed to return the real DOS version. However, in
             * PDOS we will just return the fake version here too.
             */
            else if (regsin->h.al == 6)
            {
                /* BL = Major Version, BH = Minor version */
                regsout->x.bx = PosGetDosVersion();

                /* DL = Revision, DH = Version Flags */
                regsout->x.dx = 0;
            }

            else
            {
                logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
            }
            break;

        /* INT 21,34 - Get INDOS Flag Address */
        /* INT 21,35 - Get Interrupt Vector */
        case 0x35:
            p = PosGetInterruptVector(regsin->h.al);
#ifdef __32BIT__
            regsout->d.ebx = (int)p;
#else
            regsout->x.bx = FP_OFF(p);
            sregs->es = FP_SEG(p);
#endif
            break;

        /* INT 21,36 - Get Free Space */
        case 0x36:
#ifdef __32BIT__
            PosGetFreeSpace(regsin->h.dl,
                            &regsout->d.eax,
                            &regsout->d.ebx,
                            &regsout->d.ecx,
                            &regsout->d.edx);
#else
            PosGetFreeSpace(regsin->h.dl,
                            &regsout->x.ax,
                            &regsout->x.bx,
                            &regsout->x.cx,
                            &regsout->x.dx);
#endif
            break;

        /* INT 21,37 - Get/Set Switch Character */
        /* INT 21,38 - Get/Set Country Information */
        /* INT 21,39 - Make Directory */
        case 0x39:
#ifdef __32BIT__
            regsout->d.eax = PosMakeDir((void *)(regsin->d.edx));
            if (regsout->d.eax) regsout->x.cflag = 1;
#else
            regsout->x.ax = PosMakeDir(MK_FP(sregs->ds, regsin->x.dx));
            if (regsout->x.ax) regsout->x.cflag = 1;
#endif
            break;

        /* INT 21,3A - Remove Directory */
        case 0x3a:
#ifdef __32BIT__
            regsout->d.eax = PosRemoveDir((void *)(regsin->d.edx));
            if (regsout->d.eax) regsout->x.cflag = 1;
#else
            regsout->x.ax = PosRemoveDir(MK_FP(sregs->ds, regsin->x.dx));
            if (regsout->x.ax) regsout->x.cflag = 1;
#endif
            break;

        /* INT 21,3B - Change Directory */
        case 0x3b:
#ifdef __32BIT__
            regsout->d.eax = PosChangeDir((void *)(regsin->d.edx));
            if (regsout->d.eax) regsout->x.cflag = 1;
#else
            regsout->x.ax = PosChangeDir(MK_FP(sregs->ds, regsin->x.dx));
            if (regsout->x.ax) regsout->x.cflag = 1;
#endif
            break;

        /* INT 21,3C - Create File */
        case 0x3c:
#ifdef __32BIT__
            p = (void *)(regsin->d.edx);
            regsout->d.eax = PosCreatFile(p, regsin->x.cx, &handle);
            if (regsout->d.eax) regsout->x.cflag = 1;
            else regsout->d.eax = handle;
#else
            p = MK_FP(sregs->ds, regsin->x.dx);
            regsout->x.ax = PosCreatFile(p, regsin->x.cx, &handle);
            if (regsout->x.ax) regsout->x.cflag = 1;
            else regsout->x.ax = handle;
#endif
            break;

        /* INT 21,3D - Open File */
        case 0x3d:
#ifdef __32BIT__
            p = (void *)(regsin->d.edx);
            regsout->d.eax = PosOpenFile(p, regsin->h.al, &handle);
            if (regsout->d.eax) regsout->x.cflag = 1;
            else regsout->d.eax = handle;
#else
            p = MK_FP(sregs->ds, regsin->x.dx);
            regsout->x.ax = PosOpenFile(p, regsin->h.al, &handle);
            if (regsout->x.ax) regsout->x.cflag = 1;
            else regsout->x.ax = handle;
#endif
            break;

        /* INT 21,3E - Close File */
        case 0x3e:
#ifdef __32BIT__
            regsout->d.eax = PosCloseFile(regsin->d.ebx);
#else
            regsout->x.ax = PosCloseFile(regsin->x.bx);
#endif
            break;

        /* INT 21,3F - Read File */
        case 0x3f:
#ifdef __32BIT__
            p = (void *)(regsin->d.edx);
            regsout->d.eax = PosReadFile(regsin->d.ebx,
                                         p,
                                         regsin->d.ecx,
                                         &readbytes);
            if (regsout->d.eax) regsout->x.cflag = 1;
            else regsout->d.eax = readbytes;
#else
            p = MK_FP(sregs->ds, regsin->x.dx);
            regsout->x.ax = PosReadFile(regsin->x.bx,
                                        p,
                                        regsin->x.cx,
                                        &readbytes);
            if (regsout->x.ax) regsout->x.cflag = 1;
            else regsout->x.ax = readbytes;
#endif
            break;

        /* INT 21,40 - Write File */
        case 0x40:
#ifdef __32BIT__
            p = (void *)(regsin->d.edx);
            regsout->d.eax = PosWriteFile(regsin->d.ebx,
                                          p,
                                          regsin->d.ecx,
                                          &writtenbytes);
            if (regsout->d.eax) regsout->x.cflag = 1;
            else regsout->d.eax = writtenbytes;
#else
            p = MK_FP(sregs->ds, regsin->x.dx);
            regsout->x.ax = PosWriteFile(regsin->x.bx,
                                         p,
                                         regsin->x.cx,
                                         &writtenbytes);
            if (regsout->x.ax) regsout->x.cflag = 1;
            else regsout->x.ax = writtenbytes;
#endif
            break;

        /* INT 21,41 - Delete File */
        case 0x41:
#ifdef __32BIT__
            p = (void *)(regsin->d.edx);
#else
            p = MK_FP(sregs->ds, regsin->x.dx);
#endif
            regsout->x.ax = PosDeleteFile(p);
            if (regsout->x.ax != 0)
            {
                regsout->x.cflag = 1;
            }
            break;

        /* INT 21,42 - Move File Pointer */
        case 0x42:
#ifdef __32BIT__
            regsout->d.eax = PosMoveFilePointer(regsin->d.ebx,
                                                regsin->d.ecx,
                                                regsin->h.al,
                                                &newpos);
            if (regsout->d.eax)
            {
                regsout->x.cflag = 1;
            }
            else regsout->d.eax = newpos;
#else
            newpos = ((unsigned long)regsin->x.cx << 16) | regsin->x.dx;
            regsout->x.ax = PosMoveFilePointer(regsin->x.bx,
                                               newpos,
                                               regsin->h.al,
                                               &newpos);
            if (regsout->x.ax)
            {
                regsout->x.cflag = 1;
            }
            else
            {
                regsout->x.dx = newpos >> 16;
                regsout->x.ax = newpos & 0xffff;
            }
#endif
            break;

        /* INT 21,43 - Get File Attributes */
        case 0x43:
            if (regsin->h.al == 0x00)
            {
#ifdef __32BIT__
                p = (void *)(regsin->d.edx);
#else
                p = MK_FP(sregs->ds, regsin->x.dx);
#endif

#ifdef __32BIT__
                regsout->d.eax = PosGetFileAttributes(p,&attr);
                regsout->d.ecx=attr;
#else
                regsout->x.ax = PosGetFileAttributes(p,&attr);
                regsout->x.cx=attr;
#endif

#ifdef __32BIT__
                if (regsout->d.eax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag=0;
                }
#else
                if (regsout->x.ax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag=0;
                }
#endif
            }

            else if (regsin->h.al == 0x01)
            {
#ifdef __32BIT__
                p = (void *)(regsin->d.edx);
#else
                p = MK_FP(sregs->ds, regsin->x.dx);
#endif

#ifdef __32BIT__
                regsout->d.eax = PosSetFileAttributes(p,regsin->d.ecx);
#else
                regsout->x.ax = PosSetFileAttributes(p,regsin->x.cx);
#endif

#ifdef __32BIT__
                if (regsout->d.eax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag=0;
                }

#else
                if (regsout->x.ax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag=0;
                }
#endif

            }
            else
            {
                logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
            }
            break;

        /* INT 21,44 - IOCTL functions */
        case 0x44:
            if (regsin->h.al == 0x00)
            {
#ifdef __32BIT__
                regsout->x.ax = PosGetDeviceInformation(regsin->x.bx,
                                                        &regsout->d.edx);
#else
                regsout->x.ax = PosGetDeviceInformation(regsin->x.bx,
                                                        &regsout->x.dx);
#endif
                if (regsout->x.ax != 0)
                {
                    regsout->x.cflag = 1;
                }
            }
            else if (regsin->h.al == 0x01)
            {
#ifdef __32BIT__
                regsout->x.ax = PosSetDeviceInformation(regsin->x.bx,
                                                        regsin->d.edx);
#else
                regsout->x.ax = PosSetDeviceInformation(regsin->x.bx,
                                                        regsin->x.dx);
#endif
                if (regsout->x.ax != 0)
                {
                    regsout->x.cflag = 1;
                }
            }
            else if (regsin->h.al == 0x08)
            {
#ifdef __32BIT__
                regsout->d.eax = PosBlockDeviceRemovable(regsin->h.bl);
                if ((int)regsout->d.eax < 0)
                {
                    regsout->x.cflag = 1;
                    regsout->d.eax = -regsout->d.eax;
                }
#else
                regsout->x.ax = PosBlockDeviceRemovable(regsin->h.bl);
                if ((int)regsout->x.ax < 0)
                {
                    regsout->x.cflag = 1;
                    regsout->x.ax = -regsout->x.ax;
                }
#endif
            }


            /* Implementing the Int 21h/AX=4409 call*/
            else if (regsin->h.al == 0x09)
            {
#ifdef __32BIT__
                regsout->d.eax = PosBlockDeviceRemote(regsin->h.bl,
                                                      ((int *)
                                                       &(regsout->d.edx)));
                if ((int)(regsout->d.eax) < 0)
                {
                    regsout->x.cflag = 1;
                    regsout->d.eax = -(regsout->d.eax);
                }
#else
                regsout->x.ax = PosBlockDeviceRemote(regsin->h.bl,
                                                     (int *)&(regsout->x.dx));
                if ((int)(regsout->x.ax) < 0)
                {
                    regsout->x.cflag = 1;
                    regsout->x.ax = -(regsout->x.ax);
                }
#endif
            }

/**/
/*Function to dump the contents of DS,DX register*/
            else if (regsin->h.al == 0x0D)
            {
#ifdef __32BIT__
                p = (void *)(regsin->d.edx);
                regsout->d.eax = PosGenericBlockDeviceRequest(regsin->h.bl,
                                                              regsin->h.ch,
                                                              regsin->h.cl,p);
#else
                p = MK_FP(sregs->ds, regsin->x.dx);
                regsout->x.ax =PosGenericBlockDeviceRequest(regsin->h.bl,
                                                            regsin->h.ch,
                                                            regsin->h.cl,p);
#endif
            }
            else
            {
                logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
            }
            break;

        /* INT 21,45 - Duplicate File Handle */
        case 0x45:
#ifdef __32BIT__
            ret = PosDuplicateFileHandle(regsin->d.ebx);
            if (ret < 0)
            {
                regsout->x.cflag = 1;
                ret = -ret;
            }
            regsout->d.eax = ret;
#else
            ret = PosDuplicateFileHandle(regsin->x.bx);
            if (ret < 0)
            {
                regsout->x.cflag = 1;
                ret = -ret;
            }
            regsout->x.ax = ret;
#endif
            break;

        /* INT 21,46 - Force Duplicate File Handle */
        case 0x46:
#ifdef __32BIT__
            regsout->d.eax = PosForceDuplicateFileHandle(regsin->d.ebx,
                                                         regsin->d.ecx);
            if (regsout->d.eax) regsout->x.cflag = 1;
#else
            regsout->x.ax = PosForceDuplicateFileHandle(regsin->x.bx,
                                                        regsin->x.cx);
            if (regsout->x.ax) regsout->x.cflag = 1;
#endif
            break;

        /* INT 21,47 - Get Current Directory */
        case 0x47:
#ifdef __32BIT__
            p = (void *)(regsin->d.esi);
            regsout->d.eax = PosGetCurDir(regsin->h.dl, p);
#else
            p = MK_FP(sregs->ds, regsin->x.si);
            regsout->x.ax = PosGetCurDir(regsin->h.dl, p);
#endif
            break;

        /* INT 21,48 - Allocate Memory */
        case 0x48:
#ifndef __32BIT__
#if 0
            /* Not sure if this is still relevant, and it is
               creating excess memory allocation, so it is
               being removed. */

            /* This function is only useful in 16-bit PDOS */
            /* for some bizarre reason, MSC allocates 32 bytes,
            then attempts to turn it into 16k - do this in
            advance! */
            if (regsin->x.bx == 2)
            {
                regsin->x.bx = 0x400;
            }
#endif
            p = PosAllocMemPages(regsin->x.bx, &regsout->x.bx);
            if (p == NULL)
            {
                regsout->x.cflag = 1;
                regsout->x.ax = 0x08;
            }
            else
            {
                regsout->x.ax = FP_SEG(p) + (FP_OFF(p) >> 4);
            }
#endif
            break;

        /* INT 21,49 - Free Memory */
        case 0x49:
#ifdef __32BIT__
            regsout->d.eax = PosFreeMem((void *)(regsin->d.ebx));
#else
            regsout->x.ax = PosFreeMem(MK_FP(sregs->es, 0));
#endif
            if (regsout->x.ax != 0)
            {
                regsout->x.cflag = 1;
            }
            break;

        /* INT 21,4A - Resize Memory Block */
        case 0x4a:
#ifdef __32BIT__
            regsout->d.eax = PosReallocPages((void *)(regsin->d.ecx),
                                             regsin->d.ebx,
                                             &regsout->d.ebx);
#else
            regsout->x.ax = PosReallocPages(MK_FP(sregs->es, 0),
                                            regsin->x.bx,
                                            &regsout->x.bx);
            /* regsout->x.ax = 0; */
#endif
            if (0) /* regsout->x.ax != 0) */ /* always return success */
            {
                regsout->x.cflag = 1;
            }
            else
            {
                regsout->x.cflag = 0;
            }
            break;

        /* INT 21,4B - Load and Execute Program */
        case 0x4b:
            /* AL=00: Load and Execute */
            if (regsin->h.al == 0)
            {
#ifdef __32BIT__
                regsout->d.eax = PosExec((void *)(regsin->d.edx),
                                         (void *)(regsin->d.ebx));
#else
                regsout->x.ax = PosExec(MK_FP(sregs->ds, regsin->x.dx),
                                        MK_FP(sregs->es, regsin->x.bx));
#endif
                if (regsout->x.ax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag = 0;
                }
            }
            /* AL=01: Load but don't execute (for use by debuggers) */
            /* AL=03: Load overlay */
            /* AL=04: Execute in background.
             *        ("European" multitasking MS-DOS 4.0 only) */
            else
            {
                logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
            }
            break;

        /* INT 21,4C - Terminate (with return code) */
        case 0x4c:
#if (!defined(USING_EXE))
            PosTerminate(regsin->h.al);
#endif
            break;

        /* INT 21,4D - Get Subprocess Return Code */
        case 0x4d:
            regsout->x.ax = PosGetReturnCode();
            break;

        /* INT 21,4E - Find First File */
        case 0x4e:
#ifdef __32BIT__
            regsout->d.eax = PosFindFirst((void *)(regsin->d.edx),
                                          regsin->x.cx);
            if (regsout->d.eax != 0)
            {
                regsout->x.cflag = 1;
            }
#else
            regsout->x.ax = PosFindFirst(MK_FP(sregs->ds, regsin->x.dx),
                                        regsin->x.cx);
            if (regsout->x.ax != 0)
            {
                regsout->x.cflag = 1;
            }
#endif
            break;

        /* INT 21,4F - Find Next File */
        case 0x4f:
            regsout->x.ax = PosFindNext();
            if (regsout->x.ax != 0)
            {
                regsout->x.cflag = 1;
            }
            break;

        /* INT 21,50 - Set Current Process ID */
        /* INT 21,51 - Get Current Process ID */
        case 0x51:
#ifdef __32BIT__
            regsout->d.ebx = PosGetCurrentProcessId();
#else
            regsout->x.bx = PosGetCurrentProcessId();
#endif
            break;

        /* INT 21,52 - Get INVARS Pointer */
        /* INT 21,53 - Generate Drive Parameter Table */

        /* INT 21,54 - Get Verify Flag */
        case 0x54:
            regsout->h.al = PosGetVerifyFlag();
            break;

        /* INT 21,55 - Create New PSP */
        /* INT 21,56 - Rename File */
        case 0x56:
#ifdef __32BIT__
            p = (void *)(regsin->d.edx);
            q = (void *)(regsin->d.edi);
#else
            p = MK_FP(sregs->ds, regsin->x.dx);
            q = MK_FP(sregs->es, regsin->x.di);
#endif
            regsout->x.ax = PosRenameFile(p, q);
            if (regsout->x.ax != 0)
            {
                regsout->x.cflag = 1;
            }
            break;

        /* INT 21,57 - Get/Set File Date/Time */
        case 0x57:
            if (regsin->h.al == 0x00)
            {
#ifdef __32BIT__
                regsout->d.eax=PosGetFileLastWrittenDateAndTime(regsin->d.ebx,
                                                              &regsout->d.edx,
                                                              &regsout->d.ecx);
#else
                regsout->x.ax=PosGetFileLastWrittenDateAndTime(regsin->x.bx,
                                                              &regsout->x.dx,
                                                              &regsout->x.cx);
#endif

#ifdef __32BIT__

                if (regsout->d.eax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag=0;
                }
#else
                if (regsout->x.ax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag=0;
                }
#endif
            }
            else if(regsin->h.al==0x01)
            {
#ifdef __32BIT__
                regsout->d.eax=PosSetFileLastWrittenDateAndTime(regsin->d.ebx,
                                                               regsout->d.edx,
                                                               regsout->d.ecx);
#else
                regsout->x.ax=PosSetFileLastWrittenDateAndTime(regsin->x.bx,
                                                              regsout->x.dx,
                                                              regsout->x.cx);
#endif

#ifdef __32BIT__

                if (regsout->d.eax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag=0;
                }
#else
                if (regsout->x.ax != 0)
                {
                    regsout->x.cflag = 1;
                }
                else
                {
                    regsout->x.cflag=0;
                }
#endif
            }
            else
            {
                logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
            }

            break;

        /* INT 21,58 - Get/Set Memory Allocation Strategy */
        /* INT 21,59 - Get Extended Error Information */
        /* INT 21,5A - Create Temporary File */
        /* INT 21,5B - Create New File */
        case 0x5b:
#ifdef __32BIT__
            p = (void *)(regsin->d.edx);
            regsout->d.eax = PosCreatNewFile(p, regsin->x.cx, &handle);
            if (regsout->d.eax) regsout->x.cflag = 1;
            else regsout->d.eax = handle;
#else
            p = MK_FP(sregs->ds, regsin->x.dx);
            regsout->x.ax = PosCreatNewFile(p, regsin->x.cx, &handle);
            if (regsout->x.ax) regsout->x.cflag = 1;
            else regsout->x.ax = handle;
#endif
            break;

        /* INT 21,5C - Lock/Unlock File */
        /* INT 21,5D - Miscellaneous functions (networking, SHARE.EXE) */
        /* INT 21,5E - Miscellaneous functions (networking, printing)  */
        /* INT 21,5F - Device redirection */
        /* INT 21,60 - Get True File Name */
        case 0x60:
#ifdef __32BIT__
            p = (void *)(regsin->d.esi);
            q = (void *)(regsin->d.edi);
#else
            p = MK_FP(sregs->ds, regsin->x.si);
            q = MK_FP(sregs->es, regsin->x.di);
#endif
            ret = PosTruename(p,q);

            if(ret < 0)
            {
                regsout->x.cflag=1;
#ifdef __32BIT__
                regsout->d.eax=-ret;
#else
                regsout->x.ax=-ret;
#endif
            }
            break;
        /* INT 21,61 - Unused (reserved for networking use) */
        /* INT 21,62 - Get PSP address */
        case 0x62:
#ifdef __32BIT__
            regsout->d.ebx = PosGetCurrentProcessId();
#else
            regsout->x.bx = PosGetCurrentProcessId();
#endif
            break;
        /* INT 21,63 - Double Byte Character Functions (East Asian) */
        /* INT 21,64 - Set device driver lookahead (undocumented) */
        /* INT 21,65 - Get Extended Country Information */
        /* INT 21,66 - Get/Set Code Page Table */
        /* INT 21,67 - Set Handle Count (resize Job File Table (JFT)) */
        /* INT 21,68 - Flush Handle */
        /* INT 21,69 - Get/Set Disk Serial Number */
        /* INT 21,6A - same as function 68 */
        /* INT 21,6B - legacy IFS function (DOS 4.x only) */
        /* INT 21,6C - Extended Open/Create */
        /* INT 21,6D - various (ROM DOS or OS/2 only) */
        /* INT 21,6E - various (ROM DOS or OS/2 only) */
        /* INT 21,6F - various (ROM DOS or OS/2 only) */
        /* INT 21,70 - Windows 95 internationalization */
        /* INT 21,71 - Windows 95 Long File Name API */
        /* INT 21,73 - Windows 95 Drive Lock/Flush, FAT32 functions */
        /*Function call AX=7303h*/
        case 0x73:
            if(regsin->h.al==0x03)
            {
                regsout->x.cflag=0;
#ifdef __32BIT__
                regsout->d.eax=0;
#else
                regsout->h.al=0;
#endif
            }
            else
            {
                logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
            }
            break;
        /**/
        /* emx calls are 0x7f */
#ifdef __32BIT__
        case 0x7f:
            /* sbrk */
            if (regsin->h.al == 0)
            {
                if (sbrk_start == NULL)
                {
                    sbrk_start = PosAllocMem(0x100000, POS_LOC32);
                    sbrk_end = sbrk_start + 0x10000;
                }
                regsout->d.eax = (unsigned int)sbrk_end;
                sbrk_end += regsin->d.edx;
            }
            /* version - return infinite! */
            else if (regsin->h.al == 0x0a)
            {
                regsout->d.eax = 0xffffffffU;
            }
            else
            {
                logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
            }
            break;
#endif
        /* INT 21,80 - Execute Program in Background */
#ifdef __32BIT__
        case 0x80:
            PosAExec((void *)(regsin->d.edx), (void *)(regsin->d.ebx));
            break;
#endif

        /* INT 21,F6 - PDOS extensions */
        case 0xf6:
            if (regsin->h.al == 0)
            {
#ifdef __32BIT__
                PosDisplayInteger(regsin->d.ecx);
#else
                PosDisplayInteger(regsin->x.cx);
#endif
            }
            else if (regsin->h.al == 1)
            {
                PosReboot();
            }
            else if (regsin->h.al == 3)
            {
                PosSetDosVersion(regsin->x.bx);
            }
            else if (regsin->h.al == 4)
            {
                regsout->x.ax = PosGetLogUnimplemented();
            }
            else if (regsin->h.al == 5)
            {
                PosSetLogUnimplemented(!!(regsin->x.bx));
            }
            else if (regsin->h.al == 6)
            {
                regsout->x.ax = PosGetMagic();
            }
            else if (regsin->h.al == 7)
            {
#ifdef __32BIT__
                p = (void *)(regsin->d.edx);
#else
                p = MK_FP(sregs->ds, regsin->x.dx);
#endif
                PosGetMemoryManagementStats(p);
            }
#ifdef __32BIT__
            /* this function is currently only implemented
               for 32-bit, but it could potentially be
               implemented in 16-bit too. */
            else if (regsin->h.al == 8)
            {
                regsout->d.eax = (int)PosAllocMem(regsin->d.ebx,
                                                  regsin->d.ecx);
            }
#endif
            else if (regsin->h.al == 9)
            {
                p = PosGetErrorMessageString(regsin->x.bx);
#ifdef __32BIT__
                regsout->d.edx = (int)p;
#else
                regsout->x.dx = FP_OFF(p);
                sregs->es = FP_SEG(p);
#endif
            }
            else if (regsin->h.al == 0xA)
            {
                PosPowerOff();
            }
            else if (regsin->h.al == 0xC)
            {
#ifdef __32BIT__
                p = (void *)(regsin->d.edx);
                sz = regsin->d.ecx;
                pid = regsin->d.ebx;
#else
                p = MK_FP(sregs->ds, regsin->x.dx);
                sz = regsin->x.cx;
                pid = regsin->x.bx;
#endif
                regsout->x.ax = PosProcessGetInfo(pid, p, sz);
            }
            else if (regsin->h.al == 0xD)
            {
#ifdef __32BIT__
                p = (void *)(regsin->d.edx);
                pid = regsin->d.ecx;
#else
                p = MK_FP(sregs->ds, regsin->x.dx);
                pid = regsin->x.cx;
#endif
                PosProcessGetMemoryStats(pid, p);
            }
            else if (regsin->h.al == 0x30)
            {
                PosClearScreen();
            }
            else if (regsin->h.al == 0x31)
            {
                PosMoveCursor(regsin->h.bh, regsin->h.bl);
            }
            else if (regsin->h.al == 0x32)
            {
#ifdef __32BIT__
                p = (void *) regsin->d.edx;
                regsin->d.eax = PosGetVideoInfo(p, regsin->d.ecx);
#else
                p = MK_FP(sregs->ds, regsin->x.dx);
                regsin->x.ax = PosGetVideoInfo(p, regsin->x.cx);
#endif
            }
            else if (regsin->h.al == 0x33)
            {
                regsout->x.ax = PosKeyboardHit();
            }
            else if (regsin->h.al == 0x34)
            {
                PosYield();
            }
            else if (regsin->h.al == 0x35)
            {
#ifdef __32BIT__
                unsigned long seconds = regsin->d.edx;
#else
                unsigned long seconds = (regsin->x.dx) << 16UL;
                seconds |= regsin->x.bx;
#endif
                PosSleep(seconds);
            }
            else if (regsin->h.al == 0x36)
            {
                unsigned long ticks = PosGetClockTickCount();
#ifdef __32BIT__
                regsout->d.edx = ticks;
#else
                regsout->x.dx = (ticks >> 16UL);
                regsout->x.bx = ticks;
#endif
            }
            else if (regsin->h.al == 0x37)
            {
                PosSetVideoAttribute(regsin->x.bx);
            }
            else if (regsin->h.al == 0x38)
            {
                regsout->x.ax = PosSetVideoMode(regsin->x.bx);
            }
            else if (regsin->h.al == 0x39)
            {
                regsout->x.ax = PosSetVideoPage(regsin->x.bx);
            }
            else if (regsin->h.al == 0x3A)
            {
#ifdef __32BIT__
                regsout->d.eax = PosSetEnv(
                        (char*)regsin->d.ebx,
                        (char*)regsin->d.edx
                    );
#else
                regsout->x.ax = PosSetEnv(
                        (char*)MK_FP(sregs->ds,regsin->x.bx),
                        (char*)MK_FP(sregs->es,regsin->x.dx)
                    );
#endif
            }
            else if (regsin->h.al == 0x3B)
            {
                p = PosGetEnvBlock();
#ifdef __32BIT__
                regsout->d.eax = 0;
                regsout->d.ebx = (int)p;
#else
                regsout->x.ax = 0;
                regsout->x.bx = FP_OFF(p);
                sregs->ds = FP_SEG(p);
#endif
            }
            else if (regsin->h.al == 0x3C)
            {
#ifdef __32BIT__
                regsout->d.eax = PosSetNamedFont(
                        (char*)regsin->d.ebx
                    );
#else
                regsout->x.ax = PosSetNamedFont(
                        (char*)MK_FP(sregs->ds,regsin->x.bx)
                    );
#endif
            }
#ifdef __32BIT__
            else if (regsin->h.al == 0x3D)
            {
                regsout->d.eax = (unsigned long)PosVirtualAlloc(
                    (void *)(regsin->d.ebx), regsin->d.ecx
                    );
            }
            else if (regsin->h.al == 0x3E)
            {
                PosVirtualFree((void *)(regsin->d.ebx), regsin->d.ecx);
            }
            else if (regsin->h.al == 0x3F)
            {
                regsout->d.eax = (unsigned long)PosGetCommandLine();
            }
            else if (regsin->h.al == 0x40)
            {
                regsout->d.eax = PosInp(regsin->d.ebx);
            }
            else if (regsin->h.al == 0x41)
            {
                PosOutp(regsin->d.ebx, regsin->d.ecx);
            }
            else if (regsin->h.al == 0x42)
            {
                DP *dp;
                void *p;

                dp = (void *)(regsin->d.ebx);
                p = (void *)(dp->transferaddress);
                regsout->d.eax = PosAbsoluteDriveRead(regsin->d.edx,
                                                      dp->sectornumber,
                                                      dp->numberofsectors,
                                                      p);
#ifdef __32BIT__
                if (regsout->d.eax) regsout->x.cflag = 1;
                else regsout->x.cflag = 0;
#else
                if (regsout->x.ax) regsout->x.cflag = 1;
                else regsout->x.cflag = 0;
#endif
            }
            else if (regsin->h.al == 0x43)
            {
                DP *dp;
                void *p;

                dp = (void *)(regsin->d.ebx);
                p = (void *)(dp->transferaddress);
                regsout->d.eax = PosAbsoluteDriveWrite(regsin->d.edx,
                                                       dp->sectornumber,
                                                       dp->numberofsectors,
                                                       p);
#ifdef __32BIT__
                if (regsout->d.eax) regsout->x.cflag = 1;
                else regsout->x.cflag = 0;
#else
                if (regsout->x.ax) regsout->x.cflag = 1;
                else regsout->x.cflag = 0;
#endif
            }
            else if (regsin->h.al == 0x44)
            {
                regsout->d.eax = PosDoBoot(regsin->d.edx);
            }
            else if (regsin->h.al == 0x45)
            {
                regsout->d.eax = PosScrncap(regsin->d.edx);
            }
#endif
#ifndef __32BIT__
            else if (regsin->h.al == 0x42)
            {
                DP *dp;
                void *p;

                dp = (void *)MK_FP(sregs->ds,regsin->x.bx);
                p = (void *)(dp->transferaddress);
                regsout->x.ax = PosAbsoluteDriveRead(regsin->x.dx,
                                                     dp->sectornumber,
                                                     dp->numberofsectors,
                                                     p);
            }
            else if (regsin->h.al == 0x43)
            {
                DP *dp;
                void *p;

                dp = (void *)MK_FP(sregs->ds,regsin->x.bx);
                p = (void *)(dp->transferaddress);
                regsout->x.ax = PosAbsoluteDriveWrite(regsin->x.dx,
                                                      dp->sectornumber,
                                                      dp->numberofsectors,
                                                      p);
            }
            else if (regsin->h.al == 0x44)
            {
                regsout->x.ax = PosDoBoot(regsin->x.dx);
            }
            else if (regsin->h.al == 0x45)
            {
                regsout->d.ax = PosScrncap(regsin->x.dx);
            }
#endif
            else
            {
                logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
                regsout->x.cflag = 1;
            }
            break;

        default:
            logUnimplementedCall(0x21, regsin->h.ah, regsin->h.al);
            regsout->x.cflag = 1;
            break;
    }
    return;
}
