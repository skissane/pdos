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
#include "errno.h"

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
#define INCL_DOSMEMMGR
#ifdef __16BIT__
#undef NULL
#endif
#include <os2.h>
#ifdef __16BIT__
typedef USHORT APIRET;
#endif
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

int __mmgid = 0; /* memmgr id to use - normally 0 */

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
#elif defined(__gnu_linux__) && defined(__64BIT__)
#define MAX_CHUNK 124000000 /* maximum size we will store in memmgr */
#define REQ_CHUNK 124000000 /* size that we request from OS */
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
typedef struct {
    void *addr;
    size_t length;
    int prot;
    int flags;
    int fd;
    long offset;
    long offset2;
} mmstruc;

#if defined(__ARM__) || defined(__64BIT__)
void *__mmap(void *a, int b, int prot, int flags, int fd,
             long offset, long offset2);
#else
void *__mmap(mmstruc *mms);
#endif

int __munmap(void *a, size_t b);
int __fork(void);
long __clone(int a, void *b, void *c, void *d, void *e);
int __waitid(int a, long pid, void *c, int d, void *e);
int __execve(char *a, char **argv, void *c);
void __exita(int a);

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#define MAP_ANONYMOUS 0x20
#define MAP_PRIVATE 0x2
#define MAP_32BIT 0x40
#define MAP_SHARED 0x1
#define MAP_FAILED ((void *)-1)
#define O_RDWR   0x2
#endif

void (*__userExit[__NATEXIT])(void);

/* we should migrate everyone to use these internal routines */
#if defined(__OS2__)
static void *imalloc(size_t size)
{
#ifdef __OS2__
    PVOID BaseAddress;
    ULONG ulObjectSize;
    ULONG ulAllocationFlags;
    APIRET rc;
#ifdef __16BIT__
    USHORT numsegs;
    USHORT numbytes;
    USHORT sel;
#endif

    ulObjectSize = size;
#ifdef __16BIT__
    /* note that we could make this test 65536
       and then adjust the request to 0 */
    if (ulObjectSize <= 65535U)
    {
        if (DosAllocSeg((USHORT)ulObjectSize, &sel, 0) != 0)
        {
            return (NULL);
        }
    }
    else
    {
        numsegs = ulObjectSize / 65536U;
        numbytes = ulObjectSize % 65536U;
        if (numbytes != 0)
        {
            numsegs++;
        }
        if (DosAllocHuge(numsegs, numbytes, &sel, 0, 0) != 0)
        {
            return (NULL);
        }
    }
    BaseAddress = (void *)((unsigned long)sel << 16);
#else
    /* AracOS had VIRTUALADDRESSLIMIT in config.sys set to
       1536 MiB. This can be increased to a maximum of 3072,
       but some hardware, e.g. video memory, doesn't like
       that interference with "PCI space". Whatever decimal
       value you choose, the hex value should end 00, ie be
       a multiple of 64 MiB. (Thanks Dave Yeo for info) */
    ulAllocationFlags = PAG_COMMIT | PAG_WRITE | PAG_READ;

    /* Setting this flag gets around some allocation limitation
       that is probably related to the number of selectors of
       an 80386 which we don't care about. ie it seems I can
       only do about 8000 memory requests. We could use a
       heap manager - ie memmgr - instead. */
    /* Apparently OBJ_TILE (vs OBJ_ANY) is redundant since it
       is a default, and causes memory requests to be aligned
       on a 64k boundary. So 8192 * 64k maxes out the 512 MiB
       allowed for tiled memory. OBJ_ANY doesn't give any
       specific alignment apparently, but is presumably at
       least 4k-aligned */
    ulAllocationFlags |= OBJ_ANY;

    rc = DosAllocMem(&BaseAddress, ulObjectSize, ulAllocationFlags);
    if (rc != 0)
    {
        return (NULL);
    }
#endif
    return ((void *)BaseAddress);
#endif
}

/* in case the low-level routine needs the original size,
   we pass that as a parameter */
static int ifree(void *ptr, size_t size)
{
#ifdef __OS2__
#ifdef __16BIT__
    DosFreeSeg((unsigned long)ptr >> 16);
#else
    DosFreeMem((PVOID)ptr);
#endif
#endif
    return (0);
}
#endif



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

/* Note that at some point we probably want to constrain memory allocation to
   keep memory below 4 GiB. Here is an example of memory being allocated
   below 2 GiB - 5 pages of 4096 bytes each:

   EFI_PHYSICAL_ADDRESS mem = {0x80000000, 0};
   UINTN num_pages = 5;
   if (__gBS->AllocatePages (AllocateMaxAddress, EfiLoaderData, num_pages, &mem))
   if (__gBS->FreePages (mem, num_pages))

   Note that apparently cc64 generates object files that expect the program
   to be loaded under 2 GiB. It may be the lea is getting the sign bit from
   a 32-bit computed value propagated into the upper 32 bits. So it is time
   to constrain, and the constraint will be 2 GiB, not 4 Gib.
*/

    size_t *x = NULL;

