/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  supportc - dummy support routines since none of this is going    */
/*  to work anyway, as there is no PDOS/x64 yet.                     */
/*                                                                   */
/*********************************************************************/

#include "support.h"

#if defined(__WATCOMC__) && !defined(WATNATIVE)
#define CTYP __cdecl
#else
#define CTYP
#endif

int CTYP int86(int intno, union REGS *regsin, union REGS *regsout)
{
    return (0);
}

int CTYP int86x(int intno, union REGS *regsin,
           union REGS *regsout, struct SREGS *sregs)
{
    return (0);
}
