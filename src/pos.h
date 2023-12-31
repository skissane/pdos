/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pos - some low-level dos functions                               */
/*                                                                   */
/*  In my opinion, all the MSDOS functions should have had both a    */
/*  low-level assember interrupt definition, and a functional        */
/*  interface for higher level languages such as C.  It is important */
/*  to have both interfaces as it allows anyone creating a new       */
/*  language to implement the functions themselves.  It is also a    */
/*  good idea to make the API start with a specific mneumonic like   */
/*  the Dos* and Win* etc functions you see in the OS/2 API.  It is  */
/*  not a good idea to pretend to be performing data abstraction by  */
/*  typedefing long as LONG etc.  Hence, the following is what I     */
/*  think should have been done to formalise the HLL Dos API.        */
/*                                                                   */
/*  Obviously it's a bit late to be standardizing this now, and not  */
/*  only that, but it's not much of a standard when I'm the only     */
/*  user of it either!  However, this interface is first and         */
/*  foremost to provide myself with a clearcut API so that I can     */
/*  make it 32-bit at the drop of a hat.                             */
/*                                                                   */
/*********************************************************************/

#ifndef POS_INCLUDED
#define POS_INCLUDED

#include <stddef.h>

/*Standard Error Codes for PDOS*/
/*When adding a new error code here, also add it to PosGetErrorMessageString()
 *in pdos.c
 */
#define POS_ERR_NO_ERROR 0x00
#define POS_ERR_FUNCTION_NUMBER_INVALID 0x01
#define POS_ERR_FILE_NOT_FOUND 0x02
#define POS_ERR_PATH_NOT_FOUND 0x03
#define POS_ERR_MANY_OPEN_FILES 0x04
#define POS_ERR_ACCESS_DENIED 0x05
#define POS_ERR_INVALID_HANDLE 0x06
#define POS_ERR_MEM_CONTROL_BLOCK_DESTROYED 0x07
#define POS_ERR_INSUFFICIENT_MEMORY 0x08
#define POS_ERR_MEMORY_BLOCK_ADDRESS_INVALID 0x09
#define POS_ERR_ENVIRONMENT_INVALID 0x0A
#define POS_ERR_FORMAT_INVALID 0x0B
#define POS_ERR_ACCESS_CODE_INVALID 0x0C
#define POS_ERR_DATA_INVALID 0x0D
#define POS_ERR_RESERVED 0x0E
#define POS_ERR_FIXUP_OVERFLOW 0x0E
#define POS_ERR_INVALID_DRIVE 0x0F
#define POS_ERR_ATTEMPTED_TO_REMOVE_CURRENT_DIRECTORY 0x10
#define POS_ERR_NOT_SAME_DEVICE 0x11
#define POS_ERR_NO_MORE_FILES 0x12
/**/

#define POS_ERR_FILE_EXISTS 0x50

/* for use by PosAllocMem callers */
#define POS_LOC_MASK 0x300
#define POS_LOC20 0x100
#define POS_LOC32 0x200
#define POS_LOC64 0x300

/* File attribute bits */
#define FILE_ATTR_READONLY 0x01
#define FILE_ATTR_HIDDEN 0x02
#define FILE_ATTR_SYSTEM 0x04
#define FILE_ATTR_LABEL 0x08
#define FILE_ATTR_DIRECTORY 0x10
#define FILE_ATTR_ARCHIVE 0x20

/* Video subsystem information */
typedef struct pos_video_info {
    unsigned int mode;
    unsigned int page;
    unsigned int rows;
    unsigned int cols;
    unsigned int cursorStart;
    unsigned int cursorEnd;
    unsigned int row;
    unsigned int col;
    unsigned int currentAttrib;
} pos_video_info;

