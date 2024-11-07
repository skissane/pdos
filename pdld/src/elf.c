/******************************************************************************
 * @file            elf.c
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
#include "xmalloc.h"

#include "elf_bytearray.h"

static int endianess = LITTLE_ENDIAN;

#define RELOC_SECTION_ALIGNMENT 4

static int generate_section_headers = 1;

/* Only the main ELF header has fixed position,
 * so only it needs to be counted.
 */
static long size_of_headers = SIZEOF_struct_Elf32_Ehdr_file;

static Elf32_Word translate_section_flags_to_p_flags (flag_int flags)
{
    Elf32_Word p_flags = 0;

    if (!(flags & SECTION_FLAG_READONLY)) p_flags |= PF_W;

    if (flags & SECTION_FLAG_CODE) p_flags |= PF_X;

    if (!(flags & SECTION_FLAG_NOREAD)) p_flags |= PF_R;

    return p_flags;
}

static Elf32_Word translate_section_flags_to_sh_flags (flag_int flags)
{
    Elf32_Word sh_flags = 0;

    if (!(flags & SECTION_FLAG_READONLY)) sh_flags |= SHF_WRITE;

    if (flags & SECTION_FLAG_ALLOC) sh_flags |= SHF_ALLOC;

    if (flags & SECTION_FLAG_CODE) sh_flags |= SHF_EXECINSTR;

    return sh_flags;
}

static flag_int translate_sh_flags_to_section_flags (Elf32_Word sh_flags)
{
    flag_int flags = 0;

    if (!(sh_flags & SHF_WRITE)) flags |= SECTION_FLAG_READONLY;

    if (sh_flags & SHF_ALLOC) flags |= SECTION_FLAG_ALLOC;

    if (sh_flags & SHF_EXECINSTR) flags |= SECTION_FLAG_CODE;

    return flags;
}

static size_t section_get_num_relocs (struct section *section)
{
    struct section_part *part;
    size_t num_relocs = 0;
    
    for (part = section->first_part; part; part = part->next) {
        size_t i;
        
        for (i = 0; i < part->relocation_count; i++) {
            if (ld_state->target_machine == LD_TARGET_MACHINE_I386) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]
                    || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_PC32]) {
                    num_relocs++;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_X64) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_64]
                    || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]
                    || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_PC32]) {
                    num_relocs++;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_ARM) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_ARM_32]
                    || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_ARM_PC26]) {
                    num_relocs++;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_AARCH64) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_64]
                    || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    num_relocs++;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_M68K) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]
                    || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_PC32]) {
                    num_relocs++;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_MAINFRAME) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    num_relocs++;
                }
            }
        }
    }

    return num_relocs;
}

static unsigned char *write_relocs_for_section (unsigned char *file,
                                                unsigned char *pos,
                                                struct section *section,
                                                struct Elf32_Shdr_internal *shdr_p)
{
    struct section_part *part;
    unsigned char *rel_pos;
    unsigned char *saved_pos = pos;

    pos = file + ALIGN (pos - file, RELOC_SECTION_ALIGNMENT);
    rel_pos = pos;

    for (part = section->first_part; part; part = part->next) {
        size_t i;
        
        for (i = 0; i < part->relocation_count; i++) {
            unsigned char type;
            struct Elf32_Rel_internal rel;
            
            if (ld_state->target_machine == LD_TARGET_MACHINE_I386) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_IGNORED]) {
                    continue;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    type = R_386_32;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_PC32]) {
                    type = R_386_PC32;
                } else {
                    ld_error ("%s cannot be converted to Elf32 relocation",
                              part->relocation_array[i].howto->name);
                    continue;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_ARM) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_IGNORED]) {
                    continue;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_ARM_32]) {
                    type = R_ARM_ABS32;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_ARM_PC26]) {
                    type = R_ARM_PC24;
                } else {
                    ld_error ("%s cannot be converted to Elf32 relocation",
                              part->relocation_array[i].howto->name);
                    continue;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_M68K) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_IGNORED]) {
                    continue;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    type = R_68K_32;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_PC32]) {
                    type = R_68K_PC32;
                } else {
                    ld_error ("%s cannot be converted to Elf32 relocation",
                              part->relocation_array[i].howto->name);
                    continue;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_MAINFRAME) {
                /* Assume it is the same as for i386 for now. */
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_IGNORED]) {
                    continue;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    type = R_386_32;
                } else {
                    ld_error ("%s cannot be converted to Elf32 relocation",
                              part->relocation_array[i].howto->name);
                    continue;
                }
            } else {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++relocations for target %i not supported yet",
                                             ld_state->target_machine);
            }

            rel.r_offset = ld_state->base_address + part->rva + part->relocation_array[i].offset;
            /* Symbol table is not yet supported. */
            rel.r_info = ELF32_R_INFO (0, type);

            write_struct_Elf32_Rel (rel_pos, &rel, endianess);
            rel_pos += SIZEOF_struct_Elf32_Rel_file;
        }
    }

    if (rel_pos == pos) return saved_pos;

    shdr_p->sh_type = SHT_REL;
    shdr_p->sh_flags = 0;
    shdr_p->sh_addr = 0;
    shdr_p->sh_offset = pos - file;
    shdr_p->sh_size = rel_pos - pos;
    shdr_p->sh_link = 0; /* No symbol table exists. */
    shdr_p->sh_info = section->target_index;
    shdr_p->sh_addralign = RELOC_SECTION_ALIGNMENT;
    shdr_p->sh_entsize = SIZEOF_struct_Elf32_Rel_file;
    
    return rel_pos;
}

static unsigned char *write_relocs_for_section64 (unsigned char *file,
                                                  unsigned char *pos,
                                                  struct section *section,
                                                  Elf64_Shdr *shdr_p)
{
    struct section_part *part;
    Elf64_Rel *rel;
    unsigned char *saved_pos = pos;

    pos = file + ALIGN (pos - file, RELOC_SECTION_ALIGNMENT);
    rel = (void *)pos;

    for (part = section->first_part; part; part = part->next) {
        size_t i;
        
        for (i = 0; i < part->relocation_count; i++) {
            Elf64_Xword type;
            
            if (ld_state->target_machine == LD_TARGET_MACHINE_X64) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_IGNORED]) {
                    continue;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_64]) {
                    type = R_X86_64_64;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    type = R_X86_64_32;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_PC32]) {
                    type = R_X86_64_PC32;
                } else {
                    ld_error ("%s cannot be converted to Elf64 relocation",
                              part->relocation_array[i].howto->name);
                    continue;
                }
            } else if (ld_state->target_machine == LD_TARGET_MACHINE_AARCH64) {
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_IGNORED]) {
                    continue;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_64]) {
                    type = R_AARCH64_ABS64;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    type = R_AARCH64_ABS32;
                } else {
                    /* Other relocation types are not used by the loader anyway,
                     * so currently there is no need to emit them.
                     */
                    continue;
                }
            } else {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++relocations for target %i not supported yet",
                                             ld_state->target_machine);
            }

            rel->r_offset = ld_state->base_address + part->rva + part->relocation_array[i].offset;
            /* Symbol table is not yet supported. */
            rel->r_info = ELF64_R_INFO ((Elf64_Xword)0, type);
            rel++;
        }
    }

    if (rel == (void *)pos) return saved_pos;

    shdr_p->sh_type = SHT_REL;
    shdr_p->sh_flags = 0;
    shdr_p->sh_addr = 0;
    shdr_p->sh_offset = pos - file;
    shdr_p->sh_size = (unsigned char *)rel - pos;
    shdr_p->sh_link = 0; /* No symbol table exists. */
    shdr_p->sh_info = section->target_index;
    shdr_p->sh_addralign = RELOC_SECTION_ALIGNMENT;
    shdr_p->sh_entsize = sizeof *rel;
    
    return (unsigned char *)rel;
}

