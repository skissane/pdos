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

int _assert(char *x, char *y, int z);

#ifdef NDEBUG
#define assert(ignore) (0)
#else
#define assert(x) (x) ? (0) : \
    _assert(#x, __FILE__, __LINE__)
#endif

#endif
