/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pdos - generic OS                                                */
/*                                                                   */
/*********************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <locale.h>
#include <ctype.h>
#include <assert.h>

#include <pos.h>

#include <exeload.h>

#include <__os.h>

#include <__memmgr.h>

#include <fat.h>

extern int __minstart;

#ifdef EBCDIC
#define CHAR_ESC_STR "\x27"
#else
#define CHAR_ESC_STR "\x1b"
#endif

#define BIOS_SEEK_SET SEEK_SET
#define BIOS_IONBF _IONBF
#define BIOS_IOLBF _IOLBF
#define BIOS_BUFSIZ BUFSIZ

extern OS *bios;
extern __start(char *p);

extern int __genstart;
extern int (*__genmain)(int argc, char **argv);

char mycmdline[400];

static OS os = { __start, 0, 0, mycmdline, printf, 0, malloc, NULL, NULL,
  fopen, fseek, fread, fclose, fwrite, fgets, strchr,
  strcmp, strncmp, strcpy, strlen, fgetc, fputc,
  fflush, setvbuf,
  PosGetDTA, PosFindFirst, PosFindNext,
  PosGetDeviceInformation, PosSetDeviceInformation,
  ctime, time,
  0, 0, 0, /* PosChangeDir, PosMakeDir, PosRemoveDir */
  remove,
  memcpy, strncpy, strcat, 0 /* stderr */, free, abort, memset, fputs, fprintf,
  getenv, memmove, exit, memcmp, _errno, tmpnam, vfprintf, ungetc, vsprintf,
  sprintf, signal, raise, calloc, realloc, atoi, strtol, strtoul, qsort,
  bsearch, localtime, clock, strerror, strrchr, strstr, strpbrk, strspn,
  strcspn, memchr, ftell, abs, setlocale, perror, rewind, strncat, sscanf,
  isalnum, isxdigit, rename, clearerr, _assert, atof,
  isdigit, isalpha, isprint, isspace, tolower, system,
  islower, isupper, atexit, ceil, toupper, iscntrl,
  sin, cos, tan, floor, asin, acos, atan, sinh, cosh, tanh,
  rand, srand, strftime, puts,
  pow, modf, log, log10, atan2, fabs, exp, sqrt,
  strtok, atol, mktime, vprintf, ferror, putc, feof, getc,
  getchar, putchar, PosExec,
};

static int (*pgastart)(OS *os);

static void *disk;

static DTA origdta;

static int stdin_raw = 0;

#define SECTSZ 512

static unsigned long lba;
static unsigned char sect[SECTSZ];

static FAT fat;
static FATFILE fatfile;

#ifdef __SUBC__
static char myname[1];
#else
static char *myname = "";
#endif

#define MAX_HANDLE 20

static struct {
    int inuse;
    void *fptr;
    FATFILE ff;
} handles[MAX_HANDLE];


static void runexe(char *prog_name);


static void readLogical(void *diskptr, unsigned long sector, void *buf);
static void writeLogical(void *diskptr, unsigned long sector, void *buf);
static void getDateTime(FAT_DATETIME *ptr);


/* The BIOS C library will call this, and then we call our own C library */
/* Don't rely on the BIOS having a C library capable of breaking down a
   command line buffer, but there will at least be a program name, possibly
   an empty string. Our own C library can break it down though. */
int biosmain(int argc, char **argv)
{
    if (argc >= 1)
    {
#ifndef __SUBC__
        myname = argv[0];
#endif
    }
    return (__start(0));
}