/* for some reason the old algorithm works, even though it is returning
   8-byte alignment which is probably wrong when SSE2 instructions are
   involved. New algorithm requires 16-byte alignment for some things
   at least - further testing suggests that 8-byte alignment is fine,
   and the switch to 16-byte just masked an unrelated problem. The old
   algorithm is being adjusted too, for good measure. */
#if 0
    if (__gBS->AllocatePool(EfiLoaderData,
                            size + sizeof(size_t) * 2,
                            (void **)&x)
        != EFI_SUCCESS)
    {
        return (NULL);
    }
    x++;
    *x = size;
#else
#ifndef __NO_LONGLONG_AND_LONG_IS_ONLY_32BIT__
    EFI_PHYSICAL_ADDRESS mem = 0x80000000;
#else
    EFI_PHYSICAL_ADDRESS mem = {0x80000000, 0};
#endif
    size_t num_pages;

    num_pages = size + sizeof(size_t) * 2;
    if ((num_pages % 4096) != 0)
    {
        num_pages += 4096;
    }
    num_pages /= 4096;
    x = *(size_t **)&mem;
#if 0
    printf("requesting %d bytes\n", (int)size);
#endif
    if (__gBS->AllocatePages(AllocateMaxAddress,
                             EfiLoaderData,
                             (UINTN)num_pages,
                             (void *)&x)
        != EFI_SUCCESS)
    {
        return (NULL);
    }
    if (x == NULL)
    {
        return (NULL);
    }
#if 0
    /* this can be useful to check for uninitialized memory.
       Force it to a consistent value, and an odd number so
       that it may generate an alignment issue */
    memset(x, '\x99', num_pages * 4096);
#endif
    *x = num_pages * 4096;
    x++;
    *x = size;
#if 0
    printf("returning %p\n", x + 1);
#endif

#endif
    return (x + 1);
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

#elif defined(__OS2__)
        ptr = imalloc(size + sizeof(size_t));
        if (ptr != NULL)
        {
            *(size_t *)ptr = size;
            ptr = (char *)ptr + sizeof(size_t);
        }

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
        ptr = memmgrAllocate(&__memmgr, size, __mmgid);
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

#elif defined(__OS2__)
            ptr2 = imalloc(REQ_CHUNK);
            if (ptr2 != NULL)
            {
                *(size_t *)ptr2 = size;
                ptr2 = (char *)ptr2 + sizeof(size_t);
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
            ptr = memmgrAllocate(&__memmgr, size, __mmgid);
        }
    }
    return (ptr);

#else /* not MEMMGR */

#if defined(__MVS__) || defined(__CMS__)
    return (__getm(size));

#elif defined(__OS2__)
    size_t *baseaddress;

    baseaddress = imalloc(size + sizeof(size_t));
    if (baseaddress == NULL)
    {
        return (NULL);
    }

    *(size_t *)baseaddress = size;
    baseaddress++;
    return (baseaddress);

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

#if 0
    {
        int fd;
        /* may need this on old systems */
        fd = __open("/dev/zero", O_RDWR);
        ptr = __mmap(NULL, size + sizeof(size_t),
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE, fd, 0);
        close(fd); /* I think this can be done immediately */
    }
#else
    {
        mmstruc mms;

        mms.addr = NULL;
        mms.length = size + sizeof(size_t);
#ifdef __MACOS__
        mms.prot = PROT_READ | PROT_WRITE;
#else
        mms.prot = PROT_READ | PROT_WRITE | PROT_EXEC;
#endif
        mms.flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT;
        mms.fd = -1;
        mms.offset = 0;
        mms.offset2 = 0;
#if defined(__ARM__) || defined(__64BIT__)
        ptr = __mmap(mms.addr, mms.length, mms.prot,
                     mms.flags, mms.fd, mms.offset, mms.offset2);
#else
        ptr = __mmap(&mms);
#endif

        /* errors are high pointers - not sure how high - which
           seem to be a negative errno */
        if (ptr > (void *)-256)
        {
            ptr = NULL;
        }
        else
        {
            *(size_t *)ptr = size + sizeof(size_t);
            ptr = (char *)ptr + sizeof(size_t);
        }
    }
#endif
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
#if 0
        printf("freeing %p\n", ptr);
#endif
        ptr = (char *)ptr - sizeof(size_t) * 2;

#if 0
        __gBS->FreePool(ptr);
