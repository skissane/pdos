/* written by Paul Edwards */
/* released to the public domain */
/* pdossupc - support routines for pdos */

#include "stddef.h"

#include <pos.h>
#include <support.h>

#if defined(__WATCOMC__)
#define CTYP __cdecl
#else
#define CTYP
#endif

#ifndef __PDOSGENOS__
int CTYP __open(const char *filename, int mode, int *errind)
{
    int handle;

    if (PosOpenFile(filename, 0, &handle)) *errind = 1;
    else *errind = 0;
    return (handle);
}
#endif

int CTYP __creat(const char *filename, int mode, int *errind)
{
    int handle;

    if (PosCreatFile(filename, 0, &handle)) *errind = 1;
    else *errind = 0;
    return (handle);
}

int CTYP __read(int handle, void *buf, unsigned int len, int *errind)
{
    unsigned int readbytes;

    if (PosReadFile(handle, buf, len, &readbytes)) *errind = 1;
    else *errind = 0;
    return (readbytes);
}

int CTYP __write(int handle, const void *buf, unsigned int len, int *errind)
{
    unsigned int writtenbytes;

    if (PosWriteFile(handle, buf, len, &writtenbytes)) *errind = 1;
    else *errind = 0;
    return (writtenbytes);
}

int CTYP __seek(int handle, long offset, int whence)
{
    long dummy;
    return (PosMoveFilePointer(handle, offset, whence, &dummy));
}

void CTYP __close(int handle)
{
    PosCloseFile(handle);
    return;
}

int CTYP __devginfo(int handle, unsigned int *info)
{
    int rc;

    rc = PosGetDeviceInformation(handle, info);
    return (rc);
}

void CTYP __devsinfo(int handle, unsigned int info)
{
    PosSetDeviceInformation(handle, info);
    return;
}

int CTYP __remove(const char *filename)
{
    int rc;
    rc = PosDeleteFile(filename);
    return (rc);
}

int CTYP __rename(const char *old, const char *new)
{
    int rc;
    rc = PosRenameFile(old, new);
    return (rc);
}

#if defined(__32BIT__) && !defined(NOLIBALLOC)
/* PDOS-32 uses liballoc with PosVirtualAlloc() and PosVirtualFree(). */
#include "liballoc.h"

int __liballoc_lock()
{
    return (0);
}

int __liballoc_unlock()
{
    return (0);
}

void *__liballoc_alloc(size_t num_pages)
{
    return (PosVirtualAlloc(NULL, num_pages * 0x1000));
}

int __liballoc_free(void *addr, size_t num_pages)
{
    PosVirtualFree(addr, num_pages * 0x1000);

    return (0);
}

#else

void CTYP __allocmem(size_t size, void **ptr)
{
#if defined(__32BIT__) || defined(__PDOS386__)
    *ptr = PosAllocMem(size, POS_LOC32);
#elif defined(__SMALLERC__) || defined(PDOS86)
    *ptr = PosAllocMemPages((size >> 4) + (((size % 16) != 0) ? 1 : 0), NULL);
#else
    *ptr = PosAllocMem(size, POS_LOC20);
#endif
    return;
}

void CTYP __freemem(void *ptr)
{
    PosFreeMem(ptr);
    return;
}
#endif

int CTYP __exec(char *cmd, void *env)
{
    return (PosExec(cmd, env));
}

int CTYP __getrc(void)
{
    return (PosGetReturnCode());
}

void CTYP __datetime(void *ptr)
{
    unsigned int year, month, day, dow;
    unsigned int hour, minute, second, hundredths;
    unsigned int *iptr = ptr;

    PosGetSystemDate(&year, &month, &day, &dow);
    iptr[0] = year;
    iptr[1] = month;
    iptr[2] = day;
    PosGetSystemTime(&hour, &minute, &second, &hundredths);
    iptr[3] = hour;
    iptr[4] = minute;
    iptr[5] = second;
    iptr[6] = hundredths;
    PosGetSystemDate(&year, &month, &day, &dow);
    if (day != iptr[2])
    {
        __datetime(ptr);
    }
    return;
}

int CTYP __magic(void)
{
    return (PosGetMagic());
}

#ifndef NOUNDMAIN
void __main(void)
{
    return;
}
#endif

#ifndef PDOS86
void CTYP __exita(int retcode)
{
#ifndef PDOS_RET_EXIT
     PosTerminate(retcode);
#endif
    return;
}
#endif

#if defined(__WATCOMC__) && defined(__32BIT__)

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
