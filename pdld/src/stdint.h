/******************************************************************************
 * @file            stdint.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#ifndef __STDINT_INCLUDED
#define __STDINT_INCLUDED

#ifndef NO_LONG_LONG

typedef long long int_fast64_t;
typedef unsigned long long uint_fast64_t;

#else

typedef long int_fast64_t;
typedef unsigned long uint_fast64_t;

#endif

#endif /* __STDINT_INCLUDED */
