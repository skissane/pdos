/******************************************************************************
 * @file            macho.c
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
#include "xmalloc.h"

#include "macho_bytearray.h"

void macho_write (const char *filename)
{
    ld_internal_error_at_source (__FILE__, __LINE__,
                                 "+++Mach-O output not supported");
}

static void translate_relocation (struct reloc_entry *reloc,
                                  const struct relocation_info_internal *input_reloc,
                                  struct section_part *part)
{
    if ((input_reloc->r_symbolnum >> 27) & 1) {
        reloc->symbol = part->of->symbol_array + (input_reloc->r_symbolnum & 0xffffff);
    } else {
        if (input_reloc->r_symbolnum == 0) {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "local input_reloc->r_symbolnum %#lx is not yet supported",
                                         input_reloc->r_symbolnum & 0xffffff);
        }
        reloc->symbol = part->of->symbol_array + part->of->symbol_count - (input_reloc->r_symbolnum & 0xffffff);
    }

    reloc->offset = input_reloc->r_address;            

    ld_warn ("%s: ignoring not yet supported relocation with size %u, pcrel %u and type %#x",
             part->of->filename,
             1U << ((input_reloc->r_symbolnum >> 25) & 3),
             (input_reloc->r_symbolnum >> 24) & 1,
             input_reloc->r_symbolnum >> 28);        
    reloc->howto = &reloc_howtos[RELOC_TYPE_IGNORED];
}

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("%s: corrupted input file", filename); } while (0)

static int read_macho_object (unsigned char *file, size_t file_size, const char *filename)
{
    struct mach_header_64_internal hdr64;
    unsigned long i;

    unsigned char *pos;

    struct object_file *of;
    struct section_part **part_p_array;
    size_t num_symbols = 0;
    size_t num_sections = 0;

    pos = file;
    CHECK_READ (pos, SIZEOF_struct_mach_header_64_file);
    read_struct_mach_header_64 (&hdr64, pos);
    pos += SIZEOF_struct_mach_header_64_file;

    switch (hdr64.cputype) {
        case CPU_TYPE_x86_64:
            if (ld_state->target_machine == LD_TARGET_MACHINE_X64
                || ld_state->target_machine == LD_TARGET_MACHINE_UNKNOWN) {
                ld_state->target_machine = LD_TARGET_MACHINE_X64;
            } else {
                ld_error ("%s: cputype is not CPU_TYPE_x86_64", filename);
                return 1;
            }
            break;
        
        case CPU_TYPE_ARM64:
            if (ld_state->target_machine == LD_TARGET_MACHINE_AARCH64
                || ld_state->target_machine == LD_TARGET_MACHINE_UNKNOWN) {
                ld_state->target_machine = LD_TARGET_MACHINE_AARCH64;
            } else {
                ld_error ("%s: cputype is not CPU_TYPE_ARM64", filename);
                return 1;
            }
            break;

        default:
            ld_error ("%s: unrecognized cputype %#lx", filename, hdr64.cputype);
            return 1;
    }

    if (hdr64.filetype != MH_OBJECT) {
        ld_error ("%s: filetype is not MH_OBJECT", filename);
        return 1;
    }

    CHECK_READ (pos, hdr64.sizeofcmds);
    of = NULL;
    for (i = 0; i < hdr64.ncmds; i++) {
        struct load_command_internal load_cmd;

        if (pos - file + SIZEOF_struct_load_command_file
            > SIZEOF_struct_mach_header_64_file + hdr64.sizeofcmds) {
            ld_error ("%s: invalid header sizeofcmds/ncmds", filename);
            return 1;
        }

        read_struct_load_command (&load_cmd, pos);
        if (pos - file + load_cmd.cmdsize
            > SIZEOF_struct_mach_header_64_file + hdr64.sizeofcmds) {
            ld_error ("%s: invalid load command cmdsize", filename);
            return 1;
        }
        
        if (load_cmd.cmd == LC_SYMTAB) {
            struct symtab_command_internal symtab_cmd;

            read_struct_symtab_command (&symtab_cmd, pos);
            if (SIZEOF_struct_symtab_command_file > symtab_cmd.cmdsize) {
                ld_fatal_error ("%s: symtab command cmdsize too small", filename);
            }
            
            if (num_symbols && symtab_cmd.nsyms) {
                ld_fatal_error ("%s: more than 1 non-empty symbol table per object file", filename);
            }
            num_symbols = symtab_cmd.nsyms;
        } else if (load_cmd.cmd == LC_SEGMENT_64) {
            struct segment_command_64_internal segment_cmd;

            read_struct_segment_command_64 (&segment_cmd, pos);
            if (num_sections && segment_cmd.nsects) {
                ld_fatal_error ("%s: more than 1 non-empty segment command per object file", filename);
            }
            num_sections = segment_cmd.nsects;
        }

        pos += load_cmd.cmdsize;
    }

    of = object_file_make (num_symbols + num_sections, filename);

    part_p_array = NULL;
    pos = file + SIZEOF_struct_mach_header_64_file;
    for (i = 0; i < hdr64.ncmds; i++) {
        struct load_command_internal load_cmd;

        read_struct_load_command (&load_cmd, pos);
        
        if (load_cmd.cmd == LC_SEGMENT_64) {
            struct segment_command_64_internal segment_cmd;
            unsigned long j;

            read_struct_segment_command_64 (&segment_cmd, pos);
            if (!segment_cmd.nsects) {
                pos += load_cmd.cmdsize;
                continue;
            }
            
            part_p_array = xcalloc (segment_cmd.nsects + 1, sizeof *part_p_array);

            if (SIZEOF_struct_segment_command_64_file
                + segment_cmd.nsects * SIZEOF_struct_section_64_file
                > segment_cmd.cmdsize) {
                ld_fatal_error ("%s: segment command cmdsize and nsects mismatch", filename);
            }

            for (j = 0; j < segment_cmd.nsects; j++) {
                struct section_64_internal sect_64;
                char *section_name;
                struct section *section;

                read_struct_section_64 (&sect_64,
                                        pos + SIZEOF_struct_segment_command_64_file
                                        + j * SIZEOF_struct_section_64_file);
                section_name = xstrndup (sect_64.sectname, sizeof (sect_64.sectname));
                section = section_find_or_make (section_name);
                free (section_name);

                if (1LU << sect_64.align > section->section_alignment) {
                    section->section_alignment = 1LU << sect_64.align;
                }

                /* For now assume everything has same flags as .text. */
                section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_READONLY | SECTION_FLAG_CODE;

                if ((sect_64.flags & 0xff) == S_ZEROFILL) {
                    section->is_bss = 1;
                }

                {
                    struct section_part *part = section_part_new (section, of);

                    part->alignment = 1LU << sect_64.align;

                    part->content_size = sect_64.size;
                    if (!section->is_bss) {
                        const unsigned char *content_pos = file + sect_64.offset;
                        part->content = xmalloc (part->content_size);

                        CHECK_READ (content_pos, part->content_size);
                        memcpy (part->content, content_pos, part->content_size);
                    }
                    
                    section_append_section_part (section, part);

                    part_p_array[j + 1] = part;

                    if (sect_64.nreloc) {
                        size_t k;
                        const unsigned char *rel_pos = file + sect_64.reloff;

                        CHECK_READ (rel_pos, sect_64.nreloc * SIZEOF_struct_relocation_info_file);

                        part->relocation_count = sect_64.nreloc;
                        part->relocation_array = xcalloc (part->relocation_count, sizeof *part->relocation_array);

                        for (k = 0; k < sect_64.nreloc; k++) {
                            struct relocation_info_internal relocation_info;
                            read_struct_relocation_info (&relocation_info,
                                                         rel_pos + SIZEOF_struct_relocation_info_file * k);
                            translate_relocation (part->relocation_array + k, &relocation_info, part);
                        }
                    }
                }
            }
        }

        pos += load_cmd.cmdsize;
    }

    pos = file + SIZEOF_struct_mach_header_64_file;
    for (i = 0; i < hdr64.ncmds; i++) {
        struct load_command_internal load_cmd;

        read_struct_load_command (&load_cmd, pos);
        
        if (load_cmd.cmd == LC_SYMTAB) {
            struct symtab_command_internal symtab_cmd;
            const char *string_table;
            const unsigned char *sym_pos;
            unsigned long j;

            read_struct_symtab_command (&symtab_cmd, pos);
            if (!symtab_cmd.nsyms) {
                pos += load_cmd.cmdsize;
                continue;
            }
            
            string_table = file + symtab_cmd.stroff;
            CHECK_READ (file + symtab_cmd.stroff, symtab_cmd.strsize);

            sym_pos = file + symtab_cmd.symoff;
            CHECK_READ (sym_pos, SIZEOF_struct_nlist_64_file * symtab_cmd.nsyms);

            for (j = 0; j < symtab_cmd.nsyms; j++) {
                struct nlist_64_internal nlist_64;
                struct symbol *symbol = of->symbol_array + j;

                read_struct_nlist_64 (&nlist_64, sym_pos + j * SIZEOF_struct_nlist_64_file);

                if (nlist_64.n_strx < symtab_cmd.strsize) {
                    if (string_table[nlist_64.n_strx] == '\0') {
                        symbol->name = xstrdup (UNNAMED_SYMBOL_NAME);
                    } else {
                        symbol->name = xstrdup (string_table + nlist_64.n_strx);
                    }
                } else ld_fatal_error ("%s: invalid index into string table", filename);

                symbol->value = nlist_64.n_value;

                if ((nlist_64.n_type & N_STAB)
                    || (nlist_64.n_type & N_PEXT)) {
                    ld_internal_error_at_source (__FILE__, __LINE__,
                                                 "+++%s: not yet supported symbol n_type: %#x",
                                                 filename, nlist_64.n_type);
                }

                if ((nlist_64.n_type & N_TYPE) == N_UNDF) {
                    symbol->section_number = UNDEFINED_SECTION_NUMBER;
                    symbol->part = NULL;
                } else if ((nlist_64.n_type & N_TYPE) == N_SECT) {
                    if (nlist_64.n_sect > num_sections) {
                        ld_error ("%s: invalid symbol n_sect: %u", filename, nlist_64.n_sect);
                        symbol->part = NULL;
                    }
                    symbol->section_number = nlist_64.n_sect;
                    symbol->part = part_p_array[nlist_64.n_sect];
                } else {
                    ld_internal_error_at_source (__FILE__, __LINE__,
                                                 "+++%s: not yet supported symbol n_type: %#x",
                                                 filename, nlist_64.n_type);
                }

                if (nlist_64.n_type & N_EXT) {
                    symbol_record_external_symbol (symbol);
                }
            }

            break;
        }

        pos += load_cmd.cmdsize;
    }

    /* Mach-O (just like a.out) relocations use implicit section symbols
     * which do not exist in the symbol table,
     * so section symbols need to be automatically generated by the linker.
     */
    for (i = 1; i < num_sections + 1; i++) {
        struct section_part *part;
        struct symbol *symbol = of->symbol_array + of->symbol_count - i;

        part = part_p_array[i];
        symbol->name = xstrdup (part->section->name);
        symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
        symbol->value = 0;
        symbol->size = 0;
        symbol->part = part;
        symbol->section_number = i;
    }

    free (part_p_array);
    
    return 0;
}

int macho_read (unsigned char *file, size_t file_size, const char *filename)
{
    unsigned long magic;
    
    CHECK_READ (file, 4);
    bytearray_read_4_bytes (&magic, file, LITTLE_ENDIAN);

    if (magic == MH_MAGIC_64) {
        if (read_macho_object (file, file_size, filename)) return INPUT_FILE_ERROR;
        return INPUT_FILE_FINISHED;
    }

    return INPUT_FILE_UNRECOGNIZED;
}
