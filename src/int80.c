/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*  Modified by Paul Edwards. Changes remain public domain           */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  int80.c - Interrupt 80 handler                                   */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "int80.h"
#include "support.h"
#include "pos.h"


/* You can potentially do something like: */
/* #define lin_write __write */
/* to satisfy a Linux version of PDPCLIB, to bypass the
   interrupt process. You might want to mask out the
   unused code too */

/* this is also defined in pdos.c */
/* It doesn't necessarily have to match */
#define MAXFILES 40


/* Note that we can't do text translation in pdos.c for
   stdin because Linux needs just NL but other programs
   are meant to receive CRLF. We could just set a requirement
   for applications to be flexible, but I want to prove a
   more flexible solution */

static struct {
    int handle; /* from PDOS */
    int inuse;
    int text; /* this this a text file? */
} linfiles[MAXFILES];

static void int80handler(union REGS *regsin,
                        union REGS *regsout,
                        struct SREGS *sregs);

void initlinux(void)
{
    int x;

    for (x = 0; x < MAXFILES; x++)
    {
        linfiles[x].handle = -1;
        linfiles[x].inuse = 0;
        linfiles[x].text = 0;
    }
    /* it so happens that these 3 match handle numbers,
       but this will need separate assignment when e.g.
       we switch to FILE handles instead of handles */
    for (x = 0; x <= 2; x++)
    {
        linfiles[x].handle = x;
        linfiles[x].inuse = 1;
        linfiles[x].text = 1;
    }
    return;
}

int int80(unsigned int *regs)
{
    static union REGS regsin;
    static union REGS regsout;
    static struct SREGS sregs;

    regsin.d.eax = regs[0];
    regsin.d.ebx = regs[1];
    regsin.d.ecx = regs[2];
    regsin.d.edx = regs[3];
    regsin.d.esi = regs[4];
    regsin.d.edi = regs[5];
    regsin.d.cflag = 0;
    memcpy(&regsout, &regsin, sizeof regsout);
    int80handler(&regsin, &regsout, &sregs);
    regs[0] = regsout.d.eax;
    regs[1] = regsout.d.ebx;
    regs[2] = regsout.d.ecx;
    regs[3] = regsout.d.edx;
    regs[4] = regsout.d.esi;
    regs[5] = regsout.d.edi;
    regs[6] = regsout.d.cflag;
    return (0);
}



void lin_exit(int rc);
unsigned int lin_write(int handle, void *buf, unsigned int len);
int lin_open(char *fnm, unsigned int flags, unsigned int mode);
unsigned int lin_read(int handle, void *buf, unsigned int len);
int lin_close(int handle);
int lin_getpid(void);
int lin_ioctl(int handle, int command, int parm);
void *lin_mmap(void *p);
int lin_munmap(void *p, unsigned int len);
int lin_execve(char *path, char **argv, char **envp);

static void int80handler(union REGS *regsin,
                        union REGS *regsout,
                        struct SREGS *sregs)
{
    void *p;
    unsigned int writtenbytes;

    switch (regsin->d.eax)
    {
        /* Terminate (with return code) */
        case 0x1:
            lin_exit(regsin->d.ebx);
            break;

        /* read */
        case 0x3:
            p = (void *)(regsin->d.ecx);
            regsout->d.eax = lin_read(regsin->d.ebx,
                                      p,
                                      regsin->d.edx);
            break;

        /* write */
        case 0x4:
            p = (void *)(regsin->d.ecx);
            regsout->d.eax = lin_write(regsin->d.ebx,
                                       p,
                                       regsin->d.edx);
            break;

        /* open */
        case 0x5:
            p = (void *)(regsin->d.ebx);
            regsout->d.eax = lin_open(p,
                                      regsin->d.ecx,
                                      regsin->d.edx);
            break;

        /* close */
        case 0x6:
            regsout->d.eax = lin_close(regsin->d.ebx);
            break;

        /* execve */
        case 0xb:
            regsout->d.eax = lin_execve((void *)regsin->d.ebx,
                                        (void *)regsin->d.ecx,
                                        (void *)regsin->d.edx);
            break;

        /* getpid */
        case 0x14:
            regsout->d.eax = lin_getpid();
            break;

        /* ioctl */
        case 0x36:
            regsout->d.eax = lin_ioctl(regsin->d.ebx,
                                       regsin->d.ecx,
                                       regsin->d.edx);
            break;

        /* mmap */
        case 0x5a:
            regsout->d.eax = (int)lin_mmap((void *)regsin->d.ebx);
            break;

        /* munmap */
        case 0x5b:
            regsout->d.eax = lin_munmap((void *)regsin->d.ebx,
                                        regsin->d.ecx);
            break;

        /* clone */
        case 0x78:
            /* don't bother doing anything except return success/child */
            regsout->d.eax = 0;
            break;

        default:
            printf("got int 80H call %x\n", regsin->d.eax);
            break;
    }
    return;
}

