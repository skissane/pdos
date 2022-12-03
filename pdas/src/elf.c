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
#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>

#include    "as.h"
#include    "elf.h"

#define LAST_PROGRAM_SECTION_HEADER_INDEX (sections_get_count ())
#define SYMTAB_SECTION_HEADER_INDEX (LAST_PROGRAM_SECTION_HEADER_INDEX + 1)
#define STRTAB_SECTION_HEADER_INDEX (LAST_PROGRAM_SECTION_HEADER_INDEX + 2)
#define SHSTRTAB_SECTION_HEADER_INDEX (LAST_PROGRAM_SECTION_HEADER_INDEX \
                                       + 2 + number_of_relocation_sections + 1)
/* Undefined section is located at index 0 and .shstrtab is the last section. */
#define SECTION_HEADERS_TOTAL (1 + SHSTRTAB_SECTION_HEADER_INDEX)

static int write_content_of_sections (FILE *outfile,
                                      Elf32_Word *shstrtab_index_p) {

    section_t section;

    for (section = sections; section; section = section_get_next_section (section)) {
    
        struct Elf32_Shdr *section_header = xmalloc (sizeof (*section_header));
        section_set_object_format_dependent_data (section, section_header);

        section_header->sh_name = *shstrtab_index_p;
        *shstrtab_index_p += strlen (section_get_name (section)) + 1;

        section_header->sh_addr = 0;
        section_header->sh_offset = ftell (outfile);

        section_header->sh_link = SHN_UNDEF;
        section_header->sh_info = 0;
        section_header->sh_addralign = 0;
        section_header->sh_entsize = 0;
        
        if (section == text_section) {
            section_header->sh_flags = (SHF_ALLOC | SHF_EXECINSTR);
        } else if (section == data_section) {
            section_header->sh_flags = (SHF_ALLOC | SHF_WRITE);
        } else if (section == bss_section) {
            section_header->sh_flags = (SHF_ALLOC | SHF_WRITE);
        } else if (strcmp (section_get_name (section), ".comment") == 0) {
            section_header->sh_flags = 0;
        } else {
            section_header->sh_flags = (SHF_ALLOC | SHF_WRITE);
        }
        
        if (section != bss_section) {
        
            struct frag *frag;
            section_set (section);

            section_header->sh_type = SHT_PROGBITS;
            
            section_header->sh_size = 0;
            
            for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
            
                if (frag->fixed_size == 0) {
                    continue;
                }
                
                if (fwrite (frag->buf, frag->fixed_size, 1, outfile) != 1) {
                
                    as_error_at (NULL, 0, "Failed whilst writing secton '%s'!", section_get_name (section));
                    return 1;
                
                }
                
                section_header->sh_size += frag->fixed_size;
            
            }
        
        } else {
        
            struct frag *frag;
            
            section_set (section);

            section_header->sh_type = SHT_NOBITS;
            section_header->sh_size = 0;
            
            for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
            
                if (frag->fixed_size == 0) {
                    continue;
                }
                
                section_header->sh_size += frag->fixed_size;
            
            }
        
        }
    
    }

    return 0;

}

