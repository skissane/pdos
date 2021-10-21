/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pdos - public domain operating system                            */
/*                                                                   */
/*  This is the main entry point of the operating system proper.     */
/*  There is some startup code that is called before this though,    */
/*  but after that main is called like any other pos format a.out.   */
/*                                                                   */
/*********************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "pdos.h"
#include "protint.h"
#include "pos.h"
#include "bos.h"
#include "fat.h"
#include "support.h"
#include "lldos.h"
#include "memmgr.h"
#include "patmat.h"
#include "unused.h"
#include "dow.h"
#include "process.h"
#include "int21.h"
#include "log.h"
#include "helper.h"

#ifdef __32BIT__
#include "int80.h"
#ifdef NOVM
#define PAGE_SIZE 0x1000
void *kmalloc(size_t size);
void kfree(void *ptr);
#define vmmInit(a, b)
#define vmmFree(a, b, c) kfree(b)
#define vmmAlloc(a, b, c) kmalloc(c)
#define vmmTerm(a)
#define vmmCopyCurrentMapping(a, b, c)
#define vmmSupply(a, b, c)
#define vmmAllocPages(a, num_pages) kmalloc(num_pages * PAGE_SIZE)
#define vmmFreePages(a, addr, num_pages) kfree(addr)
#define loadPageDirectory(a)
#else
#include "physmem.h"
#include "vmm.h"
#include "liballoc.h"
#endif

/* Address space layout.
 * Kernel space contents are the same for every process. */
#define KERNEL_SPACE_START  0xC0000000
#define KERNEL_SPACE_SIZE   0x30000000
#define KERNEL_HEAP_ADDR    0xE0000000
#define KERNEL_HEAP_SIZE    0x10000000
/* After 0xF0000000 are paging structures etc. */
/* Kernel still uses some memory under 4 MiB. */
#define PROCESS_SPACE_START 0x400000
#define PROCESS_SPACE_SIZE  KERNEL_SPACE_START - PROCESS_SPACE_START

#define STACKSZ32 0x40000 /* stack size for 32-bit version */
#endif

#define MAX_PATH 260 /* With trailing '\0' included. */
#define DEFAULT_DOS_VERSION 0x0004 /* 0004 = DOS 4.0, 2206 = DOS 6.22, etc. */
#define NUM_SPECIAL_FILES 5
    /* stdin, stdout, stderr, stdaux, stdprn */

typedef struct {
    FAT fat;
    BPB bpb;
    char cwd[MAX_PATH];
    int accessed;
    unsigned long sectors_per_cylinder;
    unsigned int num_heads;
    unsigned int sectors_per_track;
    unsigned long hidden;
    int drive;
    int lba;
    int valid;
} DISKINFO;

void pdosRun(void);
static void initdisks(void);
static void scanPartition(int drive);
static void processPartition(int drive, unsigned char *prm);
static void processExtended(int drive, unsigned char *prm);

static void initfiles(void);

static void make_ff(char *pat);
static void scrunchf(char *dest, char *new);
static int ff_search(void);

#ifdef __32BIT__
int int0(unsigned int *regs);
int int0E(unsigned int *regs);
int int20(unsigned int *regs);
/* INT 25 - Absolute Disk Read */
int int25(unsigned int *regs);
/* INT 26 - Absolute Disk Write */
int int26(unsigned int *regs);
/* IRQ 0 - Timer Interrupt. */
int intB0(unsigned int *regs);
/**/
#endif
static void loadConfig(void);
static void loadPcomm(void);
static void loadExe(char *prog, POSEXEC_PARMBLOCK *parmblock);
#ifdef __32BIT__
#define ASYNCHRONOUS 0
#define SYNCHRONOUS  1
static void loadExe32(char *prog, POSEXEC_PARMBLOCK *parmblock, int synchronous);
static void startExe32(void);
static void terminateExe32(void);
static int fixexe32(unsigned long entry, unsigned int sp,
                    unsigned long exeStart, unsigned long dataStart);
#endif
static int fileCreat(const char *fnm, int attrib, int *handle);
static int dirCreat(const char *dnm, int attrib);
static int newFileCreat(const char *fnm, int attrib, int *handle);
static int fileOpen(const char *fnm, int *handle);
static int fileWrite(int fno, const void *buf, size_t szbuf,
                     size_t *writtenbytes);
static int fileDelete(const char *fnm);
static int dirDelete(const char *dnm);
static int fileSeek(int fno, long offset, int whence, long *newpos);
static int opencomm(int num, int *handle);
static int fileClose(int fno);
static int fileRead(int fno, void *buf, size_t szbuf, size_t *readbytes);
static void accessDisk(int drive);
void dumplong(unsigned long x);
void dumpbuf(unsigned char *buf, int len);
static void readLogical(void *diskptr, unsigned long sector, void *buf);
static void writeLogical(void *diskptr, unsigned long sector, void *buf);
static int readAbs(void *buf,
                int sectors,
                int drive,
                int track,
                int head,
                int sect);
static int readLBA(void *buf,
                int sectors,
                int drive,
                unsigned long sector);
static int writeAbs(void *buf,
                    int sectors,
                    int drive,
                    int track,
                    int head,
                    int sect);
static int writeLBA(void *buf,
                    int sectors,
                    int drive,
                    unsigned long sector);
static void analyseBpb(DISKINFO *diskinfo, unsigned char *bpb);
int pdosstrt(void);
static int formatcwd(const char *input,char *output);
static void pdosWriteText(int ch);
static size_t envBlockSize(char *envptr);
static char *envBlockTail(char *envptr);
static char *envCopy(char *previous, char *progName);
static char *envAllocateEmpty(char *progName);
static char *envModify(char *envPtr, char *name, char *value);
static void getDateTime(FAT_DATETIME *ptr);
static int isDriveValid(int drive);

static MEMMGR memmgr;
#ifdef __32BIT__
static MEMMGR btlmem;
#endif

/* we implement special versions of allocate and free */
#ifndef __32BIT__
#define PDOS16_MEMSTART 0x3000
#define memmgrAllocate(m,b,i) pdos16MemmgrAllocate(m,b,i)
#define memmgrFree(m,p) pdos16MemmgrFree(m,p)
#define memmgrSetOwner(m,p,o) pdos16MemmgrSetOwner(m,p,o)
static void pdos16MemmgrFree(MEMMGR *memmgr, void *ptr);
static void *pdos16MemmgrAllocate(MEMMGR *memmgr, size_t bytes, int id);
static void *pdos16MemmgrAllocPages(MEMMGR *memmgr, size_t pages, int id);
static void pdos16MemmgrFreePages(MEMMGR *memmgr, size_t page);
static int pdos16MemmgrReallocPages(MEMMGR *memmgr,
                                    void *ptr,
                                    size_t newpages);
static void pdos16MemmgrSetOwner(MEMMGR *memmgr, void *ptr,
                                 unsigned long owner);
#endif

#define MAXDISKS 20
static DISKINFO disks[MAXDISKS];
static DISKINFO bootinfo;
static unsigned int currentDrive;
static unsigned int tempDrive;
static unsigned int bootDrivePhysical;
static unsigned int bootDriveLogical;
static unsigned int lastDrive;
static unsigned char *bootBPB;
static char *cwd;
static int lastrc;
static int lba;
static int memId = 0; /* give each program a unique ID for their memory */
static unsigned long psector; /* partition sector offset */
static int attr;
/* note that on EBCDIC, the alphabet isn't contiguous */
static char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define MAXFILES 40
static struct {
    FATFILE fatfile;
    FAT *fatptr;
    int inuse;
    int special;
    int comm;
} fhandle[MAXFILES];

static char ff_path[FILENAME_MAX];
static char ff_pat[FILENAME_MAX];

static char shell[100] = "";
char kernel32[] = "?:\\KERNEL32.DLL";
char msvcrt[] = "?:\\MSVCRT.DLL";

/* warn of any application errors in case application
   doesn't adequately report them. */
static int warnerror = 0;

static int ff_handle;
static DTA origdta;
static DTA *dta =&origdta;

/* DOS version reported to programs.
 * Can be changed via PosSetDosVersion() call. */
static unsigned int reportedDosVersion = DEFAULT_DOS_VERSION;

/* Extended Control+Break checking flag.
 * NOTE: We track flag value but actually changing Ctrl+Break checking
 * behaviour based on flag value isn't currently implemented.
 */
static int breakFlag = 0;

/* Verify after write flag.
 * NOTE: We track flag value but actually doing the verification isn't done
 * by PDOS API itself. But PCOMM COPY command will respond to flag.
 * Implementing VERIFY inside the command interpreter COPY command(s) is
 * basically how Windows NT handles it, see here:
 * https://blogs.msdn.microsoft.com/oldnewthing/20160121-00/?p=92911
 * (MS-DOS instead implements it inside the device drivers, but PDOS
 * doesn't really have such a thing yet.)
 */
static int verifyFlag = 0;

/* If > 0, program is a TSR, don't deallocate its memory on exit. */
static int tsrFlag;

/* Tick count at boot time. Can be used to compute system uptime. */
static unsigned long bootedAt;

/* Current color to use */
static unsigned int currentAttrib = 7;

/* Current video page */
static unsigned int currentPage = 0;

#if defined(__32BIT__) && defined(VGADRV)
#include "vgatext.h"

static VGATEXT vgatext;
#endif

#ifdef __32BIT__
unsigned long __userparm;
static void *gdt;
static char *transferbuf;
static long freem_start; /* start of free memory below 1 MiB,
                            absolute address */
static unsigned long doreboot;
static unsigned long dopoweroff;
unsigned char *loadaddr = NULL;
unsigned long entry_point;
#endif

#ifndef __32BIT__
static unsigned char *loadaddr = NULL;
static unsigned char *entry_point = NULL;
#endif

#ifdef __32BIT__
#ifdef NOVM
static MEMMGR physmemmgr;
#else
static PHYSMEMMGR physmemmgr;
#endif
/* Once mutitasking is running all accesses
 * to kernel_vmm should be protected by disabling interrupts. */
static VMM kernel_vmm;

typedef struct _TCB {
    void *stack_pointer;
    void *call32_esp;
    void *stack_bottom;
    int state;
    PDOS_PROCESS *pcb;
    struct _TCB *waitingParentTCB;
    struct _TCB *next;
} TCB;

#define TCB_STACK_SIZE 0x1000

#define TCB_STATE_READY      0
#define TCB_STATE_RUNNING    1
#define TCB_STATE_PAUSED     2
#define TCB_STATE_TERMINATED 3
#define TCB_STATE_CHILDWAIT  4

static TCB *curTCB = NULL;
static TCB *firstReadyTCB = NULL;
static TCB *lastReadyTCB = NULL;
static TCB *terminatedTCB = NULL;
static TCB *cleanerTCB = NULL;

static void initThreading(void);
static void startThread(void);
static TCB *createThread(void (*entry)(void), PDOS_PROCESS *pcb);
static void createThreadAndBlock(void (*entry)(void), PDOS_PROCESS *pcb);
static void schedule(void);
static void blockThread(int reason);
static void unblockThread(TCB *blockedTCB);
static void terminateThread(void);

static void cleanTerminatedThreads(void);

static void lockMutex(volatile int *mutex);
static void unlockMutex(volatile int *mutex);

static volatile int fatMutex = 0;

static void initThreading(void)
{
    /* Creates TCB for the initial kernel thread. */
    curTCB = kmalloc(sizeof(TCB));
    if (curTCB == NULL)
    {
        printf("Failed to allocate memory for initial TCB\n");
        printf("System halting\n");
        for (;;);
    }
    memset(curTCB, 0, sizeof(TCB));
    curTCB->state = TCB_STATE_RUNNING;

    createThread(cleanTerminatedThreads, NO_PROCESS);
    cleanerTCB = firstReadyTCB;
    {
        /* Timer interrupt handler is installed
         * after the rest of multitasking system is ready. */
        unsigned int savedEFLAGS = getEFLAGSAndDisable();
        protintHandler(0xB0, intB0);
        setEFLAGS(savedEFLAGS);
    }
}

static void startThread(void)
{
    /* This function is executed first in every thread. */
    /* Turns on interrupts as they were disabled for the switch
     * and EFLAGS were not saved, because this is the start of a new thread. */
    enable();
}

static TCB *createThread(void (*entry)(void), PDOS_PROCESS *pcb)
{
    TCB *newTCB = kmalloc(sizeof(TCB));
    unsigned int *new_stack;
    int i;

    if (newTCB == NULL)
    {
        printf("Failed to allocate memory for new TCB\n");
        printf("System halting\n");
        for (;;);
    }
    memset(newTCB, 0, sizeof(TCB));

    {
        unsigned int savedEFLAGS = getEFLAGSAndDisable();

        new_stack = vmmAlloc(&kernel_vmm, 0, TCB_STACK_SIZE);

        setEFLAGS(savedEFLAGS);
    }
    if (new_stack == NULL)
    {
        printf("Failed to allocate memory for new stack\n");
        printf("System halting\n");
        for (;;);
    }
    newTCB->stack_bottom = new_stack;
    new_stack = (void *)(((char *)new_stack) + TCB_STACK_SIZE);
    /* Puts values on the new stack. */
    /* Entry point. */
    new_stack--;
    *new_stack = (unsigned int)entry;
    /* startThread which sets up thread for running. */
    new_stack--;
    *new_stack = (unsigned int)startThread;
    /* EFLAGS, EBP, EDI, ESI, EDX, ECX, EBX, EAX. */
    for (i = 0; i < 8; i++)
    {
        new_stack--;
        *new_stack = 0;
    }

    newTCB->stack_pointer = new_stack;
    newTCB->state = TCB_STATE_READY;
    newTCB->pcb = pcb;
    {
        unsigned int savedEFLAGS = getEFLAGSAndDisable();

        if (lastReadyTCB)
        {
            lastReadyTCB->next = newTCB;
            lastReadyTCB = newTCB;
        }
        else
        {
            firstReadyTCB = newTCB;
            lastReadyTCB = newTCB;
        }

        setEFLAGS(savedEFLAGS);
    }

    return (newTCB);
}

static void createThreadAndBlock(void (*entry)(void), PDOS_PROCESS *pcb)
{
    unsigned int savedEFLAGS = getEFLAGSAndDisable();
    TCB *newTCB = createThread(entry, pcb);

    newTCB->waitingParentTCB = curTCB;
    blockThread(TCB_STATE_CHILDWAIT);
    setEFLAGS(savedEFLAGS);
}

static void schedule(void)
{
    unsigned int savedEFLAGS = getEFLAGSAndDisable();

    if (firstReadyTCB)
    {
        TCB *oldTCB;

        oldTCB = curTCB;
        curTCB = firstReadyTCB;

        if (oldTCB->state == TCB_STATE_RUNNING)
        {
            oldTCB->state = TCB_STATE_READY;
            if (firstReadyTCB->next)
            {
                firstReadyTCB = firstReadyTCB->next;
                lastReadyTCB->next = oldTCB;
                lastReadyTCB = oldTCB;
            }
            else
            {
                firstReadyTCB = oldTCB;
                lastReadyTCB = oldTCB;
            }
            lastReadyTCB->next = NULL;
        }
        else
        {
            if (firstReadyTCB->next)
            {
                firstReadyTCB = firstReadyTCB->next;
            }
            else
            {
                firstReadyTCB = NULL;
                lastReadyTCB = NULL;
            }
        }

        curTCB->state = TCB_STATE_RUNNING;
        if (oldTCB->pcb != curTCB->pcb)
        {
            curPCB = curTCB->pcb;
            /* Address space must be switched,
             * so kernel mappings are copied
             * and new page directory is loaded. */
            if (curPCB == NO_PROCESS)
            {
                vmmCopyCurrentMapping(&kernel_vmm,
                                      (void *)KERNEL_SPACE_START,
                                      KERNEL_SPACE_SIZE);
                loadPageDirectory(kernel_vmm.pd_physaddr);
            }
            else
            {
                vmmCopyCurrentMapping(curPCB->vmm,
                                      (void *)KERNEL_SPACE_START,
                                      KERNEL_SPACE_SIZE);
                loadPageDirectory(curPCB->vmm->pd_physaddr);
            }
        }
        switchFromToThread(oldTCB, curTCB);
    }
    else
    {
        if (curTCB->state != TCB_STATE_RUNNING)
        {
            printf("No active threads\n");
            printf("System halting\n");
            for (;;);
        }
    }

    setEFLAGS(savedEFLAGS);
}

static void blockThread(int reason)
{
    curTCB->state = reason;
    schedule();
}

static void unblockThread(TCB *blockedTCB)
{
    unsigned int savedEFLAGS = getEFLAGSAndDisable();

    if (blockedTCB->state == TCB_STATE_READY)
    {
        return;
    }
    blockedTCB->state = TCB_STATE_READY;
    if (firstReadyTCB)
    {
        lastReadyTCB->next = blockedTCB;
        lastReadyTCB = blockedTCB;
    }
    else
    {
        firstReadyTCB = blockedTCB;
        lastReadyTCB = blockedTCB;
    }
    lastReadyTCB->next = NULL;

    setEFLAGS(savedEFLAGS);
}

