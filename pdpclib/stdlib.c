/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  stdlib.c - implementation of stuff in stdlib.h                   */
/*                                                                   */
/*********************************************************************/

#include "stdlib.h"
#include "signal.h"
#include "string.h"
#include "ctype.h"
#include "stddef.h"

/* VSE is similar to MVS */
#if defined(__VSE__)
#define __MVS__ 1
#endif

/* PDOS and MSDOS use the same interface most of the time */
#if defined(__PDOS386__) || defined(__SMALLERC__)
#define __MSDOS__
#endif

#ifdef __PDOS386__
#include <pos.h>
#endif

#ifdef __AMIGA__
#include <clib/exec_protos.h>
#endif

#ifdef __EFI__
#include "efi.h"
#endif

#ifdef __OS2__
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#include <os2.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

#if defined(__MVS__) || defined(__CMS__)
#include "mvssupa.h"
#endif

#ifdef __MVS__
extern int __tso;
#endif

#if USE_MEMMGR
#include "__memmgr.h"
/* GCCMVS 3.4.6 requires 49 MB minimum for full optimization */
/* so we give it 60. GCCMVS 3.2.3 only requires 20 MB */
/* Note that you can set MAX_CHUNK to less than REQ_CHUNK */
/* But don't do this until MVS/380 etc have been changed to */
/* allow multiple memory requests. */
/* But bump it up to almost 64 MiB so that if CMS is misconfigured */
/* it tries to get almost 16 MiB (and from subpool 3) so should fail */

#if defined(MULMEM)
#define MAX_CHUNK 67100672
#define REQ_CHUNK 67100672
#elif defined(__gnu_linux__) || defined(__ARM__)
#define MAX_CHUNK 30000000 /* maximum size we will store in memmgr */
#define REQ_CHUNK 30000000 /* size that we request from OS */
#else
#define MAX_CHUNK 67100672 /* maximum size we will store in memmgr */
#define REQ_CHUNK 67100672 /* size that we request from OS */
#endif
void *__lastsup = NULL; /* last thing supplied to memmgr */
#endif

#ifdef __MSDOS__
#if defined(__WATCOMC__) && !defined(__32BIT__)
#define CTYP __cdecl
#else
#define CTYP
#endif
#if defined(__32BIT__) && !defined(NOLIBALLOC)
/* For PDOS-32 liballoc is used for memory management. */
#include "liballoc.h"
#else
void CTYP __allocmem(size_t size, void **ptr);
void CTYP __freemem(void *ptr);
#endif
extern unsigned char *__envptr;
int CTYP __exec(char *cmd, void *env);
int CTYP __getrc(void);
#endif

#if defined(__gnu_linux__) || defined(__ARM__)
void *__allocmem(size_t size);
#endif

void (*__userExit[__NATEXIT])(void);

