/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pos - implementation of pos.h                                    */
/*                                                                   */
/*********************************************************************/

#include <stddef.h>

#include "pos.h"
#ifdef WATNATIVE
#include <dos.h>
#else
#include "support.h"
#endif

/* Calls an interrupt without input or output registers */
/*
    Input: Interrupt number
    Returns: None
    Notes: None.
*/

static void int86n(unsigned int intno);

/* Calls an interrupt with input or output registers specified */
/*
    Input: Interrupt number and set of input or output registers.
    Returns: None.
    Notes: Set the output registers according to the interrupt called.
*/

static void int86i(unsigned int intno, union REGS *regsin);

/* PosTermNoRC-INT 20 */
/*
    Input: None
    Returns: None.
    Notes: Used to Terminate DOS Programs.
*/

void PosTermNoRC(void)
{
    int86n(0x20);
    return;
}

/* PosDisplayOutput-INT 21/AH=02h */
/*
    Input: Character to write.
    Returns: Last character output.
    Notes: If standard output is redirected to file no error checks
           are performed.
*/

unsigned int PosDisplayOutput(unsigned int ch)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x02;
    regsin.h.dl = ch;

    int86(0x21, &regsin, &regsout);

    return (regsout.h.al);
}

/* PosDirectConsoleOutput-INT 21/AH=06h */
/*
    Input: Character to write.
    Returns: Character written.
*/

unsigned int PosDirectConsoleOutput(unsigned int ch)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x06;
    regsin.h.dl = ch;

    int86(0x21, &regsin, &regsout);

    return (regsout.h.al);
}

/* PosDirectCharInputNoEcho-INT 21/AH=07h */
/*
    Input: None.
    Returns: Character read from standard input.
    Notes: Does not check ^C/^Break.
*/

unsigned int PosDirectCharInputNoEcho(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x07;

    int86(0x21, &regsin, &regsout);

    return (regsout.h.al);
}


/* Written By NECDET COKYAZICI, Public Domain */
/* PosGetCharInputNoEcho-INT 21/AH=08h */
/*
    Input: None.
    Returns: Character read from standard input.
    Notes: ^C/^Break are checked.
*/

unsigned int PosGetCharInputNoEcho(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x08;

    int86(0x21, &regsin, &regsout);

    return (regsout.h.al);
}

/* PosDisplayString-INT 21/AH=09h */
/*
    Input: Character buffer.
    Returns: '$'.
    Notes: ^C/^Break are checked.
*/

unsigned int PosDisplayString(const char *buf)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x09;

#ifdef __32BIT__
    regsin.d.edx = (int)buf;
#else
    regsin.x.dx = FP_OFF(buf);
    sregs.ds = FP_SEG(buf);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);

    return (regsout.h.al);
}

/* PosSelectDisk-INT 21/AH=0Eh */
/*
    Input: Default drive number(00h = A:, 01h = B:, etc).
    Returns: Number of potentially valid drive letters.
    Notes: None.
*/

unsigned int PosSelectDisk(unsigned int drive)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x0e;
    regsin.h.dl = drive;

    int86(0x21, &regsin, &regsout);
    return (regsout.h.al);
}

/* PosGetDefaultDrive-INT 21/AH=19h */
/*
    Input: None.
    Returns: Current Default Drive.
    Notes: AL = drive (00h = A:, 01h = B:, etc)
*/

unsigned int PosGetDefaultDrive(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x19;

    int86(0x21, &regsin, &regsout);
    return (regsout.h.al);
}

/* PosSetDTA-INT 21/AH=1Ah */
/*
    Input: None.
    Returns: None.
    Notes: Sets Disk Transfer Area Address.
*/

void PosSetDTA(void *dta)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x1a;
#ifdef __32BIT__
    regsin.d.edx = (int)dta;
#else
    sregs.ds = FP_SEG(dta);
    regsin.x.dx = FP_OFF(dta);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    return;
}

/* PosSetInterruptVector-INT 21/AH=25h */
/*
    Input: None.
    Returns: None.
    Notes: Sets the Interrupt Vector.
*/

void PosSetInterruptVector(unsigned int intnum, void *handler)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x25;
    regsin.h.al = (unsigned char)intnum;
#ifdef __32BIT__
    regsin.d.edx = (unsigned long)handler;
#else
    sregs.ds = FP_SEG(handler);
    regsin.x.dx = FP_OFF(handler);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    return;
}

/* PosGetSystemDate-INT 21/AH=2Ah */
/*
    Input: None.
    Returns: None.
    Notes: Used to get the System date calls
           BIOS INT 1A/AH=04h.(BosGetSystemDate())
*/

void PosGetSystemDate(unsigned int *year,
                      unsigned int *month,
                      unsigned int *day,
                      unsigned int *dw)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x2a;
    int86(0x21, &regsin, &regsout);

    *year = regsout.x.cx;
    *month = regsout.h.dh;
    *day = regsout.h.dl;
    *dw = regsout.h.al;

    return;
}


/* PosSetSystemDate-INT 21/AH=2Bh */
/*
    Input: None.
    Returns: None.
    Notes: Used to Set the system date calls
           BIOS INT 1A/AH=05h(BosSetSystemDate())
*/

