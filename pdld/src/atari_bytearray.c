/******************************************************************************
 * @file            atari_bytearray.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ld.h"
#include "bytearray.h"
#include "atari.h"
#include "atari_bytearray.h"

#define COPY(struct_name, field_name, bytes) \
 bytearray_read_##bytes##_bytes (&struct_name##_internal->field_name, struct_name##_file->field_name, BIG_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_internal->field_name, struct_name##_file->field_name, sizeof (struct_name##_file->field_name))

void read_struct_PH (struct PH_internal *PH_internal, const void *memory)
{
    const struct PH_file *PH_file = memory;

    COPY(PH, ph_branch, 2);

    COPY(PH, ph_tlen, 4);
    COPY(PH, ph_dlen, 4);
    COPY(PH, ph_blen, 4);
    COPY(PH, ph_slen, 4);
    COPY(PH, ph_res1, 4);

    COPY(PH, ph_prgflags, 4);
    COPY(PH, ph_absflags, 2);
}

#undef COPY_CHAR_ARRAY
#undef COPY

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file->field_name, struct_name##_internal->field_name, BIG_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_file->field_name, struct_name##_internal->field_name, sizeof (struct_name##_file->field_name))

void write_struct_PH (void *memory, const struct PH_internal *PH_internal)
{
    struct PH_file *PH_file = memory;

    COPY(PH, ph_branch, 2);

    COPY(PH, ph_tlen, 4);
    COPY(PH, ph_dlen, 4);
    COPY(PH, ph_blen, 4);
    COPY(PH, ph_slen, 4);
    COPY(PH, ph_res1, 4);

    COPY(PH, ph_prgflags, 4);
    COPY(PH, ph_absflags, 2);
}

#undef COPY_CHAR_ARRAY
#undef COPY
