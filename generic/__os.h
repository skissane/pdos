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
} OS;

extern OS *__os;

#endif