int main(int argc, char **argv)
{
    unsigned char lbabuf[4];
    void *mem_base;
    int argupto = 1;
    int havedisk = 0;
    char *config = NULL;
    char *pcomm_name = ":pcomm.exe";

    __genstart = 1;
    os.main = &__genmain;
    os.Xstdin = stdin;
    os.Xstdout = stdout;
    os.Xstderr = stderr;

    mem_base = bios->malloc(bios->mem_amt);
    if (mem_base == NULL)
    {
        bios->Xprintf("failed to do promised malloc\n");
        return (EXIT_FAILURE);
    }

    /* C library will have done this already */
    /* for some implementations maybe? */
    /* no, I don't think any implementations will do this */
    /* They probably should - zpdos is trying that */
    /* Not clear how anything couldn't */

#ifndef DONT_MM
    memmgrDefaults(&__memmgr);
    memmgrInit(&__memmgr);
#endif
    memmgrSupply(&__memmgr, mem_base, bios->mem_amt);

    /* printf(CHAR_ESC_STR "[2J"); */
    printf("welcome to PDOS-generic\n");
    printf("running as %s\n", argv[0]);
    if (argc < 2)
    {
        printf("must provide disk name as a parameter\n");
        printf("or at least -c config.sys or whatever\n");
        bios->free(mem_base);
        return (EXIT_FAILURE);
    }
    printf("before printing parm\n");
    printf("argv1 is %s\n", argv[1]);
    if ((strcmp(argv[1], "-c") == 0)
        || (strcmp(argv[1], "-C") == 0)
       )
    {
        config = argv[2];
        argupto += 2;
    }
    if (argc > argupto)
    {
    printf("about to open\n");
    /* for (;;) ; */
    disk = bios->Xfopen(argv[1], "r+b");
    if (disk == NULL)
    {
        printf("can't open hard disk\n");
        return (EXIT_FAILURE);
    }
    printf("done open\n");
    {
    /* we should be able to do this, but for now, we'll read
       an entire sector to unburden the C library */
#if 0
    bios->Xfseek(disk, 0x1be + 0x8, BIOS_SEEK_SET);
    printf("done seek\n");
    bios->Xfread(lbabuf, 1, 4, disk);
    printf("done read\n");
#else
    bios->Xfseek(disk, 0, BIOS_SEEK_SET);
    bios->Xfread(sect, 1, 512, disk);
    /* this is not ideal as the MBR code could contain this */
    /* this is to support drives with just a single VBR, no MBR */
    /* EFI effectively gives us this, and in addition, we need
       to squelch the hidden sectors */
    if ((memcmp(sect + 0x52, "FAT32", 5) == 0)
        || (memcmp(sect + 0x36, "FAT12", 5) == 0)
        || (memcmp(sect + 0x36, "FAT16", 5) == 0))
    {
        memcpy(lbabuf, "\x00\x00\x00\x00", 4);
        memcpy(sect + 11 + 17, "\x00\x00\x00\x00", 4);
    }
    else
    {
         memcpy(lbabuf, sect + 0x1be + 0x8, 4);
    }
#endif
    }
    lba = ((unsigned long)lbabuf[3] << 24)
           | ((unsigned long)lbabuf[2] << 16)
           | (lbabuf[1] << 8)
           | lbabuf[0];
    printf("lba is %lx\n", lba);
    if (lba != 0)
    {
        bios->Xfseek(disk, lba * SECTSZ, BIOS_SEEK_SET);
        bios->Xfread(sect, SECTSZ, 1, disk);
    }
    printf("fat type is %.5s\n", &sect[0x36]);
    fatDefaults(&fat);
    fatInit(&fat, &sect[11], readLogical, writeLogical, disk, getDateTime);
    }


    if (config != NULL)
    {
        if (strchr(config, ':') != NULL)
        {
            FILE *fp;
            char buf[100];
            char *p;

            printf("opening %s\n", config);
            fp = bios->Xfopen(config, "r");
            bios->Xfgets(buf, sizeof buf, fp);
            p = strchr(buf, '=');
            if (p != NULL)
            {
               p++;
               pcomm_name = p;
               p = strchr(pcomm_name, '\n');
               if (p != NULL)
               {
                   *p = '\0';
               }
               strcpy(mycmdline, pcomm_name);
               p = strchr(pcomm_name, ' ');
               if (p != NULL)
               {
                   *p = '\0';
               }
               printf("shell name is %s\n", pcomm_name);
            }
            bios->Xfclose(fp);
        }
    }
    runexe(pcomm_name);
    /* we need to rely on the C library to do the termination when it
       has finished closing files */
#ifndef DONT_MM
    memmgrTerm(&__memmgr);
#endif
    return (0);
}



/* mycmdline must have been populated first */