__PDPCLIB_API__ void *malloc(size_t size)
{
#ifdef __AMIGA__
    size_t *x;

    x = AllocMem(size + sizeof(size_t), 0);
    if (x == NULL) return (NULL);
    *x = size;
    return (x + 1);
#endif
#ifdef __EFI__
    size_t *x = NULL;

    if (__gBS->AllocPool(EfiLoaderData, size + sizeof(size_t), (void **)&x)
        != EFI_SUCCESS)
    {
        return (NULL);
    }
    *x = size;
    return (x + 1);
#endif
#ifdef __OS2__
    PVOID BaseAddress;
    ULONG ulObjectSize;
    ULONG ulAllocationFlags;
    APIRET rc;

    ulObjectSize = size + sizeof(size_t);
    ulAllocationFlags = PAG_COMMIT | PAG_WRITE | PAG_READ;
    rc = DosAllocMem(&BaseAddress, ulObjectSize, ulAllocationFlags);
    if (rc != 0) return (NULL);
    *(size_t *)BaseAddress = size;
    BaseAddress = (char *)BaseAddress + sizeof(size_t);
    return ((void *)BaseAddress);
#endif
#ifdef __MSDOS__
#if defined(__32BIT__) && !defined(NOLIBALLOC)
    return (__malloc(size));
#else
    size_t *ptr;

    __allocmem(size + sizeof(size_t), (void **)&ptr);
    if (ptr == NULL) return (NULL);
    *ptr = size;
    return (ptr + 1);
#endif
#endif
#if USE_MEMMGR
    void *ptr;

    if (size > MAX_CHUNK)
    {
#if defined(__MVS__) || defined(__CMS__) || defined(__gnu_linux__) \
    || defined(__ARM__)
#if defined(MULMEM)
        /* If we support multiple memory requests */
        ptr = __getm(size);
#else
        ptr = NULL;
#endif
#elif defined(__WIN32__)
        ptr = GlobalAlloc(0, size + sizeof(size_t));
        if (ptr != NULL)
        {
            *(size_t *)ptr = size;
            ptr = (char *)ptr + sizeof(size_t);
        }
#elif defined(__gnu_linux__) || defined(__ARM__)
        ptr = __allocmem(size + sizeof(size_t));
        if (ptr != NULL)
        {
            *(size_t *)ptr = size;
            ptr = (char *)ptr + sizeof(size_t);
        }
#endif
    }
    else
    {
        ptr = memmgrAllocate(&__memmgr, size, 0);
        if (ptr == NULL)
        {
            void *ptr2;

#if defined(__MVS__) || defined(__CMS__)
            /* until MVS/380 is fixed, don't do an additional request,
               unless MULMEM is defined */
#if defined(MULMEM)
            if (1)
#else
            if (__memmgr.start == NULL)
#endif
            {
                ptr2 = __getm(REQ_CHUNK);
            }
            else
            {
                ptr2 = NULL;
            }
#elif defined(__WIN32__)
            ptr2 = GlobalAlloc(0, REQ_CHUNK);
            if (ptr2 != NULL)
            {
                *(size_t *)ptr2 = size;
                ptr2 = (char *)ptr2 + sizeof(size_t);
            }
#elif defined(__gnu_linux__) || defined(__ARM__)
            if (__memmgr.start == NULL)
            {
                ptr2 = __allocmem(REQ_CHUNK);
            }
            else
            {
                ptr2 = NULL;
            }
            if (ptr2 != NULL)
            {
                *(size_t *)ptr2 = size;
                ptr2 = (char *)ptr2 + sizeof(size_t);
            }
#endif
            if (ptr2 == NULL)
            {
                return (NULL);
            }
            __lastsup = ptr2;
            memmgrSupply(&__memmgr, ptr2, REQ_CHUNK);
            ptr = memmgrAllocate(&__memmgr, size, 0);
        }
    }
    return (ptr);
#else /* not MEMMGR */
#if defined(__MVS__) || defined(__CMS__)
    return (__getm(size));
#elif defined(__WIN32__)
    void *ptr;

    ptr = GlobalAlloc(0, size + sizeof(size_t));
    if (ptr != NULL)
    {
        *(size_t *)ptr = size;
        ptr = (char *)ptr + sizeof(size_t);
    }
    return (ptr);
#elif defined(__gnu_linux__) || defined(__ARM__)
    void *ptr;

    ptr = __allocmem(size + sizeof(size_t));
    if (ptr != NULL)
    {
        *(size_t *)ptr = size;
        ptr = (char *)ptr + sizeof(size_t);
    }
    return (ptr);
#endif
#endif /* not MEMMGR */
}

__PDPCLIB_API__ void *calloc(size_t nmemb, size_t size)
{
    void *ptr;
    size_t total;

    if (nmemb == 1)
    {
        total = size;
    }
    else if (size == 1)
    {
        total = nmemb;
    }
    else
    {
        total = nmemb * size;
    }
    ptr = malloc(total);
    if (ptr != NULL)
    {
        memset(ptr, '\0', total);
    }
    return (ptr);
}

__PDPCLIB_API__ void *realloc(void *ptr, size_t size)
{
#if defined(__PDOS386__) && !defined(NOLIBALLOC)
    return (__realloc(ptr, size));
#else
    char *newptr;
    size_t oldsize;

    if (size == 0)
    {
        free(ptr);
        return (NULL);
    }
#if USE_MEMMGR
    if (memmgrRealloc(&__memmgr, ptr, size) == 0)
    {
        return (ptr);
    }
#endif
    newptr = malloc(size);
    if (newptr == NULL)
    {
        return (NULL);
    }
    if (ptr != NULL)
    {
        oldsize = *((size_t *)ptr - 1);
        if (oldsize < size)
        {
            size = oldsize;
        }
        memcpy(newptr, ptr, size);
        free(ptr);
    }
    return (newptr);
#endif
}