/* Input buffer used for INT 21,A */
typedef struct pos_input_buffer {
    unsigned char maxChars; /* Maximum number of chars buffer can hold */
    unsigned char actualChars; /* Number of chars actually read
                                  (not including final CR) */
    char data[1]; /* 1 is arbitrary, for C compilers that don't accept
                     incomplete arrays */
} pos_input_buffer;

/* ===== BEGINS PROCESS MANAGEMENT DATA STRUCTURES ===== */

/* Process status enumeration. */
typedef enum {
    /* Process has been loaded but not yet executed. */
    PDOS_PROCSTATUS_LOADED,
    /* Process is the current foreground process. */
    PDOS_PROCSTATUS_ACTIVE,
    /* Process is waiting for child process to finish. */
    PDOS_PROCSTATUS_CHILDWAIT,
    /* Process is a TSR */
    PDOS_PROCSTATUS_TSR,
    /* Process has voluntarily suspended itself. */
    PDOS_PROCSTATUS_SUSPENDED,
    /* Process has terminated */
    PDOS_PROCSTATUS_TERMINATED
} PDOS_PROCSTATUS;

#define PDOS_PROCESS_EXENAMESZ 13

/* PDOS_PROCINFO: data structure with info about a process */
typedef struct _PDOS_PROCINFO {
    char exeName[PDOS_PROCESS_EXENAMESZ]; /* ASCIIZ short name of executable */
    unsigned long pid; /* Process ID */
    unsigned long ppid; /* Parent Process ID; 0 if none */
    unsigned long prevPid; /* Prev PID in chain; 0 if start of chain */
    unsigned long nextPid; /* Next PID in chain; 0 if end of chain */
    PDOS_PROCSTATUS status; /* Process status */
} PDOS_PROCINFO;

/* ===== ENDING PROCESS MANAGEMENT DATA STRUCTURES ===== */

typedef struct {
#ifdef __32BIT__
    char *env;
#else
    short env;
#endif
    unsigned char *cmdtail;
    char *fcb1;
    char *fcb2;
} POSEXEC_PARMBLOCK;

/* API functions */

void PosTermNoRC(void); /* int 20h */

/* int 21h */
unsigned int PosDisplayOutput(unsigned int ch); /* func 2 */

unsigned int PosDirectConsoleOutput(unsigned int ch); /* func 6 */

unsigned int PosDirectCharInputNoEcho(void); /* func 7 */

unsigned int PosGetCharInputNoEcho(void); /* func 8 */

unsigned int PosDisplayString(const char *buf); /* func 9 */

void PosReadBufferedInput(pos_input_buffer *buf); /* func A */

unsigned int PosSelectDisk(unsigned int drive); /* func e */

unsigned int PosGetDefaultDrive(void); /* func 19 */

void PosSetDTA(void *dta); /* func 1a */

void PosSetInterruptVector(unsigned int intnum, void *handler); /* func 25 */

void PosGetSystemDate(unsigned int *year, /* func 2a */
                      unsigned int *month,
                      unsigned int *day,
                      unsigned int *dw);

unsigned int PosSetSystemDate(unsigned int year, /* func 2b */
                              unsigned int month,
                              unsigned int day);

void PosGetSystemTime(unsigned int *hour, /* func 2c */
                      unsigned int *min,
                      unsigned int *sec,
                      unsigned int *hundredths);

void PosSetVerifyFlag(int flag); /* func 2e
                               - set read-after-write verification flag */

void *PosGetDTA(void); /* func 2f */

unsigned int PosGetDosVersion(void); /* func 30 */

void PosTerminateAndStayResident(int exitCode, int paragraphs); /* func 31 */

int PosGetBreakFlag(void); /* func 33, subfunc 00
                              - get Ctrl+Break checking flag status */

void PosSetBreakFlag(int flag); /* func 33, subfunc 01
                              - set Ctrl+Break checking flag status */

int PosGetBootDrive(void); /* func 33, subfunc 05
                              - get boot drive (1=A,2=B,etc.) */

void *PosGetInterruptVector(int intnum); /* func 35 */

