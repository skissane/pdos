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

void bytearray_read_1_bytes (unsigned char *value_p, const unsigned char *src, int little_endian);
void bytearray_read_2_bytes (unsigned short *value_p, const unsigned char *src, int little_endian);
void bytearray_read_4_bytes (unsigned long *value_p, const unsigned char *src, int little_endian);
 
void bytearray_write_1_bytes (unsigned char *dest, unsigned char value, int little_endian);
void bytearray_write_2_bytes (unsigned char *dest, unsigned short value, int little_endian);
void bytearray_write_4_bytes (unsigned char *dest, unsigned long value, int little_endian);