static unsigned char *write_sections (unsigned char *file,
                                      struct Elf32_Ehdr_internal *ehdr_p,
                                      unsigned long shstrtab_i)
{
    unsigned char *pos;
    struct section *section;
    unsigned char *phdr_pos;
    unsigned char *shdr_pos = NULL;

    pos = file + size_of_headers;

    phdr_pos = file + ehdr_p->e_phoff;
    if (generate_section_headers) {
        struct Elf32_Shdr_internal shdr = {0};
        
        shdr_pos = file + ehdr_p->e_shoff;
        shdr.sh_type = SHT_NULL;
        shdr.sh_link = SHN_UNDEF;

        write_struct_Elf32_Shdr (shdr_pos, &shdr, endianess);
        shdr_pos += SIZEOF_struct_Elf32_Shdr_file;
    }

    for (section = all_sections; section; section = section->next) {
        struct Elf32_Phdr_internal phdr = {0};
        
        phdr.p_type = PT_LOAD;
        phdr.p_paddr = phdr.p_vaddr = ld_state->base_address + section->rva;
        phdr.p_memsz = section->total_size;

        if (!section->is_bss) {
            phdr.p_filesz = section->total_size;
            pos = file + ALIGN (pos - file, section->section_alignment);
            phdr.p_offset = pos - file;

            section_write (section, pos);
            pos += section->total_size;
        } else {
            phdr.p_filesz = 0;
            phdr.p_offset = 0;
        }

        phdr.p_flags = translate_section_flags_to_p_flags (section->flags);

        /* Memory alignment and file alignment must be the same
         * (so it is good to have RVA of first section
         *  to be equal to its offset)
         * but neither memory size nor file size need to be rounded.
         */
        phdr.p_align = section->section_alignment;

        if (generate_section_headers) {
            struct Elf32_Shdr_internal shdr = {0};
            
            shdr.sh_type = section->is_bss ? SHT_NOBITS : SHT_PROGBITS;
            shdr.sh_flags = translate_section_flags_to_sh_flags (section->flags);
            shdr.sh_addr = phdr.p_vaddr;
            shdr.sh_offset = phdr.p_offset;
            shdr.sh_size = phdr.p_memsz;
            shdr.sh_addralign = phdr.p_align;

            if (ld_state->emit_relocs) {
                struct Elf32_Shdr_internal rel_shdr = {0};
                unsigned char *saved_pos = pos;

                pos = write_relocs_for_section (file, pos, section, &rel_shdr);
                if (pos == saved_pos) {
                    goto no_relocs_emitted;
                }

                shdr.sh_name = shstrtab_i + sizeof (".rel") - 1;
                rel_shdr.sh_name = shstrtab_i;
                shstrtab_i += sizeof (".rel") - 1 + strlen (section->name) + 1;

                write_struct_Elf32_Shdr (shdr_pos, &shdr, endianess);
                shdr_pos += SIZEOF_struct_Elf32_Shdr_file;
                write_struct_Elf32_Shdr (shdr_pos, &rel_shdr, endianess);
                shdr_pos += SIZEOF_struct_Elf32_Shdr_file;
            } else {
no_relocs_emitted:
                shdr.sh_name = shstrtab_i;
                shstrtab_i += strlen (section->name) + 1;
                write_struct_Elf32_Shdr (shdr_pos, &shdr, endianess);
                shdr_pos += SIZEOF_struct_Elf32_Shdr_file;
            }
        }

        write_struct_Elf32_Phdr (phdr_pos, &phdr, endianess);
        phdr_pos += SIZEOF_struct_Elf32_Phdr_file;
    }

    return pos;
}

static unsigned char *write_sections64 (unsigned char *file, Elf64_Ehdr *ehdr_p)
{
    unsigned char *pos;
    struct section *section;
    Elf64_Phdr *phdr_p;
    Elf64_Shdr *shdr_p = NULL;

    pos = file + size_of_headers;

    phdr_p = (void *)(file + ehdr_p->e_phoff);
    if (generate_section_headers) {
        shdr_p = (void *)(file + ehdr_p->e_shoff);
        shdr_p->sh_type = SHT_NULL;
        shdr_p->sh_link = SHN_UNDEF;
        shdr_p++;
    }

    for (section = all_sections; section; section = section->next) {
        phdr_p->p_type = PT_LOAD;
        phdr_p->p_paddr = phdr_p->p_vaddr = ld_state->base_address + section->rva;
        phdr_p->p_memsz = section->total_size;

        if (!section->is_bss) {
            phdr_p->p_filesz = section->total_size;
            pos = file + ALIGN (pos - file, section->section_alignment);
            phdr_p->p_offset = pos - file;

            section_write (section, pos);
            pos += section->total_size;
        } else {
            phdr_p->p_filesz = 0;
            phdr_p->p_offset = 0;
        }

        phdr_p->p_flags = translate_section_flags_to_p_flags (section->flags);

        /* Memory alignment and file alignment must be the same
         * (so it is good to have RVA of first section
         *  to be equal to its offset)
         * but neither memory size nor file size need to be rounded.
         */
        phdr_p->p_align = section->section_alignment;

        if (generate_section_headers) {
            shdr_p->sh_type = section->is_bss ? SHT_NOBITS : SHT_PROGBITS;
            shdr_p->sh_flags = translate_section_flags_to_sh_flags (section->flags);
            shdr_p->sh_addr = phdr_p->p_vaddr;
            shdr_p->sh_offset = phdr_p->p_offset;
            shdr_p->sh_size = phdr_p->p_memsz;
            shdr_p->sh_addralign = phdr_p->p_align;
            shdr_p++;

            if (ld_state->emit_relocs) {
                unsigned char *saved_pos = pos;

                pos = write_relocs_for_section64 (file, pos, section, shdr_p);
                if (pos != saved_pos) {
                    shdr_p++;
                }
            }
        }

        phdr_p++;
    }

    return pos;
}

address_type elf_get_first_section_rva (void)
{
    if (all_sections == NULL) return size_of_headers;

    return ALIGN (size_of_headers, all_sections->section_alignment);
}

void elf_before_link (void)
{
    struct section *section;

    for (section = all_sections; section; section = section->next) {
        /* Same alignment as COFF default SectionAlignment. */
        if (section->section_alignment < 0x1000) {
            section->section_alignment = 0x1000;
        }
    }
}

static void translate_relocation (struct reloc_entry *reloc,
                                  struct Elf32_Rel_internal *input_reloc,
                                  struct section_part *part)
{
    Elf32_Word symbol_index;
    Elf32_Word rel_type;

    symbol_index = ELF32_R_SYM (input_reloc->r_info);
    rel_type = ELF32_R_TYPE (input_reloc->r_info);

    if (symbol_index >= part->of->symbol_count) {
        ld_error ("relocation has invalid symbol index");
        goto bad;
    }

    if (input_reloc->r_offset >= part->content_size
        || input_reloc->r_offset + 4 > part->content_size) {
        ld_error ("relocation has invalid r_offset");
        goto bad;
    }
    