static void terminateThread(void)
{
    disable();
    curTCB->next = terminatedTCB;
    terminatedTCB = curTCB;
    if (curTCB->waitingParentTCB)
    {
        unblockThread(curTCB->waitingParentTCB);
    }
    unblockThread(cleanerTCB);
    blockThread(TCB_STATE_TERMINATED);
}

static void cleanTerminatedThreads(void)
{
    TCB *toRemoveTCB = NULL;

    while (1)
    {
        disable();
        toRemoveTCB = terminatedTCB;
        terminatedTCB = NULL;
        enable();
        while (toRemoveTCB)
        {
            TCB *removingTCB = toRemoveTCB;

            toRemoveTCB = toRemoveTCB->next;
            disable();
            vmmFree(&kernel_vmm,
                    removingTCB->stack_bottom,
                    TCB_STACK_SIZE);
            enable();
            kfree(removingTCB);
        }
        blockThread(TCB_STATE_PAUSED);
    }
}

/* Mutex (Mutual Exclusion) prevents two threads
 * from accesing the same resource at one time
 * without needing the interrupts to be disabled. */
static void lockMutex(volatile int *mutex)
{
    unsigned int savedEFLAGS = getEFLAGSAndDisable();

    while (*mutex)
    {
        schedule();
    }
    *mutex = 1;

    setEFLAGS(savedEFLAGS);
}

static void unlockMutex(volatile int *mutex)
{
    unsigned int savedEFLAGS = getEFLAGSAndDisable();
    *mutex = 0;
    setEFLAGS(savedEFLAGS);
}

/* Liballoc hooks. */
static unsigned int liballocSavedEFLAGS;

int liballoc_lock()
{
    liballocSavedEFLAGS = getEFLAGSAndDisable();
    return (0);
}

int liballoc_unlock()
{
    setEFLAGS(liballocSavedEFLAGS);
    return (0);
}

void *liballoc_alloc(size_t num_pages)
{
    void *addr;
    unsigned int savedEFLAGS = getEFLAGSAndDisable();
    addr = vmmAllocPages(&kernel_vmm, num_pages);
    setEFLAGS(savedEFLAGS);

    return (addr);
}

int liballoc_free(void *addr, size_t num_pages)
{
    unsigned int savedEFLAGS = getEFLAGSAndDisable();
    vmmFreePages(&kernel_vmm, addr, num_pages);
    setEFLAGS(savedEFLAGS);

    return (0);
}

static void waitForKeystroke(void)
{
    while (!PosKeyboardHit())
    {
        schedule();
    }
}
#endif

#ifndef USING_EXE
int main(void)
{
#if defined(__32BIT__) && defined(VGADRV)
    vgatext_init(&vgatext);
#endif
    pdosRun();
    dumpbuf("\r\nNo command processor - system halted\r\n", 40);
    return (0);
}
#endif

void pdosRun(void)
{
#ifdef USING_EXE
    char *m;
    int mc;
#endif
#ifdef __32BIT__
    unsigned long memstart;
    unsigned long memavail;
#endif

#if (!defined(USING_EXE) && !defined(__32BIT__))
    instint();
#endif
    PosSetVideoAttribute(0x1E);
#ifdef __32BIT__
    printf("Welcome to PDOS/386 (aka PD-Windows)\n");
#else
    printf("Welcome to PDOS/86\n");
#endif
    PosSetVideoAttribute(0x7);

    /* Initialise BIOS tick count at startup. */
    bootedAt = BosGetClockTickCount();

#ifndef __32BIT__
    bootBPB = (unsigned char *)0x7c00 + 11;
#endif
    bootDrivePhysical = bootBPB[25];
    bootDriveLogical = 0;
    disks[0].accessed = 0;
    disks[1].accessed = 0;
    analyseBpb(&bootinfo, bootBPB);
    bootinfo.lba = 0;
    initdisks();
#ifdef EXE32
    bootDriveLogical = 3;
#endif
    if (bootDriveLogical == 0)
    {
        /* if we are booting from floppy, use the in-memory BPB */
        if (bootDrivePhysical < 2)
        {
            bootDriveLogical = bootDrivePhysical;
        }
        /* otherwise some unknown device, tack it on the end and use */
        else
        {
            bootDriveLogical = lastDrive;
            lastDrive++;
        }
        disks[bootDriveLogical] = bootinfo;
        fatDefaults(&disks[bootDriveLogical].fat);
        fatInit(&disks[bootDriveLogical].fat,
                bootBPB,
                readLogical,
                writeLogical,
                &disks[bootDriveLogical],
                getDateTime);
        strcpy(disks[bootDriveLogical].cwd, "");
        disks[bootDriveLogical].accessed = 1;
        disks[bootDriveLogical].valid = 1;
    }
    currentDrive = bootDriveLogical;
    cwd = disks[currentDrive].cwd;
    initfiles();
    loadConfig();
    memmgrDefaults(&memmgr);
    memmgrInit(&memmgr);
#if defined(USING_EXE)
    for (mc = 0; mc < 7; mc++)
    {
        m = malloc(64000);
        memmgrSupply(&memmgr, m, 64000);
    }
#elif defined(__32BIT__)
    /* We don't want to use the region 0x100000 to 0x110000 in case
    someone else wants to use it for 16-bit device drivers etc.
    And while we're at it, we'll skip 0x110000 to 0x200000 in case
    the A20 line is disabled, so this way they get at least 1 meg
    they can use. */
    memavail = BosExtendedMemorySize();
    if (memavail < 4*1024*1024)
    {
        printf("less than 4 MiB available - system halting\n");
        for (;;) ;
    }
#ifdef EXE32
    memavail -= 0x500000; /* room for disk cache */
    memmgrSupply(&memmgr, (void *)0x700000, memavail);
#else
    /* skip the first 2 MiB */
    memstart = 0x200000;
    memavail -= 0x200000;
#ifdef NOVM
    memmgrInit(&physmemmgr);
    memmgrSupply(&physmemmgr, (void *)memstart, memavail);
#else
    /* Provides memory to physical memory manager. */
    physmemmgrInit(&physmemmgr);
    physmemmgrSupply(&physmemmgr, memstart, memavail);
    {
        /* Sets up temporary addres space. */
        unsigned long *page_directory = physmemmgrAllocPageFrame(&physmemmgr);
        unsigned long *page_table = physmemmgrAllocPageFrame(&physmemmgr);
        unsigned long *pt_entry;
        unsigned long paddr;

        /* Marks all Page Directory entries as not present. */
        memset(page_directory, 0, PAGE_FRAME_SIZE);
        /* Identity paging for the first 4 MiB. */
        for (paddr = PAGE_RW | PAGE_PRESENT, pt_entry = page_table;
             paddr < 0x400000;
             paddr += PAGE_FRAME_SIZE, pt_entry++)
        {
            *pt_entry = paddr;
        }
        /* Puts the first Page Table into the Page Directory. */
        page_directory[0] = (((unsigned long)page_table)
                             | PAGE_RW | PAGE_PRESENT);
        /* Maps the Page Directory into itself
         * so all Page Tables can be accessed at 0xffc00000.
         * The Page Directory itself is at 0xfffff000. */
        page_directory[1023] = (((unsigned long)page_directory)
                                | PAGE_RW | PAGE_PRESENT);
        loadPageDirectory(page_directory);
        enablePaging();

        /* Sets up the kernel VMM. */
        vmmInit(&kernel_vmm, &physmemmgr);
        /* Copies only the needed address range. */
        vmmCopyCurrentMapping(&kernel_vmm,
                              (void *)KERNEL_SPACE_START,
                              KERNEL_SPACE_SIZE);
        /* Loads the new address space. */
        loadPageDirectory(kernel_vmm.pd_physaddr);
        /* Provides kernel heap address range to the VMM,
         * so kmalloc() can be used. */
        vmmBootstrap(&kernel_vmm, (void *)KERNEL_HEAP_ADDR, KERNEL_HEAP_SIZE);

        /* Frees memory used for the temporary address space mapping. */
        physmemmgrFreePageFrame(&physmemmgr, page_directory);
    }
#endif
    initThreading();
#endif
    memmgrDefaults(&btlmem);
    memmgrInit(&btlmem);
    memmgrSupply(&btlmem, (void *)freem_start,
                 0xa0000 - freem_start);
    {
        char *below = transferbuf;
        volatile char *above = below + 0x100000;

        *above = '\0';
        *below = 'X';
        if (*above == 'X')
        {
            printf("A20 line not enabled - random results\n");
        }
    }
#else
    /* Ok, time for some heavy hacking.  Because we want to
    be able to supply a full 64k to DOS apps, we can't
    have the normal control buffers polluting the space.
    So we redefine everything by dividing by 16.  So in
    order to supply 0x30000 to 0x90000, ie a size of
    0x60000, we divide by 16 and only supply 0x6000.
    We then waste the full 0x30000 to 0x40000 for use
    by this dummy memory block.  So we tell memmgr
    that we are supplying 0x30000 for a length of 0x6000,
    and it will happily manage the control blocks within
    that, but then before returning to the app, we multiply
    the offset by 16 and add 0x10000.  When freeing
    memory, we do the reverse, ie substract 0x10000 and
    then divide by 16.  Oh, and because we took away so
    much memory, we only end up supplying 0x5000U. */
    memmgrSupply(&memmgr, (char *)MK_FP(PDOS16_MEMSTART,0x0000), 0x5000U);
#endif
#ifndef USING_EXE
    loadPcomm();
#ifndef __32BIT__
    memmgrTerm(&memmgr);
#endif
#ifdef __32BIT__
    for(;;);
    memmgrTerm(&btlmem);
#endif
#endif
    return;
}

/* for each physical disk,
for each partition
for each extended partion
gather bpb info, plus (real) drive number */

static void initdisks(void)
{
    int rc;
    int x;

    lastDrive = 2;
    for (x = 0x80; x < 0x84; x++)
    {
        rc = BosFixedDiskStatus(x);
        if (rc != 0)
        {
            printf("status %d from disk %x being ignored\n", rc, x);
        }
        scanPartition(x);
    }
    return;
}

/* Partition table values */
#define PT_OFFSET 0x1be /* offset of PT in sector */
#define PT_LEN 16 /* length of each entry */
#define PT_ENT 4 /* number of partition table entries */
#define PTO_SYSID 4 /* offset of system id */
#define PTS_FAT12 1 /* DOS 12-bit FAT */
#define PTS_FAT16S 4 /* DOS 16-bit FAT < 32MB */
#define PTS_DOSE 5 /* DOS extended partition */
#define PTS_FAT16B 6 /* DOS 16-bit FAT >= 32MB */
#define PTS_NTFS 7 /* Windows NT NTFS */
#define PTS_FAT32 0x0b /* Win95 32-bit FAT */
#define PTS_FAT32L 0x0c /* Win95 32-bit FAT LBA */
#define PTS_FAT16L 0x0e /* Win95 16-bit FAT LBA */
#define PTS_W95EL 0x0f /* Win95 Extended LBA */

static void scanPartition(int drive)
{
    unsigned char buf[512];
    int rc;
    int sectors = 1;
    int track = 0;
    int head = 0;
    int sector = 1;
    int x;
    int systemId;

    /* read partition table */
    /* rc = readAbs(buf, sectors, drive, track, head, sector); */
    rc = readLBA(buf,
                 sectors,
                 drive,
                 0); /* sector 0 */
    if (rc != 0)
    {
        printf("can't read MBR sector 0/0/1 or LBA 0 of drive %x\n", drive);
    }
    else
    {
        psector = 0;
        /* for each partition */
        for (x = 0; x < PT_ENT; x++)
        {
            lba = 1;
            systemId = buf[PT_OFFSET + x * PT_LEN + PTO_SYSID];
            /* Currently supported systems. */
            /* +++Add support for all systems and test. */
            if ((systemId == PTS_FAT12)
                || (systemId == PTS_FAT16S)
                || (systemId == PTS_FAT16B)
                || (systemId == PTS_FAT16L)
                || (systemId == PTS_FAT32)
                || (systemId == PTS_FAT32L))
            {
                processPartition(drive, &buf[PT_OFFSET + x * PT_LEN]);
            }
        }
        for (x = 0; x < PT_ENT; x++)
        {
            lba = 1;
            systemId = buf[PT_OFFSET + x * PT_LEN + PTO_SYSID];
            if ((systemId == PTS_DOSE)
                || (systemId == PTS_W95EL))
            {
                processExtended(drive, &buf[PT_OFFSET + x * PT_LEN]);
            }
        }
    }
    return;
}

static void processPartition(int drive, unsigned char *prm)
{
    unsigned char buf[512];
    int rc;
    int sectors = 1;
    int track;
    int head;
    int sect;
    unsigned char *bpb;
    unsigned long sector;

    head = prm[1];
    sect = prm[2] & 0x1f;
    track = (((unsigned int)prm[2] & 0xc0) << 2) | prm[3];
    sector = prm[8]
            | ((unsigned long)prm[9] << 8)
            | ((unsigned long)prm[10] << 16)
            | ((unsigned long)prm[11] << 24);
    sector += psector;
    if (lba)
    {
        rc = readLBA(buf,
                    sectors,
                    drive,
                    sector);
    }
    else
    {
        rc = readAbs(buf,
                    sectors,
                    drive,
                    track,
                    head,
                    sect);
    }
    if (rc != 0)
    {
        printf("can't read sector %d/%d/%d of drive %x\n",
               track, head, sect, drive);
        return;
    }
    bpb = buf + 11;
    /* check FAT signature */
    if ((buf[510] != 0x55) || (buf[511] != 0xaa))
    {
        printf("drive %x has partition without 55AA signature"
               " in VBR (%d/%d/%d)\n",
               drive, track, head, sect);
        printf("bytes per sector would have been %d\n",
               ((unsigned int)bpb[1] << 8) | bpb[0]);
        printf("maybe we could have used LBA instead, which is %ld\n", sector);
        rc = readLBA(buf,
                    sectors,
                    drive,
                    sector);
        printf("that would have given rc %d\n", rc);
        if (rc == 0)
        {
            printf("and then bytes per sector would have been %d\n",
                   ((unsigned int)bpb[1] << 8) | bpb[0]);
        }
        return;
    }
    analyseBpb(&disks[lastDrive], bpb);

    /* we set the lba to whatever mode we are currently in */
    disks[lastDrive].lba = lba;

    /* the number of hidden sectors doesn't appear to be properly
    filled in for extended partitions when formatted with MSDOS,
    so we just use the value computed already */
    disks[lastDrive].hidden = sector;

    /* if physical disks and hidden sectors match, this is the boot drive */
    if ((drive == bootDrivePhysical)
        && (disks[lastDrive].hidden == bootinfo.hidden))
    {
        bootDriveLogical = lastDrive;
    }
    disks[lastDrive].drive = drive;
    fatDefaults(&disks[lastDrive].fat);
    fatInit(&disks[lastDrive].fat, bpb, readLogical,
        writeLogical, &disks[lastDrive], getDateTime);
    strcpy(disks[lastDrive].cwd, "");
    disks[lastDrive].accessed = 1;
    disks[lastDrive].valid = 1;
    lastDrive++;
    return;
}

static void processExtended(int drive, unsigned char *prm)
{
    unsigned char buf[512];
    int sectors = 1;
    int track;
    int head;
    int sect;
    int systemId;
    unsigned long sector;
    unsigned long extsector;

    if (prm[PTO_SYSID] == PTS_W95EL)
    {
        lba = 1;
    }
    head = prm[1];
    sect = prm[2] & 0x1f;
    track = (((unsigned int)prm[2] & 0xc0) << 2) | prm[3];
    sector = prm[8]
            | ((unsigned long)prm[9] << 8)
            | ((unsigned long)prm[10] << 16)
            | ((unsigned long)prm[11] << 24);
    extsector = sector;
    while (sect != 0)
    {
        if (lba)
        {
            readLBA(buf,
                    sectors,
                    drive,
                    sector);
        }
        else
        {
            readAbs(buf,
                    sectors,
                    drive,
                    track,
                    head,
                    sect);
        }
        systemId = buf[PT_OFFSET + 0 * PT_LEN + PTO_SYSID];
        if ((systemId == PTS_FAT12)
            || (systemId == PTS_FAT16S)
            || (systemId == PTS_FAT16B)
            || (systemId == PTS_FAT16L)
            || (systemId == PTS_FAT32)
            || (systemId == PTS_FAT32L))
        {
            psector = sector;
            processPartition(drive, buf + PT_OFFSET);
        }
        head = buf[PT_OFFSET + 1 * PT_LEN + 1];
        sect = buf[PT_OFFSET + 1 * PT_LEN + 2] & 0x1f;
        track = (((unsigned int)buf[PT_OFFSET + 1 * PT_LEN + 2]
                & 0xc0) << 2)
                | buf[PT_OFFSET + 1 * PT_LEN + 3];
        sector = buf[PT_OFFSET + 1 * PT_LEN + 8]
                | ((unsigned long)buf[PT_OFFSET + 1 * PT_LEN + 9] << 8)
                | ((unsigned long)buf[PT_OFFSET + 1 * PT_LEN + 10] << 16)
                | ((unsigned long)buf[PT_OFFSET + 1 * PT_LEN + 11] << 24);
        sector += extsector;
    }
    return;
}