unsigned int PosSetSystemDate(unsigned int year,
                              unsigned int month,
                              unsigned int day)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x2b;
    regsin.x.cx = year;
    regsin.h.dh = month;
    regsin.h.dl = day;

    int86(0x21, &regsin, &regsout);

    return (regsout.h.al);
}

/* PosGetSystemTime-INT 21/AH=2Ch */
/*
    Input: None.
    Returns:
    Notes:  On most systems, the resolution of the system clock
            is about 5/100sec,so returned times generally do not
            increment by 1. On some systems, DL may always return 00h.
*/

void PosGetSystemTime(unsigned int *hour, unsigned int *min,
                      unsigned int *sec, unsigned int *hundredths)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x2c;
    int86(0x21, &regsin, &regsout);

    *hour = regsout.h.ch;
    *min = regsout.h.cl;
    *sec = regsout.h.dh;
    *hundredths = regsout.h.dl;

    return;
}

/* func 2e - set read-after-write verification flag */
void PosSetVerifyFlag(int flag)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x2e;
    regsin.h.al = !!flag;
    int86(0x21, &regsin, &regsout);
}

void *PosGetDTA(void)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    void *dta;

    regsin.h.ah = 0x2f;
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    dta = (void *)regsout.d.ebx;
#else
    dta = MK_FP(sregs.es, regsout.x.bx);
#endif
    return (dta);
}

unsigned int PosGetDosVersion(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x30;
    int86(0x21, &regsin, &regsout);
    return ((regsout.h.al << 8) | regsout.h.ah);
}

/* INT 21,31 - PosTerminateAndStayResident */
void PosTerminateAndStayResident(int exitCode, int paragraphs)
{
    union REGS regsin;

    regsin.h.ah = 0x31;
    regsin.h.al = exitCode;
    regsin.x.dx = paragraphs;
    int86i(0x21, &regsin);
}

/* func 33, subfunc 00 - get Ctrl+Break checking flag status */
int PosGetBreakFlag()
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x33;
    regsin.h.al = 0x00;
    int86(0x21, &regsin, &regsout);
    return (regsout.h.dl);
}

/* func 33, subfunc 01 - set Ctrl+Break checking flag status */
void PosSetBreakFlag(int flag)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x33;
    regsin.h.al = 0x01;
    regsin.h.dl = !!flag;
    int86(0x21, &regsin, &regsout);
}

/* func 33, subfunc 05 - get boot drive */
int PosGetBootDrive(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x33;
    regsin.h.al = 0x05;
    int86(0x21, &regsin, &regsout);
    return (regsout.h.dl);
}

void *PosGetInterruptVector(int intnum)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x35;
    regsin.h.al = (char)intnum;
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    return ((void *)regsout.d.ebx);
#else
    return (MK_FP(sregs.es, regsout.x.bx));
#endif
}

void PosGetFreeSpace(int drive,unsigned int *secperclu,
                    unsigned int *numfreeclu,
                    unsigned int *bytpersec,
                    unsigned int *totclus)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah=0x36;
    regsin.h.dl=drive;
    int86(0x21, &regsin, &regsout);
    *secperclu = regsout.x.ax;
    *numfreeclu = regsout.x.bx;
    *bytpersec = regsout.x.cx;
    *totclus = regsout.x.dx;
    return;
}

int PosMakeDir(const char *dname)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x39;
#ifdef __32BIT__
    regsin.d.edx = (int)dname;
#else
    regsin.x.dx = FP_OFF(dname);
    sregs.ds = FP_SEG(dname);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (regsout.x.cflag)
    {
#ifdef __32BIT__
        return (regsout.d.eax);
#else
        return (regsout.x.ax);
#endif
    }
    else
    {
        return (0);
    }
}

int PosRemoveDir(const char *dname)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x3a;
#ifdef __32BIT__
    regsin.d.edx = (int)dname;
#else
    regsin.x.dx = FP_OFF(dname);
    sregs.ds = FP_SEG(dname);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (regsout.x.cflag)
    {
#ifdef __32BIT__
        return (regsout.d.eax);
#else
        return (regsout.x.ax);
#endif
    }
    else
    {
        return (0);
    }
}

int PosChangeDir(const char *to)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x3b;
#ifdef __32BIT__
    regsin.d.edx = (int)to;
#else
    regsin.x.dx = FP_OFF(to);
    sregs.ds = FP_SEG(to);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (regsout.x.cflag)
    {
#ifdef __32BIT__
        return (regsout.d.eax);
#else
        return (regsout.x.ax);
#endif
    }
    else
    {
        return (0);
    }
}

int PosCreatFile(const char *name,
                 int attrib,
                 int *handle)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x3c;
    regsin.x.cx = attrib;
#ifdef __32BIT__
    regsin.d.edx = (unsigned int)name;
#else
    sregs.ds = FP_SEG(name);
    regsin.x.dx = FP_OFF(name);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    if (regsout.x.cflag) return (regsout.d.eax);
    *handle = regsout.d.eax;
#else
    if (regsout.x.cflag) return (regsout.x.ax);
    *handle = regsout.x.ax;
#endif
    return (0);
}

int PosOpenFile(const char *name,
                int mode,
                int *handle)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x3d;
    regsin.h.al = (unsigned char)mode;
