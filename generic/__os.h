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

typedef struct {
    int (*__start)(char *p);
    int (*printf)(const char *format, ...);
    int (**main)(int argc, char **argv);
} OS;

extern OS *__os;