    /* STN_UNDEF should be treated as absolute symbol with value 0 at index 0. */
    reloc->symbol = part->of->symbol_array + symbol_index;
    reloc->offset = input_reloc->r_offset;
    
    if (ld_state->target_machine == LD_TARGET_MACHINE_I386) {
        switch (rel_type) {
            case R_386_NONE: goto bad; /* Ignored. */
            
            case R_386_32: reloc->howto = &reloc_howtos[RELOC_TYPE_32]; break;

            case R_386_PC32:
                reloc->howto = &reloc_howtos[RELOC_TYPE_PC32];
                reloc->addend += 4; /* ELF should not have the size of the field subtracted. */
                break;

            default:
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++relocation type 0x%02x not supported yet",
                                             rel_type);
                break;
        }
    } else if (ld_state->target_machine == LD_TARGET_MACHINE_ARM) {
        switch (rel_type) {
            case R_ARM_NONE: goto bad; /* Ignored. */

            case R_ARM_PC24: reloc->howto = &reloc_howtos[RELOC_TYPE_ARM_PC26]; break;
            
            case R_ARM_ABS32: reloc->howto = &reloc_howtos[RELOC_TYPE_ARM_32]; break;

            default:
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++relocation type 0x%02x not supported yet",
                                             rel_type);
                break;
        }
    } else if (ld_state->target_machine == LD_TARGET_MACHINE_MAINFRAME) {
        /* Assume it is the same as for i386 for now. */
        switch (rel_type) {
            case R_386_NONE: goto bad; /* Ignored. */
            
            case R_386_32: reloc->howto = &reloc_howtos[RELOC_TYPE_32]; break;

            default:
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++relocation type 0x%02x not supported yet",
                                             rel_type);
                break;
        }
    } else {
        ld_internal_error_at_source (__FILE__, __LINE__,
                                     "+++relocations for target %i not supported yet",
                                     ld_state->target_machine);
    }

    return;

bad:
    reloc->symbol = NULL;
    reloc->offset = 0;
    reloc->howto = &reloc_howtos[RELOC_TYPE_IGNORED];
}

static void translate_relocation64_addend (struct reloc_entry *reloc,
                                           Elf64_Rela *input_reloc,
                                           struct section_part *part)
{
    Elf64_Word symbol_index;
    Elf64_Word rel_type;

    symbol_index = ELF64_R_SYM (input_reloc->r_info);
    rel_type = ELF64_R_TYPE (input_reloc->r_info);

    if (symbol_index >= part->of->symbol_count) {
        ld_error ("relocation has invalid symbol index");
        goto bad;
    }

    if (input_reloc->r_offset >= part->content_size) {
        ld_error ("relocation has invalid r_offset");
        goto bad;
    }
    
    /* STN_UNDEF should be treated as absolute symbol with value 0 at index 0. */
    reloc->symbol = part->of->symbol_array + symbol_index;
    reloc->offset = input_reloc->r_offset;
    reloc->addend = input_reloc->r_addend;

    if (ld_state->target_machine == LD_TARGET_MACHINE_AARCH64) {
        switch (rel_type) {
            case R_AARCH64_NONE: goto bad; /* Ignored. */

            case R_AARCH64_PREL32:
                reloc->howto = &reloc_howtos[RELOC_TYPE_PC32];
                reloc->addend += 4; /* ELF should not have the size of the field subtracted. */
                break;

            case R_AARCH64_ABS64: reloc->howto = &reloc_howtos[RELOC_TYPE_64]; break;
            case R_AARCH64_ABS32: reloc->howto = &reloc_howtos[RELOC_TYPE_32]; break;

            case R_AARCH64_ADR_PREL_PG_HI21: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_ADR_PREL_PG_HI21]; break;
            
            case R_AARCH64_ADD_ABS_LO12_NC: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_ADD_ABS_LO12_NC]; break;
            case R_AARCH64_LDST8_ABS_LO12_NC: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_LDST8_ABS_LO12_NC]; break;
            case R_AARCH64_LDST16_ABS_LO12_NC: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_LDST16_ABS_LO12_NC]; break;
            case R_AARCH64_LDST32_ABS_LO12_NC: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_LDST32_ABS_LO12_NC]; break;
            case R_AARCH64_LDST64_ABS_LO12_NC: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_LDST64_ABS_LO12_NC]; break;
            case R_AARCH64_LDST128_ABS_LO12_NC: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_LDST128_ABS_LO12_NC]; break;

            case R_AARCH64_JUMP26: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_JUMP26]; break;
            case R_AARCH64_CALL26: reloc->howto = &reloc_howtos[RELOC_TYPE_AARCH64_CALL26]; break;

            case R_AARCH64_ADR_GOT_PAGE:
            case R_AARCH64_LD64_GOT_LO12_NC:
                ld_warn ("Position Independent Executables are not supported, ignoring relocation type %u",
                         rel_type);
                goto bad;

            default:
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++relocation type %u not supported yet",
                                             rel_type);
                break;
        }
    } else {
        ld_internal_error_at_source (__FILE__, __LINE__,
                                     "+++relocations for target %i not supported yet",
                                     ld_state->target_machine);
    }

    return;

bad:
    reloc->symbol = NULL;
    reloc->offset = 0;
    reloc->addend = 0;
    reloc->howto = &reloc_howtos[RELOC_TYPE_IGNORED];
}

static void elf64_write (const char *filename);

