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

extern char mycmdline[400];

int biosmain(int argc, char **argv);

/* This name is known to certain versions of "ld" as the entry
point of an application and there is no particular reason to not
use it. */

int __crt0(OS *bios_parm)
{
    bios = bios_parm;
    *bios->main = biosmain;

    /* the original command line will be clobbered by the time it
       hits biosmain (due to the call to the bios's start), and
       we need to call the PDOS-generic version of start after
       that, so the command line needs to be preserved since it is
       PDOS-generic that has the unusual situation of needing the
       command line preserved */
    bios->strncpy(mycmdline, bios->cmd_line, sizeof mycmdline);
    mycmdline[sizeof mycmdline - 1] = '\0';

    return (bios->__start(NULL));
}
