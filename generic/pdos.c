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

#include <bios.h>

#include <pos.h>

#ifdef EBCDIC
#define CHAR_ESC_STR "\x27"
#else
#define CHAR_ESC_STR "\x1b"
#endif

extern BIOS *__bios;

int main(void)
{
    __bios->printf(CHAR_ESC_STR "[2J");
    __bios->printf("hello from PDOS\n");
    return (0);
}

int PosOpenFile(const char *name, int mode, int *handle)
{
    return (0);
}

int PosCloseFile(int fno)
{
    return (0);
}

int PosCreatFile(const char *name, int attrib, int *handle)
{
    return (0);
}

int PosReadFile(int fh, void *data, size_t bytes, size_t *readbytes)
{
    return (0);
}

int PosWriteFile(int fh, const void *data, size_t len, size_t *writtenbytes)
{
    return (0);
}

int PosMoveFilePointer(int handle, long offset, int whence, long *newpos)
{
    return (0);
}

void *PosAllocMem(unsigned int size, unsigned int flags)
{
    return (0);
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