__PDPCLIB_API__ void free(void *ptr)
{
#ifdef __AMIGA__
    if (ptr != NULL)
    {
        ptr = (char *)ptr - sizeof(size_t);
        FreeMem(ptr, *(size_t *)ptr + sizeof(size_t));
    }
#endif
#ifdef __EFI__
    if (ptr != NULL)
    {
        ptr = (char *)ptr - sizeof(size_t);
        __gBS->FreePool(ptr);
    }
#endif
#ifdef __OS2__
    if (ptr != NULL)
    {
        ptr = (char *)ptr - sizeof(size_t);
        DosFreeMem((PVOID)ptr);
    }
#endif
#ifdef __MSDOS__
#if defined(__32BIT__) && !defined(NOLIBALLOC)
    __free(ptr);
#else
    if (ptr != NULL)
    {
        ptr = (char *)ptr - sizeof(size_t);
        __freemem(ptr);
    }
#endif
#endif
#if USE_MEMMGR
    if (ptr != NULL)
    {
        size_t size;

        size = *((size_t *)ptr - 1);
        if (size > MAX_CHUNK)
        {
#if defined(__MVS__) || defined(__CMS__)
#if defined(MULMEM)
            /* Ignore, unless MULMEM is defined, until MVS/380 is fixed */
            __freem(ptr);
#endif
#elif defined(__WIN32__)
            GlobalFree(((size_t *)ptr) - 1);
#endif
        }
        else
        {
            memmgrFree(&__memmgr, ptr);
        }
    }
#else /* not using MEMMGR */
#if defined(__MVS__) || defined(__CMS__)
    if (ptr != NULL)
    {
        __freem(ptr);
    }
#endif
#ifdef __WIN32__
    if (ptr != NULL)
    {
        GlobalFree(((size_t *)ptr) - 1);
    }
#endif
#endif /* not USE_MEMMGR */
    return;
}

__PDPCLIB_API__ void abort(void)
{
    raise(SIGABRT);
    exit(EXIT_FAILURE);
#if !defined(__EMX__) && !defined(__GNUC__) && !defined(__WIN32__) \
  && !defined(__gnu_linux__) && !defined(__ARM__)
    return;
#endif
}

#if !defined(__EMX__) && !defined(__GNUC__) && !defined(__gnu_linux__) \
    && !defined(__ARM__) \
    || defined(WATLIN)
void __exit(int status);
#else
void __exit(int status) __attribute__((noreturn));
#endif

__PDPCLIB_API__ void exit(int status)
{
    __exit(status);
#if !defined(__EMX__) && !defined(__GNUC__) && !defined(__WIN32__) \
  && !defined(__gnu_linux__) && !defined(__ARM__)
    return;
#endif
}

/* This qsort routine was obtained from libnix (also public domain),
 * and then reformatted.
 *
 * This qsort function does a little trick:
 * To reduce stackspace it iterates the larger interval instead of doing
 * the recursion on both intervals.
 * So stackspace is limited to 32*stack_for_1_iteration =
 * 32*4*(4 arguments+1 returnaddress+11 stored registers) = 2048 Bytes,
 * which is small enough for everybodys use.
 * (And this is the worst case if you own 4GB and sort an array of chars.)
 * Sparing the function calling overhead does improve performance, too.
 */

__PDPCLIB_API__ void qsort(void *base,
           size_t nmemb,
           size_t size,
           int (*compar)(const void *, const void *))
{
    char *base2 = (char *)base;
    size_t i,a,b,c;

    while (nmemb > 1)
    {
        a = 0;
        b = nmemb-1;
        c = (a+b)/2; /* Middle element */
        for (;;)
        {
            while ((*compar)(&base2[size*c],&base2[size*a]) > 0)
            {
                a++; /* Look for one >= middle */
            }
            while ((*compar)(&base2[size*c],&base2[size*b]) < 0)
            {
                b--; /* Look for one <= middle */
            }
            if (a >= b)
            {
                break; /* We found no pair */
            }
            for (i=0; i<size; i++) /* swap them */
            {
                char tmp=base2[size*a+i];

                base2[size*a+i]=base2[size*b+i];
                base2[size*b+i]=tmp;
            }
            if (c == a) /* Keep track of middle element */
            {
                c = b;
            }
            else if (c == b)
            {
                c = a;
            }
            a++; /* These two are already sorted */
            b--;
        } /* a points to first element of right interval now
             (b to last of left) */
        b++;
        if (b < nmemb-b) /* do recursion on smaller interval and
                            iteration on larger one */
        {
            qsort(base2,b,size,compar);
            base2=&base2[size*b];
            nmemb=nmemb-b;
        }
        else
        {
            qsort(&base2[size*b],nmemb-b,size,compar);
            nmemb=b;
        }
    }
    return;
}