void PosGetFreeSpace(int drive,
                     unsigned int *secperclu,
                     unsigned int *numfreeclu,
                     unsigned int *bytpersec,
                     unsigned int *totclus); /* func 36 */

int PosMakeDir(const char *dname); /* func 39 */

int PosRemoveDir(const char *dname); /* func 3a */

int PosChangeDir(const char *to); /* func 3b */

int PosCreatFile(const char *name, /* func 3c */
                 int attrib,
                 int *handle);

int PosOpenFile(const char *name, /* func 3d */
                int mode,
                int *handle);

int PosCloseFile(int handle); /* func 3e */

int PosReadFile(int fh, /* func 3f */
                void *data,
                unsigned int bytes,
                unsigned int *readbytes);

int PosWriteFile(int handle, /* func 40 */
                 const void *data,
                 unsigned int len,
                 unsigned int *writtenbytes);

int PosDeleteFile(const char *fname); /* func 41 */

int PosMoveFilePointer(int handle, /* func 42 */
                       long offset,
                       int whence,
                       long *newpos);

int PosGetFileAttributes(const char *fnm,int *attr);/*func 43*/

int PosSetFileAttributes(const char *fnm,int attr);/*func 43 subfunc 01*/

int PosGetDeviceInformation(int handle, unsigned int *devinfo);
      /* func 44 subfunc 0 */

int PosSetDeviceInformation(int handle, unsigned int devinfo);
      /* func 44 subfunc 1 */

int PosBlockDeviceRemovable(int drive); /* func 44 subfunc 8 */

int PosBlockDeviceRemote(int drive,int *da); /* func 44 subfunc 9 */

int PosGenericBlockDeviceRequest(int drive,
                                 int catcode,
                                 int function,
                                 void *parmblock); /*func 44 subfunc 0D*/


int PosDuplicateFileHandle(int fh); /* func 45 */

int PosForceDuplicateFileHandle(int fh, int newfh); /* func 46 */

int PosGetCurDir(int drive, char *dir); /* func 47 */

/* func 48 alternate entry really for 16-bit */
void *PosAllocMemPages(unsigned int pages, unsigned int *maxpages);

int PosFreeMem(void *ptr); /* func 49 */

/* func 4a */
int PosReallocPages(void *ptr, unsigned int newpages, unsigned int *maxp);

int PosExec(char *prog, POSEXEC_PARMBLOCK *parmblock); /* func 4b */

void PosTerminate(int rc); /* func 4c */

int PosGetReturnCode(void); /* func 4d */

int PosFindFirst(char *pat, int attrib); /* func 4e */

int PosFindNext(void); /* func 4f */

int PosGetCurrentProcessId(void); /* func 51 */

/* func 54 - get read-after-write verification flag */
int PosGetVerifyFlag(void);

int PosRenameFile(const char *old, const char *new); /* func 56 */

int PosGetFileLastWrittenDateAndTime(int handle,
                                     unsigned int *fdate,
                                     unsigned int *ftime); /*func 57*/

int PosSetFileLastWrittenDateAndTime(int handle,
                                     unsigned int fdate,
                                     unsigned int ftime);/*func 57 subfunc 1*/

int PosCreatNewFile(const char *name, int attrib, int *handle); /*func 5b*/

int PosTruename(char *prename,char *postname); /*func 60*/

void PosAExec(char *prog, POSEXEC_PARMBLOCK *parmblock); /* func 80 */

/* The following functions are extensions... */

void PosDisplayInteger(int x); /* func f6.00 */

void PosReboot(void); /* func f6.01 */

int PosRehash(int c); /* func f6.02 */

void PosSetDosVersion(unsigned int version); /* func f6.03 */

int PosGetLogUnimplemented(void); /* func f6.04 */

void PosSetLogUnimplemented(int flag); /* func f6.05 */

