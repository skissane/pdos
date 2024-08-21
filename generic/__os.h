/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  os.h - C library exported by OS                                  */
/*                                                                   */
/*********************************************************************/

/* Note that a BIOS that uses this interface is unlikely to populate
   all of these things, especially if running on hardware of the
   1980s era. You can expect restrictions such as only fopen (of a
   filename such as "0x80"), fread, fwrite, fseek actually working,
   and only working if you give them offsets and lengths that are
   multiples of 512 and/or the sector size. Also the buffer that
   data is read into/written from may have alignment requirements
   such as 16 bytes or 512 bytes. It depends on what is out there. */

#ifndef __OS_INCLUDED
#define __OS_INCLUDED

#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <setjmp.h>

#include <pos.h>

typedef struct {
    /* a BIOS may not have this, as it implies the existence of a
       complete C library, not typical for a real BIOS. Even an OS
       won't necessarily have this. */
    /* Change of plan - the C library will exist, even if "start" is
       just a few lines of code, that doesn't do parameters, but
       simply calls main */
    int (*__start)(char *p);
    /* a BIOS will typically have a block of memory it expects you
       to malloc. This is the size. A BIOS may or may not allow
       arbitrary mallocs that differ from this size. */
    size_t mem_amt;
    /* if this is true, it means that mem_amt is adjusted every
       time you call malloc, so that you can get multiple chunks
       of memory instead of requiring only contiguous memory.
       Only applicable to a BIOS. */
    int mem_rpt;
    /* the command line, including program name */
    char *cmd_line;
    int (*Xprintf)(const char *format, ...);
    int (**main)(int argc, char **argv);
    void *(*malloc)(size_t size);
    FILE *Xstdin;
    FILE *Xstdout;
#ifdef __SUBC__
    void * (*Xfopen)(const char *filename, const char *mode);
#else
    FILE *(*Xfopen)(const char *filename, const char *mode);
#endif
    int (*Xfseek)(FILE *stream, long offset, int whence);
    size_t (*Xfread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
    int (*Xfclose)(FILE *stream);
    size_t (*Xfwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream);
    char *(*Xfgets)(char *s, int n, FILE *stream);
    char *(*strchr)(const char *s, int c);
    int (*strcmp)(const char *s1, const char *s2);
    int (*strncmp)(const char *s1, const char *s2, size_t n);
    char *(*strcpy)(char *s1, const char *s2);
    size_t (*strlen)(const char *s);
    int (*Xfgetc)(FILE *stream);
    int (*Xfputc)(int c, FILE *stream);
    int (*Xfflush)(FILE *stream);
    int (*Xsetvbuf)(FILE *stream, char *buf, int mode, size_t size);
    void *(*XPosGetDTA)(void);
    int (*XPosFindFirst)(char *pat, int attrib);
    int (*XPosFindNext)(void);
    int (*XPosGetDeviceInformation)(int handle, unsigned int *devinfo);
    int (*XPosSetDeviceInformation)(int handle, unsigned int devinfo);
    char *(*Xctime)(const time_t *timer);
    time_t (*Xtime)(time_t *timer);
    int (*XPosChangeDir)(const char *to);
    int (*XPosMakeDir)(const char *dname);
    int (*XPosRemoveDir)(const char *dname);
    int (*Xremove)(const char *filename);
    void *(*memcpy)(void *s1, const void *s2, size_t n);
    char *(*strncpy)(char *s1, const char *s2, size_t n);
    char *(*strcat)(char *s1, const char *s2);
    FILE *Xstderr;
    void (*free)(void *ptr);
    void (*abort)(void);
    void *(*memset)(void *s, int c, size_t n);
    int (*Xfputs)(const char *s, FILE *stream);
    int (*Xfprintf)(FILE *stream, const char *format, ...);
    char *(*Xgetenv)(const char *name);
    void *(*memmove)(void *s1, const void *s2, size_t n);
    void (*Xexit)(int status);
    int (*memcmp)(const void *s1, const void *s2, size_t n);
    int *(*_errno)(void); /* internal use */
    char *(*Xtmpnam)(char *s);
    int (*Xvfprintf)(FILE *stream, const char *format, va_list arg);
    int (*Xungetc)(int c, FILE *stream);
    int (*Xvsprintf)(char *s, const char *format, va_list arg);
    int (*Xsprintf)(char *s, const char *format, ...);
#ifdef __SUBC__
    int (*signal)(int sig, int (*handler)());
#else
    void (*(*signal)(int sig, void (*func)(int)))(int);
#endif
    int (*raise)(int sig);
    void *(*Xcalloc)(size_t nmemb, size_t size);
    void *(*Xrealloc)(void *ptr, size_t size);
    int (*Xatoi)(const char *nptr);
    long (*Xstrtol)(const char *nptr, char **endptr, int base);
    unsigned long (*Xstrtoul)(const char *nptr, char **endptr, int base);
    void (*Xqsort)(void *a, size_t b, size_t c,
                  int (*f)(const void *d, const void *e));
#ifdef __SUBC__
    void *(*Xbsearch)(const void *key, const void *base,
                     size_t nmemb, size_t size,
                     int (*compar)());
#else
    void *(*Xbsearch)(const void *key, const void *base,
                     size_t nmemb, size_t size,
                     int (*compar)(const void *, const void *));
#endif
#ifdef __SUBC__
    void *(*Xlocaltime)();
#else
    struct tm *(*Xlocaltime)(const time_t *timer);
#endif
    clock_t (*Xclock)(void);
    char *(*strerror)(int errnum);
    char *(*strrchr)(const char *s, int c);
    char *(*strstr)(const char *s1, const char *s2);
    char *(*strpbrk)(const char *s1, const char *s2);
    size_t (*strspn)(const char *s1, const char *s2);
    size_t (*strcspn)(const char *s1, const char *s2);
    void *(*memchr)(const void *s, int c, size_t n);
    long (*Xftell)(FILE *stream);
    int (*Xabs)(int j);
    char *(*setlocale)(int category, const char *locale);
    void (*Xperror)(const char *s);
    void (*Xrewind)(FILE *stream);
    char *(*strncat)(char *s1, const char *s2, size_t n);
    int (*Xsscanf)(const char *s, const char *format, ...);
    int (*isalnum)(int c);
    int (*isxdigit)(int c);
    int (*Xrename)(const char *old, const char *newnam);
    void (*Xclearerr)(FILE *stream);
    int (*_assert)(char *x, char *y, int z); /* internal use */
    double (*Xatof)(const char *nptr);
    int (*isdigit)(int c);
    int (*isalpha)(int c);
    int (*isprint)(int c);
    int (*isspace)(int c);
    int (*tolower)(int c);
    int (*Xsystem)(const char *string);
    int (*islower)(int c);
    int (*isupper)(int c);
    int (*Xatexit)(void (*func)(void));
    double (*Xceil)(double x);
    int (*toupper)(int c);
    int (*iscntrl)(int c);
    double (*Xsin)(double x);
    double (*Xcos)(double x);
    double (*Xtan)(double x);
    double (*Xfloor)(double x);
    double (*Xasin)(double x);
    double (*Xacos)(double x);
    double (*Xatan)(double x);
    double (*Xsinh)(double x);
    double (*Xcosh)(double x);
    double (*Xtanh)(double x);
    int (*Xrand)(void);
    void (*Xsrand)(unsigned int seed);
    size_t (*Xstrftime)(char *s, size_t maxsize,
                        const char *format, const struct tm *timeptr);
    int (*Xputs)(const char *s);
    double (*Xpow)(double x, double y);
    double (*Xmodf)(double value, double *iptr);
    double (*Xlog)(double x);
    double (*Xlog10)(double x);
    double (*Xatan2)(double y, double x);
    double (*Xfabs)(double x);
    double (*Xexp)(double x);
    double (*Xsqrt)(double x);
    char *(*strtok)(char *s1, const char *s2);
    long (*Xatol)(const char *nptr);
    time_t (*Xmktime)(struct tm *timeptr);
    int (*Xvprintf)(const char *format, va_list arg);
    int (*Xferror)(FILE *stream);
    int (*Xputc)(int c, FILE *stream);
    int (*Xfeof)(FILE *stream);
    int (*Xgetc)(FILE *stream);
    int (*Xgetchar)(void);
    int (*Xputchar)(int c);
    int (*XPosExec)(char *prog_name, POSEXEC_PARMBLOCK *pb);
    void (*Xlongjmp)(jmp_buf env, int val);
} OS;

extern OS *__os;

#endif
