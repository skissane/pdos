/******************************************************************************
 * @file            aout_bytearray.c
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
#include "aout.h"
#include "aout_bytearray.h"

#define COPY(struct_name, field_name, bytes) \
 bytearray_read_##bytes##_bytes (&struct_name##_internal->field_name, struct_name##_file->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_internal->field_name, struct_name##_file->field_name, sizeof (struct_name##_file->field_name))

void read_struct_exec (struct exec_internal *exec_internal, const void *memory)
{
    const struct exec_file *exec_file = memory;

    COPY(exec, a_info, 4);
    COPY(exec, a_text, 4);
    COPY(exec, a_data, 4);
    COPY(exec, a_bss, 4);
    COPY(exec, a_syms, 4);
    COPY(exec, a_entry, 4);
    COPY(exec, a_trsize, 4);
    COPY(exec, a_drsize, 4);
}

void read_struct_relocation_info (struct relocation_info_internal *relocation_info_internal, const void *memory)
{
    const struct relocation_info_file *relocation_info_file = memory;

    COPY(relocation_info, r_address, 4);
    COPY(relocation_info, r_symbolnum, 4);
}

void read_struct_nlist (struct nlist_internal *nlist_internal, const void *memory)
{
    const struct nlist_file *nlist_file = memory;

    COPY(nlist, n_strx, 4);
    COPY(nlist, n_type, 1);
    
    COPY(nlist, n_other, 1);
    COPY(nlist, n_desc, 2);
    
    COPY(nlist, n_value, 4);
}

#undef COPY_CHAR_ARRAY
#undef COPY

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file->field_name, struct_name##_internal->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_file->field_name, struct_name##_internal->field_name, sizeof (struct_name##_file->field_name))

void write_struct_exec (void *memory, const struct exec_internal *exec_internal)
{
    struct exec_file *exec_file = memory;

    COPY(exec, a_info, 4);
    COPY(exec, a_text, 4);
    COPY(exec, a_data, 4);
    COPY(exec, a_bss, 4);
    COPY(exec, a_syms, 4);
    COPY(exec, a_entry, 4);
    COPY(exec, a_trsize, 4);
    COPY(exec, a_drsize, 4);
}

void write_struct_relocation_info (void *memory, const struct relocation_info_internal *relocation_info_internal)
{
    struct relocation_info_file *relocation_info_file = memory;

    COPY(relocation_info, r_address, 4);
    COPY(relocation_info, r_symbolnum, 4);
}

void write_struct_nlist (void *memory, const struct nlist_internal *nlist_internal)
{
    struct nlist_file *nlist_file = memory;

    COPY(nlist, n_strx, 4);
    COPY(nlist, n_type, 1);
    
    COPY(nlist, n_other, 1);
    COPY(nlist, n_desc, 2);
    
    COPY(nlist, n_value, 4);
}

#undef COPY_CHAR_ARRAY
#undef COPY
