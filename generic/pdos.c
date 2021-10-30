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

#include <bios.h>

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

extern BIOS *bios;
extern __start(char *p);

extern int __genstart;
extern int (*__genmain)(int argc, char **argv);

static OS os = { __start, printf, 0, malloc, NULL, NULL,
  fopen, fseek, fread, fclose, fwrite, fgets, strchr,
  strcmp, strncmp, fgetc, fputc };

static int (*pgastart)(OS *os);

static MEMMGR memmgr;

static void *disk;


#define SECTSZ 512

static unsigned long lba;
static unsigned char sect[SECTSZ];

static FAT fat;
static FATFILE fatfile;

static void readLogical(void *diskptr, unsigned long sector, void *buf);
static void writeLogical(void *diskptr, unsigned long sector, void *buf);
static void getDateTime(FAT_DATETIME *ptr);


int main(void)
{
    unsigned char *entry_point;
    unsigned char *p = NULL;
    int ret;
    unsigned char lbabuf[4];

    __minstart = 0;
    __genstart = 1;
    os.main = &__genmain;
    os.Xstdin = stdin;
    os.Xstdout = stdout;

    memmgrDefaults(&memmgr);
    memmgrInit(&memmgr);
    memmgrSupply(&memmgr, bios->mem_base, bios->mem_amt);

    /* printf(CHAR_ESC_STR "[2J"); */
    printf("hello from PDOS\n");
    disk = bios->fopen(bios->disk_name, "r+b");
    if (disk == NULL)
    {
        printf("can't open hard disk\n");
        return (EXIT_FAILURE);
    }
    bios->fseek(disk, 0x1be + 0x8, BIOS_SEEK_SET);
    bios->fread(lbabuf, 1, 4, disk);
    lba = ((unsigned long)lbabuf[3] << 24)
           | ((unsigned long)lbabuf[2] << 16)
           | (lbabuf[1] << 8)
           | lbabuf[0];
    printf("lba is %lx\n", lba);
    bios->fseek(disk, lba * SECTSZ, BIOS_SEEK_SET);
    bios->fread(sect, SECTSZ, 1, disk);
    printf("fat type is %.5s\n", &sect[0x36]);
    fatDefaults(&fat);
    fatInit(&fat, &sect[11], readLogical, writeLogical, disk, getDateTime);
    if (exeloadDoload(&entry_point, "command.com", &p) != 0)
    {
        printf("failed to load program\n");
        return (EXIT_FAILURE);
    }
    pgastart = (void *)entry_point;
    printf("about to call app\n");
    ret = pgastart(&os);
    printf("return from app is %d\n", ret);
    memmgrTerm(&memmgr);
    return (0);
}

int PosOpenFile(const char *name, int mode, int *handle)
{
    int ret;

    printf("got request to open %s\n", name);
    /* *handle = (int)bios->fopen(name, "rb"); */
    ret = fatOpenFile(&fat, name, &fatfile);
    if (ret != 0) return (1);
    *handle = 3;
    return (0);
}

int PosCloseFile(int fno)
{
    /* printf("got request to close\n"); */
    return (0);
}

int PosCreatFile(const char *name, int attrib, int *handle)
{
    return (0);
}

int PosReadFile(int fh, void *data, size_t bytes, size_t *readbytes)
{
    /* printf("got request to read %lu bytes\n", (unsigned long)bytes); */
    if (fh < 3)
    {
        /* fread is blocking - use fgets as kludge */
        bios->fgets(data, bytes, bios->Xstdin);
        *readbytes = strlen(data);
        /* printf("got %d bytes\n", *readbytes); */
        /* *readbytes = bios->fread(data, 1, bytes, bios->Xstdin); */
    }
    else
    {
        fatReadFile(&fat, &fatfile, data, bytes, readbytes);
    }
    /* printf("read %lu bytes\n", (unsigned long)*readbytes); */
    return (0);
}

int PosWriteFile(int fh, const void *data, size_t len, size_t *writtenbytes)
{
    bios->fwrite(data, 1, len, bios->Xstdout);
    return (0);
}

int PosMoveFilePointer(int handle, long offset, int whence, long *newpos)
{
    /* bios->fseek((void *)handle, offset, SEEK_SET);
    *newpos = offset; */
    *newpos = fatSeek(&fat, &fatfile, offset, whence);
    return (0);
}

void *PosAllocMem(unsigned int size, unsigned int flags)
{
    char *p;

    /* printf("got request to allocate %lu bytes\n",
        (unsigned long)size); */
    p = memmgrAllocate(&memmgr, size, 0);
    return (p);
}

int PosFreeMem(void *ptr)
{
    memmgrFree(&memmgr, ptr);
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

void PosExec(char *prog, POSEXEC_PARMBLOCK *parmblock)
{
    return;
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
    static char buf[10];

    strcpy(buf, "a b");
    return (buf);
}

void *PosGetEnvBlock(void)
{
    return 0;
}

static void readLogical(void *diskptr, unsigned long sector, void *buf)
{
    int ret;

    sector += fat.hidden;
    bios->fseek(diskptr, sector * SECTSZ, BIOS_SEEK_SET);
    ret = bios->fread(buf, SECTSZ, 1, diskptr);
    return;
}

static void writeLogical(void *diskptr, unsigned long sector, void *buf)
{
    return;
}

static void getDateTime(FAT_DATETIME *ptr)
{
    memset(ptr, '\0', sizeof *ptr);
    return;
}