static int write_symtab (FILE *outfile,
                         Elf32_Word *shstrtab_index_p,
                         struct Elf32_Shdr *symtab_section_header_p,
                         Elf32_Word *strtab_index_p) {

    Elf32_Word number_of_symbols = 1;
    struct Elf32_Sym symbol_entry;
    struct symbol *symbol;

    symtab_section_header_p->sh_name = *shstrtab_index_p;
    *shstrtab_index_p += strlen (".symtab") + 1;

    symtab_section_header_p->sh_type = SHT_SYMTAB;
    symtab_section_header_p->sh_flags = 0;
    symtab_section_header_p->sh_addr = 0;
    symtab_section_header_p->sh_link = STRTAB_SECTION_HEADER_INDEX;
    symtab_section_header_p->sh_addralign = 0;
    symtab_section_header_p->sh_entsize = sizeof (struct Elf32_Sym);

    symtab_section_header_p->sh_offset = ftell (outfile);

    /* An empty symbol must be written at index 0 of symbol table. */
    memset (&symbol_entry, 0, sizeof (symbol_entry));
    if (fwrite (&symbol_entry, sizeof (symbol_entry), 1, outfile) != 1) {
    
        as_error_at (NULL, 0, "Failed to write symbol table!");
        return 1;
    
    }

    /* Local symbols MUST be written before global symbols accroding to ELF specification. */
    for (symbol = symbols; symbol; symbol = symbol->next) {

        if (symbol_is_external (symbol) || symbol_is_undefined (symbol)) {
            continue;
        }

        symbol_entry.st_name = *strtab_index_p;
        *strtab_index_p += strlen (symbol->name) + 1;
        
        symbol_entry.st_value = symbol_get_value (symbol);
        symbol_entry.st_size = 0;
        symbol_entry.st_other = 0;

        if (symbol_is_section_symbol (symbol)) {
            symbol_entry.st_info = ELF32_ST_INFO (STB_LOCAL, STT_SECTION);
        } else {
            symbol_entry.st_info = ELF32_ST_INFO (STB_LOCAL, STT_NOTYPE);
        }

        if (symbol->section == absolute_section) {
            symbol_entry.st_shndx = SHN_ABS;
        } else {
            symbol_entry.st_shndx = section_get_number (symbol->section);
        }

        symbol_set_symbol_table_index (symbol, number_of_symbols);
        number_of_symbols++;

        if (fwrite (&symbol_entry, sizeof (symbol_entry), 1, outfile) != 1) {
        
            as_error_at (NULL, 0, "Failed to write local symbol!");
            return 1;
        
        }

    }

    /* sh_info for .symtab should be the index of the last local symbol + 1. */
    symtab_section_header_p->sh_info = number_of_symbols;

    for (symbol = symbols; symbol; symbol = symbol->next) {

        if (!symbol_is_external (symbol) && !symbol_is_undefined (symbol)) {
            continue;
        }

        symbol_entry.st_name = *strtab_index_p;
        *strtab_index_p += strlen (symbol->name) + 1;
        
        symbol_entry.st_value = symbol_get_value (symbol);
        symbol_entry.st_size = 0;
        symbol_entry.st_other = 0;

        if (symbol_is_section_symbol (symbol)) {
            symbol_entry.st_info = ELF32_ST_INFO (STB_GLOBAL, STT_SECTION);
        } else {
            symbol_entry.st_info = ELF32_ST_INFO (STB_GLOBAL, STT_NOTYPE);
        }

        if (symbol->section == absolute_section) {
            symbol_entry.st_shndx = SHN_ABS;
        } else if (symbol->section == undefined_section) {
            symbol_entry.st_shndx = SHN_UNDEF;
        } else {
            symbol_entry.st_shndx = section_get_number (symbol->section);
        }

        symbol_set_symbol_table_index (symbol, number_of_symbols);
        number_of_symbols++;

        if (fwrite (&symbol_entry, sizeof (symbol_entry), 1, outfile) != 1) {
        
            as_error_at (NULL, 0, "Failed to write global symbol!");
            return 1;
        
        }

    }

    symtab_section_header_p->sh_size = number_of_symbols * symtab_section_header_p->sh_entsize;

    return 0;

}

