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
#include <string.h>
#include <stdlib.h>

#include "as.h"
#include "elf.h"

#define RELOC_SECTION_ALIGNMENT 4
#define SYMTAB_SECTION_ALIGNMENT 4

#define ALIGN(to_align, alignment) \
 (((to_align) / (alignment) + (((to_align) % (alignment)) ? 1 : 0)) * (alignment))

static size_t section_get_num_relocs (struct section *section)
{
    struct fixup *fixup;
    size_t num_relocs = 0;
                    
    section_set (section);
      
    for (fixup = current_frag_chain->first_fixup;
         fixup;
         fixup = fixup->next) {
        if (fixup->done) continue;

        num_relocs++;
    }

    return num_relocs;
}

static unsigned char *write_relocs_for_section (unsigned char *file,
                                                unsigned char *pos,
                                                struct section *section,
                                                Elf32_Shdr *shdr_p,
                                                Elf32_Word symtab_index)
{
    struct fixup *fixup;
    Elf32_Rel *rel;
    unsigned char *saved_pos = pos;

    pos = file + ALIGN (pos - file, RELOC_SECTION_ALIGNMENT);
    rel = (void *)pos;

    section_set (section);
    
    for (fixup = current_frag_chain->first_fixup;
         fixup;
         fixup = fixup->next) {
        unsigned long symbol_table_index;
        unsigned char type = 0;
        
        if (fixup->done) continue;

        if (fixup->size != 4) {
            as_fatal_error_at (NULL, 0,
                               "unsupported relocation size %u bytes"
                               " (only relocations with size 4 bytes are supported by ELF32)",
                               fixup->size);
        }

        symbol_table_index = symbol_get_symbol_table_index (fixup->add_symbol);
        
        switch (fixup->reloc_type) {
            case RELOC_TYPE_DEFAULT:
                if (fixup->pcrel) {
                    type = R_386_PC32;
                } else {
                    type = R_386_32;
                }
                break;
            
            case RELOC_TYPE_RVA:
                type = R_386_RELATIVE;
                break;
        }

        rel->r_offset = fixup->frag->address + fixup->where;
        rel->r_info = ELF32_R_INFO (symbol_table_index, type);
        rel++;
    }

    if (rel == (void *)pos) return saved_pos;

    shdr_p->sh_type = SHT_REL;
    shdr_p->sh_flags = 0;
    shdr_p->sh_addr = 0;
    shdr_p->sh_offset = pos - file;
    shdr_p->sh_size = (unsigned char *)rel - pos;
    shdr_p->sh_link = symtab_index;
    shdr_p->sh_info = section_get_number (section);
    shdr_p->sh_addralign = RELOC_SECTION_ALIGNMENT;
    shdr_p->sh_entsize = sizeof *rel;
    
    return (unsigned char *)rel;
}

