/******************************************************************************
 * @file            lx_bytearray.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
void read_struct_LX_HEADER (struct LX_HEADER_internal *LX_HEADER_internal, const void *memory);
void read_struct_object_table_entry (struct object_table_entry_internal *object_table_entry_internal, const void *memory);
void read_struct_object_page_table_entry (struct object_page_table_entry_internal *object_page_table_entry_internal, const void *memory);

void write_struct_LX_HEADER (void *memory, const struct LX_HEADER_internal *LX_HEADER_internal);
void write_struct_object_table_entry (void *memory, const struct object_table_entry_internal *object_table_entry_internal);
void write_struct_object_page_table_entry (void *memory, const struct object_page_table_entry_internal *object_page_table_entry_internal);