static int write_strtab (FILE *outfile,
                         Elf32_Word *shstrtab_index_p,
                         struct Elf32_Shdr *strtab_section_header_p,
                         Elf32_Word strtab_index) {
    
    char *strtab;
    struct symbol *symbol;

    memset (strtab_section_header_p, 0, sizeof (*strtab_section_header_p));
    strtab_section_header_p->sh_name = *shstrtab_index_p;
    *shstrtab_index_p += strlen (".strtab") + 1;

    strtab_section_header_p->sh_type = SHT_STRTAB;
    strtab_section_header_p->sh_offset = ftell (outfile);
    strtab_section_header_p->sh_size = strtab_index;

    strtab = xmalloc (strtab_section_header_p->sh_size);
    strtab[0] = '\0';
    strtab_index = 1;

    for (symbol = symbols; symbol; symbol = symbol->next) {

        if (symbol_is_external (symbol) || symbol_is_undefined (symbol)) {
            continue;
        }

        strcpy (strtab + strtab_index, symbol->name);
        strtab_index += strlen (symbol->name) + 1;

    }

    for (symbol = symbols; symbol; symbol = symbol->next) {

        if (!symbol_is_external (symbol) && !symbol_is_undefined (symbol)) {
            continue;
        }

        strcpy (strtab + strtab_index, symbol->name);
        strtab_index += strlen (symbol->name) + 1;

    }

    if (fwrite (strtab, strtab_section_header_p->sh_size, 1, outfile) != 1) {
    
        as_error_at (NULL, 0, "Failed to write string table!");
        return 1;
    
    }

    free (strtab);

    return 0;

}

static int write_relocation_sections (FILE *outfile,
                                      Elf32_Word *shstrtab_index_p,
                                      struct Elf32_Shdr *relocation_section_headers,
                                      Elf32_Half *number_of_relocation_sections_p) {

    section_t section;

    for (section = sections; section; section = section_get_next_section (section)) {
    
        struct Elf32_Shdr *rel_header;
        struct fixup *fixup;
        
        rel_header = relocation_section_headers + *number_of_relocation_sections_p;
        rel_header->sh_offset = ftell (outfile);
        rel_header->sh_size = 0;
        
        section_set (section);
        
        for (fixup = current_frag_chain->first_fixup; fixup; fixup = fixup->next) {

            struct Elf32_Rel relocation_entry;
            unsigned long symbol_table_index;
        
            if (fixup->done) {
                continue;
            }

            if (fixup->size != 4) {
                as_fatal_error_at (NULL, 0,
                                   "unsupported relocation size %u bytes"
                                   " (only relocations with size 4 bytes are supported by ELF32)",
                                   fixup->size);
            }

            relocation_entry.r_offset = fixup->frag->address + fixup->where;

            symbol_table_index = symbol_get_symbol_table_index (fixup->add_symbol);
            switch (fixup->reloc_type) {
            
                case RELOC_TYPE_DEFAULT:

                    if (fixup->pcrel) {
                        relocation_entry.r_info = ELF32_R_INFO (symbol_table_index, R_386_PC32);
                    } else {
                        relocation_entry.r_info = ELF32_R_INFO (symbol_table_index, R_386_32);
                    }
                    
                    break;
                
                case RELOC_TYPE_RVA:
                
                    relocation_entry.r_info = ELF32_R_INFO (symbol_table_index, R_386_RELATIVE);
                    break;
            
            }
            
            if (fwrite (&relocation_entry, sizeof (relocation_entry), 1, outfile) != 1) {
            
                as_error_at (NULL, 0, "Failed to write relocation!");
                return 1;
            
            }
            
            rel_header->sh_size += sizeof (relocation_entry);
        
        }

        if (rel_header->sh_size == 0) {
            continue;
        }

        rel_header->sh_name = *shstrtab_index_p;
        *shstrtab_index_p += strlen (".rel") + strlen (section_get_name (section)) + 1;

        rel_header->sh_type = SHT_REL;
        rel_header->sh_flags = 0;
        rel_header->sh_addr = 0;
        rel_header->sh_link = SYMTAB_SECTION_HEADER_INDEX;
        rel_header->sh_info = section_get_number (section);
        rel_header->sh_addralign = 0;
        rel_header->sh_entsize = sizeof (struct Elf32_Rel);

        *number_of_relocation_sections_p += 1;
    
    }

    return 0;

}