static unsigned long myseed = 1;

__PDPCLIB_API__ void srand(unsigned int seed)
{
    myseed = seed;
    return;
}

__PDPCLIB_API__ int rand(void)
{
    int ret;

    myseed = myseed * 1103515245UL + 12345;
#if defined(__32BIT__) || defined(__WIN32__)
    /* I don't know if this is valid */
    /* But I don't want the high 16 bits to be zero */
    ret = (int)((myseed >> 16) & 0x7fff);
    myseed = myseed * 1103515245UL + 12345;
    ret = (ret << 16) | (int)((myseed >> 16) & 0xffff);
#else
    ret = (int)((myseed >> 16) & 0x7fff);
#endif
    return (ret);
}

__PDPCLIB_API__ double atof(const char *nptr)
{
    return (strtod(nptr, (char **)NULL));
}

__PDPCLIB_API__ double strtod(const char *nptr, char **endptr)
{
    double x = 0.0;
    double xs= 1.0;
    double es = 1.0;
    double xf = 0.0;
    double xd = 1.0;

    while( isspace( (unsigned char)*nptr ) ) ++nptr;
    if(*nptr == '-')
    {
        xs = -1;
        nptr++;
    }
    else if(*nptr == '+')
    {
        nptr++;
    }


    while (1)
    {
        if (isdigit((unsigned char)*nptr))
        {
            x = x * 10 + (*nptr - '0');
            nptr++;
        }
        else
        {
            x = x * xs;
            break;
        }
    }
    if (*nptr == '.')
    {
        nptr++;
        while (1)
        {
            if (isdigit((unsigned char)*nptr))
            {
                xf = xf * 10 + (*nptr - '0');
                xd = xd * 10;
            }
            else
            {
                x = x + xs * (xf / xd);
                break;
            }
            nptr++;
        }
    }
    if ((*nptr == 'e') || (*nptr == 'E'))
    {
        nptr++;
        if (*nptr == '-')
        {
            es = -1;
            nptr++;
        }
        xd = 1;
        xf = 0;
        while (1)
        {
            if (isdigit((unsigned char)*nptr))
            {
                xf = xf * 10 + (*nptr - '0');
                nptr++;
            }
            else
            {
                while (xf > 0)
                {
                    xd = xd * 10;
                    xf = xf - 1;
                }
                if (es < 0.0)
                {
                    x = x / xd;
                }
                else
                {
                    x = x * xd;
                }
                break;
            }
        }
    }
    if (endptr != NULL)
    {
        *endptr = (char *)nptr;
    }
    return (x);
}

__PDPCLIB_API__ int atoi(const char *nptr)
{
    return ((int)strtol(nptr, (char **)NULL, 10));
}

__PDPCLIB_API__ long int atol(const char *nptr)
{
    return (strtol(nptr, (char **)NULL, 10));
}

/* this logic is also in vvscanf - if you update this, update
   that one too */

__PDPCLIB_API__ unsigned long int strtoul(
    const char *nptr, char **endptr, int base)
{
    unsigned long x = 0;
    int undecided = 0;

    if (base == 0)
    {
        undecided = 1;
    }
    while (isspace((unsigned char)*nptr))
    {
        nptr++;
    }
    while (1)
    {
        if (isdigit((unsigned char)*nptr))
        {
            if (base == 0)
            {
                if (*nptr == '0')
                {
                    base = 8;
                }
                else
                {
                    base = 10;
                    undecided = 0;
                }
            }
            x = x * base + (*nptr - '0');
            nptr++;
        }
        else if (isalpha((unsigned char)*nptr))
        {
            if ((*nptr == 'X') || (*nptr == 'x'))
            {
                if ((base == 0) || ((base == 8) && undecided))
                {
                    base = 16;
                    undecided = 0;
                    nptr++;
                }
                else if (base == 16)
                {
                    /* hex values are allowed to have an optional 0x */
                    nptr++;
                }
                else
                {
                    break;
                }
            }
            else if (base <= 10)
            {
                break;
            }
            else
            {
                x = x * base + (toupper((unsigned char)*nptr) - 'A') + 10;
                nptr++;
            }
        }
        else
        {
            break;
        }
    }
    if (endptr != NULL)
    {
        *endptr = (char *)nptr;
    }
    return (x);
}

