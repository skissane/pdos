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

int biosmain(int argc, char **argv);

/* This name is known to certain versions of "ld" as the entry
point of an application and there is no particular reason to not
use it. */

int __crt0(OS *bios_parm)
{
    bios = bios_parm;
    *bios->main = biosmain;
    return (bios->__start(NULL));
}
