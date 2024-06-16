/******************************************************************************
 * @file            atari_bytearray.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
void read_struct_PH (struct PH_internal *PH_internal, const void *memory);

void write_struct_PH (void *memory, const struct PH_internal *PH_internal);
