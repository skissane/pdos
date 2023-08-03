/******************************************************************************
 * @file            coff_bytearray.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/ 
void read_struct_coff_header (struct coff_header_internal *coff_header_internal, void *memory);
void read_struct_section_table_entry (struct section_table_entry_internal *section_table_entry_internal, void *memory);
void read_struct_relocation_entry (struct relocation_entry_internal *relocation_entry_internal, void *memory);
void read_struct_symbol_table_entry (struct symbol_table_entry_internal *symbol_table_entry_internal, void *memory);
void read_struct_string_table_header (struct string_table_header_internal *string_table_header_internal, void *memory);
void read_struct_IMAGE_ARCHIVE_MEMBER_HEADER (struct IMAGE_ARCHIVE_MEMBER_HEADER_internal *IMAGE_ARCHIVE_MEMBER_HEADER_internal, void *memory);
void read_struct_IMPORT_OBJECT_HEADER (struct IMPORT_OBJECT_HEADER_internal *IMPORT_OBJECT_HEADER_internal, void *memory);

void write_struct_IMAGE_DOS_HEADER (void *memory, struct IMAGE_DOS_HEADER_internal *IMAGE_DOS_HEADER_internal);
void write_struct_coff_header (void *memory, struct coff_header_internal *coff_header_internal);
void write_struct_optional_header (void *memory, struct optional_header_internal *optional_header_internal);
void write_struct_optional_header_plus (void *memory, struct optional_header_plus_internal *optional_header_plus_internal);
void write_struct_IMAGE_DATA_DIRECTORY (void *memory, struct IMAGE_DATA_DIRECTORY_internal *IMAGE_DATA_DIRECTORY_internal);
void write_struct_section_table_entry (void *memory, struct section_table_entry_internal *section_table_entry_internal);
void write_struct_IMAGE_EXPORT_DIRECTORY (void *memory, struct IMAGE_EXPORT_DIRECTORY_internal *IMAGE_EXPORT_DIRECTORY_internal);
void write_struct_EXPORT_Name_Pointer_Table (void *memory, struct EXPORT_Name_Pointer_Table_internal *EXPORT_Name_Pointer_Table_internal);
void write_struct_EXPORT_Ordinal_Table (void *memory, struct EXPORT_Ordinal_Table_internal *EXPORT_Ordinal_Table_internal);
void write_struct_IMPORT_Directory_Table (void *memory, struct IMPORT_Directory_Table_internal *IMPORT_Directory_Table_internal);
void write_struct_IMAGE_BASE_RELOCATION (void *memory, struct IMAGE_BASE_RELOCATION_internal *IMAGE_BASE_RELOCATION_internal);
void write_struct_IMAGE_ARCHIVE_MEMBER_HEADER (void *memory, struct IMAGE_ARCHIVE_MEMBER_HEADER_internal *IMAGE_ARCHIVE_MEMBER_HEADER_internal);
void write_struct_IMPORT_OBJECT_HEADER (void *memory, struct IMPORT_OBJECT_HEADER_internal *IMPORT_OBJECT_HEADER_internal);
