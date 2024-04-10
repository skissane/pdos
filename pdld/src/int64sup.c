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
#include <stddef.h>

/* For unknown reason the compiler cannot generate code
 * for (u)int_fast64_t (long long) division on its own,
 * so they need to be provided this way. */
#ifndef NO_LONG_LONG

static uint_fast64_t udivmodi (uint_fast64_t a, uint_fast64_t b, uint_fast64_t *rem_p)
{
    uint_fast64_t c = 0;

    if (b > a) {
        if (rem_p) *rem_p = a;
        return 0;
    }

    if (b >> 32 == 0) {
        if (b == 0) {
            /* Zero division is done just like if it was a direct division. */
            volatile int x = 0;
            x = 1 / x;
        }
        if (b == 1) {
            if (rem_p) *rem_p = 0;
            return a;
        }
        if (a >> 32 == 0) {
            /* Both numbers fit into unsigned long,
             * so simple unsigned long division can be done. */
            if (rem_p) *rem_p = ((unsigned long)a) % ((unsigned long)b);
            return ((unsigned long)a) / ((unsigned long)b);
        }
    }
    
    while (a >= b) {
        a -= b;
        c++;
    }

    if (rem_p) *rem_p = a;
    
    return c;
}

int_fast64_t __divdi3 (int_fast64_t a, int_fast64_t b)
{
    uint_fast64_t c;
    int negate_result = 0;
    
    if (a < 0) {
        a = -a;
        negate_result = !negate_result;
    }
    if (b < 0) {
        b = -b;
        negate_result = !negate_result;
    }

    c = udivmodi (a, b, NULL);
    
    return negate_result ? (-c) : c;
}

int_fast64_t __moddi3 (int_fast64_t a, int_fast64_t b)
{
    uint_fast64_t rem;
    int negate_result = 0;

    if (a < 0) {
        a = -a;
        negate_result = 1;
    }
    if (b < 0) {
        b = -b;
    }

    udivmodi (a, b, &rem);
    
    return negate_result ? (-rem) : rem;
}

uint_fast64_t __udivdi3 (uint_fast64_t a, uint_fast64_t b)
{
    return udivmodi (a, b, NULL);
}

uint_fast64_t __umoddi3 (uint_fast64_t a, uint_fast64_t b)
{
    uint_fast64_t rem;

    udivmodi (a, b, &rem);

    return rem;
}

#endif /* NO_LONG_LONG */