#ifdef __32BIT__
    regsin.d.edx = (unsigned int)name;
#else
    sregs.ds = FP_SEG(name);
    regsin.x.dx = FP_OFF(name);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    if (regsout.x.cflag) return (regsout.d.eax);
    *handle = regsout.d.eax;
#else
    if (regsout.x.cflag) return (regsout.x.ax);
    *handle = regsout.x.ax;
#endif
    return (0);
}

int PosCloseFile(int handle)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x3e;
#ifdef __32BIT__
    regsin.d.ebx = handle;
#else
    regsin.x.bx = handle;
#endif
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}

int PosReadFile(int fh, void *data, unsigned int bytes, unsigned int *readbytes)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x3f;
#ifdef __32BIT__
    regsin.d.ebx = (unsigned int)fh;
    regsin.d.ecx = (unsigned int)bytes;
    regsin.d.edx = (unsigned int)data;
#else
    regsin.x.bx = (unsigned int)fh;
    regsin.x.cx = (unsigned int)bytes;
    sregs.ds = FP_SEG(data);
    regsin.x.dx = FP_OFF(data);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        *readbytes = 0;
        return (regsout.d.eax);
    }
    *readbytes = regsout.d.eax;
#else
    if (regsout.x.cflag)
    {
        *readbytes = 0;
        return (regsout.x.ax);
    }
    *readbytes = regsout.x.ax;
#endif
    return (0);
}

int PosWriteFile(int handle,
                 const void *data,
                 unsigned int len,
                 unsigned int *writtenbytes)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x40;
#ifdef __32BIT__
    regsin.d.ebx = (unsigned int)handle;
    regsin.d.ecx = (unsigned int)len;
    regsin.d.edx = (unsigned int)data;
#else
    regsin.x.bx = (unsigned int)handle;
    regsin.x.cx = (unsigned int)len;
    sregs.ds = FP_SEG(data);
    regsin.x.dx = FP_OFF(data);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        *writtenbytes = 0;
        return (regsout.d.eax);
    }
    *writtenbytes = regsout.d.eax;
#else
    if (regsout.x.cflag)
    {
        *writtenbytes = 0;
        return (regsout.x.ax);
    }
    *writtenbytes = regsout.x.ax;
#endif
    return (0);
}


/* func 41h - delete file */
/* PosDeleteFile returns 0 if delete was successful */

int PosDeleteFile(const char *fname)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x41;
#ifdef __32BIT__
    regsin.d.edx = (int)fname;
#else
    sregs.ds = FP_SEG(fname);
    regsin.x.dx = FP_OFF(fname);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
}

int PosMoveFilePointer(int handle, long offset, int whence, long *newpos)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x42;
#ifdef __32BIT__
    regsin.d.ebx = handle;
    regsin.d.ecx = offset;
#else
    regsin.x.bx = handle;
    regsin.x.dx = FP_OFF(offset);
    regsin.x.cx = FP_SEG(offset);
#endif
    regsin.h.al = (char)whence;
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        return (regsout.d.eax);
    }
    *newpos = regsout.d.eax;
#else
    if (regsout.x.cflag)
    {
        return (regsout.x.ax);
    }
    *newpos = ((long)regsout.x.dx << 16) | regsout.x.ax;
#endif
    return (0);
}

/*Function to get all the file attributes from the filename*/
int PosGetFileAttributes(const char *fnm,int *attr)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x43;
    regsin.h.al=0x00;
#ifdef __32BIT__
    regsin.d.edx = (int)fnm;
#else
    sregs.ds = FP_SEG(fnm);
    regsin.x.dx = FP_OFF(fnm);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    *attr = regsout.x.cx;

    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
}

/*Set the attributes of the given file*/
int PosSetFileAttributes(const char *fnm,int attr)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x43;
    regsin.h.al = 0x01;
#ifdef __32BIT__
    regsin.d.edx = (int)fnm;
    regsin.d.ecx=attr;
#else
    sregs.ds = FP_SEG(fnm);
    regsin.x.dx = FP_OFF(fnm);
    regsin.x.cx=attr;
#endif
    int86x(0x21, &regsin, &regsout, &sregs);

    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
}
/**/

/**/
int PosGetDeviceInformation(int handle, unsigned int *devinfo)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x44;
    regsin.h.al = 0x00;
#ifdef __32BIT__
    regsin.d.ebx = handle;
#else
    regsin.x.bx = handle;
#endif
    int86(0x21, &regsin, &regsout);
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
#ifdef __32BIT__
        *devinfo = regsout.d.edx;
#else
        *devinfo = regsout.x.dx;
#endif
    }
    return (regsout.x.ax);
}

int PosSetDeviceInformation(int handle, unsigned int devinfo)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x44;
    regsin.h.al = 0x01;
#ifdef __32BIT__
    regsin.d.ebx = handle;
    regsin.d.edx = devinfo;
#else
    regsin.x.bx = handle;
    regsin.x.dx = devinfo;
#endif
    int86(0x21, &regsin, &regsout);
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
}

int PosBlockDeviceRemovable(int drive)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x44;
    regsin.h.al = 0x08;
    regsin.h.bl = (char)drive;
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        regsout.d.eax = -regsout.d.eax;
    }
    return (regsout.d.eax);