void elf_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct Elf32_Ehdr_internal ehdr;
    unsigned long shstrtab_start_i = 0;

    struct section *section;

    if (ld_state->target_machine == LD_TARGET_MACHINE_X64
        || ld_state->target_machine == LD_TARGET_MACHINE_AARCH64) {
        elf64_write (filename);
        return;
    }

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    /* Relocations exist only in sections. */
    if (ld_state->emit_relocs) generate_section_headers = 1;

    memset (&ehdr, 0, sizeof (ehdr));
    
    ehdr.e_ident[EI_MAG0] = ELFMAG0;
    ehdr.e_ident[EI_MAG1] = ELFMAG1;
    ehdr.e_ident[EI_MAG2] = ELFMAG2;
    ehdr.e_ident[EI_MAG3] = ELFMAG3;
    ehdr.e_ident[EI_CLASS] = ELFCLASS32;

    if (ld_state->target_machine == LD_TARGET_MACHINE_M68K
        || ld_state->target_machine == LD_TARGET_MACHINE_MAINFRAME) {
        ehdr.e_ident[EI_DATA] = ELFDATA2MSB;
        endianess = BIG_ENDIAN;
    } else {
        ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    }

    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    if (ld_state->target_machine == LD_TARGET_MACHINE_ARM) {
        ehdr.e_ident[EI_OSABI] = ELFOSABI_ARM;
    } else if (ld_state->target_machine == LD_TARGET_MACHINE_MAINFRAME) {
        ehdr.e_ident[EI_OSABI] = ELFOSABI_LINUX;
    }

    ehdr.e_type = ET_EXEC;
    switch (ld_state->target_machine) {
        case LD_TARGET_MACHINE_I386: ehdr.e_machine = EM_386; break;
        case LD_TARGET_MACHINE_ARM: ehdr.e_machine = EM_ARM; break;
        case LD_TARGET_MACHINE_M68K: ehdr.e_machine = EM_68K; break;
        case LD_TARGET_MACHINE_MAINFRAME: ehdr.e_machine = EM_S370; break;
        default: ehdr.e_machine = EM_NONE; break;
    }
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = ld_state->entry_point + ld_state->base_address;
    ehdr.e_ehsize = SIZEOF_struct_Elf32_Ehdr_file;
    if (ld_state->target_machine == LD_TARGET_MACHINE_ARM) {
        /* Not sure what those flags mean
         * but they are required.
         */
        ehdr.e_flags = 0x01000002;
    }

    if (ld_state->target_machine == LD_TARGET_MACHINE_MAINFRAME
        && (section = section_find (".text"))) {
        /* Normal MVS has text and data mixed inside .text section,
         * so .text section should always be writable.
         */
        section->flags &= ~SECTION_FLAG_READONLY;
    }

    {
        /* Current layout of executable is:
         * Elf32_Ehdr
         * Content of sections (including section name string table)
         * Elf32_Phdr table (aligned just for readability)
         * Elf32_Shdr table (aligned just for readability)
         */
        file_size = size_of_headers;

        for (section = all_sections; section; section = section->next) {
            if (!section->is_bss) {
                file_size = ALIGN (file_size, section->section_alignment);
                file_size += section->total_size;
            }

            if (ld_state->emit_relocs) {
                size_t relocs_needed = section_get_num_relocs (section);

                if (relocs_needed) {
                    file_size = ALIGN (file_size, RELOC_SECTION_ALIGNMENT);
                    file_size += relocs_needed * SIZEOF_struct_Elf32_Rel_file;
                }
            }
        }

        if (generate_section_headers) {
            size_t shstrtab_size;
            shstrtab_start_i = shstrtab_size = 1 + sizeof (".shstrtab");

            for (section = all_sections; section; section = section->next) {
                if (ld_state->emit_relocs && section_get_num_relocs (section)) {
                    shstrtab_size += sizeof (".rel") - 1;
                }
                shstrtab_size += strlen (section->name) + 1;
            }

            file_size += shstrtab_size;
        }

        file_size = ALIGN (file_size, SIZEOF_struct_Elf32_Phdr_file);
        ehdr.e_phoff = file_size;
        ehdr.e_phentsize = SIZEOF_struct_Elf32_Phdr_file;
        ehdr.e_phnum = 0;
        for (section = all_sections; section; section = section->next) {
            ehdr.e_phnum++;
        }
        file_size += ehdr.e_phnum * SIZEOF_struct_Elf32_Phdr_file;

        if (generate_section_headers) {
            file_size = ALIGN (file_size, SIZEOF_struct_Elf32_Shdr_file);
            ehdr.e_shoff = file_size;
            ehdr.e_shentsize = SIZEOF_struct_Elf32_Shdr_file;
            ehdr.e_shnum = 1;
            for (section = all_sections; section; section = section->next) {
                section->target_index = ehdr.e_shnum++;
                if (ld_state->emit_relocs && section_get_num_relocs (section)) {
                    ehdr.e_shnum++;
                }
            }
            ehdr.e_shstrndx = ehdr.e_shnum++;
            
            file_size += ehdr.e_shnum * SIZEOF_struct_Elf32_Shdr_file;
        }
    }

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    pos = write_sections (file, &ehdr, shstrtab_start_i);

    if (generate_section_headers) {
        unsigned char *saved_pos;
        unsigned char *shdr_pos;
        
        struct Elf32_Shdr_internal shstrshdr = {0};

        shdr_pos = file + ehdr.e_shoff + ehdr.e_shstrndx * SIZEOF_struct_Elf32_Shdr_file;
        saved_pos = pos;
        
        shstrshdr.sh_name = 1;
        shstrshdr.sh_type = SHT_STRTAB;
        shstrshdr.sh_offset = saved_pos - file;
        shstrshdr.sh_addralign = 1;

        *pos = '\0';
        pos++;
        memcpy (pos, ".shstrtab", sizeof (".shstrtab"));
        pos += sizeof (".shstrtab");

        for (section = all_sections; section; section = section->next) {
            int has_relocs = ld_state->emit_relocs && section_get_num_relocs (section);
            size_t name_len = strlen (section->name);

            if (has_relocs) {
                memcpy (pos, ".rel", sizeof (".rel") - 1);
                pos += sizeof (".rel") - 1;
            }

            memcpy (pos, section->name, name_len);
            pos[name_len] = '\0';
            pos += name_len + 1;
        }

        shstrshdr.sh_size = pos - saved_pos;
        write_struct_Elf32_Shdr (shdr_pos, &shstrshdr, endianess);
    }

    pos = file;

    write_struct_Elf32_Ehdr (pos, &ehdr, endianess);

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}

static void elf64_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    Elf64_Ehdr ehdr;

    struct section *section;

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    /* Relocations exist only in sections. */
    if (ld_state->emit_relocs) generate_section_headers = 1;

    memset (&ehdr, 0, sizeof (ehdr));
    
    ehdr.e_ident[EI_MAG0] = ELFMAG0;
    ehdr.e_ident[EI_MAG1] = ELFMAG1;
    ehdr.e_ident[EI_MAG2] = ELFMAG2;
    ehdr.e_ident[EI_MAG3] = ELFMAG3;
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;

    ehdr.e_type = ET_EXEC;
    switch (ld_state->target_machine) {
        case LD_TARGET_MACHINE_X64: ehdr.e_machine = EM_X86_64; break;
        case LD_TARGET_MACHINE_AARCH64: ehdr.e_machine = EM_AARCH64; break;
        default: ehdr.e_machine = EM_NONE; break;
    }
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = ld_state->entry_point + ld_state->base_address;
    ehdr.e_ehsize = sizeof (ehdr);

    {
        /* Current layout of executable is:
         * Elf64_Ehdr
         * Content of sections (including section name string table)
         * Elf64_Phdr table (aligned just for readability)
         * Elf64_Shdr table (aligned just for readability)
         */
        file_size = size_of_headers;

        for (section = all_sections; section; section = section->next) {
            if (!section->is_bss) {
                file_size = ALIGN (file_size, section->section_alignment);
                file_size += section->total_size;
            }

            if (ld_state->emit_relocs) {
                size_t relocs_needed = section_get_num_relocs (section);

                if (relocs_needed) {
                    file_size = ALIGN (file_size, RELOC_SECTION_ALIGNMENT);
                    file_size += relocs_needed * sizeof (Elf64_Rel);
                }
            }
        }

        if (generate_section_headers) {
            size_t shstrtab_size;
            shstrtab_size = 1 + sizeof (".shstrtab");

            for (section = all_sections; section; section = section->next) {
                if (ld_state->emit_relocs && section_get_num_relocs (section)) {
                    shstrtab_size += sizeof (".rel") - 1;
                }
                shstrtab_size += strlen (section->name) + 1;
            }

            file_size += shstrtab_size;
        }

        file_size = ALIGN (file_size, sizeof (Elf64_Phdr));
        ehdr.e_phoff = file_size;
        ehdr.e_phentsize = sizeof (Elf64_Phdr);
        ehdr.e_phnum = 0;
        for (section = all_sections; section; section = section->next) {
            ehdr.e_phnum++;
        }
        file_size += ehdr.e_phnum * sizeof (Elf64_Phdr);

        if (generate_section_headers) {
            file_size = ALIGN (file_size, sizeof (Elf64_Shdr));
            ehdr.e_shoff = file_size;
            ehdr.e_shentsize = sizeof (Elf64_Shdr);
            ehdr.e_shnum = 1;
            for (section = all_sections; section; section = section->next) {
                section->target_index = ehdr.e_shnum++;
                if (ld_state->emit_relocs && section_get_num_relocs (section)) {
                    ehdr.e_shnum++;
                }
            }
            ehdr.e_shstrndx = ehdr.e_shnum++;
            
            file_size += ehdr.e_shnum * sizeof (Elf64_Shdr);
        }
    }

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    pos = write_sections64 (file, &ehdr);

    if (generate_section_headers) {
        unsigned char *saved_pos;
        Elf64_Shdr *shdr_p, *shstrshdr_p;

        shdr_p = (void *)(file + ehdr.e_shoff);
        shstrshdr_p = shdr_p + ehdr.e_shstrndx;
        shdr_p++;
        saved_pos = pos;
        
        shstrshdr_p->sh_name = 1;
        shstrshdr_p->sh_type = SHT_STRTAB;
        shstrshdr_p->sh_offset = saved_pos - file;
        shstrshdr_p->sh_addralign = 1;

        *pos = '\0';
        pos++;
        memcpy (pos, ".shstrtab", sizeof (".shstrtab"));
        pos += sizeof (".shstrtab");

        for (section = all_sections; section; section = section->next) {
            int has_relocs = ld_state->emit_relocs && section_get_num_relocs (section);
            size_t name_len = strlen (section->name);

            if (has_relocs) {
                memcpy (pos, ".rel", sizeof (".rel") - 1);
                pos += sizeof (".rel") - 1;
            }
            shdr_p->sh_name = pos - saved_pos;
            if (has_relocs) {
                shdr_p++;
                shdr_p->sh_name = pos - saved_pos - (sizeof (".rel") - 1);
            }

            memcpy (pos, section->name, name_len);
            pos[name_len] = '\0';
            pos += name_len + 1;

            shdr_p++;
        }

        shstrshdr_p->sh_size = pos - saved_pos;
    }

    pos = file;

    memcpy (pos, &ehdr, sizeof (ehdr));

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("corrupted input file"); } while (0)

