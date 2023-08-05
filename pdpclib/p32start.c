/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  p32start - startup code for non-MSVCRT WIN32                     */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

__PDPCLIB_API__ int __start(char *p);

/* This is the main entry point of a console mode executable */

/* consider adding this line so that relocatables are generated
   for some/most/all versions of "ld". The reason for this is
   that, at least with binutils 2.14a, there is code in there
   that will only generate relocatables if there are some
   exported symbols (or some other conditions) */
/*__declspec(dllexport)*/
void mainCRTStartup(void)
{
    __start(0);
    return;
}

#ifndef NOUNDMAIN
void __main(void)
{
    return;
}
#endif
