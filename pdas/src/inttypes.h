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

#ifndef NO_LONG_LONG

#define PRIdFAST64 "lld"
#define PRIiFAST64 "lli"
#define PRIoFAST64 "llo"
#define PRIuFAST64 "llu"
#define PRIxFAST64 "llx"
#define PRIXFAST64 "llX"

#else

#define PRIdFAST64 "ld"
#define PRIiFAST64 "li"
#define PRIoFAST64 "lo"
#define PRIuFAST64 "lu"
#define PRIxFAST64 "lx"
#define PRIXFAST64 "lX"

#endif

#endif /* __INTTYPES_INCLUDED */