#else
    if (regsout.x.cflag)
    {
        regsout.x.ax = -regsout.x.ax;
    }
    return (regsout.x.ax);
#endif
}

/*Get the status of the remote device */
int PosBlockDeviceRemote(int drive,int *da)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x44;
    regsin.h.al = 0x09;
    regsin.h.bl = (char)drive;
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        regsout.d.eax = -regsout.d.eax;
    }
    else
    {
        regsout.d.eax = 0;
        *da= regsout.d.edx;
    }
    return (regsout.d.eax);
#else
    if (regsout.x.cflag)
    {
        regsout.x.ax = -regsout.x.ax;
    }
    else
    {
        regsout.x.ax = 0;
        *da= regsout.x.dx;
    }
    return (regsout.x.ax);
#endif
}
/**/

/*Device request function call 440D*/
int PosGenericBlockDeviceRequest(int drive,
                                 int catcode,
                                 int function,
                                 void *parmblock)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x44;
    regsin.h.al = 0x0D;
    regsin.h.bl = drive;
    regsin.h.ch = catcode;
    regsin.h.cl = function;

#ifdef __32BIT__
    regsin.d.edx = (unsigned int)parmblock;
#else
    sregs.ds = FP_SEG(parmblock);
    regsin.x.dx = FP_OFF(parmblock);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);

#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        regsout.d.eax = -regsout.d.eax;
    }
    else
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (regsout.x.cflag)
    {
        regsout.x.ax = -regsout.x.ax;
    }
    else
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}

int PosDuplicateFileHandle(int fh)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x45;
#ifdef __32BIT__
    regsin.d.ebx = fh;
#else
    regsin.x.bx = fh;
#endif
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
        return (regsout.d.eax);
#else
        return (regsout.x.ax);
#endif
}

int PosForceDuplicateFileHandle(int fh, int newfh)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x46;
#ifdef __32BIT__
    regsin.d.ebx = fh;
    regsin.d.ecx = newfh;
#else
    regsin.x.bx = fh;
    regsin.x.cx = newfh;
#endif
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
        return (regsout.d.eax);
#else
        return (regsout.x.ax);
#endif
}

/**/
int PosGetCurDir(int drive, char *dir)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x47;
    regsin.h.dl = drive;
#ifdef __32BIT__
    regsin.d.esi = (unsigned int)dir;
#else
    regsin.x.si = FP_OFF(dir);
    sregs.ds = FP_SEG(dir);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (regsout.x.cflag)
    {
#ifdef __32BIT__
        return (regsout.d.eax);
#else
        return (regsout.x.ax);
#endif
    }
    else
    {
        return (0);
    }
}

void *PosAllocMemPages(unsigned int pages, unsigned int *maxpages)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x48;
#ifdef __32BIT__
    regsin.d.ebx = pages * 16;
#else
    regsin.x.bx = pages;
#endif
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    if (maxpages != NULL)
    {
        *maxpages = regsout.d.ebx;
    }
    return ((void *)regsout.d.eax);
#else
    if (regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    if (maxpages != NULL)
    {
        *maxpages = regsout.x.bx;
    }
    return (MK_FP(regsout.x.ax, 0));
#endif
}

int PosFreeMem(void *ptr)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x49;
#ifdef __32BIT__
    regsin.d.ebx = (int)ptr;
#else
    sregs.es = FP_SEG(ptr);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
}

int PosReallocMem(void *ptr, unsigned int newpages, unsigned int *maxp)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x4a;
#ifdef __32BIT__
    regsin.d.ecx = (int)ptr;
    regsin.d.ebx = newpages;
#else
    sregs.es = FP_SEG(ptr);
    regsin.x.bx = newpages;
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
#ifdef __32BIT__
        *maxp = regsout.d.ebx;
#else
        *maxp = regsout.x.bx;
#endif
    }
    return (regsout.x.ax);
}

int PosExec(char *prog, POSEXEC_PARMBLOCK *parmblock)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x4b;
    regsin.h.al = 0;
#ifdef __32BIT__
    regsin.d.edx = (int)prog;
    regsin.d.ebx = (int)parmblock;
#else
    sregs.ds = FP_SEG(prog);
    regsin.x.dx = FP_OFF(prog);
    sregs.es = FP_SEG(parmblock);
    regsin.x.bx = FP_OFF(parmblock);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
}

void PosReadBufferedInput(pos_input_buffer *buf)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x0A;
    regsin.h.al = 0x00;
#ifdef __32BIT__
    regsin.d.edx = (int)buf;
#else
    sregs.ds = FP_SEG(buf);
    regsin.x.dx = FP_OFF(buf);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    return;
}

void PosTerminate(int rc)
{
    union REGS regsin;

    regsin.h.ah = 0x4c;
    regsin.h.al = rc;
    int86i(0x21, &regsin);
    return;
}

int PosGetReturnCode(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x4d;
    int86(0x21, &regsin, &regsout);
    return (regsout.x.ax);
}

int PosFindFirst(char *pat, int attrib)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x4e;
    regsin.x.cx = attrib;
#ifdef __32BIT__
    regsin.d.edx = (int)pat;
#else
    regsin.x.dx = FP_OFF(pat);
    sregs.ds = FP_SEG(pat);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (regsout.x.cflag)
    {
        return (regsout.x.ax);
    }
    else
    {
        return (0);
    }
}