__PDPCLIB_API__ long int strtol(const char *nptr, char **endptr, int base)
{
    unsigned long y;
    long x;
    int neg = 0;

    while (isspace((unsigned char)*nptr))
    {
        nptr++;
    }
    if (*nptr == '-')
    {
        neg = 1;
        nptr++;
    }
    else if (*nptr == '+')
    {
        nptr++;
    }
    y = strtoul(nptr, endptr, base);
    if (neg)
    {
        x = (long)-y;
    }
    else
    {
        x = (long)y;
    }
    return (x);
}

__PDPCLIB_API__ int mblen(const char *s, size_t n)
{
    if (s == NULL)
    {
        return (0);
    }
    if (n == 1)
    {
        return (1);
    }
    else
    {
        return (-1);
    }
}

__PDPCLIB_API__ int mbtowc(wchar_t *pwc, const char *s, size_t n)
{
    if (s == NULL)
    {
        return (0);
    }
    if (n == 1)
    {
        if (pwc != NULL)
        {
            *pwc = *s;
        }
        return (1);
    }
    else
    {
        return (-1);
    }
}

__PDPCLIB_API__ int wctomb(char *s, wchar_t wchar)
{
    if (s != NULL)
    {
        *s = wchar;
        return (1);
    }
    else
    {
        return (0);
    }
}

__PDPCLIB_API__ size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
    strncpy((char *)pwcs, s, n);
    if (strlen(s) >= n)
    {
        return (n);
    }
    return (strlen((char *)pwcs));
}

__PDPCLIB_API__ size_t wcstombs(char *s, const wchar_t *pwcs, size_t n)
{
    strncpy(s, (const char *)pwcs, n);
    if (strlen((const char *)pwcs) >= n)
    {
        return (n);
    }
    return (strlen(s));
}

#ifdef abs
#undef abs
#endif
__PDPCLIB_API__ int abs(int j)
{
    if (j < 0)
    {
        j = -j;
    }
    return (j);
}

__PDPCLIB_API__ div_t div(int numer, int denom)
{
    div_t x;

    x.quot = numer / denom;
    x.rem = numer % denom;
    return (x);
}

#ifdef labs
#undef labs
#endif
__PDPCLIB_API__ long int labs(long int j)
{
    if (j < 0)
    {
        j = -j;
    }
    return (j);
}

__PDPCLIB_API__ ldiv_t ldiv(long int numer, long int denom)
{
    ldiv_t x;

    x.quot = numer / denom;
    x.rem = numer % denom;
    return (x);
}

__PDPCLIB_API__ int atexit(void (*func)(void))
{
    int x;

    for (x = 0; x < __NATEXIT; x++)
    {
        if (__userExit[x] == 0)
        {
            __userExit[x] = func;
            return (0);
        }
    }
    return (-1);
}

static int ins_strncmp(const char *one, const char *two, size_t len)
{
    size_t x = 0;

    if (len == 0) return (0);
    while ((x < len)
           && (toupper((unsigned char)*one) == toupper((unsigned char)*two)))
    {
        if (*one == '\0')
        {
            return (0);
        }
        one++;
        two++;
        x++;
    }
    if (x == len) return (0);
    return (toupper((unsigned char)*one) - toupper((unsigned char)*two));
}

__PDPCLIB_API__ char *getenv(const char *name)
{
#ifdef __OS2__
    PSZ result;

    if (DosScanEnv((void *)name, (void *)&result) == 0)
    {
        return ((char *)result);
    }
#endif
#if defined(__MSDOS__) || defined(__WIN32__)
    char *env;
    size_t lenn;

#ifdef __WIN32__
    env = GetEnvironmentStrings();
#else
    env = (char *)__envptr;
#endif
    lenn = strlen(name);
    while (*env != '\0')
    {
        if (ins_strncmp(env, name, lenn) == 0)
        {
            if (env[lenn] == '=')
            {
                return (&env[lenn + 1]);
            }
        }
        env = env + strlen(env) + 1;
    }
#endif
    return (NULL);
}

/* The following code was taken from Paul Markham's "EXEC" program,
   and adapted to create a system() function.  The code is all
   public domain */

