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
#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 0

#define bytearray_read_1_bytes bar1
#define bytearray_read_2_bytes bar2
#define bytearray_read_3_bytes bar3
#define bytearray_read_4_bytes bar4
#define bytearray_read_8_bytes bar8

#define bytearray_write_1_bytes baw1
#define bytearray_write_2_bytes baw2
#define bytearray_write_3_bytes baw3
#define bytearray_write_4_bytes baw4
#define bytearray_write_8_bytes baw8


void bytearray_write_1_bytes (unsigned char *dest, unsigned char value, int little_endian);
void bytearray_write_2_bytes (unsigned char *dest, unsigned short value, int little_endian);
void bytearray_write_4_bytes (unsigned char *dest, unsigned long value, int little_endian);