static void runexe(char *prog_name)
{
    unsigned char *entry_point;
    unsigned char *p = NULL;
    int ret;

    if (exeloadDoload(&entry_point, prog_name, &p) != 0)
    {
        printf("failed to load program\n");
        for (;;) ;
        exit(EXIT_FAILURE);
    }
    pgastart = (void *)entry_point;

#ifdef NEED_DELAY

#ifdef __ARMGEN__
    {
        /* 1, 0 works */
        /* 0, 50000000 works */
        /* 0, 5000000 sometimes fails */
        /* 0, 10000000 works, so double to be hopefully safe */
        /* Although that is apparently enough for a Pinebook Pro,
           it wasn't enough for an armv7 netbook, which continued
           to randomly fail. So 1 second was added */
        static unsigned int val1[2] = {1, 20000000UL};
        unsigned int val2[2];
        printf("sleeping before executing BSS memory\n");
        __nanosleep(val1, val2);
    }
#else
    for (rc = 0; rc < 500; rc++)
    {
        printf("please accept a delay before we execute program "
               "in BSS memory\n");
    }
#endif

#endif


#ifdef NEED_FLUSH
    __cacheflush(mem_base, bios->mem_amt, 0);
#endif


    printf("about to call app\n");
    ret = pgastart(&os);
    printf("return from app is %d\n", ret);
    return;
}



int PosOpenFile(const char *name, int mode, int *handle)
{
    int ret;
    int x;
    int bios_file = 0;

    printf("got request to open %s\n", name);
    for (x = 3; x < MAX_HANDLE; x++)
    {
        if (!handles[x].inuse) break;
    }
    if (x == MAX_HANDLE)
    {
        return (1);
    }
    if (name[0] == ':')
    {
        name++;
        bios_file = 1;
    }
    else if (strchr(name, ':') != NULL)
    {
        /* this allows a device to be opened, but we need better
           logic for when we want to reference a file on an FAT
           drive */
        bios_file = 1;
    }
    if (bios_file)
    {
        handles[x].fptr = bios->Xfopen(name, "rb");
        if (handles[x].fptr != NULL)
        {
            *handle = x;
            handles[x].inuse = 1;
            return (0);
        }
        else
        {
            return (1);
        }
    }
    ret = fatOpenFile(&fat, name, &handles[x].ff);
    if (ret != 0) return (1);
    *handle = x;
    handles[x].inuse = 1;
    return (0);
}

int PosCloseFile(int fno)
{
    /* printf("got request to close %d\n", fno); */
    if (handles[fno].fptr)
    {
        bios->Xfclose(handles[fno].fptr);
        handles[fno].fptr = NULL;
    }
    else
    {
    }
    handles[fno].inuse = 0;
    return (0);
}

int PosCreatFile(const char *name, int attrib, int *handle)
{
    int ret;
    int x;

    printf("got request to create %s\n", name);
    for (x = 3; x < MAX_HANDLE; x++)
    {
        if (!handles[x].inuse) break;
    }
    if (x == MAX_HANDLE)
    {
        return (1);
    }
    if (name[0] == ':')
    {
        handles[x].fptr = bios->Xfopen(name + 1, "wb");
        if (handles[x].fptr != NULL)
        {
            *handle = x;
            handles[x].inuse = 1;
            return (0);
        }
        else
        {
            return (1);
        }
    }
    ret = fatCreatFile(&fat, name, &handles[x].ff, attrib);
    if (ret != 0) return (1);
    *handle = x;
    handles[x].inuse = 1;
    return (0);
}

int PosReadFile(int fh, void *data, unsigned int bytes, unsigned int *readbytes)
{
    /* printf("got request to read %lu bytes\n", (unsigned long)bytes); */
    if (fh < 3)
    {
        /* we need to use fgets, not fread, because we need a function
           that will terminate at a newline. If the caller is actually
           trying to do an fgets themselves, then we will be reading 1
           less byte here because of the terminating NUL allowance. But
           that doesn't matter either, because if we hit that border
           condition, the caller will just issue another PosReadFile
           until they get the newline character they need.

           Note that that is normal line mode reading from stdin. The
           caller doesn't have the option to wait until the buffer is
           full, so that doesn't need to be considered. And if they are
           doing character-oriented input, then a call to this function
           with a length of 1 will also work when passed to fgets. */
        bios->Xfgets(data, bytes, bios->Xstdin);
        *readbytes = strlen(data);
        /* printf("got %d bytes\n", *readbytes); */
    }
    else
    {
        if (handles[fh].fptr != NULL)
        {
            *readbytes = bios->Xfread(data, 1, bytes, handles[fh].fptr);
            /* printf("got %d bytes from bios\n", *readbytes); */
        }
        else
        {
            fatReadFile(&fat, &handles[fh].ff, data, bytes, readbytes);
        }
    }
    /* printf("read %lu bytes\n", (unsigned long)*readbytes); */
    return (0);
}