static unsigned char *write_sections (unsigned char *file, const Elf32_Ehdr *ehdr_p)
{
    unsigned char *pos;
    struct section *section;
    Elf32_Shdr *shdr_p;

    pos = file + sizeof (*ehdr_p);
    
    shdr_p = (void *)(file + ehdr_p->e_shoff);
    shdr_p->sh_type = SHT_NULL;
    shdr_p->sh_link = SHN_UNDEF;
    shdr_p++;

    for (section = sections; section; section = section_get_next_section (section)) {
        shdr_p->sh_type = (section == bss_section) ? SHT_NOBITS : SHT_PROGBITS;
        shdr_p->sh_addr = 0;
        shdr_p->sh_addralign = 1 << section_get_alignment_power (section);

        pos = file + ALIGN (pos - file, shdr_p->sh_addralign);
        shdr_p->sh_offset = pos - file;

        if (section != bss_section) {
            struct frag *frag;
            
            section_set (section);

            for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
                if (frag->fixed_size == 0) continue;
                
                memcpy (pos, frag->buf, frag->fixed_size);
                pos += frag->fixed_size;
            }

            shdr_p->sh_size = pos - file - shdr_p->sh_offset;
        } else {
            struct frag *frag;
            
            section_set (section);

            shdr_p->sh_size = 0;
            
            for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
                if (frag->fixed_size == 0) continue;
                
                shdr_p->sh_size += frag->fixed_size;
            }
        }

        if (section == text_section) {
            shdr_p->sh_flags = (SHF_ALLOC | SHF_EXECINSTR);
        } else if (section == data_section) {
            shdr_p->sh_flags = (SHF_ALLOC | SHF_WRITE);
        } else if (section == bss_section) {
            shdr_p->sh_flags = (SHF_ALLOC | SHF_WRITE);
        } else if (strcmp (section_get_name (section), ".comment") == 0) {
            shdr_p->sh_flags = 0;
        } else {
            shdr_p->sh_flags = (SHF_ALLOC | SHF_WRITE);
        }

        {
            Elf32_Shdr *custom_shdr_p = section_get_object_format_dependent_data (section);

            if (custom_shdr_p) {
                if (custom_shdr_p->sh_type) {
                    shdr_p->sh_type = custom_shdr_p->sh_type;
                }
                if (custom_shdr_p->sh_flags) {
                    shdr_p->sh_flags = custom_shdr_p->sh_flags;
                }
                if (custom_shdr_p->sh_entsize) {
                    shdr_p->sh_entsize = custom_shdr_p->sh_entsize;
                }
                free (custom_shdr_p);
                section_set_object_format_dependent_data (section, NULL);
            }
        }
        shdr_p++;

        {
            unsigned char *saved_pos = pos;

            pos = write_relocs_for_section (file,
                                            pos,
                                            section,
                                            shdr_p,
                                            ehdr_p->e_shstrndx + 1);
            if (pos != saved_pos) {
                shdr_p++;
            }
        }
    }

    return pos;
}

static void write_symbol (Elf32_Sym *sym, struct symbol *symbol)
{
    unsigned char bind, type;
    Elf32_Sym *custom_sym;
    
    sym->st_value = symbol_get_value (symbol);
    sym->st_size = 0;
    sym->st_other = 0;

    if (symbol_is_external (symbol)
        || symbol_is_undefined (symbol)) {
        bind = STB_GLOBAL;
    } else {
        bind = STB_LOCAL;
    }

    if (symbol_is_section_symbol (symbol)) {
        type = STT_SECTION;
    } else {
        type = STT_NOTYPE;
    }

    if (symbol->section == absolute_section) {
        sym->st_shndx = SHN_ABS;
    } else if (symbol->section == undefined_section && sym->st_value) {
        sym->st_shndx = SHN_COMMON;
        sym->st_size = sym->st_value;
        sym->st_value = 0;
    } else if (symbol->section == undefined_section) {
        sym->st_shndx = SHN_UNDEF;
    } else {
        sym->st_shndx = section_get_number (symbol->section);
    }

    if ((custom_sym = symbol->object_format_dependent_data)) {
        if (custom_sym->st_value) {
            sym->st_value = custom_sym->st_value;
        }
        free (custom_sym);
        symbol->object_format_dependent_data = NULL;
    }

    sym->st_info = ELF32_ST_INFO (bind, type);
}

