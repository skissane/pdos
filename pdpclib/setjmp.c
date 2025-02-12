/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  setjmp.c - implementation of stuff in setjmp.h                   */
/*                                                                   */
/*********************************************************************/

#include "setjmp.h"
#include "stddef.h"

#if defined(__WATCOMC__) && !defined(__32BIT__)
#define CTYP __cdecl
#else
#define CTYP
#endif

extern int CTYP __setj(jmp_buf env);
extern int CTYP __longj(void *);


/* It is inappropriate to have a setjmp function here - the macro
   needs to directy call some assembler function. Different optimization
   levels can determine whether ebp is pushed, so the stack changes -
   while you are attempting to get back up to the caller of this
   C function, which is what longjmp will need. */

#if 0

#ifdef __MSC__
#undef setjmp
#endif

#ifdef __MSC__
__PDPCLIB_API__ int setjmp(jmp_buf env)
#else
__PDPCLIB_API__ int _setjmp(jmp_buf env)
#endif
{
    return (__setj(env));
}

#endif




/* this function is no longer exported in msvcrt.lib and msvcrt.dll */
/* you now need to explicitly link with this object code, plus
   the supporting assembler code (winsupa.obj or whatever) */
/* see setjmp.h for an explanation as to why */

void longjmp(jmp_buf env, int val)
{
    if (val == 0)
    {
        val = 1;
    }
    env[0].retval = val;
    /* load regs */
    __longj(env);
    return;
}
