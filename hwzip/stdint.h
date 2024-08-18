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

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#ifndef NO_LONG_LONG

typedef long int32_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

#else

typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

#endif

/* Defined as 2**N - 1. */
#define UINT8_MAX ((1U << 8) - 1)
#define UINT16_MAX ((1U << 16) - 1)
#define UINT32_MAX 0xffffffff

#include <stddef.h>

#define SIZE_MAX (-(size_t)1)

#endif /* __STDINT_INCLUDED */