/* So programs can reliably determine if they are running under PDOS-16 or
 * some other implementation of PDOS API, such as FreeDOS, MS-DOS, PC-DOS,
 * DR-DOS, DOSBox, etc. INT 21,AX=F606 will return AX=1234 under PDOS, but not
 * under these other operating systems.
 */
#define PDOS_MAGIC 0x1234

int PosGetMagic(void); /* func f6.06 */

void PosGetMemoryManagementStats(void *stats); /* func f6.07 */

void *PosAllocMem(unsigned int size, unsigned int flags); /* func f6.08 */

/* Given an error code return corresponding message */
char *PosGetErrorMessageString(unsigned int errorCode); /* func f6.09 */

void PosPowerOff(void); /* func f6.0a */

/* Func F6.0C - Get info about a process.
 * pid = PID to get info on (0=get info on init process)
 * infoSz should be sizeof(PDOS_PROCINFO). It is passed for future-proofing
 * (future versions might make the structure bigger, we know whether client
 * wants old or new version based on the passed size)
 */
int PosProcessGetInfo(unsigned long pid,
                      PDOS_PROCINFO *info,
                      unsigned int infoSz);

/* Func F6.0D - Get memory usage stats for given process
 * pid=0 for current process
 */
void PosProcessGetMemoryStats(unsigned long pid, void *stats);

void PosClearScreen(void); /* func f6.30 */

void PosMoveCursor(int row, int col); /* func f6.31 */

int PosGetVideoInfo(pos_video_info *info, unsigned int size); /* func f6.32 */

int PosKeyboardHit(void); /* func f6.33 */

/* F6,34 - Yield the CPU. Currently calls APM BIOS to reduce CPU usage.
 * One day it might be used for multi-tasking.
 */
void PosYield(void);

/* F6,35 - Sleep for given number of seconds */
void PosSleep(unsigned long seconds);

/* F6,36 - Get tick count */
unsigned long PosGetClockTickCount(void);

/* F6,37 - Set Video Attribute */
void PosSetVideoAttribute(unsigned int attribute);

/* F6,38 - Set Video Mode */
int PosSetVideoMode(unsigned int mode);

/* F6,39 - Set Video Page */
int PosSetVideoPage(unsigned int page);

/* F6,3A - Set Environment Variable
 * To unset a variable, pass NULL for the value. The name and value string
 * will be copied into the environment block, so are not required after
 * this call.
 */
int PosSetEnv(char *name, char *value);

/* F6,3B - Get Environment Block
 * Returns the environment block for the current process.
 */
void * PosGetEnvBlock(void);

/* F6,3C - Set Named Font */
int PosSetNamedFont(char *fontName);

/* F6,3D - Allocate Virtual Memory */
void *PosVirtualAlloc(void *addr, size_t size);

/* F6,3E - Free Virtual Memory */
void PosVirtualFree(void *addr, size_t size);

/* F6,3F - Get Command Line String For The Current Process */
char *PosGetCommandLine(void);

/* F6,40 - Read Byte From Port */
unsigned char PosInp(unsigned int port);

/* F6,41 - Write Byte To Port */
void PosOutp(unsigned int port, unsigned char data);

/* F6,42 - Absolute Drive Read */
unsigned int PosAbsoluteDriveRead(int drive,unsigned long start_sector,
                                  unsigned int sectors,void *buf);
/* F6,43 - Absolute Drive Write */
unsigned int PosAbsoluteDriveWrite(int drive,unsigned long start_sector,
                                   unsigned int sectors,void *buf);

/* F6,44 - Boot a Disk */
unsigned int PosDoBoot(int disknum);

/* F6,45 - start or stop screen capture */
unsigned int PosScrncap(int disknum);

/* F6,46 - GetStdHandle */
void *PosGetStdHandle(unsigned int nStdHandle);

/* F6,47 - SetStdHandle */
unsigned int PosSetStdHandle(unsigned int nStdHandle, void *hHandle);

