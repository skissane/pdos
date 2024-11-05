/******************************************************************************
 * @file            bytearray.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <limits.h>
#include <stdint.h>

/* It is necessary to avoid undefined behavior
 * that would be caused by the right operand of shift
 * being greater than or equal to the width in bits of the promoted left operand.
 * Also, some preprocessors cannot handle too large integer constants,
 * so that needs to be avoided by the first comparison.
 */
#if defined (NO_LONG_LONG) && (ULONG_MAX == 0xffffffff || ULONG_MAX < 0xffffffffffffffff)
#define NO_64_BIT_INT
#endif

void bytearray_read_1_bytes (unsigned char *value_p, const unsigned char *src, int little_endian)
{
    *value_p = src[0];
}

void bytearray_read_2_bytes (unsigned short *value_p, const unsigned char *src, int little_endian)
{
    *value_p = 0;

    if (little_endian) {
        int i;

        for (i = 0; i < 2; i++) {
            *value_p |= src[i] << (CHAR_BIT * i);
        }
    } else {
        int i;

        for (i = 0; i < 2; i++) {
            *value_p |= src[2 - 1 - i] << (CHAR_BIT * i);
        }
    }
}

void bytearray_read_3_bytes (unsigned long *value_p, const unsigned char *src, int little_endian)
{
    *value_p = 0;

    if (little_endian) {
        int i;

        for (i = 0; i < 3; i++) {
            *value_p |= (unsigned long)src[i] << (CHAR_BIT * i);
        }
    } else {
        int i;

        for (i = 0; i < 3; i++) {
            *value_p |= (unsigned long)src[3 - 1 - i] << (CHAR_BIT * i);
        }
    }
}

void bytearray_read_4_bytes (unsigned long *value_p, const unsigned char *src, int little_endian)
{
    *value_p = 0;

    if (little_endian) {
        int i;

        for (i = 0; i < 4; i++) {
            *value_p |= (unsigned long)src[i] << (CHAR_BIT * i);
        }
    } else {
        int i;

        for (i = 0; i < 4; i++) {
            *value_p |= (unsigned long)src[4 - 1 - i] << (CHAR_BIT * i);
        }
    }
}

#ifndef NO_64_BIT_INT
void bytearray_read_8_bytes (uint_fast64_t *value_p, const unsigned char *src, int little_endian)
{
    *value_p = 0;

    if (little_endian) {
        int i;

        for (i = 0; i < 8; i++) {
            *value_p |= (uint_fast64_t)src[i] << (CHAR_BIT * i);
        }
    } else {
        int i;

        for (i = 0; i < 8; i++) {
            *value_p |= (uint_fast64_t)src[8 - 1 - i] << (CHAR_BIT * i);
        }
    }
}
#else
void bytearray_read_8_bytes (uint_fast64_t *value_p, const unsigned char *src, int little_endian)
{
    *value_p = 0;

    if (little_endian) {
        int i;

        for (i = 0; i < 4; i++) {
            *value_p |= (uint_fast64_t)src[i] << (CHAR_BIT * i);
        }
    } else {
        int i;

        for (i = 0; i < 4; i++) {
            *value_p |= (uint_fast64_t)src[8 - 1 - i] << (CHAR_BIT * i);
        }
    }
}
#endif

void bytearray_write_1_bytes (unsigned char *dest, unsigned char value, int little_endian)
{
    dest[0] = value;
}

void bytearray_write_2_bytes (unsigned char *dest, unsigned short value, int little_endian)
{
    if (little_endian) {
        int i;

        for (i = 0; i < 2; i++) {
            dest[i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    } else {
        int i;

        for (i = 0; i < 2; i++) {
            dest[2 - 1 - i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    }
}

void bytearray_write_3_bytes (unsigned char *dest, unsigned long value, int little_endian)
{
    if (little_endian) {
        int i;

        for (i = 0; i < 3; i++) {
            dest[i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    } else {
        int i;

        for (i = 0; i < 3; i++) {
            dest[3 - 1 - i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    }
}

void bytearray_write_4_bytes (unsigned char *dest, unsigned long value, int little_endian)
{
    if (little_endian) {
        int i;

        for (i = 0; i < 4; i++) {
            dest[i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    } else {
        int i;

        for (i = 0; i < 4; i++) {
            dest[4 - 1 - i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    }
}

#ifndef NO_64_BIT_INT
void bytearray_write_8_bytes (unsigned char *dest, uint_fast64_t value, int little_endian)
{
    if (little_endian) {
        int i;

        for (i = 0; i < 8; i++) {
            dest[i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    } else {
        int i;

        for (i = 0; i < 8; i++) {
            dest[8 - 1 - i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    }
}
#else
void bytearray_write_8_bytes (unsigned char *dest, uint_fast64_t value, int little_endian)
{
    if (little_endian) {
        int i;

        for (i = 0; i < 4; i++) {
            dest[i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    } else {
        int i;

        for (i = 0; i < 4; i++) {
            dest[8 - 1 - i] = (value >> (CHAR_BIT * i)) & UCHAR_MAX;
        }
    }
}
#endif