static void initfiles(void)
{
    int x;

    for (x = 0; x < NUM_SPECIAL_FILES; x++)
    {
        fhandle[x].inuse = 1;
        fhandle[x].special = 1;
        fhandle[x].comm = 0;
    }
    for (x = NUM_SPECIAL_FILES; x < MAXFILES; x++)
    {
        fhandle[x].inuse = 0;
        fhandle[x].special = 0;
        fhandle[x].comm = 0;
    }
    return;
}

/* !!! START OF POS FUNCTIONS !!! */

/* INT 20 */
void PosTermNoRC(void)
{
    PosTerminate(0);
}

/* INT 21/AH=02h */
unsigned int PosDisplayOutput(unsigned int ch)
{
    unsigned char buf[1];
    size_t writtenbytes;

    buf[0] = ch;
    PosWriteFile(1, buf, 1, &writtenbytes);

    return ch;
}

/* INT 21/AH=06h */
unsigned int PosDirectConsoleOutput(unsigned int ch)
{
    unsigned char buf[1];
    size_t writtenbytes;

    buf[0] = ch;
    PosWriteFile(1, buf, 1, &writtenbytes);

    return ch;
}

/* INT 21/AH=07h */
unsigned int PosDirectCharInputNoEcho(void)
{
    int scan;
    int ascii;
    static int gotextend = 0;

    if (gotextend != 0)
    {
        ascii = gotextend;
        gotextend = 0;
    }
    else
    {
#ifdef __32BIT__
        waitForKeystroke();
#endif
        BosReadKeyboardCharacter(&scan, &ascii);
        if (ascii == 0)
        {
            gotextend = scan;
        }
    }

    return ascii;
}

/* Written By NECDET COKYAZICI, Public Domain */

/* INT 21/AH=08h */
unsigned int PosGetCharInputNoEcho(void)
{
    static int scan = 0;
    int ascii;

    if(scan != 0)
    {
        int retval = scan;

        scan = 0;
        return retval;
    }

#ifdef __32BIT__
    waitForKeystroke();
#endif
    BosReadKeyboardCharacter(&scan, &ascii);

    if(ascii) scan = 0;

    return ascii;
}

/* INT 21/AH=09h */
unsigned int PosDisplayString(const char *buf)
{
    const char *p;
    size_t writtenbytes;

    p = memchr(buf, '$', (size_t)-1);
    if (p == NULL) p = buf;
    PosWriteFile(1, buf, p - buf, &writtenbytes);
    return ('$');
}

/* INT 21/AH=0Eh */
unsigned int PosSelectDisk(unsigned int drive)
{
    unsigned int ret;

    if (drive < 2)
    {
        accessDisk(drive);
    }
    if (isDriveValid(drive))
    {
        currentDrive = drive;

        cwd = disks[drive].cwd;
    }

    ret = lastDrive;

    if (ret < 5)
    {
        ret = 5;
    }

    return (ret);
}

/* INT 21/AH=19h */
unsigned int PosGetDefaultDrive(void)
{
    return (currentDrive);
}

/* INT 21/AH=1Ah */
void PosSetDTA(void *p)
{
    dta = p;
    return;
}

/* INT 21/AH=25h */
void PosSetInterruptVector(unsigned int intnum, void *handler)
{
#ifdef __32BIT__
    protintHandler(intnum, handler);
#else
    disable();
    *((void **)0 + intnum) = handler;
    enable();
#endif
}

/* INT 21/AH=2Ah */
void PosGetSystemDate(unsigned int *year,
                      unsigned int *month,
                      unsigned int *day,
                      unsigned int *dw)
{
    int c,y,m,d;
    int retval;

    retval = BosGetSystemDate(&c,&y,&m,&d);

    if(retval == 0)
    {
        *year = bcd2int(c) * 100 + bcd2int(y);
        *month = bcd2int(m);
        *day = bcd2int(d);
        *dw = dow(*year,*month,*day);
    }

    return;
}

/* INT 21/AH=2Bh */
unsigned int PosSetSystemDate(unsigned int year,
                              unsigned int month,
                              unsigned int day)
{
    BosSetSystemDate(year / 100,year % 100,month,day);

    return 0;
}

/* INT 21/AH=2Ch */
void PosGetSystemTime(unsigned int *hour, unsigned int *minute,
                      unsigned int *second, unsigned int *hundredths)
{
    unsigned long ticks,t;
    unsigned int midnight;

    BosGetSystemTime(&ticks,&midnight);

    t=(ticks*1000)/182;
    *hundredths=(unsigned int)t%100;
    t/=100;
    *second=(unsigned int)t%60;
    t/=60;
    *minute=(unsigned int)t%60;
    t/=60;
    *hour=(unsigned int)t;
    return;
}

void *PosGetDTA(void)
{
    return (dta);
}

unsigned int PosGetDosVersion(void)
{
    return (reportedDosVersion);
}

void PosSetDosVersion(unsigned int version)
{
    reportedDosVersion = version;
}

void PosSetLogUnimplemented(int flag)
{
    logUnimplementedFlag = !!flag;
}

int PosGetLogUnimplemented(void)
{
    return logUnimplementedFlag;
}

void PosSetBreakFlag(int flag)
{
    breakFlag = !!flag;
}

int PosGetBreakFlag(void)
{
    return breakFlag;
}

void PosSetVerifyFlag(int flag)
{
    verifyFlag = !!flag;
}

int PosGetVerifyFlag(void)
{
    return verifyFlag;
}

int PosGetMagic(void)
{
    return PDOS_MAGIC;
}

void *PosGetInterruptVector(int intnum)
{
    return *((void **)0 + intnum);
}

/*To find out the free space in hard disk given by drive*/
void PosGetFreeSpace(int drive,
                     unsigned int *secperclu,
                     unsigned int *numfreeclu,
                     unsigned int *bytpersec,
                     unsigned int *totclus)
{
    if (drive==0)
    {
        drive=currentDrive;
    }
    else
    {
        drive--;
    }
    if ((drive >= 0) && (drive < lastDrive))
    {
        *secperclu=4;
        *numfreeclu=25335;
        *bytpersec=512;
        *totclus=25378;
    }
    else if (drive >=lastDrive)
    {
        *secperclu=0xFFFF;
    }
    return;
}
/**/

int PosMakeDir(const char *dname)
{
    int ret;
    char dirname[MAX_PATH];

    ret = formatcwd(dname, dirname);
    if (ret) return (ret);
    ret = dirCreat(dirname, 0);

    return (ret);
}

int PosRemoveDir(const char *dname)
{
    int ret;
    char dirname[MAX_PATH];

    ret = formatcwd(dname, dirname);
    if (ret) return (ret);
    ret = dirDelete(dirname);

    return (ret);
}

int PosChangeDir(const char *to)
{
    char newcwd[MAX_PATH];
    int attr;
    int ret;

    ret = formatcwd(to, newcwd);
    if (ret) return (ret);
    /* formatcwd gave us "[drive]:\[rest]" but we want only the [rest],
     * so we use to to point at the [rest]. */
    to = newcwd + 3;

#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    /* formatcwd also provided us with tempDrive from the path we gave it. */
    ret = fatGetFileAttributes(&disks[tempDrive].fat, to, &attr);
    if (ret || !(attr & DIRENT_SUBDIR))
    {
#ifdef __32BIT__
        unlockMutex(&fatMutex);
#endif
        return (POS_ERR_PATH_NOT_FOUND);
    }

    /* If to is "", we should just change to root
     * by copying newcwd into cwd. */
    if (strcmp(to, "") == 0) strcpy(disks[tempDrive].cwd, to);
    /* fatPosition provides us with corrected path with LFNs
     * where possible and correct case, so we use it as cwd. */
    else strcpy(disks[tempDrive].cwd, disks[tempDrive].fat.corrected_path);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif

    return (0);
}

int PosCreatFile(const char *name, int attrib, int *handle)
{
    char filename[MAX_PATH];
    int ret;

    ret = formatcwd(name, filename);
    if (ret) return (ret);
    return (fileCreat(filename, attrib, handle));
}

int PosOpenFile(const char *name, int mode, int *handle)
{
    char filename[MAX_PATH];
    int ret;

    if (((strncmp(name, "COM", 3) == 0)
         || (strncmp(name, "com", 3) == 0))
        && isdigit((unsigned char)name[3]))
    {
        return (opencomm(atoi(name + 3), handle));
    }
    ret = formatcwd(name, filename);
    if (ret) return (ret);
    ret = fileOpen(filename, handle);
    if ((ret != 0) && warnerror)
    {
        printf("warning - failed to open %s\n", name);
    }
    return (ret);
}

int PosCloseFile(int fno)
{
    int ret;

    ret = fileClose(fno);
    return (ret);
}

int PosReadFile(int fh, void *data, size_t bytes, size_t *readbytes)
{
    unsigned char *p;
    size_t x = 0;
    int ret;

    if (fh < NUM_SPECIAL_FILES)
    {
        p = (unsigned char *)data;
        while (x < bytes)
        {
            int scan;
            int ascii;

#ifdef __32BIT__
            waitForKeystroke();
#endif
            BosReadKeyboardCharacter(&scan, &ascii);
            if ((ascii == '\b') && (x > 0))
            {
                x--;
                p[x] = '\0';
            }
            else
            {
                p[x] = ascii;
            }
            pdosWriteText(ascii);
            if (ascii == '\r')
            {
                pdosWriteText('\n');
                /* NB: this will need to be fixed, potential
                buffer overflow - bummer! */
                x++;
                p[x] = '\n';
                x++;
                break;
            }
            if (ascii != '\b')
            {
                x++;
            }
        }
        *readbytes = x;
        ret = 0;
    }
    else if (fhandle[fh].comm)
    {
        int c;
        int port;

        port = fhandle[fh].comm - 1;
        while (1)
        {
            c = BosSerialReadChar(port);
            if ((c & 0x8000U) == 0) break;
        }
        c &= 0xff;
        *(char *)data = c;
        *readbytes = 1;
        ret = 0;
    }
    else
    {
        ret = fileRead(fh, data, bytes, readbytes);
    }
    return (ret);
}

int PosWriteFile(int fh, const void *data, size_t len, size_t *writtenbytes)
{
    unsigned char *p;
    size_t x;
    int ret;

    if (fh < NUM_SPECIAL_FILES)
    {
        p = (unsigned char *)data;
        for (x = 0; x < len; x++)
        {
            /* need this for Linux calls */
            if (p[x] == '\n')
            {
                pdosWriteText('\r');
            }
            pdosWriteText(p[x]);
        }
        *writtenbytes = len;
        ret = 0;
    }
    else if (fhandle[fh].comm)
    {
        const char *buf = data;
        int port;
        size_t x;

        port = fhandle[fh].comm - 1;
        for (x = 0; x < len; x++)
        {
            BosSerialWriteChar(port, buf[x]);
        }
        *writtenbytes = x;
        ret = 0;
    }
    else
    {
        ret = fileWrite(fh, data, len, writtenbytes);
    }
    return (ret);
}

/* To delete a given file with fname as its filename */
int PosDeleteFile(const char *name)
{
    char filename[MAX_PATH];
    int ret;

    ret = formatcwd(name, filename);
    if (ret) return (ret);
    ret = fileDelete(filename);
    if (ret < 0)
    {
        return(ret);
    }
    return (ret);
}
/**/

int PosMoveFilePointer(int handle, long offset, int whence, long *newpos)
{
    /* Checks if a valid handle was provided. */
    if (handle < NUM_SPECIAL_FILES || handle >= MAXFILES
        || !(fhandle[handle].inuse))
    {
        return (POS_ERR_INVALID_HANDLE);
    }
    return (fileSeek(handle, offset, whence, newpos));
}

/*To get the attributes of a given file*/
int PosGetFileAttributes(const char *fnm,int *attr)
{
    int rc;
    char tempf[FILENAME_MAX];

    rc = formatcwd(fnm, tempf);
    if (rc) return (rc);
    fnm = tempf;
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatGetFileAttributes(&disks[tempDrive].fat,tempf + 2,attr);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    return (rc);
}
/**/

/*To set the attributes of a given file*/
int PosSetFileAttributes(const char *fnm,int attr)
{
    int rc;
    char tempf[FILENAME_MAX];

    rc = formatcwd(fnm, tempf);
    if (rc) return (rc);
    fnm = tempf;
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatSetFileAttributes(&disks[tempDrive].fat,tempf + 2,attr);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    return (rc);
}
/**/

int PosGetDeviceInformation(int handle, unsigned int *devinfo)
{
    *devinfo = 0;
    if (handle < NUM_SPECIAL_FILES)
    {
        if (handle == 0)
        {
            *devinfo |= (1 << 0); /* standard input device */
        }
        else if ((handle == 1) || (handle == 2))
        {
            *devinfo |= (1 << 1); /* standard output device */
        }
        *devinfo |= (1 << 7); /* character device */
    }
    else
    {
        /* block device (disk file) */
    }
    return (0);
}

int PosBlockDeviceRemovable(int drive)
{
    if (drive < 2)
    {
        return (0);
    }
    else
    {
        return (1);
    }
}
/* Gets the status of remote device if error returns the error code */
int PosBlockDeviceRemote(int drive, int *da)
{
    *da=0;
    return (0);
}
/**/

/*Implementation of the function call 440D*/
int PosGenericBlockDeviceRequest(int drive,int catcode,int function,
                                void *parm_block)
{
    PB_1560 *pb;

    if (drive == 0)
    {
        drive = currentDrive;
    }
    else
    {
        drive--;
    }
    pb=parm_block;
    memcpy(&pb->bpb, &disks[drive].bpb, sizeof pb->bpb);
    return (0);
}
/**/

int PosDuplicateFileHandle(int fh)
{
    int x;

    if (fh >= MAXFILES || fh < 0)
    {
        return (-POS_ERR_INVALID_HANDLE);
    }

    for (x = NUM_SPECIAL_FILES; x < MAXFILES; x++)
    {
        if (!fhandle[x].inuse)
        {
            break;
        }
    }
    if (x == MAXFILES) return (-POS_ERR_MANY_OPEN_FILES);

    fhandle[x] = fhandle[fh];

    return (x);
}

int PosForceDuplicateFileHandle(int fh, int newfh)
{
    if (fh >= MAXFILES || fh < 0 || newfh >= MAXFILES || newfh < 0)
    {
        return (POS_ERR_INVALID_HANDLE);
    }

    if (fhandle[newfh].inuse) fileClose(newfh);

    fhandle[newfh] = fhandle[fh];

    return (0);
}

int PosGetCurDir(int drive, char *buf)
{
    if (drive == 0)
    {
        drive = currentDrive;
    }
    else
    {
        drive--;
    }
    strcpy(buf, disks[drive].cwd);
    return (0);
}

#ifdef __32BIT__
void *PosAllocMem(unsigned int size, unsigned int flags)
{
    unsigned int subpool;

    subpool = flags & 0xff;
    if ((flags & POS_LOC_MASK) == POS_LOC20)
    {
        return (memmgrAllocate(&btlmem, size, memId + subpool));
    }
    return (kmalloc(size));
}
#endif

#ifdef __32BIT__
void *PosAllocMemPages(unsigned int pages, unsigned int *maxpages)
{
    void *p;

    p = kmalloc(pages * 16);
    if (p == NULL && maxpages != NULL)
    {
        *maxpages = 0;
    }
    return (p);
}
#else
void *PosAllocMemPages(unsigned int pages, unsigned int *maxpages)
{
    void *p;

    p = pdos16MemmgrAllocPages(&memmgr, pages, memId);
    if (p == NULL && maxpages != NULL)
    {
        *maxpages = memmgrMaxSize(&memmgr);
    }
    if (p && curPCB)
    {
        memmgrSetOwner(&memmgr, p, FP_SEG(curPCB));
    }
    return (p);
}
#endif

static int pdosMemmgrIsBlockPtr(void *ptr)
{
#if !defined(__32BIT__) && !defined(__SMALLERC__)
    unsigned long abs;

    abs = ADDR2ABS(ptr);
    abs -= 0x10000UL;
    abs -= (unsigned long)PDOS16_MEMSTART * 16;
    abs /= 16;
    abs += (unsigned long)PDOS16_MEMSTART * 16;
    ptr = ABS2ADDR(abs);
#endif
    return memmgrIsBlockPtr(ptr);
}

/* Before process control blocks were implemented, the memory layout looked
 * like this:
 *    Main Code Segment:   |MEMMGRN|PSP|Program code...
 *    Other Segments:      |MEMMGRN|Code or data...
 * However, with the implementation of PCBs, it now looks like this:
 *    Main Code Segment:   |MEMMGRN|PCB|PSP|Program code...
 *    Other Segments:      |MEMMGRN|Code or data...
 * The memory management APIs expect to get the PSP pointer, and subtract the
 * MEMMGRN size, to get the MEMMGRN pointer. But, with the PCB in between the
 * MEMMGRN and PSP, that won't work. So, this method detects when there is a
 * PCB immediately before the pointer instead of a MEMMGRN, and in that case
 * it returns the PCB pointer (so the memory manager finds the MEMMGRN before
 * it). Otherwise, it just returns the input pointer.
 */