static unsigned char *write_symtab (unsigned char *file,
                                    const Elf32_Ehdr *ehdr_p,
                                    unsigned char *pos)
{

    Elf32_Shdr *shdr_p;
    
    shdr_p = (void *)(file + ehdr_p->e_shoff);
    shdr_p += ehdr_p->e_shstrndx + 1;

    {
        Elf32_Sym *sym;
        struct symbol *symbol;
        size_t num_local_symbols;
        size_t strtab_index = 1;

        pos = file + ALIGN (pos - file, SYMTAB_SECTION_ALIGNMENT);
        sym = (void *)pos;

        /* Empty symbol. */
        sym++;
        
        /* Local symbols must precede global symbols. */
        for (symbol = symbols; symbol; symbol = symbol->next) {
            if (symbol_is_external (symbol)
                || symbol_is_undefined (symbol)) {
                continue;
            }

            sym->st_name = strtab_index;
            strtab_index += strlen (symbol->name) + 1;
            write_symbol (sym, symbol);
            sym++;
        }

        num_local_symbols = sym - (Elf32_Sym *)pos;

        for (symbol = symbols; symbol; symbol = symbol->next) {
            if (!(symbol_is_external (symbol)
                  || symbol_is_undefined (symbol))) {
                continue;
            }

            sym->st_name = strtab_index;
            strtab_index += strlen (symbol->name) + 1;
            write_symbol (sym, symbol);
            sym++;
        }

        shdr_p->sh_type = SHT_SYMTAB;
        shdr_p->sh_flags = 0;
        shdr_p->sh_addr = 0;
        shdr_p->sh_offset = pos - file;
        shdr_p->sh_size = (unsigned char *)sym - pos;
        shdr_p->sh_link = ehdr_p->e_shstrndx + 2;
        shdr_p->sh_info = num_local_symbols;;
        shdr_p->sh_addralign = SYMTAB_SECTION_ALIGNMENT;
        shdr_p->sh_entsize = sizeof *sym;
        shdr_p++;

        pos = (void *)sym;
    }

    {
        struct symbol *symbol;
        char *str_pos = (void *)pos;

        str_pos[0] = '\0';
        str_pos++;
        
        for (symbol = symbols; symbol; symbol = symbol->next) {
            size_t name_len;
            
            if (symbol_is_external (symbol)
                || symbol_is_undefined (symbol)) {
                continue;
            }

            name_len = strlen (symbol->name) + 1;
            memcpy (str_pos, symbol->name, name_len);
            str_pos += name_len;
        }

        for (symbol = symbols; symbol; symbol = symbol->next) {
            size_t name_len;
            
            if (!(symbol_is_external (symbol)
                  || symbol_is_undefined (symbol))) {
                continue;
            }

            name_len = strlen (symbol->name) + 1;
            memcpy (str_pos, symbol->name, name_len);
            str_pos += name_len;
        }

        shdr_p->sh_type = SHT_STRTAB;
        shdr_p->sh_flags = 0;
        shdr_p->sh_addr = 0;
        shdr_p->sh_offset = pos - file;
        shdr_p->sh_size = (unsigned char *)str_pos - pos;
        shdr_p->sh_link = 0;
        shdr_p->sh_info = 0;;
        shdr_p->sh_addralign = 0;
        shdr_p->sh_entsize = 0;

        pos = (void *)str_pos;
    }
    
    return pos;
}

