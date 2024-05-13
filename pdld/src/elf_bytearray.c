/******************************************************************************
 * @file            elf_bytearray.c
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
#include "elf.h"
#include "elf_bytearray.h"

#define COPY(struct_name, field_name, bytes) \
 bytearray_read_##bytes##_bytes (&struct_name##_internal->field_name, struct_name##_file->field_name, endianess)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_internal->field_name, struct_name##_file->field_name, sizeof (struct_name##_file->field_name))

void read_struct_Elf32_Ehdr (struct Elf32_Ehdr_internal *Elf32_Ehdr_internal, const void *memory, int endianess)
{
    const struct Elf32_Ehdr_file *Elf32_Ehdr_file = memory;

    COPY_CHAR_ARRAY (Elf32_Ehdr, e_ident);

    COPY(Elf32_Ehdr, e_type, 2);
    COPY(Elf32_Ehdr, e_machine, 2);
    COPY(Elf32_Ehdr, e_version, 4);
    COPY(Elf32_Ehdr, e_entry, 4);
    COPY(Elf32_Ehdr, e_phoff, 4);
    COPY(Elf32_Ehdr, e_shoff, 4);
    COPY(Elf32_Ehdr, e_flags, 4);
    COPY(Elf32_Ehdr, e_ehsize, 2);
    COPY(Elf32_Ehdr, e_phentsize, 2);
    COPY(Elf32_Ehdr, e_phnum, 2);
    COPY(Elf32_Ehdr, e_shentsize, 2);
    COPY(Elf32_Ehdr, e_shnum, 2);
    COPY(Elf32_Ehdr, e_shstrndx, 2);
}

void read_struct_Elf32_Shdr (struct Elf32_Shdr_internal *Elf32_Shdr_internal, const void *memory, int endianess)
{
    const struct Elf32_Shdr_file *Elf32_Shdr_file = memory;

    COPY(Elf32_Shdr, sh_name, 4);
    COPY(Elf32_Shdr, sh_type, 4);
    COPY(Elf32_Shdr, sh_flags, 4);
    COPY(Elf32_Shdr, sh_addr, 4);
    COPY(Elf32_Shdr, sh_offset, 4);
    COPY(Elf32_Shdr, sh_size, 4);
    COPY(Elf32_Shdr, sh_link, 4);
    COPY(Elf32_Shdr, sh_info, 4);
    COPY(Elf32_Shdr, sh_addralign, 4);
    COPY(Elf32_Shdr, sh_entsize, 4);
}

void read_struct_Elf32_Sym (struct Elf32_Sym_internal *Elf32_Sym_internal, const void *memory, int endianess)
{
    const struct Elf32_Sym_file *Elf32_Sym_file = memory;
    
    COPY(Elf32_Sym, st_name, 4);
    COPY(Elf32_Sym, st_value, 4);
    COPY(Elf32_Sym, st_size, 4);
    COPY(Elf32_Sym, st_info, 1);
    COPY(Elf32_Sym, st_other, 1);
    COPY(Elf32_Sym, st_shndx, 2);
}

void read_struct_Elf32_Rel (struct Elf32_Rel_internal *Elf32_Rel_internal, const void *memory, int endianess)
{
    const struct Elf32_Rel_file *Elf32_Rel_file = memory;
    
    COPY(Elf32_Rel, r_offset, 4);
    COPY(Elf32_Rel, r_info, 4);
}

void read_struct_Elf32_Phdr (struct Elf32_Phdr_internal *Elf32_Phdr_internal, const void *memory, int endianess)
{
    const struct Elf32_Phdr_file *Elf32_Phdr_file = memory;
    
    COPY(Elf32_Phdr, p_type, 4);
    COPY(Elf32_Phdr, p_offset, 4);
    COPY(Elf32_Phdr, p_vaddr, 4);
    COPY(Elf32_Phdr, p_paddr, 4);
    COPY(Elf32_Phdr, p_filesz, 4);
    COPY(Elf32_Phdr, p_memsz, 4);
    COPY(Elf32_Phdr, p_flags, 4);
    COPY(Elf32_Phdr, p_align, 4);
}

#undef COPY_CHAR_ARRAY
#undef COPY

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file->field_name, struct_name##_internal->field_name, endianess)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_file->field_name, struct_name##_internal->field_name, sizeof (struct_name##_file->field_name))

void write_struct_Elf32_Ehdr (void *memory, const struct Elf32_Ehdr_internal *Elf32_Ehdr_internal, int endianess)
{
    struct Elf32_Ehdr_file *Elf32_Ehdr_file = memory;

    COPY_CHAR_ARRAY (Elf32_Ehdr, e_ident);

    COPY(Elf32_Ehdr, e_type, 2);
    COPY(Elf32_Ehdr, e_machine, 2);
    COPY(Elf32_Ehdr, e_version, 4);
    COPY(Elf32_Ehdr, e_entry, 4);
    COPY(Elf32_Ehdr, e_phoff, 4);
    COPY(Elf32_Ehdr, e_shoff, 4);
    COPY(Elf32_Ehdr, e_flags, 4);
    COPY(Elf32_Ehdr, e_ehsize, 2);
    COPY(Elf32_Ehdr, e_phentsize, 2);
    COPY(Elf32_Ehdr, e_phnum, 2);
    COPY(Elf32_Ehdr, e_shentsize, 2);
    COPY(Elf32_Ehdr, e_shnum, 2);
    COPY(Elf32_Ehdr, e_shstrndx, 2);
}

void write_struct_Elf32_Shdr (void *memory, const struct Elf32_Shdr_internal *Elf32_Shdr_internal, int endianess)
{
    struct Elf32_Shdr_file *Elf32_Shdr_file = memory;

    COPY(Elf32_Shdr, sh_name, 4);
    COPY(Elf32_Shdr, sh_type, 4);
    COPY(Elf32_Shdr, sh_flags, 4);
    COPY(Elf32_Shdr, sh_addr, 4);
    COPY(Elf32_Shdr, sh_offset, 4);
    COPY(Elf32_Shdr, sh_size, 4);
    COPY(Elf32_Shdr, sh_link, 4);
    COPY(Elf32_Shdr, sh_info, 4);
    COPY(Elf32_Shdr, sh_addralign, 4);
    COPY(Elf32_Shdr, sh_entsize, 4);
}

void write_struct_Elf32_Sym (void *memory, const struct Elf32_Sym_internal *Elf32_Sym_internal, int endianess)
{
    struct Elf32_Sym_file *Elf32_Sym_file = memory;

    COPY(Elf32_Sym, st_name, 4);
    COPY(Elf32_Sym, st_value, 4);
    COPY(Elf32_Sym, st_size, 4);
    COPY(Elf32_Sym, st_info, 1);
    COPY(Elf32_Sym, st_other, 1);
    COPY(Elf32_Sym, st_shndx, 2);
}

void write_struct_Elf32_Rel (void *memory, const struct Elf32_Rel_internal *Elf32_Rel_internal, int endianess)
{
    struct Elf32_Rel_file *Elf32_Rel_file = memory;

    COPY(Elf32_Rel, r_offset, 4);
    COPY(Elf32_Rel, r_info, 4);
}

void write_struct_Elf32_Phdr (void *memory, const struct Elf32_Phdr_internal *Elf32_Phdr_internal, int endianess)
{
    struct Elf32_Phdr_file *Elf32_Phdr_file = memory;

    COPY(Elf32_Phdr, p_type, 4);
    COPY(Elf32_Phdr, p_offset, 4);
    COPY(Elf32_Phdr, p_vaddr, 4);
    COPY(Elf32_Phdr, p_paddr, 4);
    COPY(Elf32_Phdr, p_filesz, 4);
    COPY(Elf32_Phdr, p_memsz, 4);
    COPY(Elf32_Phdr, p_flags, 4);
    COPY(Elf32_Phdr, p_align, 4);
}

#undef COPY_CHAR_ARRAY
#undef COPY