#else
        {
            UINTN num_pages;

            /* for EFI we maintain both the size requested
               by the user (realloc needs this) plus the size we
               actually requested from EFI (we now need this) */
            num_pages = *(size_t *)ptr / 4096;
#if 0
            /* this may be useful for debugging purposes - trash
	       the memory being freed before freeing it, with a
	       consistent and odd value, so that if anyone attempts
	       to use it after freeing they will hopefully get an
	       alignment error */
            memset(ptr, '\x77', num_pages * 4096);
#endif
            __gBS->FreePages(ptr, num_pages);
        }
#endif
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

#elif defined(__OS2__)
            ptr = (char *)ptr - sizeof(size_t);
            ifree(ptr, *(size_t *)ptr);

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

#ifdef __OS2__
    if (ptr != NULL)
    {
        ptr = (char *)ptr - sizeof(size_t);
        ifree(ptr, *(size_t *)ptr);
    }
#endif

#ifdef __WIN32__
    if (ptr != NULL)
    {
        GlobalFree(((size_t *)ptr) - 1);
    }
#endif

#ifdef __gnu_linux__
    if (ptr != NULL)
    {
        ptr = (char *)ptr - sizeof(size_t);
        /* this returns 0 on success */
        __munmap(ptr, *(size_t *)ptr);
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

#if !defined(__OS2__)
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
#endif

__PDPCLIB_API__ char *getenv(const char *name)
{
#if defined(__OS2__) && !defined(__16BIT__)
    PSZ result;

    if (DosScanEnv((void *)name, (void *)&result) == 0)
    {
        return ((char *)result);
    }
#elif defined(__OS2__)
    USHORT seg, junk;
    char *env;

    if (DosGetEnv(&seg, &junk) != 0)
    {
        return (NULL);
    }
    /* this far pointer construction is unfortunate, but
       it allows us to use only Family API which may be
       useful to some */
    env = (char *)((unsigned long)seg << 16);
#endif
#if defined(__MSDOS__) || defined(__WIN32__)
    char *env;
    size_t lenn;

#ifdef __WIN32__
    env = GetEnvironmentStrings();
#else
    env = (char *)__envptr;
#endif
    if (env == NULL) return (NULL);
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
    char cmdbuf[400 + 10];
    char *cmdproc;

    cmdproc = getenv("COMSPEC");
    if (cmdproc == NULL)
    {
        return (-1);
    }
    if (string == NULL)
    {
        return (1);
    }
    memcpy(cmdbuf, "cmd\0/c ", 7);
    strncpy(cmdbuf + 7, string, sizeof cmdbuf - 1 - 7 - 1);
    cmdbuf[sizeof cmdbuf - 1] = '\0';
    cmdbuf[sizeof cmdbuf - 2] = '\0';
    rc = DosExecPgm(err_obj, sizeof err_obj, EXEC_SYNC,
                    (PSZ)cmdbuf, NULL, &results, (PSZ)cmdproc);
    if (rc != 0)
    {
        return (rc);
    }

#if 0
    if (results.codeTerminate != 0)
    {
        return (-(int)results.codeTerminate);
    }
#endif

    return ((int)results.codeResult);
#endif

#ifdef __WIN32__
    BOOL rc;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    DWORD ExitCode;
    char *cmdproc;
    char cmdbuf[400];

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
    static unsigned char cmdt[400];
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

#if defined(__gnu_linux__)
    int rc;
    long pid;
    static int argc = 4;
    static char *argv[] = { "sh",
                            "-c",
                            NULL, /* will be filled in */
                            NULL };

    /* printf("in system\n"); */
    argv[2] = (char *)string;
    errno = 0;

    /* CLONE_VFORK 0x4000 was needed to give the child a
       chance to start, and waitid is still required, which
       doesn't seem to match the documentation, but the
       documentation wasn't really for the syscall */
#ifndef __MACOS__
    pid = __clone(0x4000, NULL, NULL, NULL, NULL);
#else
    pid = __fork();
#endif
    if (pid != 0)
    {
        /* printf("in parent\n"); */
        /* first parm is 0 for P_ALL or 1 for P_PID */
        /* 4 in 4th parm = W_EXITED */
        rc = __waitid(0, pid, NULL, 4, NULL);
        /* printf("finished waiting\n"); */
    }
    else
    {
        /* printf("in child\n"); */
        /* on real Linux  doesn't return, but on PDOS/386
           it will. And the PDOS/386 app may return -1 as a valid
           return code. So we instead rely on Linux setting
           errno if we get a -1, and we clear it first. But we
           do that clear before getting here, so that we fit
           within the rules of vfork where we are supposed to
           immediately execute execve. Just in case that is
           needed. */
        rc = __execve("/bin/sh", argv, NULL);
        if ((rc == -1) && (errno != 0))
        {
            /* failure to execute on Linux - exit with -1 */
            /* as per vfork rules, don't go through full C library
               exit as that is unnecessary and not allowed */
            __exita(-1);
        }
        /* printf("will not print normally\n"); */ /* PDOS will print */
    }
    return (rc);
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