static void *translateProcessPtr(void *ptr)
{
    void *prev;
#ifndef __32BIT__
    unsigned long abs;
#endif

#if defined(__32BIT__) || defined(__SMALLERC__)
    prev = (void*)(((char*)ptr)-PDOS_PROCESS_SIZE);
#else
    abs = ADDR2ABS(ptr);
    abs -= PDOS_PROCESS_SIZE;
    prev = FP_NORM(ABS2ADDR(abs));
#endif

    if (!pdosMemmgrIsBlockPtr(ptr) && isProcessPtr(ptr) &&
            pdosMemmgrIsBlockPtr(prev))
    {
        return prev;
    }
    return ptr;
}

int PosFreeMem(void *ptr)
{
    ptr = translateProcessPtr(ptr);
#ifdef __32BIT__
    if ((unsigned long)ptr < 0x0100000)
    {
        memmgrFree(&btlmem, ptr);
        return (0);
    }
    kfree(ptr);
    return (0);
#endif
    memmgrFree(&memmgr, ptr);
    return (0);
}

#ifdef __32BIT__
int PosReallocPages(void *ptr, unsigned int newpages, unsigned int *maxp)
{
    /* Incompatible with krealloc() (and realloc() as well)
     * because the address of the memory cannot be changed.
     * Error is always returned to prevent memory corruption. */
    return (POS_ERR_INSUFFICIENT_MEMORY);
}
#else
int PosReallocPages(void *ptr, unsigned int newpages, unsigned int *maxp)
{
    int ret;

    ptr = translateProcessPtr(ptr);
    ret = pdos16MemmgrReallocPages(&memmgr, ptr, newpages);
    if (ret != 0)
    {
        *maxp = memmgrMaxSize(&memmgr);
        ret = 8;
    }
    return (ret);
}
#endif

void PosExec(char *prog, POSEXEC_PARMBLOCK *parmblock)
{
    char tempp[FILENAME_MAX];

    if (formatcwd(prog, tempp)) return;
    /* +++Add a way to let user know it failed on formatcwd. */
    prog = tempp;
    loadExe(prog, parmblock);
    return;
}

void PosTerminate(int rc)
{
    callwithbypass(rc);
    return;
}

#ifndef __32BIT__
void exit(int rc)
{
    PosTerminate(rc);
}
#endif

int PosGetReturnCode(void)
{
    return (lastrc);
}

int PosFindFirst(char *pat, int attrib)
{
    int ret;
    FFBLK *ffblk;

    attr = attrib;
    memset(dta, '\0', 0x15); /* clear out reserved area */
    make_ff(pat);
    ret = fileOpen(ff_path, &ff_handle);
    if (ret) return (3);
    ret = ff_search();
    if (ret == 0x12)
    {
        ret = 2;
    }

    /*On multiple FindFirst calls the contents in the DTA are erased,
      to avoid this the ff_pat formed by the current FindFirst call
      is stored in a temporary buffer and set in DTA so that proper
      data gets populated DTA*/
    ffblk = (FFBLK *) dta;
    ffblk->handle = ff_handle;
    strcpy(ffblk->pat, ff_pat);

    return (ret);
}

int PosFindNext(void)
{
    FFBLK *ffblk;

    ffblk = (FFBLK *) dta;
    ff_handle = ffblk->handle;
    strcpy(ff_pat, ffblk->pat);

    return (ff_search());
}

/* To rename a given file */
int PosRenameFile(const char *old, const char *new)
{
    int rc;
    char tempf1[FILENAME_MAX];
    char tempf2[FILENAME_MAX];
    /* +++Add support for moving files using rename. */

    rc = formatcwd(old,tempf1);
    if (rc) return (rc);
    strcpy(tempf2, new);

    old = tempf1;
    new = tempf2;

#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatRenameFile(&disks[tempDrive].fat, tempf1 + 2,new);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    return (rc);
}
/**/

/*Get last written date and time*/
int PosGetFileLastWrittenDateAndTime(int handle,
                                     unsigned int *fdate,
                                     unsigned int *ftime)
{
    FATFILE *fatfile;

    fatfile=&fhandle[handle].fatfile;
    *fdate=fatfile->fdate;
    *ftime=fatfile->ftime;
    return 0;
}
/**/

/*Set the last written date and time for a given file*/
int PosSetFileLastWrittenDateAndTime(int handle,
                                     unsigned int fdate,
                                     unsigned int ftime)
{
    FATFILE *fatfile;
    FAT *fat;

    fatfile=&fhandle[handle].fatfile;
    fat=fhandle[handle].fatptr;
    fatfile->fdate=fdate;
    fatfile->ftime=ftime;
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    fatUpdateDateAndTime(fat,fatfile);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    return 0;
}
/**/

int PosCreatNewFile(const char *name, int attrib, int *handle)
{
    int ret;
    char filename[MAX_PATH];

    ret = formatcwd(name, filename);
    if (ret) return (ret);
    return (newFileCreat(filename, attrib, handle));
}

int PosTruename(char *prename,char *postname)
{
    formatcwd(prename,postname);
    return(0);
}

#ifdef __32BIT__
void PosAExec(char *prog, POSEXEC_PARMBLOCK *parmblock)
{
    char tempp[FILENAME_MAX];

    if (formatcwd(prog, tempp)) return;
    /* +++Add a way to let user know it failed on formatcwd. */
    prog = tempp;
    loadExe32(prog, parmblock, ASYNCHRONOUS);
    return;
}
#endif

void PosDisplayInteger(int x)
{
    printf("integer is %x\n", x);
#ifdef __32BIT__
    printf("normalized is %p\n", (void *)x);
    printf("absolute is %p\n", (void *)x);
#endif
    return;
}

#ifdef __32BIT__
void PosReboot(void)
{
    runreal_p(doreboot, 0);
    return;
}
#else
void PosReboot(void)
{
    int *posttype = (int *)0x472;
    void (*postfunc)(void) = (void (*)(void))MK_FP(0xffff, 0x0000);

    *posttype = 0x1234;
    postfunc();
    return;
}
#endif

#ifdef __32BIT__
void PosPowerOff(void)
{
    runreal_p(dopoweroff, 0);
    return;
}
#else
void PosPowerOff(void)
{
    poweroff();
    return;
}
#endif

/* !!! END OF POS FUNCTIONS !!! */


static void make_ff(char *pat)
{
    char *p;

    strcpy(ff_pat, pat);
    upper_str(ff_pat);
    scrunchf(ff_path, ff_pat);
    p = strrchr(ff_path, '\\');
    if (p != NULL)
    {
        strcpy(ff_pat, p + 1);
        *p = '\0';
    }
    else
    {
        p = strrchr(ff_path, ':');
        if (p != NULL)
        {
            strcpy(ff_pat, p + 1);
            *(p + 1) = '\0';
        }
        else
        {
            strcpy(ff_pat, ff_path);
            ff_path[0] = '\0';
        }
    }
    if ((strcmp(ff_pat, "") == 0) || (strcmp(ff_pat, "*.*") == 0))
    {
        strcpy(ff_pat, "*");
    }
    return;
}

static void scrunchf(char *dest, char *new)
{
    char *mycwd;
    char *p;
    int drive;

    strcpy(dest, "");
    mycwd = cwd;
    if (strchr(new, ':') != NULL)
    {
        drive = *new - 'A';
        if (drive < 2)
        {
            accessDisk(drive);
        }
        mycwd = disks[drive].cwd;
        memcpy(dest, new, 2);
        dest[2] = '\0';
        new += 2;
    }
    if ((*new == '/') || (*new == '\\'))
    {
        strcat(dest, new);
    }
    else if ((strchr(new, '\\') != NULL) || (strchr(new, '/') != NULL))
    {
        strcat(dest, "\\");
        strcat(dest, new);
    }
    else
    {
        strcat(dest, mycwd);
        while (memcmp(new, "..", 2) == 0)
        {
            p = strrchr(dest, '\\');
            if (p != NULL)
            {
                *p = '\0';
            }
            else
            {
                strcpy(dest, "\\");
            }
            new += 3;
        }
        strcat(dest, "\\");
        strcat(dest, new);
    }
    return;
}

static int ff_search(void)
{
    size_t readbytes;
    char file[13];
    char *p;
    DIRENT dirent;
    unsigned char lfn[255]; /*+++Add UCS-2 support. */
    unsigned int lfn_len = 0;
    unsigned char checksum;
    /* Upper lfn which is a string. */
    unsigned char testlfn[256];

    fileRead(ff_handle, &dirent, sizeof dirent, &readbytes);
    while ((readbytes == sizeof dirent) && (dirent.file_name[0] != '\0'))
    {
        if (dirent.file_name[0] != DIRENT_DEL)
        {
            if (dirent.file_attr == DIRENT_LFN)
            {
                /* Reads LFN entry and stores the checksum. */
                checksum = readLFNEntry(&dirent, lfn, &lfn_len);
            }
            else
            {
                /* If LFN was found before, checks
                 * if it belongs to this 8.3 entry. */
                if (lfn_len)
                {
                    /* Checks if the checksum is correct for this entry. */
                    if (checksum == generateChecksum(dirent.file_name))
                    {
                        /* If it is correct, copies LFN, makes it string
                         * and uppers it so it is compatible with patmat. */
                        memcpy(testlfn, lfn, lfn_len);
                        testlfn[lfn_len] = '\0';
                        /* +++Make UCS-2 compatible, because upper_str
                         * uses only char, not unsigned int. Also patmat
                         * is not yet UCS-2 compatible. */
                        upper_str(testlfn);
                    }
                    /* If it is not, makes sure that
                     * this LFN will not be used again. */
                    else lfn_len = 0;
                }
                memcpy(file, dirent.file_name, sizeof(dirent.file_name));
                file[sizeof(dirent.file_name)] = '\0';
                p = strchr(file, ' ');
                if (p != NULL)
                {
                    *p = '\0';
                }
                else
                {
                    p = file + strlen(file);
                }
                *p++ = '.';
                memcpy(p, dirent.file_ext,sizeof(dirent.file_ext));
                p[3] = '\0';
                p = strchr(file, ' ');
                if (p != NULL)
                {
                    *p = '\0';
                }
                p = file + strlen(file);
                if (p > file) p--;
                if (*p == '.') *p = '\0';
                if ((patmat(file, ff_pat) ||
                    /* If it has LFN, it is enough if only one of them fits. */
                     (lfn_len && patmat(testlfn, ff_pat)))

                    /* if it is not a directory, or they asked for
                    directories, then that is OK */
                    && (((dirent.file_attr & DIRENT_SUBDIR) == 0)
                        || ((attr & DIRENT_SUBDIR) != 0))

                    /* if it is not a volume label, or they asked
                    for volume labels, then that is OK */
                    && (((dirent.file_attr & DIRENT_EXTRAB3) == 0)
                        || ((attr & DIRENT_EXTRAB3) != 0))

                )
                {
                    dta->attrib = dirent.file_attr; /* attribute */

                    dta->file_time = dirent.last_modtime[0]   /*time*/
                    | ((unsigned int)dirent.last_modtime[1] << 8);

                    dta->file_date = dirent.last_moddate[0]   /*date*/
                    | ((unsigned int)dirent.last_moddate[1] << 8);

                    dta->file_size = dirent.file_size[0]      /*size*/
                    | ((unsigned long)dirent.file_size[1] << 8)
                    | ((unsigned long)dirent.file_size[2] << 16)
                    | ((unsigned long)dirent.file_size[3] << 24);

                    dta->startcluster=dirent.start_cluster[0]
                    |(dirent.start_cluster[1]<<8);

                    memset(dta->file_name, '\0', sizeof(dta->file_name));
                    strcpy(dta->file_name, file);

                    /* Checks if this file has LFN associated
                     * and stores it in the DTA. This check
                     * is sufficient, because the check before
                     * sets lfn_len to 0 when it fails. */
                    if (lfn_len)
                    {
                        memcpy(dta->lfn, lfn, lfn_len);
                        /* Adds null terminator at the end of LFN. */
                        dta->lfn[lfn_len] = '\0';
                    }
                    else dta->lfn[0] = '\0';
                    return (0);
                }
            }
        }
        fileRead(ff_handle, &dirent, sizeof dirent, &readbytes);
    }
    fileClose(ff_handle);
    return (0x12);
}

#ifdef __32BIT__
/* registers come in as eax, ebx, ecx, edx, esi, edi, cflag */
int int0E(unsigned int *regs)
{
    printf("Page Fault occurred (Protected Mode Exception 0xE)\n");
    printf("while accessing virtual address 0x%08x\n", readCR2());
    printf("Error code is %08x\n", regs[8]);
    printf("System halting\n");
    for (;;);

    return (0);
}

int int0(unsigned int *regs)
{
    unsigned int *oldsp;
    unsigned int *ebp;
    unsigned int *retaddr;

    printf("Divide by zero fault occurred (Protected Mode Exception 0x0)\n");
    printf("EAX %08X EBX %08X ECX %08X EDX %08X\n",
           regs[0], regs[1], regs[2], regs[3]);
    printf("ESI %08X EDI %08X\n",
           regs[4], regs[5]);
    printf("module loaded at %p, entry point %08lX\n", loadaddr, entry_point);
    printf("regs are at %p\n", regs);
    oldsp = (unsigned int *)regs[8];
    printf("old stack starts at %p\n", oldsp);
    ebp = (unsigned int *)oldsp[-1];
    printf("EBP is probably %p\n", ebp);
    printf("interrupt address is %08X\n", oldsp[8]);
    printf("flags are %08X\n", oldsp[10]);
    printf("EBP chain to EBP is %08X\n", ebp[0]);
    retaddr = (unsigned int *)ebp[1];
    printf("previous function's return address is %p\n", retaddr);
    printf("called address was possibly relative %08X\n", retaddr[-1]);
    printf("which would make it absolute address %08X\n",
           (char *)retaddr + retaddr[-1]);

    printf("System halting\n");
    for (;;);

    return (0);
}

int int0D(unsigned int *regs)
{
    printf("General Protection Fault occurred\n");
    printf("(Protected Mode Exception 0xD)\n");
    printf("Error code is %08x\n", regs[8]);
    if (regs[8])
    {
        int table = (regs[8] >> 1) & 0x3;

        if (regs[8] & 0x1) printf("Origin external to processor\n");
        printf("Selector index 0x%x in ", regs[8] >> 3);
        if (table == 0) printf("GDT\n");
        else if (table == 0x2) printf("LDT\n");
        else printf("IDT\n");
    }
    printf("System halting\n");
    for (;;);

    return (0);
}

int int20(unsigned int *regs)
{
    static union REGS regsin;
    static union REGS regsout;

    regsin.d.eax = regs[0];
    regsin.d.ebx = regs[1];
    regsin.d.ecx = regs[2];
    regsin.d.edx = regs[3];
    regsin.d.esi = regs[4];
    regsin.d.edi = regs[5];
    regsin.d.cflag = 0;
    memcpy(&regsout, &regsin, sizeof regsout);
    PosTermNoRC();
    regs[0] = regsout.d.eax;
    regs[1] = regsout.d.ebx;
    regs[2] = regsout.d.ecx;
    regs[3] = regsout.d.edx;
    regs[4] = regsout.d.esi;
    regs[5] = regsout.d.edi;
    regs[6] = regsout.d.cflag;
    return (0);
}

/* INT 25 - Absolute Disk Read */
int int25(unsigned int *regs)
{
    static union REGS regsin;
    static union REGS regsout;
    DP *dp;
    void *p;

    regsin.d.eax = regs[0];
    regsin.d.ebx = regs[1];
    regsin.d.ecx = regs[2];
    regsin.d.edx = regs[3];
    regsin.d.esi = regs[4];
    regsin.d.edi = regs[5];
    regsin.d.cflag = 0;
    memcpy(&regsout, &regsin, sizeof regsout);
    dp = (void *)(regsin.d.ebx);
    p = (void *)(dp->transferaddress);
    PosAbsoluteDiskRead(regsin.h.al,dp->sectornumber,dp->numberofsectors,p);
    regs[0] = regsout.d.eax;
    regs[1] = regsout.d.ebx;
    regs[2] = regsout.d.ecx;
    regs[3] = regsout.d.edx;
    regs[4] = regsout.d.esi;
    regs[5] = regsout.d.edi;
    regs[6] = regsout.d.cflag;
    return (0);
}
/* INT 26 - Absolute Disk Write */
int int26(unsigned int *regs)
{
    static union REGS regsin;
    static union REGS regsout;
    DP *dp;
    void *p;

    regsin.d.eax = regs[0];
    regsin.d.ebx = regs[1];
    regsin.d.ecx = regs[2];
    regsin.d.edx = regs[3];
    regsin.d.esi = regs[4];
    regsin.d.edi = regs[5];
    regsin.d.cflag = 0;
    memcpy(&regsout, &regsin, sizeof regsout);
    dp = (void *)(regsin.d.ebx);
    p = (void *)(dp->transferaddress);
    PosAbsoluteDiskWrite(regsin.h.al,dp->sectornumber,dp->numberofsectors,p);
    regs[0] = regsout.d.eax;
    regs[1] = regsout.d.ebx;
    regs[2] = regsout.d.ecx;
    regs[3] = regsout.d.edx;
    regs[4] = regsout.d.esi;
    regs[5] = regsout.d.edi;
    regs[6] = regsout.d.cflag;
    return (0);
}

/* IRQ 0 - Timer Interrupt. */
int intB0(unsigned int *regs)
{
    schedule();

    return (0);
}


