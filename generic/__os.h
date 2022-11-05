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

typedef struct {
    /* a BIOS may not have this, as it implies the existence of a
       complete C library, not typical for a real BIOS. Even an OS
       won't necessarily have this. */
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
    /* the suggested name of the primary disk. Only applicable to
       BIOSes. Can be NULL on a diskless system */
    char *disk_name;
    int (*Xprintf)(const char *format, ...);
    int (**main)(int argc, char **argv);
    void *(*malloc)(size_t size);
    FILE *Xstdin;
    FILE *Xstdout;
    FILE *(*Xfopen)(const char *filename, const char *mode);
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
    int (*fflush)(FILE *stream);
    int (*Xsetvbuf)(FILE *stream, char *buf, int mode, size_t size);
    void *(*PosGetDTA)(void);
    int (*PosFindFirst)(char *pat, int attrib);
    int (*PosFindNext)(void);
    int (*PosGetDeviceInformation)(int handle, unsigned int *devinfo);
    int (*PosSetDeviceInformation)(int handle, unsigned int devinfo);
    char *(*ctime)(const time_t *timer);
    time_t (*time)(time_t *timer);
} OS;

extern OS *__os;

#endif