static int write_shstrtab (FILE *outfile,
                           struct Elf32_Shdr *shstrtab_section_header_p,
                           Elf32_Word shstrtab_index,
                           struct Elf32_Shdr *relocation_section_headers,
                           Elf32_Half number_of_relocation_sections) {

    char *shstrtab;
    section_t section;

    memset (shstrtab_section_header_p, 0, sizeof (*shstrtab_section_header_p));
    shstrtab_section_header_p->sh_name = shstrtab_index;
    shstrtab_section_header_p->sh_size = shstrtab_index + strlen (".shstrtab") + 1;

    shstrtab_section_header_p->sh_type = SHT_STRTAB;
    shstrtab_section_header_p->sh_offset = ftell (outfile);

    shstrtab = xmalloc (shstrtab_section_header_p->sh_size);
    shstrtab[0] = '\0';
    shstrtab_index = 1;

    for (section = sections; section; section = section_get_next_section (section)) {

        strcpy (shstrtab + shstrtab_index, section_get_name (section));
        shstrtab_index += strlen (section_get_name (section)) + 1;

    }

    strcpy (shstrtab + shstrtab_index, ".symtab");
    shstrtab_index += strlen (".symtab") + 1;
    strcpy (shstrtab + shstrtab_index, ".strtab");
    shstrtab_index += strlen (".strtab") + 1;

    for ( ; number_of_relocation_sections--; relocation_section_headers++) {

        const char *section_name;

        section_name = section_get_name (section_find_by_number (relocation_section_headers->sh_info));

        strcpy (shstrtab + shstrtab_index, ".rel");
        shstrtab_index += strlen (".rel");
        strcpy (shstrtab + shstrtab_index, section_name);
        shstrtab_index += strlen (section_name) + 1;

    }
    
    strcpy (shstrtab + shstrtab_index, ".shstrtab");

    if (fwrite (shstrtab, shstrtab_section_header_p->sh_size, 1, outfile) != 1) {
    
        as_error_at (NULL, 0, "Failed to write section header string table!");
        return 1;
    
    }

    free (shstrtab);

    return 0;

}

static int write_section_headers (FILE *outfile,
                                  struct Elf32_Shdr *symtab_section_header_p,
                                  struct Elf32_Shdr *strtab_section_header_p,
                                  struct Elf32_Shdr *shstrtab_section_header_p,
                                  struct Elf32_Shdr *relocation_section_headers,
                                  Elf32_Half number_of_relocation_sections) {

    struct Elf32_Shdr section_header_0;
    section_t section;

    /* Undefined section has a real section header at index 0. */
    memset (&section_header_0, 0, sizeof (section_header_0));
    if (fwrite (&section_header_0, sizeof (section_header_0), 1, outfile) != 1) {
    
        as_error_at (NULL, 0, "Failed to write section header!");
        return 1;
    
    }

    for (section = sections; section; section = section_get_next_section (section)) {
    
        struct Elf32_Shdr *section_header = section_get_object_format_dependent_data (section);
        
        if (fwrite (section_header, sizeof (*section_header), 1, outfile) != 1) {
        
            as_error_at (NULL, 0, "Failed to write section header!");
            return 1;
        
        }

        free (section_header);
    
    }

    if (fwrite (symtab_section_header_p, sizeof (*symtab_section_header_p), 1, outfile) != 1) {
    
        as_error_at (NULL, 0, "Failed to write section header!");
        return 1;
    
    }

    if (fwrite (strtab_section_header_p, sizeof (*strtab_section_header_p), 1, outfile) != 1) {
    
        as_error_at (NULL, 0, "Failed to write section header!");
        return 1;
    
    }

    if (number_of_relocation_sections) {
        
        if (fwrite (relocation_section_headers,
                    sizeof (*relocation_section_headers) * number_of_relocation_sections,
                    1, outfile) != 1) {
        
            as_error_at (NULL, 0, "Failed to write section header!");
            return 1;
        
        }
        
    }

    if (fwrite (shstrtab_section_header_p, sizeof (*shstrtab_section_header_p), 1, outfile) != 1) {
    
        as_error_at (NULL, 0, "Failed to write section header!");
        return 1;
    
    }

    return 0;

}