/**/
#else
void int0(unsigned int *regptrs)
{
    unsigned short ss;
    unsigned char *chain;
    unsigned short *retaddr;

    printf("got a divide by zero\n");
    printf("AX %04X BX %04X CX %04X DX %04X\n",
           regptrs[8], regptrs[6], regptrs[5], regptrs[4]);
    printf("SI %04X DI %04X DS %04X ES %04X\n",
           regptrs[3], regptrs[2], regptrs[1], regptrs[0]);
    printf("BP %04X CS %04X IP %04X FLAGS %04X\n",
           regptrs[9], regptrs[11], regptrs[10], regptrs[12]);
    printf("module loaded at %p, entry point %p\n", loadaddr, entry_point);
    printf("interrupt address is %p\n", MK_FP(regptrs[11], regptrs[10]));
    printf("regptrs is %p\n", regptrs);
    ss = FP_SEG(regptrs);
    chain = MK_FP(ss, regptrs[9]); /* bp chains back */
    /* Watcom isn't pushing bp first, on entry, so we have a
       temporary hack of 6 */
    retaddr = (unsigned short *)*(char **)(chain + 2 + 6);
    printf("retaddr is %p\n", retaddr);
    printf("and that caller might have done a near call to %p\n",
           MK_FP(regptrs[11],
                 (unsigned short)(FP_OFF(retaddr) + retaddr[-1])));
    printf("halting\n");
    for (;;) ;
    return;
}

void int20(unsigned int *regptrs,
        unsigned int es,
        unsigned int ds,
        unsigned int di,
        unsigned int si,
        unsigned int dx,
        unsigned int cx,
        unsigned int bx,
        unsigned int cflag,
        unsigned int ax)
{
    static union REGS regsin;
    static union REGS regsout;
    static struct SREGS sregs;

    regsin.x.ax = ax;
    regsin.x.bx = bx;
    regsin.x.cx = cx;
    regsin.x.dx = dx;
    regsin.x.si = si;
    regsin.x.di = di;
    regsin.x.cflag = 0;
    sregs.ds = ds;
    sregs.es = es;
    memcpy(&regsout, &regsin, sizeof regsout);
    PosTermNoRC();
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

/* INT 25 - Absolute Disk Read */
void int25(unsigned int *regptrs,
        unsigned int es,
        unsigned int ds,
        unsigned int di,
        unsigned int si,
        unsigned int dx,
        unsigned int cx,
        unsigned int bx,
        unsigned int cflag,
        unsigned int ax)
{
    static union REGS regsin;
    static union REGS regsout;
    static struct SREGS sregs;
    DP *dp;
    void *p;

    regsin.x.ax = ax;
    regsin.x.bx = bx;
    regsin.x.cx = cx;
    regsin.x.dx = dx;
    regsin.x.si = si;
    regsin.x.di = di;
    regsin.x.cflag = 0;
    sregs.ds = ds;
    sregs.es = es;
    memcpy(&regsout, &regsin, sizeof regsout);
    dp=MK_FP(sregs.ds,regsin.x.bx);
    p=dp->transferaddress;
    PosAbsoluteDiskRead(regsin.h.al,dp->sectornumber,dp->numberofsectors,p);
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
/**/
/* INT 26 - Absolute Disk Write */
void int26(unsigned int *regptrs,
        unsigned int es,
        unsigned int ds,
        unsigned int di,
        unsigned int si,
        unsigned int dx,
        unsigned int cx,
        unsigned int bx,
        unsigned int cflag,
        unsigned int ax)
{
    static union REGS regsin;
    static union REGS regsout;
    static struct SREGS sregs;
    DP *dp;
    void *p;

    regsin.x.ax = ax;
    regsin.x.bx = bx;
    regsin.x.cx = cx;
    regsin.x.dx = dx;
    regsin.x.si = si;
    regsin.x.di = di;
    regsin.x.cflag = 0;
    sregs.ds = ds;
    sregs.es = es;
    memcpy(&regsout, &regsin, sizeof regsout);
    dp=MK_FP(sregs.ds,regsin.x.bx);
    p=dp->transferaddress;
    PosAbsoluteDiskWrite(regsin.h.al,dp->sectornumber,dp->numberofsectors,p);
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
/**/
#endif

static void loadConfig(void)
{
    size_t readbytes;
    unsigned char buf[512];
    int x;
    int fh;
    int ret;

    ret = fileOpen("CONFIG.SYS", &fh);
    if (!ret)
    {
        do
        {
            fileRead(fh, buf, sizeof buf, &readbytes);
            for (x = 0; x < readbytes; x++)
            {
                if (memcmp(buf, "SHELL=", 6) == 0)
                {
                    char *p;

                    memcpy(shell, buf + 6, sizeof shell);
                    shell[sizeof shell - 1] = '\0';
                    p = strchr(shell, '\r');
                    if (p != NULL)
                    {
                        *p = '\0';
                    }
                }
                /* Sets if the Last access date should be recorded.
                 * Syntax is "[drive][sign]", '+' for yes,
                 * '-' for no, '[drive]' is the drive letter.
                 * Default option is no.
                 * Only the last entry and the last sign
                 * for drive are important.
                 * Invalid values or combinations are ignored. */
                else if (memcmp(buf + x, "ACCDATE=", 8) == 0)
                {
                    int drive = -1;

                    for (;
                         ((buf[x] != '\r') &&
                          (buf[x] != '\n') &&
                          (x < readbytes));
                         x++)
                    {
                        if ((buf[x] >= 'a' && buf[x] <= 'z') ||
                            (buf[x] >= 'A' && buf[x] <= 'Z'))
                        {
                            drive = toupper((unsigned char)buf[x]) - 'A';
                        }
                        else if (buf[x] == '+')
                        {
                            if (drive >= 0 && drive < MAXDISKS)
                            {
                                disks[drive].fat.last_access_recording = 1;
                            }
                        }
                        else if (buf[x] == '-')
                        {
                            if (drive >= 0 && drive < MAXDISKS)
                            {
                                disks[drive].fat.last_access_recording = 0;
                            }
                        }
                    }
                }
                /*pdosWriteText(buf[x]);*/
            }
        } while (readbytes == 0x200);
        fileClose(fh);
    }
    return;
}

static void loadPcomm(void)
{
    static POSEXEC_PARMBLOCK p = { 0, "\x2/p\r", NULL, NULL };
    static POSEXEC_PARMBLOCK altp = { 0, "\x0\r", NULL, NULL };
    POSEXEC_PARMBLOCK *pb = &altp;
#ifdef __32BIT__
    char parm[FILENAME_MAX + 20];
#endif

    kernel32[0] = msvcrt[0] = alphabet[bootDriveLogical];
    if (strcmp(shell, "") == 0)
    {
        strcpy(shell,"?:\\COMMAND.COM");
        shell[0] = alphabet[bootDriveLogical];
        pb = &p;
#ifdef __32BIT__
        strcpy(parm, shell);
        strcat(parm, " /p"); /* no \r required */
        pb->cmdtail = parm;
#endif
    }
    loadExe(shell, pb);
    return;
}

/* loadExe - load an executable into memory */
/* 1. read first 10 bytes of header to get header len */
/* 2. allocate room for header */
/* 3. load rest of header */
/* 4. allocate room for environment */
/* 5. allocate room for executable + psp */
/* 6. load executable */
/* 7. fix up executable using relocation table */
/* 8. extract SS + SP from header */
/* 9. call executable */

/* for 32-bit we make the following mods: */
/* 1. header is larger */
/* 6. executable needs 2-stage load */

static void loadExe(char *prog, POSEXEC_PARMBLOCK *parmblock)
{
#ifdef __32BIT__
    loadExe32(prog, parmblock, SYNCHRONOUS);
    return;
#else
    unsigned char firstbit[10];
    unsigned int headerLen;
    unsigned char *header = NULL;
    unsigned char *envptr;
    unsigned long exeLen;
    unsigned long extraLen = 0;
    unsigned char *psp;
    unsigned char *pcb;
    PDOS_PROCESS *newProc;
    unsigned char *exeStart;
    unsigned char *old_loadaddr;
    unsigned int numReloc;
    unsigned long *relocStart;
    unsigned int relocI;
    unsigned int *fixSeg;
    unsigned int addSeg;
    unsigned int ss;
    unsigned int sp;
    unsigned char *exeEntry;
    unsigned char *old_entry_point;
    unsigned int maxPages;
    int fno;
    size_t readbytes;
    int ret;
    unsigned char *bss;
    int isexe = 0;
    DTA *olddta;

    if (fileOpen(prog, &fno)) return;
    fileRead(fno, firstbit, sizeof firstbit, &readbytes);
    if (memcmp(firstbit, "MZ", 2) == 0)
    {
        isexe = 1;
        headerLen = *(unsigned int *)&firstbit[8];
        headerLen *= 16;
        header = memmgrAllocate(&memmgr, headerLen, 0);
        memcpy(header, firstbit, sizeof firstbit);
        fileRead(fno, header + sizeof firstbit, headerLen - sizeof firstbit,
                 &readbytes);
    }

    /* If curPCB == NULL, we are launching init process (PCOMM),
     * setup initial environment.
     */
    if (curPCB == NULL)
    {
        char sBOOTDRIVE[2];
        sBOOTDRIVE[0] = 'A' + bootDriveLogical;
        sBOOTDRIVE[1] = 0;
        envptr = envAllocateEmpty(prog);
        envptr = envModify(envptr, "COMSPEC", prog);
        envptr = envModify(envptr, "BOOTDRIVE", sBOOTDRIVE);
    }
    else
    {
        /* Launching a child process, copy parent's environment */
        envptr = envCopy(curPCB->envBlock,prog);
    }

    if (isexe)
    {
        exeLen = *(unsigned int *)&header[4];
        extraLen = *(unsigned short *)&header[0x0a] * 16UL;
        exeLen = (exeLen + 1) * 512 - headerLen;
    }
    else
    {
        exeLen = 0x10000UL;
    }
    extraLen += 0x100; /* PSP */
    extraLen += PDOS_PROCESS_SIZE;
    extraLen += 16; /* for good measure and to allow division by 16 */
    maxPages = memmgrMaxSize(&memmgr);
    if (((unsigned long)maxPages * 16) < (exeLen+extraLen))
    {
        printf("insufficient memory to load program\n");
        printf("required %lu, available %lu\n",
            exeLen+extraLen, (unsigned long)maxPages * 16);
        memmgrFree(&memmgr, header);
        memmgrFree(&memmgr, envptr);
        return;
    }
    pcb = pdos16MemmgrAllocPages(&memmgr, (exeLen + extraLen) / 16, 0);
    if (pcb != NULL)
    {
        psp = pcb + PDOS_PROCESS_SIZE;
        psp = (unsigned char *)FP_NORM(psp);
        newProc = (PDOS_PROCESS*)pcb;
        initPCB(newProc, FP_SEG(psp), prog, envptr);
        memmgrSetOwner(&memmgr, pcb, FP_SEG(psp));
        memmgrSetOwner(&memmgr, envptr, FP_SEG(psp));
        memmgrSetOwner(&memmgr, header, FP_SEG(psp));
    }
    psp = (unsigned char *)FP_NORM(psp);
    if (pcb == NULL)
    {
        printf("insufficient memory to load program\n");
        if (header != NULL) memmgrFree(&memmgr, header);
        memmgrFree(&memmgr, envptr);
        fileClose(fno);
        return;
    }

    /* set various values in the psp */
    psp[0] = 0xcd;
    psp[1] = 0x20;
    *(unsigned int *)&psp[0x2c] = FP_SEG(envptr);
    /* set to say 640k in use */
    *(unsigned int *)(psp + 2) = FP_SEG(psp) +
            (maxPages-PDOS_PROCESS_SIZE_PARAS);
    if (parmblock != NULL)
    {
        /* 1 for the count and 1 for the return */
        memcpy(psp + 0x80, parmblock->cmdtail, parmblock->cmdtail[0] + 1 + 1);
    }

    exeStart = psp + 0x100;
    exeStart = (unsigned char *)FP_NORM(exeStart);

    if (isexe)
    {
        unsigned int maxread = 32768;
        unsigned long totalRead = 0;
        char *upto;

        while (totalRead < exeLen)
        {
            if ((exeLen - totalRead) < maxread)
            {
                maxread = exeLen - totalRead;
            }
            upto = ABS2ADDR(ADDR2ABS(exeStart) + totalRead);
            upto = FP_NORM(upto);
            fileRead(fno, upto, maxread, &readbytes);
            totalRead += maxread;
        }
    }
    else
    {
        memcpy(exeStart, firstbit, sizeof firstbit);
        fileRead(fno, exeStart + sizeof firstbit, 32768, &readbytes);
        fileRead(fno, FP_NORM(exeStart + sizeof firstbit + 32768), 32768,
                 &readbytes);
    }
    fileClose(fno);

    if (isexe)
    {
        numReloc = *(unsigned int *)&header[6];
        relocStart = (unsigned long *)(header +
                *(unsigned int *)&header[0x18]);
        addSeg = FP_SEG(exeStart);
        for (relocI = 0; relocI < numReloc; relocI++)
        {
            /* This 16:16 arithmetic will work because the exeStart
            offset is 0. */
            fixSeg = (unsigned int *)
                    ((unsigned long)exeStart + relocStart[relocI]);
            *fixSeg = *fixSeg + addSeg;
        }

        ss = *(unsigned int *)&header[0xe];
        ss += addSeg;
        sp = *(unsigned int *)&header[0x10];
        if (sp < 0x400)
        {
            printf("ridiculously low sp of %x being converted to 0x400\n",
                   sp);
            sp = 0x400;
        }

        /* This 16:16 arithmetic will work because the exeStart
        offset is 0 */
        exeEntry = (unsigned char *)((unsigned long)exeStart
                                    + *(unsigned long *)&header[0x14]);
    }
    else
    {
        ss = FP_SEG(psp);
        sp = 0xfffe;
        *(unsigned int *)MK_FP(ss, sp) = 0;
        exeEntry = psp + 0x100;
    }

    /* printf("exeEntry: %lx, psp: %lx, ss: %x, sp: %x\n",
        exeEntry, psp, ss, sp); */

    /* Before executing program, set tsrFlag = 0.
     * If program is a TSR, it will change this to non-zero.
     */
    tsrFlag = 0;

    /* Add this process to the process chain */
    addToProcessChain(newProc);

    /* Set this process as the running process */
    curPCB = newProc;

#if (!defined(USING_EXE))
    olddta = dta;
    dta = (DTA *)(psp + 0x80);
    newProc->status = PDOS_PROCSTATUS_ACTIVE;
    if (newProc->parent != NULL)
        newProc->parent->status = PDOS_PROCSTATUS_CHILDWAIT;
    old_loadaddr = loadaddr;
    old_entry_point = entry_point;
    loadaddr = exeStart;
    entry_point = exeEntry;
    memId += 256;
    ret = callwithpsp(exeEntry, psp, ss, sp);
    /* printf("ret is %x\n", ret); */
    memmgrFreeId(&memmgr, memId);
    memId -= 256;
    entry_point = old_entry_point;
    loadaddr = old_loadaddr;
    dta = olddta;
#endif
    lastrc = ret;

    /* Process finished, parent becomes current */
    curPCB = newProc->parent;
    if (curPCB != NULL && curPCB->status == PDOS_PROCSTATUS_CHILDWAIT)
        curPCB->status = PDOS_PROCSTATUS_ACTIVE;

    /*
     * Don't free all of program's memory if it is a TSR. Only the
     * "un-reserved" portion.
     */
    if (tsrFlag == 0)
    {
        newProc->status = PDOS_PROCSTATUS_TERMINATED;
        if (header != NULL) memmgrFree(&memmgr, header);
        memmgrFree(&memmgr, newProc->envBlock);
        removeFromProcessChain(newProc);
        memmgrFree(&memmgr, pcb);
    }
    else
    {
        /* PDOS EXTENSION: tsrFlag=-1 means free no memory */
        if (tsrFlag != -1)
        {
            PosReallocPages(pcb, tsrFlag, &maxPages);
        }
        /* Mark process as a TSR. */
        newProc->status = PDOS_PROCSTATUS_TSR;
    }

    /* Set TSR flag back to 0 */
    tsrFlag = 0;

    return;
#endif
}

#ifdef __32BIT__
#include "a_out.h"
#include "elf.h"
#include "exeload.h"

static void loadExe32(char *prog, POSEXEC_PARMBLOCK *parmblock, int synchronous)
{
    char *commandLine = "";
    char *envptr;
    PDOS_PROCESS *newProc;
    char *ct = "";

    /* Allocates kernel memory and stores the command line string in it. */
    if (parmblock)
    {
        ct = parmblock->cmdtail;
    }
    commandLine = kmalloc(strlen(ct) + 1);
    if (commandLine == NULL)
    {
        printf("insufficient memory to allocate parameter block\n");
        return;
    }
    strcpy(commandLine, ct);

    /* If curPCB == NULL, we are launching init process (PCOMM),
     * setup initial environment.
     */
    if (curPCB == NULL)
    {
        char sBOOTDRIVE[2];
        sBOOTDRIVE[0] = alphabet[bootDriveLogical];
        sBOOTDRIVE[1] = 0;
        envptr = envAllocateEmpty(prog);
        envptr = envModify(envptr, "COMSPEC", prog);
        envptr = envModify(envptr, "BOOTDRIVE", sBOOTDRIVE);
    }
    else
    {
        /* Launching a child process, copy parent's environment */
        envptr = envCopy(curPCB->envBlock, prog);
    }

    newProc = (PDOS_PROCESS*)kmalloc(sizeof(PDOS_PROCESS));
    initPCB(newProc, (unsigned long)newProc, prog, envptr);

    /* Stores the pointer to the command line string in the new PCB. */
    newProc->commandLine = commandLine;

    /* Creates a new VMM with new address space. */
    newProc->vmm = kmalloc(sizeof(VMM));
    vmmInit(newProc->vmm, &physmemmgr);

    /* Add this process to the process chain */
    {
        unsigned int savedEFLAGS = getEFLAGSAndDisable();

        addToProcessChain(newProc);

        setEFLAGS(savedEFLAGS);
    }

    newProc->status = PDOS_PROCSTATUS_ACTIVE;
    if (newProc->parent != NULL)
        newProc->parent->status = PDOS_PROCSTATUS_CHILDWAIT;

    if (synchronous) createThreadAndBlock(startExe32, newProc);
    else createThread(startExe32, newProc);
}

static void startExe32(void)
{
    unsigned long old_entry_point;
    char *old_loadaddr;
    int ret;
    char *prog;

    memId += 256;
    /* Tells the new VMM what address range can it use. */
    vmmSupply(curPCB->vmm, (void *)PROCESS_SPACE_START, PROCESS_SPACE_SIZE);

    {
        /* Obtains the path to the executable
         * from the command line string stored in the current PCB. */
        char *a;
        size_t progLen;

        a = strchr(curPCB->commandLine, ' ');
        if (a)
        {
            progLen = a - (curPCB->commandLine);
        }
        else
        {
            progLen = strlen(curPCB->commandLine);
        }

        prog = kmalloc(progLen + 1);
        if (prog == NULL)
        {
            printf("Not enough memory for storing executable path\n");
            terminateExe32();
        }
        memcpy(prog, curPCB->commandLine, progLen);
        prog[progLen] = '\0';
    }

    old_entry_point = entry_point;
    old_loadaddr = loadaddr;
    ret = exeloadDoload(&entry_point, prog);
    if (ret == 0)
    {
        /* Program has been successfully loaded,
         * new stack in the process space is created
         * and the program is launched. */
        void *newsp;

        /* Creates new stack and sets the stack pointer to its top. */
        newsp = PosVirtualAlloc(0, STACKSZ32);
        newsp = ((char *)newsp) + STACKSZ32;

        ret = fixexe32(entry_point, (unsigned int)newsp, 0, 0);
        lastrc = ret;
    }
    else
    {
        lastrc = -1;
    }
    loadaddr = old_loadaddr;
    entry_point = old_entry_point;

    terminateExe32();
}

static void terminateExe32(void)
{
    PDOS_PROCESS *newProc = curPCB;

    /* Process finished, parent becomes current */
    curPCB = newProc->parent;
    if (curPCB != NULL && curPCB->status == PDOS_PROCSTATUS_CHILDWAIT)
        curPCB->status = PDOS_PROCSTATUS_ACTIVE;

    /* Terminates the process. */
    newProc->status = PDOS_PROCSTATUS_TERMINATED;
    /* Frees the process address space. */
#ifndef NOVM
    vmmFree(newProc->vmm, (void *)PROCESS_SPACE_START, PROCESS_SPACE_SIZE);
#else
    memmgrFreeId(&physmemmgr, memId);
#endif
    memId -= 256;

    {
        /* No more process memory will be handled,
         * so this thread will use kernel page directory. */
        unsigned int savedEFLAGS = getEFLAGSAndDisable();

        vmmCopyCurrentMapping(&kernel_vmm,
                              (void *)KERNEL_SPACE_START,
                              KERNEL_SPACE_SIZE);
        loadPageDirectory(kernel_vmm.pd_physaddr);
        curPCB = NO_PROCESS;
        curTCB->pcb = NO_PROCESS;

        setEFLAGS(savedEFLAGS);
    }

    {
        unsigned int savedEFLAGS = getEFLAGSAndDisable();

        removeFromProcessChain(newProc);

        setEFLAGS(savedEFLAGS);
    }

    /* Terminates the VMM belonging to the terminated process. */
    vmmTerm(newProc->vmm);
    kfree(newProc->vmm);

    kfree(newProc->commandLine);
    kfree(newProc->envBlock);
    kfree(newProc);

    terminateThread();
}

static int fixexe32(unsigned long entry, unsigned int sp,
                    unsigned long exeStart, unsigned long dataStart)
{
    struct {
        unsigned short limit_15_0;
        unsigned short base_15_0;
        unsigned char base_23_16;
        unsigned char access;
        unsigned char gran_limit;
        unsigned char base_31_24;
    } *realcode, savecode, *realdata, savedata;
    int ret;

    /* now we set the GDT entry directly */
    /* after first of all saving the old values */
    realcode = (void *)((char *)gdt + 0x28);
    savecode = *realcode;
    realcode->base_15_0 = exeStart & 0xffff;
    realcode->base_23_16 = (exeStart >> 16) & 0xff;
    realcode->base_31_24 = (exeStart >> 24) & 0xff;

    /* now set the data values */
    realdata = (void *)((char *)gdt + 0x30);
    savedata = *realdata;
    realdata->base_15_0 = dataStart & 0xffff;
    realdata->base_23_16 = (dataStart >> 16) & 0xff;
    realdata->base_31_24 = (dataStart >> 24) & 0xff;

    ret = call32(entry, sp, curTCB);
    /* printf("ret is %x\n", ret); */

    *realcode = savecode;
    *realdata = savedata;
    return (ret);
}
#endif

static int fileCreat(const char *fnm, int attrib, int *handle)
{
    int x;
    const char *p;
    int drive;
    int rc;

    p = strchr(fnm, ':');
    if (p == NULL)
    {
        p = fnm;
        drive = currentDrive;
    }
    else
    {
        drive = *(p - 1);
        drive = toupper(drive) - 'A';
        p++;
    }
    for (x = NUM_SPECIAL_FILES; x < MAXFILES; x++)
    {
        if (!fhandle[x].inuse)
        {
            break;
        }
    }
    if (x == MAXFILES) return (-4); /* 4 = too many open files */
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatCreatFile(&disks[drive].fat, p, &fhandle[x].fatfile, attrib);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    if (rc) return (rc);
    fhandle[x].inuse = 1;
    fhandle[x].fatptr = &disks[drive].fat;
    *handle = x;
    return (0);
}

static int isDriveValid(int drive)
{
    return drive >= 0 && drive < MAXDISKS && disks[drive].valid;
}

static int dirCreat(const char *dnm, int attrib)
{
    const char *p;
    int drive;
    int rc;
    char parentname[MAX_PATH];
    char *end;
    char *temp;

    p = strchr(dnm, ':');
    if (p == NULL)
    {
        p = dnm;
        drive = currentDrive;
    }
    else
    {
        drive = *(p - 1);
        drive = toupper(drive) - 'A';
        p++;
    }

    if ((p[0] == '\\') || (p[0] == '/'))
    {
        p++;
    }

    memset(parentname,'\0',sizeof(parentname));
    end = strrchr(p, '\\');
    temp = strrchr(p, '/');

    if(!end || (temp > end))
    {
        end = temp;
    }
    if(end)
    {
        strncpy(parentname,p,(end-p));
    }
    if (!isDriveValid(drive))
        return POS_ERR_INVALID_DRIVE;
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatCreatDir(&disks[drive].fat, p, parentname, attrib);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    return (rc);
}

static int newFileCreat(const char *fnm, int attrib, int *handle)
{
    int x;
    const char *p;
    int drive;
    int rc;

    p = strchr(fnm, ':');
    if (p == NULL)
    {
        p = fnm;
        drive = currentDrive;
    }
    else
    {
        drive = *(p - 1);
        drive = toupper(drive) - 'A';
        p++;
    }
    for (x = NUM_SPECIAL_FILES; x < MAXFILES; x++)
    {
        if (!fhandle[x].inuse)
        {
            break;
        }
    }
    if (x == MAXFILES) return (-POS_ERR_MANY_OPEN_FILES);
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatCreatNewFile(&disks[drive].fat, p, &fhandle[x].fatfile, attrib);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    if (rc) return (rc);
    fhandle[x].inuse = 1;
    fhandle[x].fatptr = &disks[drive].fat;
    *handle = x;
    return (0);
}

static int fileOpen(const char *fnm, int *handle)
{
    int x;
    const char *p;
    int drive;
    int rc;

    p = strchr(fnm, ':');
    if (p == NULL)
    {
        p = fnm;
        drive = currentDrive;
    }
    else
    {
        drive = *(p - 1);
        drive = toupper(drive) - 'A';
        p++;
    }
    for (x = NUM_SPECIAL_FILES; x < MAXFILES; x++)
    {
        if (!fhandle[x].inuse)
        {
            break;
        }
    }
    if (x == MAXFILES) return (-POS_ERR_MANY_OPEN_FILES);
    if (!isDriveValid(drive))
        return POS_ERR_INVALID_DRIVE;
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatOpenFile(&disks[drive].fat, p, &fhandle[x].fatfile);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    if (rc) return (rc);
    fhandle[x].inuse = 1;
    fhandle[x].fatptr = &disks[drive].fat;
    *handle = x;
    return (0);
}

static int opencomm(int num, int *handle)
{
    int x;

    if (num == 0) return (-1); /* +++ need proper return value */
    for (x = NUM_SPECIAL_FILES; x < MAXFILES; x++)
    {
        if (!fhandle[x].inuse)
        {
            break;
        }
    }
    if (x == MAXFILES) return (-POS_ERR_MANY_OPEN_FILES);
    fhandle[x].inuse = 1;
    fhandle[x].comm = num;
    *handle = x;
    return (0);
}

static int fileClose(int fno)
{
    if (fhandle[fno].comm)
    {
        fhandle[fno].comm = 0;
        fhandle[fno].inuse = 0;
    }
    else if (!fhandle[fno].special)
    {
        fhandle[fno].inuse = 0;
    }
    return (0);
}

static int fileRead(int fno, void *buf, size_t szbuf, size_t *readbytes)
{
    int ret;

#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    ret = fatReadFile(fhandle[fno].fatptr, &fhandle[fno].fatfile, buf,
                      szbuf, readbytes);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif

    return (ret);
}


static int fileWrite(int fno, const void *buf, size_t szbuf,
                     size_t *writtenbytes)
{
    int ret;

#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    ret = fatWriteFile(fhandle[fno].fatptr, &fhandle[fno].fatfile, buf,
                       szbuf, writtenbytes);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif

    return (ret);
}

/*Fatdelete function to delete a file when fnm is given as filename*/
static int fileDelete(const char *fnm)
{
    const char *p;
    int drive;
    int rc;
    int attr;

    p = strchr(fnm, ':');
    if (p == NULL)
    {
        p = fnm;
        drive = currentDrive;
    }
    else
    {
        drive = *(p - 1);
        drive = toupper(drive) - 'A';
        p++;
    }

    /* Checks if the file that should be deleted is not a directory.
     * Directories must be deleted using dirDelete instead. */
    if (!isDriveValid(drive))
        return POS_ERR_INVALID_DRIVE;
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatGetFileAttributes(&disks[drive].fat, p, &attr);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    if (rc || (attr & DIRENT_SUBDIR)) return (POS_ERR_FILE_NOT_FOUND);

#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatDeleteFile(&disks[drive].fat, p);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    if (rc != 0) return (-1);
    return (rc);
}

static int dirDelete(const char *dnm)
{
    const char *p;
    int drive;
    int rc;
    int attr;
    int dotcount = 0;
    int fh;
    size_t readbytes;
    DIRENT dirent;

    p = strchr(dnm, ':');
    if (p == NULL)
    {
        p = dnm;
        drive = currentDrive;
    }
    else
    {
        drive = *(p - 1);
        drive = toupper(drive) - 'A';
        p++;
    }

    if (!isDriveValid(drive))
        return POS_ERR_INVALID_DRIVE;
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatGetFileAttributes(&disks[drive].fat, p, &attr);
    if (rc || !(attr & DIRENT_SUBDIR))
    {
#ifdef __32BIT__
        unlockMutex(&fatMutex);
#endif
        return (POS_ERR_PATH_NOT_FOUND);
    }

    if (drive == currentDrive)
    {
        if (strcmp(disks[drive].fat.corrected_path,cwd) == 0)
        {
#ifdef __32BIT__
            unlockMutex(&fatMutex);
#endif
            return (POS_ERR_ATTEMPTED_TO_REMOVE_CURRENT_DIRECTORY);
        }
    }
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif

    fileOpen(dnm, &fh);

    fileRead(fh, &dirent, sizeof dirent, &readbytes);
    while ((readbytes == sizeof dirent) && (dirent.file_name[0] != '\0'))
    {
        /* LFNs should be ignored when checking if the directory is empty. */
        if (dirent.file_name[0] != DIRENT_DEL &&
            dirent.file_attr != DIRENT_LFN)
        {
            if (dirent.file_name[0] == DIRENT_DOT) dotcount++;
            if (dirent.file_name[0] != DIRENT_DOT || dotcount > 2)
            {
                fileClose(fh);
                return (POS_ERR_PATH_NOT_FOUND);
            }
        }
        fileRead(fh, &dirent, sizeof dirent, &readbytes);
    }
    fileClose(fh);

#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    rc = fatDeleteFile(&disks[drive].fat, p);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    if (rc == POS_ERR_FILE_NOT_FOUND) return (POS_ERR_PATH_NOT_FOUND);
    return (rc);
}

static int fileSeek(int fno, long offset, int whence, long *newpos)
{
    /* Checks if whence value is valid. */
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END)
    {
        /* MSDOS returns 0x1 (function number invalid). */
        return (POS_ERR_FUNCTION_NUMBER_INVALID);
    }
#ifdef __32BIT__
    lockMutex(&fatMutex);
#endif
    *newpos = fatSeek(fhandle[fno].fatptr, &(fhandle[fno].fatfile),
                      offset, whence);
#ifdef __32BIT__
    unlockMutex(&fatMutex);
#endif
    return (0);
}
/**/

