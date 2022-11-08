/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  errno.h - errno header file.                                     */
/*                                                                   */
/*********************************************************************/

#ifndef __ERRNO_INCLUDED
#define __ERRNO_INCLUDED

#define EDOM 1
#define ERANGE 2

#define errno (*(_errno()))

int *_errno(void);

#if defined(__PDOSGEN__)
#include <__os.h>
#undef errno
#define errno (*(__os->_errno()))
#endif

#endif