int PosFindNext(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x4f;
    int86(0x21, &regsin, &regsout);
    if (regsout.x.cflag)
    {
        return (regsout.x.ax);
    }
    else
    {
        return (0);
    }
}

/* func 51 - get current process ID */
int PosGetCurrentProcessId(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x51;
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
    return (regsout.d.ebx);
#else
    return (regsout.x.bx);
#endif
}

/* func 54 - get read-after-write verification flag */
int PosGetVerifyFlag()
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0x54;
    int86(0x21, &regsin, &regsout);
    return (regsout.h.al);
}

int PosRenameFile(const char *old, const char *new)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x56;
#ifdef __32BIT__
    regsin.d.edx = (int)old;
    regsin.d.edi = (int)new;
#else
    sregs.ds = FP_SEG(old);
    regsin.x.dx = FP_OFF(old);
    sregs.es = FP_SEG(new);
    regsin.x.di = FP_OFF(new);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
}

/*Get the last written date and time of the file*/
int PosGetFileLastWrittenDateAndTime(int handle,
                                     unsigned int *fdate,
                                     unsigned int *ftime)
{
    union REGS regsin;
    union REGS regsout;

    regsin.x.ax=0x5700;
    regsin.x.bx=handle;

    int86(0x21, &regsin, &regsout);

    if (!regsout.x.cflag)
    {
#ifdef __32BIT__
        *fdate=regsout.d.edx;
        *ftime=regsout.d.ecx;
        regsout.d.eax=0;
#else
        *fdate=regsout.x.dx;
        *ftime=regsout.x.cx;
        regsout.x.ax = 0;
#endif
    }
#ifdef __32BIT__
    return (regsout.d.eax);
#else
    return (regsout.x.ax);
#endif
}
/**/

/**/
int PosSetFileLastWrittenDateAndTime(int handle,
                                     unsigned int fdate,
                                     unsigned int ftime)
{
    union REGS regsin;
    union REGS regsout;

    regsin.x.ax=0x5701;
    regsin.x.bx=handle;
#ifdef __32BIT__
    regsin.d.edx=fdate;
    regsin.d.ecx=ftime;
#else
    regsin.x.dx=fdate;
    regsin.x.cx=ftime;
#endif

    int86(0x21, &regsin, &regsout);

    if (!regsout.x.cflag)
    {
#ifdef __32BIT__
        regsout.d.eax=0;
#else
        regsout.x.ax = 0;
#endif
    }
#ifdef __32BIT__
    return (regsout.d.eax);
#else
    return (regsout.x.ax);
#endif
}

int PosCreatNewFile(const char *name, int attrib, int *handle)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x5b;
    regsin.x.cx = attrib;
#ifdef __32BIT__
    regsin.d.edx = (unsigned int)name;
#else
    sregs.ds = FP_SEG(name);
    regsin.x.dx = FP_OFF(name);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    if (regsout.x.cflag) return (regsout.d.eax);
    *handle = regsout.d.eax;
#else
    if (regsout.x.cflag) return (regsout.x.ax);
    *handle = regsout.x.ax;
#endif
    return (0);
}

/**/
/*Determine the canonical name of the specified filename or path*/
int PosTruename(char *prename,char *postname)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah=0x60;
#ifdef __32BIT__
    regsin.d.esi = (int)prename;
    regsin.d.edi = (int)postname;
#else
    regsin.x.si = FP_OFF(prename);
    sregs.ds = FP_SEG(prename);
    regsin.x.di = FP_OFF(postname);
    sregs.es = FP_SEG(postname);
#endif
    int86x(0x21,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        regsout.d.eax = -regsout.d.eax;
    }
    else
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (regsout.x.cflag)
    {
        regsout.x.ax = -regsout.x.ax;
    }
    else
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}

#ifdef __32BIT__
void PosAExec(char *prog, POSEXEC_PARMBLOCK *parmblock)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x80;
    regsin.d.edx = (int)prog;
    regsin.d.ebx = (int)parmblock;

    int86x(0x21, &regsin, &regsout, &sregs);
    return;
}
#endif

/* extension to display an integer for debugging purposes */

void PosDisplayInteger(int x)
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0;
#ifdef __32BIT__
    regsin.d.ecx = x;
#else
    regsin.x.cx = x;
#endif
    int86i(0x21, &regsin);
    return;
}

/* extension to reboot computer */

void PosReboot(void)
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 1;
    int86i(0x21, &regsin);
    return;
}

/* pos extension to set DOS version */
void PosSetDosVersion(unsigned int version)
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 3;
    regsin.x.bx = version;
    int86i(0x21, &regsin);
}

/* pos extension to get "log unimplemented" flag */
int PosGetLogUnimplemented(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xf6;
    regsin.h.al = 4;
    int86(0x21, &regsin, &regsout);
    return (regsout.x.ax);
}

/* pos extension to set "log unimplemented" flag */
void PosSetLogUnimplemented(int flag)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xf6;
    regsin.h.al = 5;
    regsin.x.bx = !!flag;
    int86(0x21, &regsin, &regsout);
}