__PDPCLIB_API__ int system(const char *string)
{
#ifdef __OS2__
    char err_obj[100];
    APIRET rc;
    RESULTCODES results;

    if (string == NULL)
    {
        return (1);
    }
    rc = DosExecPgm(err_obj, sizeof err_obj, EXEC_SYNC,
                    (PSZ)string, NULL, &results, (PSZ)string);
    if (rc != 0)
    {
        return (rc);
    }
    return ((int)results.codeResult);
#endif
#ifdef __WIN32__
    BOOL rc;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    DWORD ExitCode;
    char *cmdproc;
    char cmdbuf[300];

    cmdproc = getenv("COMSPEC");
    if (cmdproc == NULL)
    {
        return (-1);
    }
    if (strlen(string) + strlen(cmdproc) > sizeof cmdbuf - 10)
    {
        return (-2);
    }
    strcpy(cmdbuf, cmdproc);
    strcat(cmdbuf, " /c ");
    strcat(cmdbuf, string);

    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    memset(&pi, 0, sizeof pi);

    rc = CreateProcess(cmdproc,
                       cmdbuf,
                       NULL,
                       NULL,
                       FALSE,
                       0,
                       NULL,
                       NULL,
                       &si,
                       &pi);
    if (!rc)
    {
        int ret;

        ret = GetLastError();
        if (ret == 0)
        {
            ret = -1; /* force an error */
        }
        return (ret);
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &ExitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (ExitCode);
#endif
#ifdef __MSDOS__
    int rc;
#ifdef __PDOS386__
    static unsigned char cmdt[300];
#else
    static unsigned char cmdt[140];
#endif
    static
#ifdef __PDOS386__
    POSEXEC_PARMBLOCK
#else
    struct {
        unsigned short env;
        unsigned char *cmdtail;
        char *fcb1;
        char *fcb2;
    }
#endif
        parmblock = { 0, cmdt, NULL, NULL };
    size_t len;
    char *cmd;

    if (string == NULL)
    {
        return (1);
    }
    cmd = getenv("COMSPEC");
    if (cmd == NULL)
    {
        cmd = "\\command.com";
    }
#ifdef __PDOS386__
    if ((strlen(cmd) + strlen(string) + 5) > sizeof cmdt)
    {
        return (-1);
    }
    strcpy(cmdt, cmd);
    strcat(cmdt, " /c ");
    strcat(cmdt, string);
#else
    len = strlen(string);
    cmdt[0] = (unsigned char)(len + 3);
    memcpy(&cmdt[1], "/c ", 3);
    memcpy(&cmdt[4], string, len);
    memcpy(&cmdt[len + 4], "\r", 2);
#endif
    rc = __exec(cmd, &parmblock);
    if (rc != 0) return (-rc);
    return (__getrc());
#endif
#if defined(MUSIC)
    return (__system(strlen(string), string));
#elif defined(__MVS__)
    char pgm[9];
    size_t pgm_len;
    size_t cnt;
    char *p;

    p = strchr(string, ' ');
    if (p == NULL)
    {
        p = strchr(string, '\0');
    }

    pgm_len = p - string;
    /* don't allow a program name greater than 8 */

    if (pgm_len > 8)
    {
        return (-1);
    }
    memcpy(pgm, string, pgm_len);
    pgm[pgm_len] = '\0';

    /* uppercase the program name */
    for (cnt = 0; cnt < pgm_len; cnt++)
    {
        pgm[cnt] = toupper((unsigned char)pgm[cnt]);
    }

    /* point to parms */
    if (*p != '\0')
    {
        p++;
    }

    /* all parms now available */
    /* we use 1 = batch or 2 = tso */
    return (__system(__tso ? 2: 1, pgm_len, pgm, strlen(p), p));
#endif
#if defined(__CMS__)
    /* not implemented yet */
    return (0);
#endif
}

__PDPCLIB_API__ void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
    size_t try;
    int res;
    const void *ptr;

    while (nmemb > 0)
    {
        try = nmemb / 2;
        ptr = (void *)((char *)base + try * size);
        res = compar(ptr, key);
        if (res == 0)
        {
            return ((void *)ptr);
        }
        else if (res < 0)
        {
            nmemb = nmemb - try - 1;
            base = (const void *)((const char *)ptr + size);
        }
        else
        {
            nmemb = try;
        }
    }
    return (NULL);
}