void write_elf_file (void)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    Elf32_Ehdr ehdr;

    struct section *section;

    if (!(outfile = fopen (state->outfile, "wb"))) {
        as_error_at (NULL, 0, "cannot open '%s' for writing", state->outfile);
        return;
    }

    sections_number (1);

    memset (&ehdr, 0, sizeof (ehdr));
    
    ehdr.e_ident[EI_MAG0] = ELFMAG0;
    ehdr.e_ident[EI_MAG1] = ELFMAG1;
    ehdr.e_ident[EI_MAG2] = ELFMAG2;
    ehdr.e_ident[EI_MAG3] = ELFMAG3;
    ehdr.e_ident[EI_CLASS] = ELFCLASS32;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;

    ehdr.e_type = ET_REL;
    ehdr.e_machine = EM_386;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = 0;
    ehdr.e_ehsize = sizeof (ehdr);

    {
        /* Current layout of object file is:
         * Elf32_Ehdr
         * Content of sections (including section name string table,
                                symbol table and symbol string table)
         * Elf32_Shdr table (aligned just for readability)
         */
        file_size = sizeof (ehdr);

        for (section = sections; section; section = section_get_next_section (section)) {
            size_t relocs_needed;
            size_t alignment;

            if (section_get_alignment_power (section) < 0) {
                section_set_alignment_power (section, 2); /* Arbitrary. */
            }

            alignment = 1 << section_get_alignment_power (section);

            file_size = ALIGN (file_size, alignment);
            if (section != bss_section) {
                struct frag *frag;
                
                section_set (section);
                
                for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
                    file_size += frag->fixed_size;
                }
            }

            if ((relocs_needed = section_get_num_relocs (section))) {
                file_size = ALIGN (file_size, RELOC_SECTION_ALIGNMENT);
                file_size += relocs_needed * sizeof (Elf32_Rel);
            }
        }

        {
            size_t shstrtab_size;
            
            shstrtab_size = 1 + sizeof (".symtab") + sizeof (".strtab") + sizeof (".shstrtab");

            for (section = sections; section; section = section_get_next_section (section)) {
                if (section_get_num_relocs (section)) {
                    shstrtab_size += sizeof (".rel") - 1;
                }
                shstrtab_size += strlen (section_get_name (section)) + 1;
            }

            file_size += shstrtab_size;
        }

        {
            struct symbol *symbol;
            size_t num_symbols = 1;
            size_t strtab_size = 1;
            
            /* Local symbols must precede global symbols. */
            for (symbol = symbols; symbol; symbol = symbol->next) {
                if (symbol_is_external (symbol)
                    || symbol_is_undefined (symbol)) {
                    continue;
                }

                symbol_set_symbol_table_index (symbol, num_symbols);
                strtab_size += strlen (symbol->name) + 1;
                num_symbols++;
            }

            for (symbol = symbols; symbol; symbol = symbol->next) {
                if (!(symbol_is_external (symbol)
                      || symbol_is_undefined (symbol))) {
                    continue;
                }

                symbol_set_symbol_table_index (symbol, num_symbols);
                strtab_size += strlen (symbol->name) + 1;
                num_symbols++;
            }

            file_size = ALIGN (file_size, SYMTAB_SECTION_ALIGNMENT);
            file_size += num_symbols * sizeof (Elf32_Sym);
            file_size += strtab_size;
        }

        {
            file_size = ALIGN (file_size, sizeof (Elf32_Shdr));
            ehdr.e_shoff = file_size;
            ehdr.e_shentsize = sizeof (Elf32_Shdr);
            ehdr.e_shnum = 1;
            for (section = sections; section; section = section_get_next_section (section)) {
                section_set_number (section, ehdr.e_shnum++);
                if (section_get_num_relocs (section)) {
                    ehdr.e_shnum++;
                }
            }
            ehdr.e_shstrndx = ehdr.e_shnum++;
            ehdr.e_shnum += 2;
            
            file_size += ehdr.e_shnum * sizeof (Elf32_Shdr);
        }
    }

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    pos = write_sections (file, &ehdr);
    pos = write_symtab (file, &ehdr, pos);

    {
        unsigned char *saved_pos;
        Elf32_Shdr *shdr_p, *shstrshdr_p;

        shdr_p = (void *)(file + ehdr.e_shoff);
        shstrshdr_p = shdr_p + ehdr.e_shstrndx;
        shdr_p++;
        saved_pos = pos;
        
        shstrshdr_p->sh_type = SHT_STRTAB;
        shstrshdr_p->sh_offset = saved_pos - file;
        shstrshdr_p->sh_addralign = 1;

        *pos = '\0';
        pos++;

        shstrshdr_p[1].sh_name = pos - saved_pos;
        memcpy (pos, ".symtab", sizeof (".symtab"));
        pos += sizeof (".symtab");

        shstrshdr_p[2].sh_name = pos - saved_pos;
        memcpy (pos, ".strtab", sizeof (".strtab"));
        pos += sizeof (".strtab");
        
        shstrshdr_p->sh_name = pos - saved_pos;
        memcpy (pos, ".shstrtab", sizeof (".shstrtab"));
        pos += sizeof (".shstrtab");

        for (section = sections; section; section = section_get_next_section (section)) {
            int has_relocs = !!section_get_num_relocs (section);
            size_t name_len = strlen (section_get_name (section));

            if (has_relocs) {
                memcpy (pos, ".rel", sizeof (".rel") - 1);
                pos += sizeof (".rel") - 1;
            }
            shdr_p->sh_name = pos - saved_pos;
            if (has_relocs) {
                shdr_p++;
                shdr_p->sh_name = pos - saved_pos - (sizeof (".rel") - 1);
            }

            memcpy (pos, section_get_name (section), name_len);
            pos[name_len] = '\0';
            pos += name_len + 1;

            shdr_p++;
        }

        shstrshdr_p->sh_size = pos - saved_pos;
    }

    pos = file;

    memcpy (pos, &ehdr, sizeof (ehdr));

    if (fwrite (file, file_size, 1, outfile) != 1) {
        as_error_at (NULL, 0, "writing '%s' file failed", state->outfile);
    }
    
    free (file);
    fclose (outfile);
}

