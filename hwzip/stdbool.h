/******************************************************************************
 * @file            stdbool.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#ifndef __STDBOOL_INCLUDED
#define __STDBOOL_INCLUDED

typedef unsigned char _new_Bool;
#define bool _new_Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1

#endif /* __STDBOOL_INCLUDED */
