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

__PDPCLIB_API__ void longjmp(jmp_buf env, int val)
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