/* pos extension to get "PDOS magic" */
int PosGetMagic(void)
{
    union REGS regsin;
    union REGS regsout;

    /* On genuine MSDOS, since this is unimplemented, AL will be set
       to 0, which makes it different from the PDOS magic number which
       is non-zero in AL. So there will not be a clash. Note that the
       CF won't be changed on MSDOS, so that can't be used as an
       indicator. */
    regsin.h.ah = 0xf6;
    regsin.h.al = 6;
    int86(0x21, &regsin, &regsout);
    return (regsout.x.ax);
}

/* pos extension to get memory manager statistics */
void PosGetMemoryManagementStats(void *stats)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x7;
#ifdef __32BIT__
    regsin.d.edx = (int)stats;
#else
    sregs.ds = FP_SEG(stats);
    regsin.x.dx = FP_OFF(stats);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    return;
}

/* pos extension to get per-process memory manager statistics */
void PosProcessGetMemoryStats(unsigned long pid, void *stats) /* func f6.0d */
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0xd;
#ifdef __32BIT__
    regsin.d.edx = (int)stats;
    regsin.d.ecx = pid;
#else
    sregs.ds = FP_SEG(stats);
    regsin.x.dx = FP_OFF(stats);
    regsin.x.cx = (int)pid;
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    return;
}

/* func f6.0c - pos extension to get info about a process */
int PosProcessGetInfo(unsigned long pid, PDOS_PROCINFO *info, unsigned int infoSz)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x0c;
#ifdef __32BIT__
    regsin.d.ebx = pid;
    regsin.d.ecx = infoSz;
    regsin.d.edx = (unsigned long)info;
#else
    regsin.x.bx = pid;
    regsin.x.cx = infoSz;
    sregs.ds = FP_SEG(info);
    regsin.x.dx = FP_OFF(info);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    return regsout.x.ax;
}

void *PosAllocMem(unsigned int size, unsigned int flags)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x8;
#ifdef __32BIT__
    regsin.d.ebx = size;
    regsin.d.ecx = flags;
#else
    regsin.x.bx = size;
    regsin.x.bx >>= 4;
    if ((size % 16) != 0)
    {
        regsin.x.bx++;
    }
#endif
    int86(0x21, &regsin, &regsout);
#ifdef __32BIT__
    if (regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return ((void *)regsout.d.eax);
#else
    if (regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (MK_FP(regsout.x.ax, 0));
#endif
}

/* POS extension to get error message string */
char *PosGetErrorMessageString(unsigned int errorCode)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    char *msg;

    regsin.h.ah = 0xf6;
    regsin.h.al = 9;
    regsin.x.bx = errorCode;
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    msg = (char *)regsout.d.edx;
#else
    msg = (char *)(MK_FP(sregs.es, regsout.x.dx));
#endif
    return (msg);
}

/* extension to power off computer */
void PosPowerOff(void)
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x0A;
    int86i(0x21, &regsin);
    return;
}

/* extension to clear screen */
void PosClearScreen(void)
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x30;
    int86i(0x21, &regsin);
    return;
}

void PosMoveCursor(int row, int col) /* func f6.31 */
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x31;
    regsin.h.bh = row;
    regsin.h.bl = col;
    int86i(0x21, &regsin);
    return;
}

int PosGetVideoInfo(pos_video_info *info, unsigned int size) /* func f6.32 */
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x32;
#ifdef __32BIT__
    regsin.d.edx = (unsigned int) info;
    regsin.d.ecx = size;
#else
    sregs.ds = FP_SEG(info);
    regsin.x.dx = FP_OFF(info);
    regsin.x.cx = size;
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    return regsout.d.eax;
#else
    return regsout.x.ax;
#endif
}

int PosKeyboardHit(void) /* func f6.33 */
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x33;
    int86x(0x21, &regsin, &regsout, &sregs);
    return regsout.x.ax;
}

void PosYield(void) /* func f6.34 */
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x34;
    int86i(0x21, &regsin);
    return;
}

void PosSleep(unsigned long seconds) /* func f6.35 */
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x35;
#ifdef __32BIT__
    regsin.d.edx = seconds;
#else
    regsin.x.dx = seconds >> 16;
    regsin.x.bx = seconds;
#endif
    int86i(0x21, &regsin);
    return;
}

/* INT 21,F6,36 - Get tick count */
unsigned long PosGetClockTickCount(void)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    unsigned long ticks;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x36;
    int86x(0x21, &regsin, &regsout, &sregs);
#ifdef __32BIT__
    ticks = regsout.d.edx;
#else
    ticks = regsout.x.dx;
    ticks = ticks << 16UL;
    ticks |= regsout.x.bx;
#endif
    return ticks;
}

void PosSetVideoAttribute(unsigned int attr) /* func f6.37 */
{
    union REGS regsin;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x37;
    regsin.x.bx = attr;
    int86i(0x21, &regsin);
    return;
}

int PosSetVideoMode(unsigned int mode) /* func f6.38 */
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x38;
    regsin.x.bx = mode;
    int86x(0x21, &regsin, &regsout, &sregs);
    return regsout.x.ax;
}

int PosSetVideoPage(unsigned int page) /* func f6.39 */
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x39;
    regsin.x.bx = page;
    int86x(0x21, &regsin, &regsout, &sregs);
    return regsout.x.ax;
}

