/******************************************************************************
 * @file            aout_bytearray.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
void read_struct_exec (struct exec_internal *exec_internal, const void *memory);
void read_struct_relocation_info (struct relocation_info_internal *relocation_info_internal, const void *memory);
void read_struct_nlist (struct nlist_internal *nlist_internal, const void *memory);

void write_struct_exec (void *memory, const struct exec_internal *exec_internal);
void write_struct_relocation_info (void *memory, const struct relocation_info_internal *relocation_info_internal);
void write_struct_nlist (void *memory, const struct nlist_internal *nlist_internal);