/**/
static void accessDisk(int drive)
{
    unsigned char buf[512];
    int rc;
    int sectors = 1;
    int track = 0;
    int head = 0;
    int sector = 1;
    unsigned char *bpb;

    rc = readAbs(buf,
                sectors,
                drive,
                track,
                head,
                sector);
    if (rc != 0)
    {
        return;
    }
    bpb = buf + 11;
    if (disks[drive].accessed)
    {
        fatTerm(&disks[drive].fat);
    }
    analyseBpb(&disks[drive], bpb);
    disks[drive].lba = 0;
    fatDefaults(&disks[drive].fat);
    fatInit(&disks[drive].fat, bpb, readLogical, writeLogical, &disks[drive],
            getDateTime);
    strcpy(disks[drive].cwd, "");
    disks[drive].accessed = 1;
    disks[drive].valid = 1;
    return;
}

void dumplong(unsigned long x)
{
    int y;
    char *z = "0123456789abcdef";
    char buf[9];

    for (y = 0; y < 8; y++)
    {
        buf[7 - y] = z[x & 0x0f];
        x /= 16;
    }
    buf[8] = '\0';
    dumpbuf(buf, 8);
    return;
}

void dumpbuf(unsigned char *buf, int len)
{
    int x;

    for (x = 0; x < len; x++)
    {
        pdosWriteText(buf[x]);
    }
    return;
}

static void readLogical(void *diskptr, unsigned long sector, void *buf)
{
    int track;
    int head;
    int sect;
    DISKINFO *diskinfo;
    int ret;

    diskinfo = (DISKINFO *)diskptr;
    sector += diskinfo->hidden;
    track = sector / diskinfo->sectors_per_cylinder;
    head = sector % diskinfo->sectors_per_cylinder;
    sect = head % diskinfo->sectors_per_track + 1;
    head = head / diskinfo->sectors_per_track;
    if (diskinfo->lba)
    {
        ret = readLBA(buf, 1, diskinfo->drive, sector);
    }
    else
    {
        ret = readAbs(buf, 1, diskinfo->drive, track, head, sect);
    }
    if (ret != 0)
    {
        printf("failed to read sector %ld - halting\n", sector);
        for (;;) ;
    }
    return;
}

static void writeLogical(void *diskptr, unsigned long sector, void *buf)
{
    int track;
    int head;
    int sect;
    DISKINFO *diskinfo;
    int ret;

    diskinfo = (DISKINFO *)diskptr;
    sector += diskinfo->hidden;
    track = sector / diskinfo->sectors_per_cylinder;
    head = sector % diskinfo->sectors_per_cylinder;
    sect = head % diskinfo->sectors_per_track + 1;
    head = head / diskinfo->sectors_per_track;
    if (diskinfo->lba)
    {
        ret = writeLBA(buf, 1, diskinfo->drive, sector);
    }
    else
    {
        ret = writeAbs(buf, 1, diskinfo->drive, track, head, sect);
    }
    if (ret != 0)
    {
        printf("failed to write sector %ld - halting\n", sector);
        for (;;) ;
    }
    return;
}

