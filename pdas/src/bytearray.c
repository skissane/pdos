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
#include    <limits.h>

void bytearray_write_1_bytes (unsigned char *dest, unsigned char value, int little_endian) {

    dest[0] = value;

}

void bytearray_write_2_bytes (unsigned char *dest, unsigned short value, int little_endian) {

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

void bytearray_write_4_bytes (unsigned char *dest, unsigned long value, int little_endian) {

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
