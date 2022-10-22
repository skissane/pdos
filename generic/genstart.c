/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  genstart - startup routine for generic BIOS/OS interface         */
/*                                                                   */
/*********************************************************************/

#include "__os.h"

OS *bios;

extern int __minstart;

int __start(char *p);


/* This name is known to certain versions of "ld" as the entry
point of an application and there is no particular reason to not
use it. */

int __crt0(OS *bios_parm)
{
    bios = bios_parm;
    __minstart = 1;
    return (__start(NULL));
}
