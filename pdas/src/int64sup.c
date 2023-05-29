/******************************************************************************
 * @file            int64sup.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdint.h>

/* For unknown reason the compiler cannot generate code
 * for (u)int_fast64_t (long long) division on its own,
 * so they need to be provided this way. */

int_fast64_t __divdi3 (int_fast64_t a, int_fast64_t b)
{
    int_fast64_t c = 0;
    int negate_result = 0;
    
    if (b == 0) {
        /* Zero division is done just like if it was a direct division. */
        volatile int x = 0;
        x = 1 / x;
    }

    if (a < 0) {
        a = -a;
        negate_result = !negate_result;
    }
    if (b < 0) {
        b = -b;
        negate_result = !negate_result;
    }

    while (a >= b) {
        a -= b;
        c++;
    }
    
    return negate_result ? (-c) : c;
}

int_fast64_t __moddi3 (int_fast64_t a, int_fast64_t b)
{
    int_fast64_t c = 0;
    int negate_result = 0;
    
    if (b == 0) {
        /* Zero division is done just like if it was a direct division. */
        volatile int x = 0;
        x = 1 / x;
    }

    if (a < 0) {
        a = -a;
        negate_result = 1;
    }
    if (b < 0) {
        b = -b;
    }

    while (a >= b) {
        a -= b;
        c++;
    }
    
    return negate_result ? (-a) : a;
}

uint_fast64_t __udivdi3 (uint_fast64_t a, uint_fast64_t b)
{
    uint_fast64_t c = 0;

    if (b == 0) {
        /* Zero division is done just like if it was a direct division. */
        volatile int x = 0;
        x = 1 / x;
    }

    while (a >= b) {
        a -= b;
        c++;
    }
    
    return c;
}

uint_fast64_t __umoddi3 (uint_fast64_t a, uint_fast64_t b)
{
    if (b == 0) {
        /* Zero division is done just like if it was a direct division. */
        volatile int x = 0;
        x = 1 / x;
    }
    
    while (a >= b) {
        a -= b;
    }

    return a;
}