/* F6,3A - Set Environment Variable */
int PosSetEnv(char *name, char *value)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    regsin.h.ah = 0xf6;
    regsin.h.al = 0x3A;
#ifdef __32BIT__
    regsin.d.ebx = (int)name;
    regsin.d.edx = (int)value;
#else
    regsin.x.bx = FP_OFF(name);
    sregs.ds = FP_SEG(name);
    regsin.x.dx = FP_OFF(value);
    sregs.es = FP_SEG(value);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    return regsout.x.ax;
}

/* F6,3B - Get Environment Block */
void * PosGetEnvBlock(void)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    regsin.h.ah = 0xf6;
    regsin.h.al = 0x3B;
    int86x(0x21, &regsin, &regsout, &sregs);
    if (regsout.x.ax != 0)
        return NULL;
#ifdef __32BIT__
    return (void*) regsout.d.ebx;
#else
    return MK_FP(sregs.ds, regsout.x.bx);
#endif
}

/* F6,3C - Set Named Font */
int PosSetNamedFont(char *fontName)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    regsin.h.ah = 0xf6;
    regsin.h.al = 0x3C;
#ifdef __32BIT__
    regsin.d.ebx = (int)fontName;
#else
    regsin.x.bx = FP_OFF(fontName);
    sregs.ds = FP_SEG(fontName);
#endif
    int86x(0x21, &regsin, &regsout, &sregs);
    return regsout.x.ax;
}

#ifdef __32BIT__
/* F6,3D - Allocate Virtual Memory */
void *PosVirtualAlloc(void *addr, size_t size)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x3D;

    regsin.d.ebx = (unsigned long)addr;
    regsin.d.ecx = size;

    int86(0x21, &regsin, &regsout);
    return ((void *)(regsout.d.eax));
}

/* F6,3E - Free Virtual Memory */
void PosVirtualFree(void *addr, size_t size)
{
    union REGS regsin;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x3E;

    regsin.d.ebx = (unsigned long)addr;
    regsin.d.ecx = size;

    int86i(0x21, &regsin);
}

/* F6,3F - Get Command Line String For The Current Process */
char *PosGetCommandLine(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x3F;

    int86(0x21, &regsin, &regsout);
    return ((char *)(regsout.d.eax));
}

/* F6,40 - Read Byte From Port */
unsigned char PosInp(unsigned int port)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xf6;
    regsin.h.al = 0x40;

    regsin.d.ebx = port;

    int86(0x21, &regsin, &regsout);
    return ((unsigned char)(regsout.d.eax));
}

/* F6,41 - Write Byte To Port */
void PosOutp(unsigned int port, unsigned char data)
{
    union REGS regsin;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x41;

    regsin.d.ebx = port;
    regsin.d.ecx = data;

    int86i(0x21, &regsin);
}
#endif

/* F6,42 - Absolute Drive Read */
unsigned int PosAbsoluteDriveRead(int drive,unsigned long start_sector,
                                  unsigned int sectors,void *buf)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    DP dp;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x42;
#ifdef __32BIT__
    regsin.d.edx = drive;
    regsin.d.ebx=(int)&dp;
#else
    regsin.x.dx = drive;
    sregs.ds = FP_SEG(&dp);
    regsin.x.bx = FP_OFF(&dp);
#endif
    dp.sectornumber=start_sector;
    dp.numberofsectors=sectors;
    dp.transferaddress=buf;
    int86x(0x21,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}

/* F6,43 - Absolute Drive Write */
unsigned int PosAbsoluteDriveWrite(int drive,unsigned long start_sector,
                                   unsigned int sectors,void *buf)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    DP dp;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x43;
#ifdef __32BIT__
    regsin.d.edx = drive;
    regsin.d.ebx=(int)&dp;
#else
    regsin.x.dx = drive;
    sregs.ds = FP_SEG(&dp);
    regsin.x.bx = FP_OFF(&dp);
#endif
    dp.sectornumber=start_sector;
    dp.numberofsectors=sectors;
    dp.transferaddress=buf;
    int86x(0x21,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/* F6,44 - Boot a Disk */
unsigned int PosDoBoot(int disknum)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x44;
#ifdef __32BIT__
    regsin.d.edx = disknum;
#else
    regsin.x.dx = disknum;
#endif
    int86(0x21,&regsin,&regsout);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/* F6,45 - screen capture */
/* disk of -1 terminates */
unsigned int PosScrncap(int disknum)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x45;
#ifdef __32BIT__
    regsin.d.edx = disknum;
#else
    regsin.x.dx = disknum;
#endif
    int86(0x21,&regsin,&regsout);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}

#ifdef __32BIT__
/* F6,46 - GetStdHandle */
void *PosGetStdHandle(unsigned int nStdHandle)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x46;

    regsin.d.edx = nStdHandle;

    int86(0x21, &regsin, &regsout);
    return ((void *)(regsout.d.eax));
}


/* F6,47 - SetStdHandle */
unsigned int PosSetStdHandle(unsigned int nStdHandle, void *hHandle)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x47;

    regsin.d.edx = nStdHandle;
    regsin.d.ebx = (unsigned long)hHandle;

    int86(0x21, &regsin, &regsout);
    return ((unsigned int)(regsout.d.eax));
}
#endif


