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

#define APIENTRY _pascal _far

typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef long LONG;
typedef char *PSZ;
typedef void VOID;
typedef void *PVOID;
typedef struct {
    unsigned short codeResult;
} RESULTCODES; /* guess */
typedef struct {
    unsigned short year;
    unsigned short month;
    unsigned short day;
    unsigned short hours;
    unsigned short minutes;
    unsigned short seconds;
} DATETIME; /* guess */

/* guess all these */
#define OPEN_ACTION_FAIL_IF_NEW 1
#define OPEN_ACTION_OPEN_IF_EXISTS 2
#define OPEN_ACTION_CREATE_IF_NEW 4
#define OPEN_ACTION_REPLACE_IF_EXISTS 8
#define OPEN_SHARE_DENYWRITE 16
#define OPEN_ACCESS_READONLY 32
#define OPEN_ACCESS_WRITEONLY 64
#define OPEN_ACCESS_READWRITE 128
#define OPEN_FLAGS_DASD 256
#define EXEC_SYNC 512

void * APIENTRY DosHugeIncr(void);
void * APIENTRY DosHugeShift(void);
void APIENTRY DosExit(int a, int b);
USHORT APIENTRY DosOpen(char *fnm, USHORT *handle, USHORT *action1,
               USHORT newsize, USHORT fileattr, USHORT action2,
               USHORT mode, void *junk);
USHORT APIENTRY DosClose(short handle);
USHORT APIENTRY DosRead(USHORT hfile, void *ptr, USHORT toread, USHORT *tempRead);
USHORT APIENTRY DosWrite(USHORT hfile, void *ptr, USHORT towrite, USHORT *tempWritten);
USHORT APIENTRY DosDelete(char *name, USHORT junk);
USHORT APIENTRY DosMove(char *a, char *b, USHORT junk);
USHORT APIENTRY DosGetEnv(USHORT *seg, USHORT *offs);
USHORT APIENTRY DosExecPgm(char *err_obj, USHORT sz, USHORT flags,
                  char *string, void *junk1, USHORT *results, char *string2);
void APIENTRY DosFreeSeg(USHORT seg);
USHORT APIENTRY DosAllocHuge(USHORT numsegs, USHORT numbytes, USHORT *sel,
                    USHORT junk1, USHORT junk2);
USHORT APIENTRY DosChgFilePtr(USHORT hfile, LONG newpos, int dir, ULONG *retpos);
