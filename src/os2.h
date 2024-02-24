/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  OS/2 1.0 prototypes                                              */
/*                                                                   */
/*********************************************************************/

#include <stddef.h>

#define NULL ((void *)0)

#ifdef __16BIT__
#define APIENTRY _pascal _far
#else
#ifndef __SUPPRESS__
#define APIENTRY _System
#endif
#endif

/* I think this only exists in 32-bit, but maybe
   we can change that later */
#ifndef __16BIT__
#define FILE_BEGIN 0
#endif

/* i think this is only available in 32-bit,
   but we may want to change that regardless one day */
#ifndef __16BIT__
#define APIRET int
#endif

/* I think this is only relevant to 32-bit */
#ifndef __16BIT__
#define PAG_COMMIT 0x10
#define PAG_WRITE 0x2
#define PAG_READ 0x1
#endif

#ifndef __SUPPRESS__
typedef unsigned short USHORT;
#endif

typedef unsigned long ULONG;
#ifndef __SUPPRESS__
typedef int LONG;
#endif
typedef char *PSZ;
#define VOID void
#ifndef __SUPPRESS__
typedef void *PVOID;
#endif

#ifndef __SUPPRESS__
typedef unsigned char BYTE;
#endif
typedef unsigned short HFILE;
typedef unsigned char UCHAR;

typedef struct {
    unsigned short codeTerminate;
    unsigned short codeResult;
} RESULTCODES;

typedef struct {
    unsigned char hours;
    unsigned char minutes;
    unsigned char seconds;
    unsigned char hundredths;
    unsigned char day;
    unsigned char month;
    unsigned short year;
    short timezone;
    unsigned char weekday;
} DATETIME;

#define OPEN_ACTION_FAIL_IF_NEW 0x0000
#define OPEN_ACTION_OPEN_IF_EXISTS 0x0001
#define OPEN_ACTION_CREATE_IF_NEW 0x0010
#define OPEN_ACTION_REPLACE_IF_EXISTS 0x0002
#define OPEN_SHARE_DENYWRITE 0x0020
#define OPEN_SHARE_DENYNONE 0x0040
#define OPEN_ACCESS_READONLY 0x0000
#define OPEN_ACCESS_WRITEONLY 0x0001
#define OPEN_ACCESS_READWRITE 0x0002
#define OPEN_FLAGS_DASD 0x8000U
#define EXEC_SYNC 0x0000

void APIENTRY DosHugeIncr(void);
void APIENTRY DosHugeShift(void);
void APIENTRY DosExit(int a, int b);

/* these short/long should be changed to "int" -
   that's exactly what it's meant for - to
   naturally change */
#ifdef __16BIT__
USHORT APIENTRY DosOpen(char *fnm, USHORT *handle, USHORT *action1,
               ULONG newsize, USHORT fileattr, USHORT action2,
               USHORT mode, ULONG resvd);
USHORT APIENTRY DosRead(USHORT hfile, void *ptr,
                        USHORT toread, USHORT *tempRead);
USHORT APIENTRY DosWrite(USHORT hfile, void *ptr,
                         USHORT towrite, USHORT *tempWritten);
USHORT APIENTRY DosDelete(char *name, USHORT junk);
USHORT APIENTRY DosMove(char *a, char *b, USHORT junk);
USHORT APIENTRY DosChgFilePtr(USHORT hfile, LONG newpos,
                              int dir, ULONG *retpos);
#else
ULONG APIENTRY DosOpen(char *fnm, ULONG *handle, ULONG *action1,
               ULONG newsize, ULONG fileattr, ULONG action2,
               ULONG mode, ULONG resvd);
ULONG APIENTRY DosRead(ULONG hfile, void *ptr,
                        ULONG toread, ULONG *tempRead);
ULONG APIENTRY DosWrite(ULONG hfile, void *ptr,
                         ULONG towrite, ULONG *tempWritten);
ULONG APIENTRY DosDelete(char *name);
ULONG APIENTRY DosMove(char *a, char *b);
ULONG APIENTRY DosSetFilePtr(ULONG hfile, LONG newpos,
                              int dir, ULONG *retpos);
#endif
USHORT APIENTRY DosClose(short handle);
USHORT APIENTRY DosGetEnv(USHORT *seg, USHORT *offs);
USHORT APIENTRY DosExecPgm(char *err_obj, USHORT sz, USHORT flags,
                           char *string, void *junk1, RESULTCODES *results,
                           char *string2);

#ifdef INCL_DOSMEMMGR
USHORT APIENTRY DosAllocHuge(USHORT numsegs, USHORT numbytes, USHORT *sel,
                    USHORT junk1, USHORT junk2);
#endif

USHORT APIENTRY DosGetDateTime(DATETIME *dt);

#ifdef __16BIT__
void APIENTRY DosFreeSeg(USHORT seg);
USHORT APIENTRY DosAllocSeg(USHORT bytes, USHORT *seg, USHORT flags);
USHORT APIENTRY DosCreateCSAlias(USHORT dseg, USHORT *cseg);
#else
ULONG APIENTRY DosAllocMem(void *base, ULONG size, ULONG flags);
void APIENTRY DosFreeMem(void *base);
ULONG APIENTRY DosScanEnv(void *name, void *result);
ULONG APIENTRY DosSetRelMaxFH(LONG *req, ULONG *max);
#endif

/* this is not a family API function, but it does
   exist in OS/2 1.x, so you could for example do:
   DosSetMaxFH(FOPEN_MAX + 10); */
USHORT APIENTRY DosSetMaxFH(USHORT nfiles);

#ifdef INCL_DOS
#ifdef __16BIT__
USHORT APIENTRY DosDevIOCtl(void *data, void *parm,
                            USHORT function, USHORT category,
                            USHORT handle);
#else
ULONG APIENTRY DosDevIOCtl(ULONG handle,
                           ULONG category,
                           ULONG function,
                           void *parmptr,
                           ULONG parmmax,
                           ULONG *parmlen,
                           void *dataptr,
                           ULONG datamax,
                           ULONG *datalen);
#endif
#endif

#ifdef INCL_KBD
typedef struct {
  UCHAR chChar;
  UCHAR chScan;
  UCHAR fbStatus;
  UCHAR bNlsShift;
  USHORT fsState;
  ULONG time;
} KBDKEYINFO;
#endif
