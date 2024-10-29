/******************************************************************************
 * @file            inttypes.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#ifndef __INTTYPES_INCLUDED
#define __INTTYPES_INCLUDED

#include <stdint.h>

#if defined(NO_LONG_LONG)
 #define I64_FMT "l"
#elif defined(_MSVC_) || defined(__WIN32__)
 #define I64_FMT "I64"
#elif defined(__PRI_64_LENGTH_MODIFIER__) /* Mac */
 #define I64_FMT __PRI_64_LENGTH_MODIFIER__
#elif defined(SIZEOF_LONG) && SIZEOF_LONG >= 8
 #define I64_FMT "l"
#else
 #define I64_FMT "ll"
#endif

#define PRIdFAST64 I64_FMT"d"
#define PRIiFAST64 I64_FMT"i"
#define PRIoFAST64 I64_FMT"o"
#define PRIuFAST64 I64_FMT"u"
#define PRIxFAST64 I64_FMT"x"
#define PRIXFAST64 I64_FMT"X"

#endif /* __INTTYPES_INCLUDED */
