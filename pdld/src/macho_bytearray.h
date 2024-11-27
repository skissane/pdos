/******************************************************************************
 * @file            macho_bytearray.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/

#define read_struct_mach_header_64 rsmh64
#define read_struct_load_command rslc
#define read_struct_segment_command_64 rssc64
#define read_struct_section_64 rss64
#define read_struct_relocation_info rsri
#define read_struct_symtab_command rssc
#define read_struct_nlist_64 rsn64

#define write_struct_mach_header_64 wsmh64
#define write_struct_load_command wslc
#define write_struct_segment_command_64 wssc64
#define write_struct_section_64 wss64
#define write_struct_relocation_info wsri
#define write_struct_symtab_command wssc
#define write_struct_nlist_64 wsn64

void read_struct_mach_header_64 (struct mach_header_64_internal *mach_header_64_internal, const void *memory);
void read_struct_load_command (struct load_command_internal *load_command_internal, const void *memory);
void read_struct_segment_command_64 (struct segment_command_64_internal *segment_command_64_internal, const void *memory);
void read_struct_section_64 (struct section_64_internal *section_64_internal, const void *memory);
void read_struct_relocation_info (struct relocation_info_internal *relocation_info_internal, const void *memory);
void read_struct_symtab_command (struct symtab_command_internal *symtab_command_internal, const void *memory);
void read_struct_nlist_64 (struct nlist_64_internal *nlist_64_internal, const void *memory);

void write_struct_mach_header_64 (void *memory, const struct mach_header_64_internal *mach_header_64_internal);
void write_struct_load_command (void *memory, const struct load_command_internal *load_command_internal);
void write_struct_segment_command_64 (void *memory, const struct segment_command_64_internal *segment_command_64_internal);
void write_struct_section_64 (void *memory, const struct section_64_internal *section_64_internal);
void write_struct_relocation_info (void *memory, const struct relocation_info_internal *relocation_info_internal);
void write_struct_symtab_command (void *memory, const struct symtab_command_internal *symtab_command_internal);
void write_struct_nlist_64 (void *memory, const struct nlist_64_internal *nlist_64_internal);
