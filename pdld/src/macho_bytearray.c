/******************************************************************************
 * @file            macho_bytearray.c
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
#include "macho.h"
#include "macho_bytearray.h"

#define COPY(struct_name, field_name, bytes) \
 bytearray_read_##bytes##_bytes (&struct_name##_internal->field_name, struct_name##_file->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_internal->field_name, struct_name##_file->field_name, sizeof (struct_name##_file->field_name))

void read_struct_mach_header_64 (struct mach_header_64_internal *mach_header_64_internal, const void *memory)
{
    const struct mach_header_64_file *mach_header_64_file = memory;

    COPY(mach_header_64, magic, 4);
    COPY(mach_header_64, cputype, 4);
    COPY(mach_header_64, cpusubtype, 4);
    COPY(mach_header_64, filetype, 4);
    COPY(mach_header_64, ncmds, 4);
    COPY(mach_header_64, sizeofcmds, 4);
    COPY(mach_header_64, flags, 4);
    COPY(mach_header_64, reserved, 4);
}

void read_struct_load_command (struct load_command_internal *load_command_internal, const void *memory)
{
    const struct load_command_file *load_command_file = memory;

    COPY(load_command, cmd, 4);
    COPY(load_command, cmdsize, 4);
}

void read_struct_segment_command_64 (struct segment_command_64_internal *segment_command_64_internal, const void *memory)
{
    const struct segment_command_64_file *segment_command_64_file = memory;

    COPY(segment_command_64, cmd, 4);
    COPY(segment_command_64, cmdsize, 4);
    COPY_CHAR_ARRAY (segment_command_64, segname);
    COPY(segment_command_64, vmaddr, 8);
    COPY(segment_command_64, vmsize, 8);
    COPY(segment_command_64, fileoff, 8);
    COPY(segment_command_64, filesize, 8);
    COPY(segment_command_64, maxprot, 4);
    COPY(segment_command_64, initprot, 4);
    COPY(segment_command_64, nsects, 4);
    COPY(segment_command_64, flags, 4);
}

void read_struct_section_64 (struct section_64_internal *section_64_internal, const void *memory)
{
    const struct section_64_file *section_64_file = memory;
    
    COPY_CHAR_ARRAY (section_64, sectname);
    COPY_CHAR_ARRAY (section_64, segname);
    COPY(section_64, addr, 8);
    COPY(section_64, size, 8);
    COPY(section_64, offset, 4);
    COPY(section_64, align, 4);
    COPY(section_64, reloff, 4);
    COPY(section_64, nreloc, 4);
    COPY(section_64, flags, 4);
    COPY(section_64, reserved1, 4);
    COPY(section_64, reserved2, 4);
    COPY(section_64, reserved3, 4);
}

#if 0
/* Exactly same as for a.out. */
void read_struct_relocation_info (struct relocation_info_internal *relocation_info_internal, const void *memory)
{
    const struct relocation_info_file *relocation_info_file = memory;

    COPY(relocation_info, r_address, 4);
    COPY(relocation_info, r_symbolnum, 4);
}
#endif

void read_struct_symtab_command (struct symtab_command_internal *symtab_command_internal, const void *memory)
{
    const struct symtab_command_file *symtab_command_file = memory;

    COPY(symtab_command, cmd, 4);
    COPY(symtab_command, cmdsize, 4);
    COPY(symtab_command, symoff, 4);
    COPY(symtab_command, nsyms, 4);
    COPY(symtab_command, stroff, 4);
    COPY(symtab_command, strsize, 4);
}

void read_struct_nlist_64 (struct nlist_64_internal *nlist_64_internal, const void *memory)
{
    const struct nlist_64_file *nlist_64_file = memory;

    COPY(nlist_64, n_strx, 4);
    COPY(nlist_64, n_type, 1);
    COPY(nlist_64, n_sect, 1);
    COPY(nlist_64, n_desc, 2);
    COPY(nlist_64, n_value, 8);
}

#undef COPY_CHAR_ARRAY
#undef COPY

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file->field_name, struct_name##_internal->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_file->field_name, struct_name##_internal->field_name, sizeof (struct_name##_file->field_name))

void write_struct_mach_header_64 (void *memory, const struct mach_header_64_internal *mach_header_64_internal)
{
    struct mach_header_64_file *mach_header_64_file = memory;

    COPY(mach_header_64, magic, 4);
    COPY(mach_header_64, cputype, 4);
    COPY(mach_header_64, cpusubtype, 4);
    COPY(mach_header_64, filetype, 4);
    COPY(mach_header_64, ncmds, 4);
    COPY(mach_header_64, sizeofcmds, 4);
    COPY(mach_header_64, flags, 4);
    COPY(mach_header_64, reserved, 4);
}

void write_struct_load_command (void *memory, const struct load_command_internal *load_command_internal)
{
    struct load_command_file *load_command_file = memory;

    COPY(load_command, cmd, 4);
    COPY(load_command, cmdsize, 4);
}

void write_struct_segment_command_64 (void *memory, const struct segment_command_64_internal *segment_command_64_internal)
{
    struct segment_command_64_file *segment_command_64_file = memory;

    COPY(segment_command_64, cmd, 4);
    COPY(segment_command_64, cmdsize, 4);
    COPY_CHAR_ARRAY (segment_command_64, segname);
    COPY(segment_command_64, vmaddr, 8);
    COPY(segment_command_64, vmsize, 8);
    COPY(segment_command_64, fileoff, 8);
    COPY(segment_command_64, filesize, 8);
    COPY(segment_command_64, maxprot, 4);
    COPY(segment_command_64, initprot, 4);
    COPY(segment_command_64, nsects, 4);
    COPY(segment_command_64, flags, 4);
}

void write_struct_section_64 (void *memory, const struct section_64_internal *section_64_internal)
{
    struct section_64_file *section_64_file = memory;

    COPY_CHAR_ARRAY (section_64, sectname);
    COPY_CHAR_ARRAY (section_64, segname);
    COPY(section_64, addr, 8);
    COPY(section_64, size, 8);
    COPY(section_64, offset, 4);
    COPY(section_64, align, 4);
    COPY(section_64, reloff, 4);
    COPY(section_64, nreloc, 4);
    COPY(section_64, flags, 4);
    COPY(section_64, reserved1, 4);
    COPY(section_64, reserved2, 4);
    COPY(section_64, reserved3, 4);
}

#if 0
/* Exactly same as for a.out. */
void write_struct_relocation_info (void *memory, const struct relocation_info_internal *relocation_info_internal)
{
    struct relocation_info_file *relocation_info_file = memory;

    COPY(relocation_info, r_address, 4);
    COPY(relocation_info, r_symbolnum, 4);
}
#endif

void write_struct_symtab_command (void *memory, const struct symtab_command_internal *symtab_command_internal)
{
    struct symtab_command_file *symtab_command_file = memory;

    COPY(symtab_command, cmd, 4);
    COPY(symtab_command, cmdsize, 4);
    COPY(symtab_command, symoff, 4);
    COPY(symtab_command, nsyms, 4);
    COPY(symtab_command, stroff, 4);
    COPY(symtab_command, strsize, 4);
}

void write_struct_nlist_64 (void *memory, const struct nlist_64_internal *nlist_64_internal)
{
    struct nlist_64_file *nlist_64_file = memory;

    COPY(nlist_64, n_strx, 4);
    COPY(nlist_64, n_type, 1);
    COPY(nlist_64, n_sect, 1);
    COPY(nlist_64, n_desc, 2);
    COPY(nlist_64, n_value, 8);
}

#undef COPY_CHAR_ARRAY
#undef COPY