void write_elf_file (void) {

    FILE *outfile;
    struct Elf32_Ehdr header;
    Elf32_Off section_header_table_offset;
    struct Elf32_Shdr shstrtab_section_header, symtab_section_header, strtab_section_header;
    Elf32_Half number_of_relocation_sections = 0;
    struct Elf32_Shdr *relocation_section_headers;

    Elf32_Word shstrtab_index = 1;
    Elf32_Word strtab_index = 1;
    
    sections_number (1);

    relocation_section_headers = xmalloc (sizeof (*relocation_section_headers) * sections_get_count ());

    if ((outfile = fopen (state->outfile, "wb")) == NULL) {
    
        as_error_at (NULL, 0, "Failed to open '%s' as output file", state->outfile);
        return;
    
    }

    if (fseek (outfile, sizeof (header), SEEK_SET)) {
    
        as_error_at (NULL, 0, "Failed to fseek");
        return;
    
    }

    if (write_content_of_sections (outfile, &shstrtab_index)) {
        return;
    }

    if (write_symtab (outfile, &shstrtab_index, &symtab_section_header, &strtab_index)) {
        return;
    }

    if (write_strtab (outfile, &shstrtab_index, &strtab_section_header, strtab_index)) {
        return;
    }

    if (write_relocation_sections(outfile,
                                  &shstrtab_index,
                                  relocation_section_headers,
                                  &number_of_relocation_sections)) {
        return;
    }

    if (write_shstrtab (outfile,
                        &shstrtab_section_header,
                        shstrtab_index,
                        relocation_section_headers,
                        number_of_relocation_sections)) {
        return;
    }

    section_header_table_offset = ftell (outfile);

    if (write_section_headers (outfile,
                               &symtab_section_header,
                               &strtab_section_header,
                               &shstrtab_section_header,
                               relocation_section_headers,
                               number_of_relocation_sections)) {
        return;
    }

    free (relocation_section_headers);

    /* ELF header. */

    rewind (outfile);

    header.e_ident[EI_MAG0] = ELFMAG0;
    header.e_ident[EI_MAG1] = ELFMAG1;
    header.e_ident[EI_MAG2] = ELFMAG2;
    header.e_ident[EI_MAG3] = ELFMAG3;

    header.e_ident[EI_CLASS] = ELFCLASS32;
    header.e_ident[EI_DATA] = ELFDATA2LSB;
    header.e_ident[EI_VERSION] = EV_CURRENT;
    header.e_ident[EI_OSABI] = ELFOSABI_NONE;
    header.e_ident[EI_ABIVERSION] = 0;
    memset (&header.e_ident[EI_PAD], '\0', EI_NIDENT - EI_PAD);

    header.e_type = ET_REL;
    header.e_machine = EM_386;
    header.e_entry = 0;
    header.e_phoff = 0;
    header.e_shoff = section_header_table_offset;
    header.e_flags = 0;
    header.e_ehsize = sizeof (header);
    header.e_phentsize = 0;
    header.e_phnum = 0;
    header.e_shentsize = sizeof (struct Elf32_Shdr);
    header.e_shnum = SECTION_HEADERS_TOTAL;
    header.e_shstrndx = SHSTRTAB_SECTION_HEADER_INDEX;

    if (fwrite (&header, sizeof (header), 1, outfile) != 1) {
    
        as_error_at (NULL, 0, "Failed to write header!");
        return;
    
    }

    if (fclose (outfile)) {
        as_error_at (NULL, 0, "Failed to close file!");
    }

}

static void handler_bss (char **pp) {

    section_subsection_set (bss_section, (subsection_t) get_result_of_absolute_expression (pp));
    demand_empty_rest_of_line (pp);

}

static void handler_ident (char **pp) {

    section_t saved_section = current_section;
    subsection_t saved_subsection = current_subsection;

    section_set_by_name (".comment");

    handler_asciz (pp);

    section_subsection_set (saved_section, saved_subsection);

}

static struct pseudo_op_entry pseudo_op_table[] = {

    { "bss",        &handler_bss        },
    { "ident",      &handler_ident      },
    { 0,            0                   }

};

struct pseudo_op_entry *elf_get_pseudo_op_table (void) {
    return pseudo_op_table;
}