/* F6,48 - start system monitor */
unsigned int PosMonitor(void);

/* F6,49 - show return codes */
unsigned int PosShowret(int flag);

/* F6,4A - map keyboard */
unsigned int PosKeyboardMap(unsigned char *newmap);

/* F6,4B - map screen */
unsigned int PosScreenMap(unsigned char *newmap);

/* F6,4C - map scancode */
unsigned int PosScancodeMap(unsigned char *newmap, int type);

/* F6,4D - load fonts */
unsigned int PosLoadFonts(int start,
                          int count,
                          int depth,
                          void *table,
                          int block);

/* F6,4E - map accent */
unsigned int PosAccentMap(int accent_key, unsigned char *newmap);

/* F6,4F - Internal system call for DLLs */
unsigned int PosWinSyscall(unsigned int function_index, void *arguments);

unsigned int PosAbsoluteDiskRead(int drive, unsigned long start_sector,
                                 unsigned int sectors,void *buf); /*INT25 */

unsigned int PosAbsoluteDiskWrite(int drive, unsigned long start_sector,
                                  unsigned int sectors,void *buf); /*INT26 */


/*Structure for BPB*/
typedef struct {
    unsigned char BytesPerSector[2];
    unsigned char SectorsPerClustor;
    unsigned char ReservedSectors[2];
    unsigned char FatCount;
    unsigned char RootEntries[2];
    unsigned char TotalSectors16[2];
    unsigned char MediaType;
    unsigned char FatSize16[2];
    unsigned char SectorsPerTrack[2];
    unsigned char Heads[2];
    unsigned char HiddenSectors_Low[2];
    unsigned char HiddenSectors_High[2];
    unsigned char TotalSectors32[4];
} BPB;
/**/

/*Structure  for function call 440D*/
typedef struct {
    unsigned char special_function;
    unsigned char reservedspace[6];
    BPB bpb;
    unsigned char reserve2[100]; /* for OSes with bigger BPBs */
}PB_1560;
/**/

/*Structure for int25/26*/
typedef struct{
    unsigned long sectornumber;
    unsigned int numberofsectors;
    void *transferaddress;
}DP;
/**/

/*Structure for tempDTA in case of multiple PosFindFirst Calls*/
typedef struct {
    int handle;
    char pat[20];
}FFBLK;
/**/

/*Structure to define variables used in DTA*/
typedef struct {
    char attr;             /*attribute of the search(0x00)*/
    char drive;            /*drive used in search(0x01)*/
    char search[11];       /*search name used(0x02)*/
    int direntno;          /*directory entry number(0x0D)*/
    int startcluster;      /*starting cluster number for
                             current directory zero for
                             root directory(0x0F)*/
    int reserved;          /*reserved(0x11)*/
    int startcluster2;     /*starting cluster number for
                             current directory zero for
                             root directory(0x13)*/
    unsigned char attrib;  /*attribute of the matching file
                             (0x15)*/
    int file_time;         /*file time(0x16)*/
    int file_date;         /*file date(0x18)*/
    long file_size;        /*file size(0x1A)*/
    char file_name[13];    /*ASCIIZ file name and extension in
                             form NAME.EXT with blanks stripped
                             0x1E)*/
    unsigned char lfn[256]; /*Stores LFN (255 characters max.) and
                             *null terminator.*/
                             /*+++Add support for UCS-2 and find
                              *better place for LFN provided to DIR.*/

}DTA;
/**/


#if defined(__PDOSGEN__)
#include <__os.h>

#define PosGetDTA __os->PosGetDTA
#define PosFindFirst __os->PosFindFirst
#define PosFindNext __os->PosFindNext
#define PosChangeDir __os->PosChangeDir
#define PosMakeDir __os->PosMakeDir
#define PosRemoveDir __os->PosRemoveDir
#endif


#endif /* POS_INCLUDED */