int PosWriteFile(int fh,
                 const void *data,
                 unsigned int len,
                 unsigned int *writtenbytes)
{
    if (fh < 3)
    {
        bios->Xfwrite(data, 1, len, bios->Xstdout);
        bios->Xfflush(bios->Xstdout);
    }
    else
    {
        if (handles[fh].fptr != NULL)
        {
            *writtenbytes = bios->Xfwrite(data, 1, len, handles[fh].fptr);
        }
        else
        {
            fatWriteFile(&fat, &handles[fh].ff, data, len, writtenbytes);
        }
    }
    return (0);
}

int PosMoveFilePointer(int handle, long offset, int whence, long *newpos)
{
    if (handles[handle].fptr != NULL)
    {
        bios->Xfseek((void *)handle, offset, SEEK_SET);
        *newpos = offset;
    }
    else
    {
        *newpos = fatSeek(&fat, &handles[handle].ff, offset, whence);
    }
    return (0);
}

void *PosAllocMem(unsigned int size, unsigned int flags)
{
    char *p;

    /* printf("got request to allocate %lu bytes\n",
        (unsigned long)size); */
    p = memmgrAllocate(&__memmgr, size, 0);
    return (p);
}

int PosFreeMem(void *ptr)
{
    memmgrFree(&__memmgr, ptr);
    return (0);
}

int PosGetDeviceInformation(int handle, unsigned int *devinfo)
{
    *devinfo = stdin_raw << 5;
    return (0);
}

int PosSetDeviceInformation(int handle, unsigned int devinfo)
{
    stdin_raw = ((devinfo & (1 << 5)) != 0);
    if (stdin_raw)
    {
        bios->Xsetvbuf(bios->Xstdin, NULL, BIOS_IONBF, 0);
    }
    else
    {
        bios->Xsetvbuf(bios->Xstdin, NULL, BIOS_IOLBF, BIOS_BUFSIZ);
    }
    return (0);
}

void PosGetSystemDate(unsigned int *year,
                      unsigned int *month,
                      unsigned int *day,
                      unsigned int *dw)
{
    return;
}

void PosGetSystemTime(unsigned int *hour, unsigned int *minute,
                      unsigned int *second, unsigned int *hundredths)
{
    return;
}

int PosDeleteFile(const char *name)
{
    return (0);
}

int PosRenameFile(const char *old, const char *new)
{
    return (0);
}

int PosExec(char *prog, POSEXEC_PARMBLOCK *parmblock)
{
    strcpy(mycmdline, parmblock->cmdtail);
    runexe(prog);
    return (0);
}

int PosGetReturnCode(void)
{
    return (0);
}

void PosTerminate(int rc)
{
    return;
}

char *PosGetCommandLine(void)
{
    return (mycmdline);
}

void *PosGetEnvBlock(void)
{
    return (void *)0;
}

void *PosGetDTA(void)
{
    return (&origdta);
}

static int ff_search(void)
{
    DIRENT dirent;
    unsigned int readbytes;

    while (1)
    {
        fatReadFile(&fat, &fatfile, &dirent, sizeof dirent, &readbytes);
        if (readbytes != sizeof dirent) return (1);
        if (dirent.file_name[0] == '\0') return (1);
        if (dirent.file_name[0] == DIRENT_DEL) continue;
        strncpy(origdta.file_name, dirent.file_name, 11);
        origdta.file_name[11] = '\0';
        break;
    }
    return (0);
}

int PosFindFirst(char *pat, int attrib)
{
    fatOpenFile(&fat, "\\", &fatfile);
    return (ff_search());
}

int PosFindNext(void)
{
    return (ff_search());
}

unsigned int PosMonitor(void)
{
    printf("no monitor available\n");
    for (;;) ;
}

int PosGetMagic(void)
{
    return (PDOS_MAGIC);
}

static void readLogical(void *diskptr, unsigned long sector, void *buf)
{
    int ret;

    sector += fat.hidden;
    bios->Xfseek(diskptr, sector * SECTSZ, BIOS_SEEK_SET);
    ret = bios->Xfread(buf, SECTSZ, 1, diskptr);
    return;
}

static void writeLogical(void *diskptr, unsigned long sector, void *buf)
{
    int ret;

    sector += fat.hidden;
    bios->Xfseek(diskptr, sector * SECTSZ, BIOS_SEEK_SET);
    ret = bios->Xfwrite(buf, SECTSZ, 1, diskptr);
    return;
}

static void getDateTime(FAT_DATETIME *ptr)
{
    memset(ptr, '\0', sizeof *ptr);
    return;
}
