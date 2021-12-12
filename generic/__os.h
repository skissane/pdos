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
    int (*__start)(char *p);
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
    int (*fgetc)(void *stream);
    int (*fputc)(int c, void *stream);
    void *(*PosGetDTA)(void);
    int (*PosFindFirst)(char *pat, int attrib);
    int (*PosFindNext)(void);
    int (*PosGetDeviceInformation)(int handle, unsigned int *devinfo);
    int (*PosSetDeviceInformation)(int handle, unsigned int devinfo);
} OS;

extern OS *__os;

#endif
