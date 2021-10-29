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

#include <bios.h>

#include <pos.h>

#include <exeload.h>

#include <__os.h>

extern int __minstart;

#ifdef EBCDIC
#define CHAR_ESC_STR "\x27"
#else
#define CHAR_ESC_STR "\x1b"
#endif

extern BIOS *__bios;
extern __start(char *p);

OS os = { __start, printf };

static int (*pgastart)(OS *os);

static char loadbuf[10000];
static char membuf[10000];

int main(void)
{
    unsigned char *entry_point;
    unsigned char *p = loadbuf;

    __minstart = 0;
    /* printf(CHAR_ESC_STR "[2J"); */
    printf("hello from PDOS\n");
    if (exeloadDoload(&entry_point, "../pdpclib/pdptest.exe", &p) != 0)
    {
        printf("failed to load program\n");
        return (EXIT_FAILURE);
    }
    pgastart = (void *)entry_point;
    pgastart(&os);
    return (0);
}

int PosOpenFile(const char *name, int mode, int *handle)
{
    printf("got request to open %s\n", name);
    *handle = (int)__bios->fopen(name, "rb");
    return (0);
}

int PosCloseFile(int fno)
{
    printf("got request to close\n");
    return (0);
}

int PosCreatFile(const char *name, int attrib, int *handle)
{
    return (0);
}

int PosReadFile(int fh, void *data, size_t bytes, size_t *readbytes)
{
    printf("got request to read %lu bytes\n", (unsigned long)bytes);
    *readbytes = __bios->fread(data, 1, bytes, (void *)fh);
    return (0);
}

int PosWriteFile(int fh, const void *data, size_t len, size_t *writtenbytes)
{
    __bios->fwrite(data, 1, len, __bios->Xstdout);
    return (0);
}

int PosMoveFilePointer(int handle, long offset, int whence, long *newpos)
{
    __bios->fseek((void *)handle, offset, SEEK_SET);
    *newpos = offset;
    return (0);
}

void *PosAllocMem(unsigned int size, unsigned int flags)
{
    static char *mb = membuf;
    char *p;

    printf("got request to allocate %lu bytes\n",
        (unsigned long)size);
    p = mb;
    mb += size;
    return (p);
}

int PosFreeMem(void *ptr)
{
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
    return (0);
}

void *PosGetEnvBlock(void)
{
    return 0;
}
