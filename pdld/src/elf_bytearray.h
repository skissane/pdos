/******************************************************************************
 * @file            elf_bytearray.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
void read_struct_Elf32_Ehdr (struct Elf32_Ehdr_internal *Elf32_Ehdr_internal, const void *memory, int endianess);
void read_struct_Elf32_Shdr (struct Elf32_Shdr_internal *Elf32_Shdr_internal, const void *memory, int endianess);
void read_struct_Elf32_Sym (struct Elf32_Sym_internal *Elf32_Sym_internal, const void *memory, int endianess);
void read_struct_Elf32_Rel (struct Elf32_Rel_internal *Elf32_Rel_internal, const void *memory, int endianess);
void read_struct_Elf32_Rela (struct Elf32_Rela_internal *Elf32_Rela_internal, const void *memory, int endianess);
void read_struct_Elf32_Phdr (struct Elf32_Phdr_internal *Elf32_Phdr_internal, const void *memory, int endianess);

void write_struct_Elf32_Ehdr (void *memory, const struct Elf32_Ehdr_internal *Elf32_Ehdr_internal, int endianess);
void write_struct_Elf32_Shdr (void *memory, const struct Elf32_Shdr_internal *Elf32_Shdr_internal, int endianess);
void write_struct_Elf32_Sym (void *memory, const struct Elf32_Sym_internal *Elf32_Sym_internal, int endianess);
void write_struct_Elf32_Rel (void *memory, const struct Elf32_Rel_internal *Elf32_Rel_internal, int endianess);
void write_struct_Elf32_Rela (void *memory, const struct Elf32_Rela_internal *Elf32_Rela_internal, int endianess);
void write_struct_Elf32_Phdr (void *memory, const struct Elf32_Phdr_internal *Elf32_Phdr_internal, int endianess);