void elf_comm_symbol_align (struct symbol *symbol, offset_t alignment)
{
    Elf32_Sym *sym = symbol->object_format_dependent_data;

    if (!sym) {
        sym = xmalloc (sizeof *sym);
        memset (sym, 0, sizeof *sym);
        symbol->object_format_dependent_data = sym;
    }

    sym->st_value = 1 << alignment;
}

static void handler_bss (char **pp)
{
    section_subsection_set (bss_section, (subsection_t) get_result_of_absolute_expression (pp));
    demand_empty_rest_of_line (pp);
}

static void handler_ident (char **pp)
{
    section_t saved_section = current_section;
    subsection_t saved_subsection = current_subsection;

    section_set_by_name (".comment");

    handler_asciz (pp);

    section_subsection_set (saved_section, saved_subsection);
}

static void handler_local (char **pp)
{
    for (;;) {
        struct symbol *symbol;
        
        const char *name;
        char ch;
        
        *pp = skip_whitespace (*pp);
        name = *pp;
        
        ch = get_symbol_name_end (pp);
        
        if (name == *pp) {
            as_error ("expected symbol name");
            **pp = ch;
            ignore_rest_of_line (pp);
            return;
        }
        
        symbol = symbol_find_or_make (name);
        symbol->flags |= SYMBOL_FLAG_LOCAL;
        symbol->flags &= ~SYMBOL_FLAG_EXTERNAL;
        
        **pp = ch;
        *pp = skip_whitespace (*pp);
        
        if (**pp != ',') break;
        
        ++*pp;
    }
    
    demand_empty_rest_of_line (pp);
}