void lin_exit(int rc)
{
    PosTerminate(rc);
    return;
}

unsigned int lin_write(int handle, void *buf, unsigned int len)
{
    unsigned int writtenbytes;
    int rc;

    if (linfiles[handle].text)
    {
        unsigned int remain = len;
        char *p = buf;
        char *q;
        unsigned int thiswrite;

        writtenbytes = 0;
        while (remain != 0)
        {
            q = memchr(p, '\n', remain);
            if (q != NULL)
            {
                rc = PosWriteFile(linfiles[handle].handle,
                                  p,
                                  q - p,
                                  &thiswrite);
                if (rc != 0)
                {
                    return (-1);
                }
                writtenbytes += thiswrite;
                rc = PosWriteFile(linfiles[handle].handle,
                                  "\r\n",
                                  2,
                                  &thiswrite);
                if (rc != 0)
                {
                    return (-1);
                }
                writtenbytes += 1; /* not 2 */
                remain -= (q + 1 - p);
                p = q + 1;
            }
            else
            {
                rc = PosWriteFile(linfiles[handle].handle,
                                  p,
                                  remain,
                                  &thiswrite);
                if (rc != 0)
                {
                    return (-1);
                }
                writtenbytes += thiswrite;
                remain = 0;
            }
        }
    }
    else
    {
        rc = PosWriteFile(linfiles[handle].handle, buf, len, &writtenbytes);
        if (rc != 0)
        {
            return (-1);
        }
    }
    return (writtenbytes);
}

#define O_TEXT 0x40000000

int lin_open(char *fnm, unsigned int flags, unsigned int mode)
{
    int handle;
    int x;
    int rc;

    if (strncmp(fnm, "/proc", 5) == 0)
    {
        /* not a real file */
        return (MAXFILES);
    }
    else
    {
        for (x = 3; x < MAXFILES; x++)
        {
            if (!linfiles[x].inuse)
            {
                if (flags & 1)
                {
                    rc = PosCreatFile(fnm, 0, &handle);
                }
                else
                {
                    rc = PosOpenFile(fnm, 2, &handle);
                }
                if (rc == 0)
                {
                    linfiles[x].handle = handle;
                    if (flags & O_TEXT)
                    {
                        linfiles[x].text = 1;
                    }
                    else
                    {
                        linfiles[x].text = 0;
                    }
                    linfiles[x].inuse = 1;
                    /* the handle to Linux is different from the one
                       used by PDOS */
                    return (x);
                }
                break;
            }
        }
    }

    return (-1);
}

unsigned int lin_read(int handle, void *buf, unsigned int len)
{
    int rc;
    unsigned int readbytes;

    if (handle == MAXFILES)
    {
        if (len >= 300)
        {
            char *p;

            p = PosGetCommandLine();
            len = strlen(p);
            if (strlen(p) < 300)
            {
                strcpy(buf, p);
                p = buf;
                while ((p = strchr(p, ' ')) != NULL)
                {
                    *p++ = '\0';
                }
            }
            return (len);
        }
    }
    else
    {
        /* we should really have a more complex algorithm here, to
           translate CRLF into LF. However, currently PDPCLIB - even
           on Linux - is doing that translation anyway, so it will
           take a bit more effort to test */
        rc = PosReadFile(linfiles[handle].handle, buf, len, &readbytes);
        if (rc == 0)
        {
            return (readbytes);
        }
    }
    return (-1);
}

int lin_close(int handle)
{
    if (handle == MAXFILES)
    {
        return (0);
    }
    if (handle < MAXFILES)
    {
        PosCloseFile(linfiles[handle].handle);
        linfiles[handle].inuse = 0;
        return (0);
    }
    return (-1);
}

int lin_getpid(void)
{
    return (0);
}

int lin_ioctl(int handle, int command, int parm)
{
    return (0);
}

void *lin_mmap(void *p)
{
    int *q;

    q = p;
    return (PosAllocMem(q[1], 0));
}

int lin_munmap(void *p, unsigned int len)
{
    PosFreeMem(p);
    return (0);
}

int lin_execve(char *path, char **argv, char **envp)
{
    return (system(argv[2]));
}
