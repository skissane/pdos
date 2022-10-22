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

#ifndef __OS_INCLUDED
#define __OS_INCLUDED

#include <stddef.h>

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
    int (*printf)(const char *format, ...);
    int (**main)(int argc, char **argv);
    void *(*malloc)(size_t size);
    void *Xstdin;
    void *Xstdout;
    void *(*fopen)(const char *filename, const char *mode);
    int (*fseek)(void *stream, long offset, int whence);
    size_t (*fread)(void *ptr, size_t size, size_t nmemb, void *stream);
    int (*fclose)(void *stream);
    size_t (*fwrite)(const void *ptr, size_t size, size_t nmemb, void *stream);
    char *(*fgets)(char *s, int n, void *stream);
    char *(*strchr)(const char *s, int c);
    int (*strcmp)(const char *s1, const char *s2);
    int (*strncmp)(const char *s1, const char *s2, size_t n);
    char (*strcpy)(char *s1, const char *s2);
    size_t (*strlen)(const char *s);
    int (*fgetc)(void *stream);
    int (*fputc)(int c, void *stream);
    int (*fflush)(void *stream);
    int (*setvbuf)(void *stream, char *buf, int mode, size_t size);
    void *(*PosGetDTA)(void);
    int (*PosFindFirst)(char *pat, int attrib);
    int (*PosFindNext)(void);
    int (*PosGetDeviceInformation)(int handle, unsigned int *devinfo);
    int (*PosSetDeviceInformation)(int handle, unsigned int devinfo);
} OS;

extern OS *__os;

#endif