static void handler_section (char **pp)
{
    const char *name;
    char ch;
    Elf32_Word sh_flags = 0;
    Elf32_Word sh_type = 0;
    offset_t entsize = 0;
    
    *pp = skip_whitespace(*pp);
    
    /* Names with '-' ("...-note") and other characters that do not belong
     * into names should be accepted too.
     */
    name = *pp;
    while (**pp != ',' && **pp != ' ' && !is_end_of_line[(unsigned char)**pp]) {
        ++*pp;
    }
    ch = **pp;
    **pp = '\0';
    
    section_set_by_name (name);
    
    **pp = ch;
    *pp = skip_whitespace (*pp);
    
    if (**pp == ',') {
        char attribute;
        
        *pp = skip_whitespace (++*pp);

        if (**pp != '"') {
            as_warn ("character following name is not '\"'");
            as_warn ("rest of line ignored; first ignored character is '%c'", **pp);
            ignore_rest_of_line (pp);
            return;
        }

        while ((attribute = *++*pp), attribute != '"' && !is_end_of_line[(int)attribute]) {
            switch (attribute) {
                case 'a':
                    sh_flags |= SHF_ALLOC;
                    break;

                case 'e':
                    sh_flags |= SHF_EXCLUDE;
                    break;

                case 'w':
                    sh_flags |= SHF_WRITE;
                    break;

                case 'x':
                    sh_flags |= SHF_EXECINSTR;
                    break;

                case 'M':
                    sh_flags |= SHF_MERGE;
                    break;

                case 'S':
                    sh_flags |= SHF_STRINGS;
                    break;

                case 'T':
                    sh_flags |= SHF_TLS;
                    break;

                default:
                    as_warn ("unknown section attribute '%c'", attribute);
                    break;

            }
        }

        if (attribute == '"') {
            ++*pp;
        }
    } else {
        demand_empty_rest_of_line (pp);
        return;
    }        

    *pp = skip_whitespace (*pp);
    if (**pp == ',') {
        *pp = skip_whitespace (++*pp);

        if (**pp == '@') {
            ++*pp;
            name = *pp;
            ch = get_symbol_name_end (pp);

            if (!strcmp (name, "progbits")) {
                sh_type = SHT_PROGBITS;
            } else if (!strcmp (name, "nobits")) {
                sh_type = SHT_NOBITS;
            } else {
                as_warn ("unrecognized section type");
            }
            
            **pp = ch;
        } else if (!is_end_of_line[(unsigned char)**pp]) {
            as_warn ("rest of line ignored; first ignored character is '%c'", **pp);
            ignore_rest_of_line (pp);
            return;
        }
    }

    *pp = skip_whitespace (*pp);
    if (**pp == ',') {
        *pp = skip_whitespace (++*pp);

        entsize = get_result_of_absolute_expression (pp);
        if (entsize < 0) {
            as_warn ("invalid merge entity size");
            entsize = 0;
            sh_flags &= ~SHF_MERGE;
        }            
    } else if (sh_flags & SHF_MERGE) {
        as_warn ("entity size for SHF_MERGE not specified; removed SHF_MERGE");
    }

    {
        Elf32_Shdr *shdr_p;

        shdr_p = section_get_object_format_dependent_data (current_section);
        if (shdr_p) {
            if (shdr_p->sh_flags != sh_flags) {
                as_warn ("ignoring changed section attributes for %s",
                         section_get_name (current_section));
            }
            if (shdr_p->sh_type != sh_type) {
                as_warn ("ignoring changed section type for %s",
                         section_get_name (current_section));
            }
            if (shdr_p->sh_entsize != entsize) {
                as_warn ("ignoring changed entity size for %s",
                         section_get_name (current_section));
            }
        } else {            
            shdr_p = xmalloc (sizeof *shdr_p);
            memset (shdr_p, 0, sizeof *shdr_p);
            shdr_p->sh_flags = sh_flags;
            shdr_p->sh_type = sh_type;
            shdr_p->sh_entsize = entsize;
            section_set_object_format_dependent_data (current_section, shdr_p);

            if (shdr_p->sh_flags & SHF_STRINGS) {
                section_set_alignment_power (current_section, 0);
            }
            if (shdr_p->sh_flags & SHF_EXECINSTR) {
                flag_int flags = section_get_flags (current_section);
                section_set_flags (current_section, flags | SECTION_FLAG_CODE);
            }
        }
    }
    
    demand_empty_rest_of_line (pp);
}

static void handler_size (char **pp)
{
    as_warn ("ELF .size is not yet supported");
    ignore_rest_of_line (pp);
}

static void handler_type (char **pp)
{
    as_warn ("ELF .type is not yet supported");
    ignore_rest_of_line (pp);
}

static struct pseudo_op_entry pseudo_op_table[] = {

    { "bss",        &handler_bss        },
    { "ident",      &handler_ident      },
    { "local",      &handler_local      },
    { "section",    &handler_section    },
    { "size",       &handler_size       },
    { "type",       &handler_type       },
    { 0,            0                   }

};

struct pseudo_op_entry *elf_get_pseudo_op_table (void)
{
    return pseudo_op_table;
}