/* F6,48 - invoke system monitor */
unsigned int PosMonitor(void)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x48;
    int86(0x21,&regsin,&regsout);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/* F6,49 - show return codes */
unsigned int PosShowret(int flag)
{
    union REGS regsin;
    union REGS regsout;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x49;
#ifdef __32BIT__
    regsin.d.edx = flag;
#else
    regsin.x.dx = flag;
#endif
    int86(0x21,&regsin,&regsout);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/* F6,4A - Keyboard Map */
unsigned int PosKeyboardMap(unsigned char *newmap)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x4A;
#ifdef __32BIT__
    regsin.d.ebx=(int)newmap;
#else
    sregs.ds = FP_SEG(newmap);
    regsin.x.bx = FP_OFF(newmap);
#endif
    int86x(0x21,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/* F6,4B - Screen Map */
unsigned int PosScreenMap(unsigned char *newmap)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x4B;
#ifdef __32BIT__
    regsin.d.ebx=(int)newmap;
#else
    sregs.ds = FP_SEG(newmap);
    regsin.x.bx = FP_OFF(newmap);
#endif
    int86x(0x21,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/* F6,4C - Scancode Map */
unsigned int PosScancodeMap(unsigned char *newmap, int type)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x4C;
#ifdef __32BIT__
    regsin.d.ebx=(int)newmap;
    regsin.d.ecx = type;
#else
    sregs.ds = FP_SEG(newmap);
    regsin.x.bx = FP_OFF(newmap);
    regsin.x.cx = type;
#endif
    int86x(0x21,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/* F6,4D - load fonts */
unsigned int PosLoadFonts(int start,
                          int count,
                          int depth,
                          void *table,
                          int block)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x4D;
#ifdef __32BIT__
    regsin.d.ebx=(int)table;
    regsin.d.ecx = start;
    regsin.d.edx = count;
    regsin.d.esi = depth;
    regsin.d.edi = block;
#else
    sregs.ds = FP_SEG(table);
    regsin.x.bx = FP_OFF(table);
    regsin.x.cx = start;
    regsin.x.dx = count;
    regsin.x.si = depth;
    regsin.x.di = block;
#endif
    int86x(0x21,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/* F6,4E - Accent Map */
unsigned int PosAccentMap(int accent_key, unsigned char *newmap)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0xF6;
    regsin.h.al = 0x4E;
#ifdef __32BIT__
    regsin.d.ebx=(int)newmap;
    regsin.d.ecx = accent_key;
#else
    sregs.ds = FP_SEG(newmap);
    regsin.x.bx = FP_OFF(newmap);
    regsin.x.cx = accent_key;
#endif
    int86x(0x21,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}


/*int 25 function call*/
unsigned int PosAbsoluteDiskRead(int drive,unsigned long start_sector,
                                 unsigned int sectors,void *buf)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    DP dp;

    regsin.h.al = drive;
#ifdef __32BIT__
    regsin.d.ecx = 0xffff;
    regsin.d.ebx=(int)&dp;
#else
    regsin.x.cx = 0xffff;
    sregs.ds = FP_SEG(&dp);
    regsin.x.bx = FP_OFF(&dp);
#endif
    dp.sectornumber=start_sector;
    dp.numberofsectors=sectors;
    dp.transferaddress=buf;
    int86x(0x25,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}
/**/

/*int 26 function call*/
unsigned int PosAbsoluteDiskWrite(int drive,unsigned long start_sector,
                                  unsigned int sectors,void *buf)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    DP dp;

    regsin.h.al = drive;
#ifdef __32BIT__
    regsin.d.ecx = 0xffff;
    regsin.d.ebx=(int)&dp;
#else
    regsin.x.cx = 0xffff;
    sregs.ds = FP_SEG(&dp);
    regsin.x.bx = FP_OFF(&dp);
#endif
    dp.sectornumber=start_sector;
    dp.numberofsectors=sectors;
    dp.transferaddress=buf;
    int86x(0x26,&regsin,&regsout,&sregs);
#ifdef __32BIT__
    if (!regsout.x.cflag)
    {
        regsout.d.eax = 0;
    }
    return (regsout.d.eax);
#else
    if (!regsout.x.cflag)
    {
        regsout.x.ax = 0;
    }
    return (regsout.x.ax);
#endif
}
/**/


/* int86n - do an interrupt with no registers */

static void int86n(unsigned int intno)
{
    union REGS regsin;
    union REGS regsout;

    int86(intno, &regsin, &regsout);
    return;
}

/* int86i - do an interrupt with input registers only */

static void int86i(unsigned int intno, union REGS *regsin)
{
    union REGS regsout;

    int86(intno, regsin, &regsout);
    return;
}

int intdos(union REGS *regsin, union REGS *regsout)
{
    return (int86(0x21, regsin, regsout));
}

#ifndef WATNATIVE
int bdos(int func, int dx, int al)
{
    union REGS regs;

    regs.h.ah = (char)func;
    regs.h.al = al;
#ifdef __32BIT__
    regs.d.edx = dx;
#else
    regs.x.dx = dx;
#endif
    int86(0x21, &regs, &regs);
#ifdef __32BIT__
    return (regs.d.eax);
#else
    return (regs.x.ax);
#endif
}
#endif