static int readAbs(void *buf,
                int sectors,
                int drive,
                int track,
                int head,
                int sect)
{
    int rc;
    int ret = -1;
    int tries;
#ifdef __32BIT__
    void *readbuf = transferbuf;
#else
    void *readbuf = buf;
#endif

    unused(sectors);
    tries = 0;
    while (tries < 10)
    {
        rc = BosDiskSectorRead(readbuf, 1, drive, track, head, sect);
        if (rc == 0)
        {
#ifdef __32BIT__
            memcpy(buf, transferbuf, 512);
#endif
            ret = 0;
            break;
        }
        BosDiskReset(drive);
        tries++;
    }
    return (ret);
}

static int readLBA(void *buf,
                int sectors,
                int drive,
                unsigned long sector)
{
    int rc;
    int ret = -1;
    int tries;
#ifdef __32BIT__
    void *readbuf = transferbuf;
#else
    void *readbuf = buf;
#endif

    unused(sectors);
    tries = 0;
    while (tries < 5)
    {
        rc = BosDiskSectorRLBA(readbuf, 1, drive, sector, 0);
        if (rc == 0)
        {
#ifdef __32BIT__
            memcpy(buf, transferbuf, 512);
#endif
            ret = 0;
            break;
        }
        BosDiskReset(drive);
        tries++;
    }
    return (ret);
}

static int writeAbs(void *buf,
                    int sectors,
                    int drive,
                    int track,
                    int head,
                    int sect)
{
    int rc;
    int ret = -1;
    int tries;
#ifdef __32BIT__
    void *writebuf = transferbuf;
#else
    void *writebuf = buf;
#endif

    unused(sectors);
#ifdef __32BIT__
    memcpy(transferbuf, buf, 512);
#endif
    tries = 0;
    while (tries < 5)
    {
        rc = BosDiskSectorWrite(writebuf, 1, drive, track, head, sect);
        if (rc == 0)
        {
            ret = 0;
            break;
        }
        BosDiskReset(drive);
        tries++;
    }
    return (ret);
}

static int writeLBA(void *buf,
                    int sectors,
                    int drive,
                    unsigned long sector)
{
    int rc;
    int ret = -1;
    int tries;
#ifdef __32BIT__
    void *writebuf = transferbuf;
#else
    void *writebuf = buf;
#endif

    unused(sectors);
#ifdef __32BIT__
    memcpy(transferbuf, buf, 512);
#endif
    tries = 0;
    while (tries < 5)
    {
        rc = BosDiskSectorWLBA(writebuf, 1, drive, sector, 0);
        if (rc == 0)
        {
            ret = 0;
            break;
        }
        BosDiskReset(drive);
        tries++;
    }
    return (ret);
}

static void analyseBpb(DISKINFO *diskinfo, unsigned char *bpb)
{
    diskinfo->drive = bpb[25];
    diskinfo->num_heads = bpb[15];
    diskinfo->hidden = bpb[17]
                    | ((unsigned long)bpb[18] << 8)
                    | ((unsigned long)bpb[19] << 16)
                    | ((unsigned long)bpb[20] << 24);
    diskinfo->sectors_per_track = (bpb[13] | ((unsigned int)bpb[14] << 8));
    diskinfo->sectors_per_cylinder = diskinfo->sectors_per_track
                                    * diskinfo->num_heads;
    memcpy(&diskinfo->bpb, bpb, sizeof diskinfo->bpb);
    return;
}

#ifdef __32BIT__
extern int __minstart;

int pdosstrt(void)
{
    pdos_parms *pp;

    /* Tells __start() to not use any API calls. */
    __minstart = 1;
    gdt = (void *)(rp_parms->gdt);
    freem_start = rp_parms->freem_start;
    pp = (void *)__userparm;
    transferbuf = (void *)(pp->transferbuf);
    doreboot = pp->doreboot;
    dopoweroff = pp->dopoweroff;
    bootBPB = (void *)(pp->bpb);
    protintHandler(0x0, int0);
    protintHandler(0x0E, int0E);
    protintHandler(0x20, int20);
    protintHandler(0x21, int21);
    protintHandler(0x25, int25);
    protintHandler(0x26, int26);
    protintHandler(0x80, int80);
    return (__start(0));
    return (0);
}
#endif

#ifndef __32BIT__
static void *pdos16MemmgrAllocate(MEMMGR *memmgr, size_t bytes, int id)
{
    size_t pages;

    /* we need to round up bytes to nearest page */
    pages = bytes / 16 + ((bytes & 0x0f) != 0 ? 1 : 0);
    return (pdos16MemmgrAllocPages(memmgr, pages, id));
}

static void pdos16MemmgrFree(MEMMGR *memmgr, void *ptr)
{
    unsigned long abs;

    abs = ADDR2ABS(ptr);
    abs -= 0x10000UL;
    abs -= (unsigned long)PDOS16_MEMSTART * 16;
    abs /= 16;
    /* ignore strange free requests */
    if (abs > 0x6000U)
    {
        return;
    }
    abs += (unsigned long)PDOS16_MEMSTART * 16;
    ptr = ABS2ADDR(abs);
    (memmgrFree)(memmgr, ptr);
    return;
}

static void *pdos16MemmgrAllocPages(MEMMGR *memmgr, size_t pages, int id)
{
    void *ptr;
    unsigned long abs;

    /* I don't know why some apps request 0 pages, but we'd better
    give them a decent pointer. */
    if (pages == 0)
    {
        pages = 1;
    }
    ptr = (memmgrAllocate)(memmgr, pages, id);
    if (ptr == NULL)
    {
        return (ptr);
    }
    abs = ADDR2ABS(ptr);

    /* and because we wasted 0x10000 for control blocks, we
    skip that, and the bit above 3000 we multiply by 16. */
    abs -= (unsigned long)PDOS16_MEMSTART * 16;
    abs *= 16;
    abs += (unsigned long)PDOS16_MEMSTART * 16;
    abs += 0x10000UL;
    ptr = ABS2ADDR(abs);
    ptr = FP_NORM(ptr);
    return (ptr);
}

static void pdos16MemmgrFreePages(MEMMGR *memmgr, size_t page)
{
    void *ptr;

    ptr = MK_FP(page, 0);
    pdos16MemmgrFree(memmgr, ptr);
    return;
}

static int pdos16MemmgrReallocPages(MEMMGR *memmgr,
                                    void *ptr,
                                    size_t newpages)
{
    unsigned long abs;
    int ret;

    abs = ADDR2ABS(ptr);
    abs -= 0x10000UL;
    abs -= (unsigned long)PDOS16_MEMSTART * 16;
    abs /= 16;
    abs += (unsigned long)PDOS16_MEMSTART * 16;
    ptr = ABS2ADDR(abs);
    ret = (memmgrRealloc)(memmgr, ptr, newpages);
    return (ret);
}

static void pdos16MemmgrSetOwner(MEMMGR *memmgr, void *ptr,
                                 unsigned long owner)
{
    unsigned long abs;

    abs = ADDR2ABS(ptr);
    abs -= 0x10000UL;
    abs -= (unsigned long)PDOS16_MEMSTART * 16;
    abs /= 16;
    abs += (unsigned long)PDOS16_MEMSTART * 16;
    ptr = ABS2ADDR(abs);
    (memmgrSetOwner)(memmgr, ptr, owner);
    return;
}

#endif

/*int 25 function call*/
unsigned int PosAbsoluteDiskRead(int drive, unsigned long start_sector,
                                unsigned int sectors, void *buf)
{
    long x;
    for(x=0;x<sectors;x++)
    {
    readLogical(&disks[drive],x,(char *)buf+x*512);
    }
    return(0);
}
/**/

/*int 26 function call*/
unsigned int PosAbsoluteDiskWrite(int drive, unsigned long start_sector,
                                unsigned int sectors, void *buf)
{
    long x;
    for(x=0;x<sectors;x++)
    {
    writeLogical(&disks[drive],x,(char *)buf+x*512);
    }
    return(0);
}
/**/

/*
 Different cases for cwd. The user can input
 directory name, file name in a format convenient to
 user. In each case the cwd must prepended or appended
 with the appropriate directory name, drive name and current
 working directory, ending '\' or '/' must be removed
 and dotdots ("..") must be resolved.
*/
static int formatcwd(const char *input,char *output)
{
    char *p;
    char *temp;
    char *read;
    char *write;

    /* The user provided '.'. */
    if (input[0] == '.')
    {
        if (input[1] == '\\' || input[1] == '/')
        {
            input += 2;
        }
        else if (input[1] == '\0')
        {
            input++;
        }
    }
   /*
     The user only provides the <folder-name>
     e.g. \from\1.txt the function corrects it to
     e.g. c:\from\1.txt.
     */
    if (input[0] == '\\')
    {
        output[0] = 'A' + currentDrive;
        strcpy(output + 1,":");
        strcat(output,input);
    }

    /*
     The user provides the file name in full format
     e.g. c:\from\1.txt
     */
    else if((strlen(input) >= 3)
            && (memcmp(input + 1, ":\\", 2) == 0)
           )
    {
        strcpy(output,input);
    }

    /*
     The user misses the '\' e.g. c:1.txt.
     */
    else if (strlen(input) >= 3 && input[1] == ':'
             && input[2] != '\\')
    {
        char *cwd;

        memcpy(output, input, 2);
        memcpy(output + 2, "\\", 2);
        /*
         The current working directory must be fetched from
         the disk array so that the working directory does
         not change on drive switch.
         */
        cwd = disks[toupper((unsigned char)(input[0]))-'A'].cwd;
        strcat(output,cwd);
        if(strcmp(cwd, "") != 0)
        {
            strcat(output, "\\");
        }
        strcat(output, input + 2);
    }

    /*
     The user provides only the <file-name>
     e.g. 1.txt in that case the drive name,'\'
     and currect working directory needs to be
     prepended e.g. c:\from\1.txt.
     */
    else
    {
        output[0] = 'A' + currentDrive;
        strcpy(output + 1,":");
        strcat(output,"\\");
        strcat(output,cwd);
        if(strcmp(cwd,"")!= 0)
        {
            strcat(output,"\\");
        }
        strcat(output,input);
    }
    tempDrive=toupper((unsigned char)(output[0])) - 'A';
    if (!isDriveValid(tempDrive))
    {
        return POS_ERR_INVALID_DRIVE;
    }
    /* Checks for '\' or '/' before the null terminator and removes it. */
    p = strchr(output, '\0') - 1;
    if (p[0] == '\\' || p[0] == '/') p[0] = '\0';
    /* Checks the output for ".."s and changes the output accordingly. */
    read = output + 2;
    write = read;
    while (1)
    {
        read++;
        p = strchr(read, '\\');
        temp = strchr(read, '/');
        if (temp && (temp < p || !p))
        {
            p = temp;
            p[0] = '\\';
        }
        /* Checks if we are still inside the string. */
        if (p)
        {
            if (p - read == 2 && strncmp(read, "..", 2) == 0)
            {
                write[0] = '\0';
                /* If we would ascend above root with "..", return error. */
                if (write == output + 2) return(POS_ERR_PATH_NOT_FOUND);
                /* Finds last '\' and positions write on it. */
                write = strrchr(output + 2, '\\');
            }
            else
            {
                /* Write is positioned on '\' so we move it further. */
                write++;
                /* Copies this part of processed string onto write. */
                memcpy(write, read, p - read);
                /* Increments write for it to be after the copied part. */
                write += p - read;
                /* Adds '\' at the end of copied part. */
                write[0] = '\\';
            }
            /* Moves read further. */
            read = p;
            continue;
        }
        /* We are at the last part of string, so we set p to the end. */
        p = strchr(read, '\0');
        if (strcmp(read, "..") == 0)
        {
            write[0] = '\0';
            /* If we would ascend above root with "..", return error. */
            if (write == output + 2) return(POS_ERR_PATH_NOT_FOUND);
            /* Finds last '\' and positions write on it. */
            write = strrchr(output + 2, '\\');
            /* If we are at the first backslash (the one in "c:\"),
             * we add null terminator after it. */
            if (write == output + 2) write++;
        }
        else
        {
            /* Write is positioned on '\' so we move it further. */
            write++;
            /* Copies this part of processed string onto write. */
            memcpy(write, read, p - read);
            /* Increments write for it to be after the copied part. */
            write += p - read;
        }
        /* Adds null terminator at the end of the output. */
        write[0] = '\0';
        break;
    }
    return (0);
}
/**/

/* INT 21,AH=0A: Read Buffered Input */
void PosReadBufferedInput(pos_input_buffer *buf)
{
    size_t readBytes;
    unsigned char cbuf[3]; /* Only 1 byte needed, but protect against any
                              buffer overflow bug in PosReadFile */

    /* Reset actual chars read to 0 */
    buf->actualChars = 0;

    /* If no chars asked to be read, return immediately */
    if (buf->maxChars == 0)
        return;

    /* Loop until we read either max chars or CR */
    for (;;)
    {
        /* Read enough already, exit loop */
        if (buf->actualChars == buf->maxChars)
            break;

        /* Read one byte */
        readBytes = 0;
        PosReadFile(1, &cbuf, 1, &readBytes);
        if (readBytes > 0)
        {
            /* Handle backspace */
            if (cbuf[0] == '\b')
            {
                if (buf->actualChars == 0)
                {
                    /* Nothing to backspace, ring bell */
                    PosDisplayOutput('\a');
                }
                else
                {
                    /* Something to backspace, remove from buffer and from
                     * screen */
                    buf->actualChars--;
                    PosDisplayOutput(' ');
                    PosDisplayOutput('\b');
                }

            }
            /* Read CR, we exit loop */
            else if (cbuf[0] == '\r')
            {
                /* Don't increment actualChars
                 * since CR not included in count */
                buf->data[buf->actualChars] = cbuf[0];
                break;
            }
            else
            {
                /* Add the byte we read to the buffer */
                buf->data[buf->actualChars++] = cbuf[0];
            }
        }
        else
        {
            /* Read nothing? Probably an IO error. Exit loop then */
            break;
        }
    }

    /* Okay, we are done, return to caller. */
    return;
}

/* INT 21,31 - PosTerminateAndStayResident */
void PosTerminateAndStayResident(int exitCode, int paragraphs)
{
    tsrFlag = paragraphs;
    PosTerminate(exitCode);
}

void PosGetMemoryManagementStats(void *stats)
{
    MEMMGRSTATS *s = (MEMMGRSTATS*)stats;
#ifndef __32BIT__
    memmgrGetStats(&memmgr, stats);
#endif
#ifdef __32BIT__
    /* In 32-bit, divide bytes by 16 before returning.
     * This is so we use consistent units of paragraphs (16 bytes)
     * for both PDOS-16 and PDOS-32.
     */
    s->maxFree = s->maxFree >> 4;
    s->maxAllocated = s->maxAllocated >> 4;
    s->totalFree = s->totalFree >> 4;
    s->totalAllocated = s->totalAllocated >> 4;
#endif
}

void PosProcessGetMemoryStats(unsigned long pid, void *stats)
{
    MEMMGRSTATS *s = (MEMMGRSTATS*)stats;
#ifndef __32BIT__
    memmgrGetOwnerStats(&memmgr, pid, stats);
#endif
#ifdef __32BIT__
    /* In 32-bit, divide bytes by 16 before returning.
     * This is so we use consistent units of paragraphs (16 bytes)
     * for both PDOS-16 and PDOS-32.
     */
    s->maxFree = s->maxFree >> 4;
    s->maxAllocated = s->maxAllocated >> 4;
    s->totalFree = s->totalFree >> 4;
    s->totalAllocated = s->totalAllocated >> 4;
#endif
}

/* INT 21, function 33, subfunc 05 - get boot drive */
int PosGetBootDrive(void)
{
    return bootDriveLogical + 1;
}

/* INT 21, function 51/62 - get current process ID */
int PosGetCurrentProcessId(void)
{
    return curPCB != NULL ? curPCB->pid : 0;
}

#define ERR2STR(n) case POS_ERR_##n: return #n

char *PosGetErrorMessageString(unsigned int errorCode) /* func f6.09 */
{
    switch (errorCode)
    {
        ERR2STR(NO_ERROR);
        ERR2STR(FUNCTION_NUMBER_INVALID);
        ERR2STR(FILE_NOT_FOUND);
        ERR2STR(PATH_NOT_FOUND);
        ERR2STR(MANY_OPEN_FILES);
        ERR2STR(ACCESS_DENIED);
        ERR2STR(INVALID_HANDLE);
        ERR2STR(MEM_CONTROL_BLOCK_DESTROYED);
        ERR2STR(INSUFFICIENT_MEMORY);
        ERR2STR(MEMORY_BLOCK_ADDRESS_INVALID);
        ERR2STR(ENVIRONMENT_INVALID);
        ERR2STR(FORMAT_INVALID);
        ERR2STR(ACCESS_CODE_INVALID);
        ERR2STR(DATA_INVALID);
        ERR2STR(FIXUP_OVERFLOW);
        ERR2STR(INVALID_DRIVE);
        ERR2STR(ATTEMPTED_TO_REMOVE_CURRENT_DIRECTORY);
        ERR2STR(NOT_SAME_DEVICE);
        ERR2STR(NO_MORE_FILES);
        ERR2STR(FILE_EXISTS);
    }
    return NULL;
}