static int read_elf64_object (unsigned char *file, size_t file_size, const char *filename);

static int read_elf_object (unsigned char *file, size_t file_size, const char *filename)
{
    struct Elf32_Ehdr_internal ehdr;
    struct Elf32_Shdr_internal shdr;

    const char *section_name_string_table = NULL;
    Elf32_Word section_name_string_table_size;

    unsigned char *pos;

    struct object_file *of;
    struct section_part **part_p_array;
    Elf32_Half i;

    struct section *bss_section = NULL;
    long bss_section_number = 0;

    endianess = LITTLE_ENDIAN;

    pos = file;
    CHECK_READ (pos, SIZEOF_struct_Elf32_Ehdr_file);
    read_struct_Elf32_Ehdr (&ehdr, pos, endianess);

    if (ehdr.e_ident[EI_CLASS] == ELFCLASS64) {
        return read_elf64_object (file, file_size, filename);
    }

    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        ld_error ("%s: Unsupported ELF file class", filename);
        return 1;
    }

    if (ehdr.e_ident[EI_DATA] == ELFDATA2LSB) {
        /* Nothing needs to be done. */
    } else if (ehdr.e_ident[EI_DATA] == ELFDATA2MSB) {
        endianess = BIG_ENDIAN;
        read_struct_Elf32_Ehdr (&ehdr, pos, endianess);
    } else {
        ld_error ("%s: Unsupported ELF data encoding", filename);
        return 1;
    }

    if (ehdr.e_ident[EI_VERSION] != EV_CURRENT) {
        ld_error ("%s: Unsupported ELF version", filename);
        return 1;
    }

    if (ehdr.e_type != ET_REL) {
        ld_error ("%s: e_type is not ET_REL", filename);
        return 1;
    }

    switch (ehdr.e_machine) {
        case EM_386:
            if (ld_state->target_machine == LD_TARGET_MACHINE_I386
                || ld_state->target_machine == LD_TARGET_MACHINE_UNKNOWN) {
                ld_state->target_machine = LD_TARGET_MACHINE_I386;
            } else {
                ld_error ("%s: e_machine is not EM_386", filename);
                return 1;
            }
            break;

        case EM_S370:
            if (ld_state->target_machine == LD_TARGET_MACHINE_MAINFRAME
                || ld_state->target_machine == LD_TARGET_MACHINE_UNKNOWN) {
                ld_state->target_machine = LD_TARGET_MACHINE_MAINFRAME;
            } else {
                ld_error ("%s: e_machine is not EM_S370", filename);
                return 1;
            }
            break;

        case EM_ARM:
            if (ld_state->target_machine == LD_TARGET_MACHINE_ARM
                || ld_state->target_machine == LD_TARGET_MACHINE_UNKNOWN) {
                ld_state->target_machine = LD_TARGET_MACHINE_ARM;
            } else {
                ld_error ("%s: e_machine is not EM_ARM", filename);
                return 1;
            }
            break;

        default:
            ld_error ("%s: unrecognized e_machine %i", filename, ehdr.e_machine);
            return 1;
    }

    if (ehdr.e_version != EV_CURRENT) {
        ld_error ("%s: e_version is not EV_CURRENT", filename);
        return 1;
    }

    if (ehdr.e_ehsize < SIZEOF_struct_Elf32_Ehdr_file) {
        ld_error ("%s: e_ehsize is too small", filename);
        return 1;
    }

    if (ehdr.e_shoff == 0 || ehdr.e_shentsize == 0 || ehdr.e_shnum == 0) {
        ld_error ("%s: missing section header table", filename);
        return 1;
    }

    if (ehdr.e_shentsize < SIZEOF_struct_Elf32_Shdr_file) {
        ld_error ("%s: e_shentsize is too small", filename);
        return 1;
    }

    if (ehdr.e_shstrndx == 0 || ehdr.e_shstrndx >= ehdr.e_shnum) {
        ld_error ("%s: missing section name string table", filename);
        return 1;
    }

    pos = file + ehdr.e_shoff;
    CHECK_READ (pos, ehdr.e_shentsize * ehdr.e_shnum);

    pos += ehdr.e_shentsize * ehdr.e_shstrndx;
    read_struct_Elf32_Shdr (&shdr, pos, endianess);

    if (shdr.sh_type != SHT_STRTAB) {
        ld_error ("section name string table does not have SHT_STRTAB type");
        return 1;
    }

    section_name_string_table_size = shdr.sh_size;
    pos = file + shdr.sh_offset;
    CHECK_READ (pos, section_name_string_table_size);
    section_name_string_table = (char *)pos;

    part_p_array = xmalloc (sizeof (*part_p_array) * (ehdr.e_shnum + 1));
    of = NULL;

    for (i = 1; i < ehdr.e_shnum; i++) {
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        read_struct_Elf32_Shdr (&shdr, pos, endianess);

        if (shdr.sh_type != SHT_SYMTAB) continue;

        if (of) ld_fatal_error ("more than 1 symbol table per object file");

        of = object_file_make (shdr.sh_size / shdr.sh_entsize, filename);
    }

    if (!of) of = object_file_make (1, filename);

    for (i = 1; i < ehdr.e_shnum; i++) {
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        read_struct_Elf32_Shdr (&shdr, pos, endianess);

        {
            struct section *section;

            {
                char *section_name;
                
                if (shdr.sh_name < section_name_string_table_size) {
                    section_name = xstrdup (section_name_string_table + shdr.sh_name);
                } else ld_fatal_error ("invalid offset into string table");

                if (shdr.sh_type != SHT_PROGBITS
                    && shdr.sh_type != SHT_NOBITS) {
                    part_p_array[i] = NULL;
                    free (section_name);
                    continue;
                }

                section = section_find_or_make (section_name);
                
                if (shdr.sh_addralign > section->section_alignment) {
                    section->section_alignment = shdr.sh_addralign;
                }

                section->flags = translate_sh_flags_to_section_flags (shdr.sh_flags);

                if (shdr.sh_type == SHT_NOBITS) {
                    section->is_bss = 1;
                }

                free (section_name);
            }

            {
                struct section_part *part = section_part_new (section, of);

                part->alignment = shdr.sh_addralign;

                part->content_size = shdr.sh_size;
                if (shdr.sh_type != SHT_NOBITS) {
                    pos = file + shdr.sh_offset;
                    part->content = xmalloc (part->content_size);

                    CHECK_READ (pos, part->content_size);
                    memcpy (part->content, pos, part->content_size);
                }

                if (section->is_bss) {
                    bss_section_number = i;
                    bss_section = section;
                }
                
                section_append_section_part (section, part);

                part_p_array[i] = part;
            }
        }
    }

    for (i = 1; i < ehdr.e_shnum; i++) {
        const char *sym_strtab;
        Elf32_Word sym_strtab_size;
        Elf32_Word j;
        
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        read_struct_Elf32_Shdr (&shdr, pos, endianess);

        if (shdr.sh_type != SHT_SYMTAB) continue;

        if (shdr.sh_link == 0 || shdr.sh_link >= ehdr.e_shnum) {
            ld_fatal_error ("symtab has invalid sh_link");
        }
        
        pos = file + ehdr.e_shoff + shdr.sh_link * ehdr.e_shentsize;
        {
            struct Elf32_Shdr_internal strtabhdr;

            read_struct_Elf32_Shdr (&strtabhdr, pos, endianess);

            if (strtabhdr.sh_type != SHT_STRTAB) {
                ld_fatal_error ("symbol name string table does not have SHT_STRTAB type");
            }

            sym_strtab_size = strtabhdr.sh_size;
            pos = file + strtabhdr.sh_offset;
            CHECK_READ (pos, sym_strtab_size);
            sym_strtab = (char *)pos;
        }

        pos = file + shdr.sh_offset;
        CHECK_READ (pos, shdr.sh_size);

        if (shdr.sh_entsize < SIZEOF_struct_Elf32_Sym_file) {
            ld_fatal_error ("symbol table sh_entsize is too small");
        }

        for (j = 1; j < shdr.sh_size / shdr.sh_entsize; j++) {
            struct Elf32_Sym_internal elf_symbol;
            struct symbol *symbol = of->symbol_array + j;
            
            pos = file + shdr.sh_offset + j * shdr.sh_entsize;
            read_struct_Elf32_Sym (&elf_symbol, pos, endianess);

            if (elf_symbol.st_name < sym_strtab_size) {
                if (sym_strtab[elf_symbol.st_name] == '\0') {
                    symbol->name = xstrdup (UNNAMED_SYMBOL_NAME);
                } else {
                    symbol->name = xstrdup (sym_strtab + elf_symbol.st_name);
                }
            } else ld_fatal_error ("invalid offset into string table");

            symbol->value = elf_symbol.st_value;
            symbol->size = elf_symbol.st_size;
            symbol->section_number = elf_symbol.st_shndx;

            if (elf_symbol.st_shndx == SHN_UNDEF) {
                symbol->section_number = UNDEFINED_SECTION_NUMBER;
                symbol->part = NULL;
            } else if (elf_symbol.st_shndx == SHN_ABS) {
                symbol->section_number = ABSOLUTE_SECTION_NUMBER;
                symbol->part = NULL;
            } else if (elf_symbol.st_shndx == SHN_COMMON) {
                if (symbol->size) {
                    struct symbol *old_symbol = symbol_find (symbol->name);

                    if (ELF32_ST_BIND (elf_symbol.st_info) != STB_GLOBAL) {
                        ld_fatal_error ("non-global common symbol");
                    }

                    if (!old_symbol || symbol_is_undefined (old_symbol)) {
                        struct section_part *bss_part;
                        
                        if (bss_section == NULL) {
                            bss_section = section_find_or_make (".bss");

                            if (bss_section->section_alignment < 4) {
                                bss_section->section_alignment = 4;
                            }
                            bss_section->flags = translate_sh_flags_to_section_flags (SHF_WRITE | SHF_ALLOC);
                            bss_section->is_bss = 1;
                            bss_section_number = ehdr.e_shnum ? ehdr.e_shnum : 1;
                        }
                        
                        bss_part = section_part_new (bss_section, of);
                        section_append_section_part (bss_section, bss_part);

                        bss_part->content_size = symbol->size;
                        bss_part->alignment = symbol->value;
                        symbol->part = bss_part;
                        symbol->value = 0;
                        symbol->section_number = bss_section_number;
                    } else {
                        if (symbol->size > old_symbol->size) {
                            old_symbol->part->content_size = old_symbol->size = symbol->size;
                        }
                        if (symbol->value > old_symbol->part->alignment) {
                            old_symbol->part->alignment = symbol->value;
                        }
                        
                        symbol->value = 0;
                        symbol->section_number = UNDEFINED_SECTION_NUMBER;
                        symbol->part = NULL;
                    }
                } else {
                    symbol->section_number = UNDEFINED_SECTION_NUMBER;
                    symbol->part = NULL;
                }
            } else if (elf_symbol.st_shndx >= SHN_LORESERVE
                       && elf_symbol.st_shndx <= SHN_HIRESERVE) {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++not yet supported symbol st_shndx: %hu",
                                             elf_symbol.st_shndx);
            } else if (elf_symbol.st_shndx >= ehdr.e_shnum) {
                ld_error ("invalid symbol st_shndx: %hu", elf_symbol.st_shndx);
                symbol->part = NULL;
            } else {
                symbol->part = part_p_array[elf_symbol.st_shndx];
            }

            switch (ELF32_ST_BIND (elf_symbol.st_info)) {
                case STB_LOCAL:
                    /* Do nothing */
                    break;
                
                case STB_GLOBAL:
                    symbol_record_external_symbol (symbol);
                    break;
                
                default:
                    ld_internal_error_at_source (__FILE__, __LINE__,
                                                 "+++not yet supported symbol ELF32_ST_BIND: %u",
                                                 ELF32_ST_BIND (elf_symbol.st_info));
                    break;
            }

            if (ELF32_ST_TYPE (elf_symbol.st_info) == STT_SECTION) {
                symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
            }
        }
    }

    for (i = 1; i < ehdr.e_shnum; i++) {
        struct section_part *part;
        size_t j;
        
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        read_struct_Elf32_Shdr (&shdr, pos, endianess);

        if ((shdr.sh_type != SHT_RELA
             && shdr.sh_type != SHT_REL)
            || shdr.sh_size == 0) continue;

        if (shdr.sh_info >= ehdr.e_shnum) {
            ld_fatal_error ("%s: relocation section has invalid sh_info", filename);
        }

        part = part_p_array[shdr.sh_info];
        if (!part) {
            ld_fatal_error ("%s: relocation section has invalid sh_info", filename);
        }
        if (part->section->is_bss) {
            ld_fatal_error ("%s: relocation section sh_info points to BSS section",
                            filename);
        }

        if ((shdr.sh_type == SHT_RELA
             && shdr.sh_entsize != SIZEOF_struct_Elf32_Rela_file)
            || (shdr.sh_type == SHT_REL
                && shdr.sh_entsize != SIZEOF_struct_Elf32_Rel_file)) {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "+++relocation shdr.sh_entsize not yet supported");
        }

        pos = file + shdr.sh_offset;
        CHECK_READ (pos, shdr.sh_size);

        part->relocation_count = shdr.sh_size / shdr.sh_entsize;
        part->relocation_array = xcalloc (part->relocation_count, sizeof *part->relocation_array);

        if (shdr.sh_type == SHT_RELA) {
            for (j = 0; j < part->relocation_count; j++) {
                struct Elf32_Rela_internal rela;
                struct Elf32_Rel_internal rel;
                
                read_struct_Elf32_Rela (&rela, pos + shdr.sh_entsize * j, endianess);
                rel.r_offset = rela.r_offset;
                rel.r_info = rela.r_info;
                part->relocation_array[j].addend = rela.r_addend;
                translate_relocation (part->relocation_array + j, &rel, part);
            }
        } else {
            for (j = 0; j < part->relocation_count; j++) {
                struct Elf32_Rel_internal rel;
                read_struct_Elf32_Rel (&rel, pos + shdr.sh_entsize * j, endianess);
                translate_relocation (part->relocation_array + j, &rel, part);
            }
        }
    }

    free (part_p_array);

    return 0;
}

