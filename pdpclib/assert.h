/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  assert.h - assert header file.                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __ASSERT_INCLUDED
#define __ASSERT_INCLUDED

#ifdef __PDPCLIB_DLL
#define __PDPCLIB_HEADFUNC __declspec(dllexport)
#endif

#ifndef __PDPCLIB_HEADFUNC
#define __PDPCLIB_HEADFUNC
#endif

__PDPCLIB_HEADFUNC int _assert(char *x, char *y, int z);

#ifdef NDEBUG
#define assert(ignore) (0)
#else
#define assert(x) (x) ? (0) : \
    _assert(#x, __FILE__, __LINE__)
#endif

#if defined(__PDOSGEN__)
#include <__os.h>

#define _assert __os->_assert

#endif


#endif