/* Func F6.0B - Get info about a process. */
int PosProcessGetInfo(unsigned long pid, PDOS_PROCINFO *info, size_t infoSz)
{
    /* Find the process */
    PDOS_PROCESS *proc = findProc(pid);

    /* We reuse FILE_NOT_FOUND error for process not found.
     * Maybe we should create PROC_NOT_FOUND instead??? */
    if (proc == NULL)
    {
        return POS_ERR_FILE_NOT_FOUND;
    }

    /* Validate structure */
    if (info == NULL || infoSz != sizeof(PDOS_PROCINFO))
    {
        return POS_ERR_DATA_INVALID;
    }

    /* Populate structure */
    strncpy(info->exeName, proc->exeName, PDOS_PROCESS_EXENAMESZ-1);
    info->status = proc->status;
    info->pid = proc->pid;
    info->ppid = proc->parent==NULL?0:proc->parent->pid;
    info->prevPid = proc->prev==NULL?0:proc->prev->pid;
    info->nextPid = proc->next==NULL?0:proc->next->pid;

    /* Report success */
    return POS_ERR_NO_ERROR;
}

#if defined(__32BIT__) && defined(VGADRV)
void PosClearScreen()
{
    vgatext_clear_screen(&vgatext, currentAttrib);
}

void PosMoveCursor(int row, int col) /* func f6.31 */
{
    vgatext_set_cursor_position(&vgatext, row, col);
}
#else
void PosClearScreen()
{
    BosClearScreen(currentAttrib);
}

void PosMoveCursor(int row, int col) /* func f6.31 */
{
    BosSetCursorPosition(currentPage, row, col);
}
#endif

int PosGetVideoInfo(pos_video_info *info, size_t size) /* func f6.32 */
{
    if (info == NULL || size != sizeof(pos_video_info))
        return POS_ERR_DATA_INVALID;
    BosGetVideoMode(&(info->cols), &(info->mode), &(info->page));
    info->rows = BosGetTextModeRows();
    BosReadCursorPosition(
        info->page,
        &(info->cursorStart),
        &(info->cursorEnd),
        &(info->row),
        &(info->col)
    );
    info->currentAttrib = currentAttrib;
    currentPage = info->page;
    return POS_ERR_NO_ERROR;
}

int PosKeyboardHit(void) /* func f6.33 */
{
    int scancode, ascii, ret;

    ret = BosReadKeyboardStatus(&scancode, &ascii);
    return (ret == 0);
}

void PosYield(void) /* func f6.34 */
{
    /* Reduce battery consumption on laptops - tell BIOS we are doing nothing */
    BosCpuIdle();
}

/* INT 21,F6,35 - Sleep */
void PosSleep(unsigned long seconds)
{
    BosSleep(seconds);
}

/* INT 21,F6,36 - Get tick count */
unsigned long PosGetClockTickCount(void)
{
    return BosGetClockTickCount();
}

#if defined(__32BIT__) && defined(VGADRV)
static void pdosWriteText(int ch)
{
    unsigned int row;
    unsigned int column;
    if (ch == 8)
    {
        vgatext_read_cursor_position(&vgatext,&row,&column);
        if (column > 0)
        {
            column--;
            vgatext_set_cursor_position(&vgatext,row,column);
            vgatext_write_with_attr(&vgatext, ' ', currentAttrib);
        }
    }
    else if (ch == 7 || ch == 0xA || ch == 0xD)
        vgatext_write_text(&vgatext,ch,0);
    else
    {
        vgatext_read_cursor_position(&vgatext,&row,&column);
        vgatext_write_with_attr(&vgatext, ch, currentAttrib);
        column++;
        if (column == vgatext_get_max_cols(&vgatext))
        {
            vgatext_write_text(&vgatext,'\r',0);
            vgatext_write_text(&vgatext,'\n',0);
            vgatext_read_cursor_position(&vgatext,&row,&column);
        }
        vgatext_set_cursor_position(&vgatext,row,column);
    }
}
#else
static void pdosWriteText(int ch)
{
    unsigned int cursorStart;
    unsigned int cursorEnd;
    unsigned int row;
    unsigned int column;
    if (ch == 8)
    {
        BosReadCursorPosition(currentPage,&cursorStart,&cursorEnd,&row,&column);
        if (column > 0)
        {
            column--;
            BosSetCursorPosition(currentPage,row,column);
            BosWriteCharAttrib(currentPage, ' ', currentAttrib, 1);
        }
    }
    else if (ch == 7 || ch == 0xA || ch == 0xD)
        BosWriteText(currentPage,ch,0);
    else
    {
        BosReadCursorPosition(currentPage,&cursorStart,&cursorEnd,&row,&column);
        if (column == BosGetTextModeCols())
        {
            BosWriteText(currentPage,'\r',0);
            BosWriteText(currentPage,'\n',0);
            BosReadCursorPosition(currentPage,
                    &cursorStart,&cursorEnd,&row,&column);
        }
        BosWriteCharAttrib(currentPage, ch, currentAttrib, 1);
        column++;
        BosSetCursorPosition(currentPage,row,column);
    }
}
#endif

void PosSetVideoAttribute(unsigned int attr) /* func f6.37 */
{
    currentAttrib = attr;
}

int PosSetVideoMode(unsigned int mode) /* func f6.38 */
{
    unsigned int actual, cols, page;
    BosSetVideoMode(mode);
    BosGetVideoMode(&cols, &actual, &page);
    if (mode == actual) {
        currentPage = page;
        return POS_ERR_NO_ERROR;
    }
    return POS_ERR_ACCESS_DENIED;
}

int PosSetVideoPage(unsigned int page) /* func f6.39 */
{
    unsigned int mode, cols, actual;
    BosSetActiveDisplayPage(page);
    BosGetVideoMode(&cols, &mode, &actual);
    if (page == actual) {
        currentPage = page;
        return POS_ERR_NO_ERROR;
    }
    return POS_ERR_ACCESS_DENIED;
}

/*======== ENVIRONMENT MANAGEMENT FUNCTIONS ========*/

/*
 * Structure of DOS environment block is as follows:
 * NAME=VALUE\0NAME=VALUE\0NAME=VALUE\0\0 - this is env vars
 * Note that it must always terminate with two NULs \0\0 even if empty. After
 * this comes the "footer" or tail", which starts with a 16 bit value which is
 * the number of ASCIIZ strings following. Then that many ASCIIZ strings. That
 * 16-bit value is almost always 1 (maybe it is always 1?). The first such
 * ASCIIZ string is full path to the application. I don't think any further
 * such ASCIIZ strings were ever used, although the format was designed to
 * support such future extensibility.
 */

/* Get variable defined in given environment block */
static char * envBlockGetVar(char *envptr, char *name)
{
    size_t namelen = strlen(name);
    for (;;)
    {
        size_t n = strlen(envptr);
        if (n == 0)
            return NULL;
        if (strncmp(envptr,name,namelen) == 0 && envptr[namelen] == '=')
        {
            return envptr;
        }
        envptr += n + 1;
    }
}

/* Return environment block tail start (footer count pointer) */
static char *envBlockTail(char *envptr)
{
    size_t size = 0;
    for (;;)
    {
        size_t n = strlen(envptr + size);
        if (n == 0)
        {
            return envptr + size + 1 + (size == 0 ? 1 : 0);
        }
        size += n + 1;
    }
}

/* Count variables in environment block */
static int envBlockVarCount(char *envptr)
{
    size_t size = 0;
    int count = 0;
    for (;;)
    {
        size_t n = strlen(envptr + size);
        if (n == 0)
        {
            return count;
        }
        size += n + 1;
        count ++;
    }
}

/* Computes the size of an environment block */
static size_t envBlockSize(char *envptr)
{
    size_t size = 0;
    int footers, i;

    /* Skip over the environment variables */
    for (;;)
    {
        size_t n = strlen(envptr + size);
        if (n == 0)
        {
            size++;
            break;
        }
        size += n + 1;
    }
    if (size < 2)
        size = 2;
    /* envptr is now a 16-bit word count of footer ASCIIZ strings. This is
     * always 1, but the format technically allows more than one such string,
     * but the meaning of the second and subsequent strings isn't defined.
     * The first such string is full path to program executable.
     */
    footers = *((unsigned short *)&(envptr[size]));
    size += sizeof(unsigned short);
    /* Now skip over the footer strings */
    for (i = 0; i < footers; i++)
    {
        size_t n = strlen(envptr + size);
        size += n + 1;
    }
    /* Return overall size */
    return size;
}

static char * envAllocateEmpty(char *progName)
{
    char *envptr = NULL;
    size_t  envSize =
        2 /* \0\0 empty variables marker */ +
        sizeof(unsigned short) /* footer string count */ +
        strlen(progName) /* the program name (footer string 1) */ +
        1; /* \0 terminator of program name */
#ifdef __32BIT__
    envptr = kmalloc(envSize);
#else
    envptr = memmgrAllocate(&memmgr, envSize, 0);
    envptr = (unsigned char *)FP_NORM(envptr);
#endif
    /* Empty environment is two NUL bytes */
    envptr[0] = 0;
    envptr[1] = 0;
    *((unsigned short *)(envptr + 2)) = 1; /* footer count */
    strcpy(envptr + 2 + sizeof(unsigned short), progName);
    return envptr;
}

static char * envCopy(char *previous, char *progName)
{
    char *envptr = NULL;
    char *envTail;
    size_t envSize = (strlen(progName) + 1
                      + sizeof(unsigned short) + envBlockSize(previous));

#ifdef __32BIT__
    envptr = kmalloc(envSize);
#else
    envptr = memmgrAllocate(&memmgr, envSize, 0);
    envptr = FP_NORM(envptr);
#endif
    memcpy(envptr, previous, envSize);
    envTail = envBlockTail(envptr);
    *((unsigned short *)(envTail)) = 1;
    strcpy(envTail + sizeof(unsigned short), progName);

    return (envptr);
}

static void envBlockCopyWithMods(char *src, char *dest, char *name, char *value)
{
    /* Did we find the variable to modify? */
    int found = 0;
    /* Size of name, value */
    size_t nameLen = strlen(name);
    size_t valueLen = value == NULL ? 0 : strlen(value);
    /* Count of footers */
    unsigned short footerCount = 0;
    /* Loop counter */
    int i;
    int count = envBlockVarCount(src);

    if (count == 0)
    {
        strcpy(dest,name);
        dest[nameLen] = '=';
        dest += nameLen + 1;
        strcpy(dest,value);
        dest += valueLen + 1;
        *dest = 0;
        dest++;
        src += 2;
    }
    else
    {
        /* Copy the environment variables */
        for (i=0;;i++)
        {
            size_t n = strlen(src);
            if (n > 0)
            {
                if (strncmp(src,name,nameLen) == 0 && src[nameLen] == '=')
                {
                    if (value != NULL)
                    {
                        memcpy(dest,src,nameLen+1);
                        dest += nameLen + 1;
                        strcpy(dest,value);
                        dest += valueLen + 1;
                    }
                    found = 1;
                }
                else
                {
                    strcpy(dest,src);
                    dest += n + 1;
                }
                src += n + 1;
            }
            else
            {
                if (!found && value != NULL)
                {
                    strcpy(dest,name);
                    dest[nameLen] = '=';
                    dest += nameLen + 1;
                    strcpy(dest,value);
                    dest += valueLen + 1;
                }
                *dest = 0;
                dest++;
                src++;
                /*
                if (i == 0)
                {
                    *dest = 0;
                    dest++;
                    src++;
                }
                */
                break;
            }
        }
    }

    /* Copy footer count */
    footerCount = *((unsigned short *)src);
    *((unsigned short *)dest) = footerCount;
    src += sizeof(unsigned short);
    dest += sizeof(unsigned short);

    /* Copy the footers */
    for (i = 0; i < footerCount; i++)
    {
        size_t n = strlen(src);
        strcpy(dest, src);
        dest += n + 1;
        src += n + 1;
    }
}

static char *envModify(char *envPtr, char *name, char *value)
{
    size_t size = envBlockSize(envPtr);
    int count = envBlockVarCount(envPtr);
    char *existing = envBlockGetVar(envPtr,name);
    int offset;
    char *newPtr;

    /* Remove when value is NULL */
    if (value == NULL)
    {
        /* If name to be removed not in environment, just return success */
        if (existing == NULL)
            return envPtr;
        /* Removing doesn't require any extra memory, so we can use the
         * current environment block. We just have to update the existing
         * segment.
         */
        /* Handle case when we are emptying the environment.
         * This is special-cased because we still need two NUL bytes even
         * in an empty environment.
        */
        if (count == 1)
        {
            memmove(envPtr, envPtr + strlen(existing),
                    size - strlen(existing));
            /* Return success */
            return envPtr;
        }
        /* Find offset */
        offset = existing - envPtr;
        /* Remove the variable */
        memmove(existing, existing + strlen(existing) + 1,
                size - strlen(existing) - 1);
        /* Return success */
        return envPtr;
    }

    /* We need to allocate a new segment */
    size += strlen(name) + strlen(value) + 2;
    newPtr = PosAllocMemPages(((size/16)+1), NULL);
    if (newPtr == NULL)
    {
        return NULL;
    }
#ifndef __32BIT__
    memmgrSetOwner(&memmgr, newPtr, curPCB->pid);
#endif

    /* Populate new segment */
    envBlockCopyWithMods(envPtr, newPtr, name, value);

    /* Free old segment */
    PosFreeMem(envPtr);

    /* Return success */
    return newPtr;
}

/* F6,3A - Set Environment Variable */
int PosSetEnv(char *name, char *value)
{
    char *envPtr = curPCB->envBlock;
#ifndef __32BIT__
    char *psp;
#endif

    envPtr = envModify(envPtr, name, value);
    if (envPtr == NULL)
    {
        return POS_ERR_INSUFFICIENT_MEMORY;
    }

    /* Update PCB */
    curPCB->envBlock = envPtr;
#ifndef __32BIT__
    /* Update the PSP (in 16-bit only) */
    psp = MK_FP(curPCB->pid, 0);
    *(unsigned int *)&psp[0x2c] = FP_SEG(envPtr);
#endif

    /* Return success */
    return POS_ERR_NO_ERROR;
}

/* F6,3B - Get Environment Block */
void * PosGetEnvBlock(void)
{
    return curPCB->envBlock;
}

static int ins_strcmp(char *one, char *two)
{
    while (toupper((unsigned char)*one) == toupper((unsigned char)*two))
    {
        if (*one == '\0')
        {
            return (0);
        }
        one++;
        two++;
    }
    if (toupper((unsigned char)*one) < toupper((unsigned char)*two))
    {
        return (-1);
    }
    return (1);
}

/* F6,3C - Set Named Font */
int PosSetNamedFont(char *fontName)
{
    if (fontName == NULL)
        return POS_ERR_DATA_INVALID;
    if (ins_strcmp(fontName,"8x14") == 0)
    {
        if (BosLoadTextModeRomFont(BOS_TEXTMODE_ROMFONT_8X14,0) == 0)
            return POS_ERR_NO_ERROR;
    }
    if (ins_strcmp(fontName,"8x8") == 0)
    {
        if (BosLoadTextModeRomFont(BOS_TEXTMODE_ROMFONT_8X8,0) == 0)
            return POS_ERR_NO_ERROR;
    }
    if (ins_strcmp(fontName,"8x16") == 0)
    {
        if (BosLoadTextModeRomFont(BOS_TEXTMODE_ROMFONT_8X16,0) == 0)
            return POS_ERR_NO_ERROR;
    }
    return POS_ERR_FILE_NOT_FOUND;
}

#ifdef __32BIT__
/* F6,3D - Allocate Virtual Memory */
void *PosVirtualAlloc(void *addr, size_t size)
{
    return (vmmAlloc(curPCB->vmm, addr, size));
}

/* F6,3E - Free Virtual Memory */
void PosVirtualFree(void *addr, size_t size)
{
    vmmFree(curPCB->vmm, addr, size);
}

/* F6,3F - Get Command Line String For The Current Process */
char *PosGetCommandLine(void)
{
    /* The process is allowed to modify the string,
     * so the string is copied into its virtual memory. */
    char *commandLine;

    commandLine = PosVirtualAlloc(0, strlen(curPCB->commandLine) + 1);
    if (commandLine == NULL) return (NULL);
    strcpy(commandLine, curPCB->commandLine);

    return (commandLine);
}

/* F6,40 - Read Byte From Port */
unsigned char PosInp(unsigned int port)
{
    return (inp(port));
}

/* F6,41 - Write Byte To Port */
void PosOutp(unsigned int port, unsigned char data)
{
    outp(port, data);
}
#endif

static void getDateTime(FAT_DATETIME *ptr)
{
    unsigned int dow1, dow2;

    PosGetSystemDate(&ptr->year, &ptr->month, &ptr->day, &dow1);
    PosGetSystemTime(&ptr->hours, &ptr->minutes, &ptr->seconds,
                     &ptr->hundredths);
    PosGetSystemDate(&ptr->year, &ptr->month, &ptr->day, &dow2);
    if (dow1 != dow2)
    {
        getDateTime(ptr);
    }
    return;
}

#ifdef NOVM

void *kmalloc(size_t size)
{
    return (memmgrAllocate(&physmemmgr, size, memId));
}

void kfree(void *ptr)
{
    memmgrFree(&physmemmgr, ptr);
    return;
}

#endif