static int read_elf64_object (unsigned char *file, size_t file_size, const char *filename)
{
    Elf64_Ehdr ehdr;
    Elf64_Shdr *shdr_p;

    const char *section_name_string_table = NULL;
    Elf64_Word section_name_string_table_size;

    unsigned char *pos;

    struct object_file *of;
    struct section_part **part_p_array;
    Elf64_Half i;

    struct section *bss_section = NULL;
    long bss_section_number = 0;

    pos = file;
    CHECK_READ (pos, sizeof (ehdr));
    ehdr = *(Elf64_Ehdr *)pos;

    if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB) {
        ld_error ("%s: Unsupported ELF data encoding", filename);
        return 1;
    }

    if (ehdr.e_ident[EI_VERSION] != EV_CURRENT) {
        ld_error ("%s: Unsupported ELF version", filename);
        return 1;
    }

    if (ehdr.e_type != ET_REL) {
        ld_error ("%s: e_type is not ET_REL", filename);
        return 1;
    }

    switch (ehdr.e_machine) {
        case EM_AARCH64:
            if (ld_state->target_machine == LD_TARGET_MACHINE_AARCH64
                || ld_state->target_machine == LD_TARGET_MACHINE_UNKNOWN) {
                ld_state->target_machine = LD_TARGET_MACHINE_AARCH64;
            } else {
                ld_error ("%s: e_machine is not EM_AARCH64", filename);
                return 1;
            }
            break;

        default:
            ld_error ("%s: unrecognized e_machine %i", filename, ehdr.e_machine);
            return 1;
    }

    if (ehdr.e_version != EV_CURRENT) {
        ld_error ("%s: e_version is not EV_CURRENT", filename);
        return 1;
    }

    if (ehdr.e_ehsize < sizeof (ehdr)) {
        ld_error ("%s: e_ehsize is too small", filename);
        return 1;
    }

    if (ehdr.e_shoff == 0 || ehdr.e_shentsize == 0 || ehdr.e_shnum == 0) {
        ld_error ("%s: missing section header table", filename);
        return 1;
    }

    if (ehdr.e_shentsize < sizeof (Elf64_Shdr)) {
        ld_error ("%s: e_shentsize is too small", filename);
        return 1;
    }

    if (ehdr.e_shstrndx == 0 || ehdr.e_shstrndx >= ehdr.e_shnum) {
        ld_error ("%s: missing section name string table", filename);
        return 1;
    }

    pos = file + ehdr.e_shoff;
    CHECK_READ (pos, ehdr.e_shentsize * ehdr.e_shnum);

    pos += ehdr.e_shentsize * ehdr.e_shstrndx;
    shdr_p = (void *)pos;

    if (shdr_p->sh_type != SHT_STRTAB) {
        ld_error ("section name string table does not have SHT_STRTAB type");
        return 1;
    }

    section_name_string_table_size = shdr_p->sh_size;
    pos = file + shdr_p->sh_offset;
    CHECK_READ (pos, section_name_string_table_size);
    section_name_string_table = (char *)pos;

    part_p_array = xmalloc (sizeof (*part_p_array) * (ehdr.e_shnum + 1));
    of = NULL;

    for (i = 1; i < ehdr.e_shnum; i++) {
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        shdr_p = (void *)pos;

        if (shdr_p->sh_type != SHT_SYMTAB) continue;

        if (of) ld_fatal_error ("more than 1 symbol table per object file");

        of = object_file_make (shdr_p->sh_size / shdr_p->sh_entsize, filename);
    }

    if (!of) of = object_file_make (1, filename);

    for (i = 1; i < ehdr.e_shnum; i++) {
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        shdr_p = (void *)pos;

        {
            struct section *section;

            {
                char *section_name;
                
                if (shdr_p->sh_name < section_name_string_table_size) {
                    section_name = xstrdup (section_name_string_table + shdr_p->sh_name);
                } else ld_fatal_error ("invalid offset into string table");

                if (*section_name) {
                    /* Strips the ".something" suffix from ".section.something"
                     * so all ".section..." content goes into one ".section" section.
                     */
                    char *p = strchr (section_name + 1, '.');
                    if (p) *p = '\0';
                }

                if (shdr_p->sh_type != SHT_PROGBITS
                    && shdr_p->sh_type != SHT_NOBITS) {
                    part_p_array[i] = NULL;
                    free (section_name);
                    continue;
                }

                section = section_find_or_make (section_name);

                if (shdr_p->sh_addralign > section->section_alignment) {
                    section->section_alignment = shdr_p->sh_addralign;
                }

                section->flags = translate_sh_flags_to_section_flags (shdr_p->sh_flags);

                if (shdr_p->sh_type == SHT_NOBITS) {
                    section->is_bss = 1;
                }

                free (section_name);
            }

            {
                struct section_part *part = section_part_new (section, of);

                part->alignment = shdr_p->sh_addralign;

                part->content_size = shdr_p->sh_size;
                if (shdr_p->sh_type != SHT_NOBITS) {
                    pos = file + shdr_p->sh_offset;
                    part->content = xmalloc (part->content_size);

                    CHECK_READ (pos, part->content_size);
                    memcpy (part->content, pos, part->content_size);
                }

                if (section->is_bss) {
                    bss_section_number = i;
                    bss_section = section;
                }
                
                section_append_section_part (section, part);

                part_p_array[i] = part;
            }
        }
    }

    for (i = 1; i < ehdr.e_shnum; i++) {
        const char *sym_strtab;
        Elf64_Word sym_strtab_size;
        Elf64_Word j;
        
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        shdr_p = (void *)pos;

        if (shdr_p->sh_type != SHT_SYMTAB) continue;

        if (shdr_p->sh_link == 0 || shdr_p->sh_link >= ehdr.e_shnum) {
            ld_fatal_error ("symtab has invalid sh_link");
        }
        
        pos = file + ehdr.e_shoff + shdr_p->sh_link * ehdr.e_shentsize;
        {
            const Elf64_Shdr *strtabhdr_p = (void *)pos;

            if (strtabhdr_p->sh_type != SHT_STRTAB) {
                ld_fatal_error ("symbol name string table does not have SHT_STRTAB type");
            }

            sym_strtab_size = strtabhdr_p->sh_size;
            pos = file + strtabhdr_p->sh_offset;
            CHECK_READ (pos, sym_strtab_size);
            sym_strtab = (char *)pos;
        }

        pos = file + shdr_p->sh_offset;
        CHECK_READ (pos, shdr_p->sh_size);

        if (shdr_p->sh_entsize < sizeof (Elf64_Sym)) {
            ld_fatal_error ("symbol table sh_entsize is too small");
        }

        for (j = 1; j < shdr_p->sh_size / shdr_p->sh_entsize; j++) {
            Elf64_Sym *elf_symbol;
            struct symbol *symbol = of->symbol_array + j;
            
            pos = file + shdr_p->sh_offset + j * shdr_p->sh_entsize;
            elf_symbol = (void *)pos;

            if (elf_symbol->st_name < sym_strtab_size) {
                if (sym_strtab[elf_symbol->st_name] == '\0') {
                    symbol->name = xstrdup (UNNAMED_SYMBOL_NAME);
                } else {
                    symbol->name = xstrdup (sym_strtab + elf_symbol->st_name);
                }
            } else ld_fatal_error ("invalid offset into string table");

            symbol->value = elf_symbol->st_value;
            symbol->size = elf_symbol->st_size;
            symbol->section_number = elf_symbol->st_shndx;

            if (elf_symbol->st_shndx == SHN_UNDEF) {
                symbol->section_number = UNDEFINED_SECTION_NUMBER;
                symbol->part = NULL;
            } else if (elf_symbol->st_shndx == SHN_ABS) {
                symbol->section_number = ABSOLUTE_SECTION_NUMBER;
                symbol->part = NULL;
            } else if (elf_symbol->st_shndx == SHN_COMMON) {
                if (symbol->size) {
                    struct symbol *old_symbol = symbol_find (symbol->name);

                    if (ELF64_ST_BIND (elf_symbol->st_info) != STB_GLOBAL) {
                        ld_fatal_error ("non-global common symbol");
                    }

                    if (!old_symbol || symbol_is_undefined (old_symbol)) {
                        struct section_part *bss_part;
                        
                        if (bss_section == NULL) {
                            bss_section = section_find_or_make (".bss");

                            if (bss_section->section_alignment < 4) {
                                bss_section->section_alignment = 4;
                            }
                            bss_section->flags = translate_sh_flags_to_section_flags (SHF_WRITE | SHF_ALLOC);
                            bss_section->is_bss = 1;
                            bss_section_number = ehdr.e_shnum ? ehdr.e_shnum : 1;
                        }
                        
                        bss_part = section_part_new (bss_section, of);
                        section_append_section_part (bss_section, bss_part);

                        bss_part->content_size = symbol->size;
                        bss_part->alignment = symbol->value;
                        symbol->part = bss_part;
                        symbol->value = 0;
                        symbol->section_number = bss_section_number;
                    } else {
                        if (symbol->size > old_symbol->size) {
                            old_symbol->part->content_size = old_symbol->size = symbol->size;
                        }
                        if (symbol->value > old_symbol->part->alignment) {
                            old_symbol->part->alignment = symbol->value;
                        }
                        
                        symbol->value = 0;
                        symbol->section_number = UNDEFINED_SECTION_NUMBER;
                        symbol->part = NULL;
                    }
                } else {
                    symbol->section_number = UNDEFINED_SECTION_NUMBER;
                    symbol->part = NULL;
                }
            } else if (elf_symbol->st_shndx >= SHN_LORESERVE
                       && elf_symbol->st_shndx <= SHN_HIRESERVE) {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++not yet supported symbol st_shndx: %hu",
                                             elf_symbol->st_shndx);
            } else if (elf_symbol->st_shndx >= ehdr.e_shnum) {
                ld_error ("invalid symbol st_shndx: %hu", elf_symbol->st_shndx);
                symbol->part = NULL;
            } else {
                symbol->part = part_p_array[elf_symbol->st_shndx];
            }

            switch (ELF64_ST_BIND (elf_symbol->st_info)) {
                case STB_LOCAL:
                    /* Do nothing */
                    break;
                
                case STB_GLOBAL:
                    symbol_record_external_symbol (symbol);
                    break;
                
                default:
                    ld_internal_error_at_source (__FILE__, __LINE__,
                                                 "+++not yet supported symbol ELF64_ST_BIND: %u",
                                                 ELF64_ST_BIND (elf_symbol->st_info));
                    break;
            }

            if (ELF64_ST_TYPE (elf_symbol->st_info) == STT_SECTION) {
                symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
            }
        }
    }

    for (i = 1; i < ehdr.e_shnum; i++) {
        struct section_part *part;
        size_t j;
        
        pos = file + ehdr.e_shoff + i * ehdr.e_shentsize;
        shdr_p = (void *)pos;

        if ((shdr_p->sh_type != SHT_RELA
             && shdr_p->sh_type != SHT_REL)
            || shdr_p->sh_size == 0) continue;

        if (shdr_p->sh_info >= ehdr.e_shnum) {
            ld_fatal_error ("%s: relocation section has invalid sh_info", filename);
        }

        part = part_p_array[shdr_p->sh_info];
        if (!part) {
            ld_fatal_error ("%s: relocation section has invalid sh_info", filename);
        }
        if (part->section->is_bss) {
            ld_fatal_error ("%s: relocation section sh_info points to BSS section",
                            filename);
        }

        if ((shdr_p->sh_type == SHT_RELA
             && shdr_p->sh_entsize != sizeof (Elf64_Rela))
            || (shdr_p->sh_type == SHT_REL
                && shdr_p->sh_entsize != sizeof (Elf64_Rel))) {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "+++relocation shdr_p->sh_entsize not yet supported");
        }

        pos = file + shdr_p->sh_offset;
        CHECK_READ (pos, shdr_p->sh_size);

        part->relocation_count = shdr_p->sh_size / shdr_p->sh_entsize;
        part->relocation_array = xcalloc (part->relocation_count, sizeof *part->relocation_array);
        
        for (j = 0; j < part->relocation_count; j++) {
            if (shdr_p->sh_type == SHT_RELA) {
                translate_relocation64_addend (part->relocation_array + j,
                                               (Elf64_Rela *)(pos + shdr_p->sh_entsize * j),
                                               part);
            } else {
                ld_internal_error_at_source (__FILE__, __LINE__,
                                             "+++ELF64 relocations without addend not yet supported");
            }
        }
    }

    free (part_p_array);

    return 0;
}

int elf_read (unsigned char *file, size_t file_size, const char *filename)
{

    CHECK_READ (file, 4);

    if (file[EI_MAG0] == ELFMAG0
        && file[EI_MAG1] == ELFMAG1
        && file[EI_MAG2] == ELFMAG2
        && file[EI_MAG3] == ELFMAG3) {
        if (read_elf_object (file, file_size, filename)) return INPUT_FILE_ERROR;
        return INPUT_FILE_FINISHED;
    }

    return INPUT_FILE_UNRECOGNIZED;
}
