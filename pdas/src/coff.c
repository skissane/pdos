/******************************************************************************
 * @file            coff.c
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
#include    <ctype.h>

#include    "as.h"
#include    "coff.h"
#include    "bytearray.h"

#define COFF_DEFAULT_SECTION_FLAGS (SECTION_FLAG_LOAD | SECTION_FLAG_DATA)

static unsigned short target_Machine;

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file.field_name, struct_name##_internal->field_name, LITTLE_ENDIAN)

static int write_struct_coff_header (FILE *outfile, struct coff_header_internal *coff_header_internal) {

    struct coff_header_file coff_header_file;

    COPY(coff_header, Machine, 2);
    COPY(coff_header, NumberOfSections, 2);
    COPY(coff_header, TimeDateStamp, 4);
    COPY(coff_header, PointerToSymbolTable, 4);
    COPY(coff_header, NumberOfSymbols, 4);
    COPY(coff_header, SizeOfOptionalHeader, 2);
    COPY(coff_header, Characteristics, 2);

    if (fwrite (&coff_header_file, sizeof (coff_header_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

static int write_struct_section_table_entry (FILE *outfile, struct section_table_entry_internal *section_table_entry_internal) {

    struct section_table_entry_file section_table_entry_file;

    memcpy (section_table_entry_file.Name, section_table_entry_internal->Name, sizeof (section_table_entry_file.Name));

    COPY(section_table_entry, VirtualSize, 4);
    COPY(section_table_entry, VirtualAddress, 4);
    COPY(section_table_entry, SizeOfRawData, 4);
    COPY(section_table_entry, PointerToRawData, 4);
    COPY(section_table_entry, PointerToRelocations, 4);
    COPY(section_table_entry, PointerToLinenumbers, 4);
    COPY(section_table_entry, NumberOfRelocations, 2);
    COPY(section_table_entry, NumberOfLinenumbers, 2);
    COPY(section_table_entry, Characteristics, 4);
    

    if (fwrite (&section_table_entry_file, sizeof (section_table_entry_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

static int write_struct_relocation_entry (FILE *outfile, struct relocation_entry_internal *relocation_entry_internal) {

    struct relocation_entry_file relocation_entry_file;

    COPY(relocation_entry, VirtualAddress, 4);
    COPY(relocation_entry, SymbolTableIndex, 4);
    COPY(relocation_entry, Type, 2);

    if (fwrite (&relocation_entry_file, sizeof (relocation_entry_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

static int write_struct_symbol_table_entry (FILE *outfile, struct symbol_table_entry_internal *symbol_table_entry_internal) {

    struct symbol_table_entry_file symbol_table_entry_file;

    memcpy (symbol_table_entry_file.Name, symbol_table_entry_internal->Name, sizeof (symbol_table_entry_file.Name));

    COPY(symbol_table_entry, Value, 4);
    COPY(symbol_table_entry, SectionNumber, 2);
    COPY(symbol_table_entry, Type, 2);
    COPY(symbol_table_entry, StorageClass, 1);
    COPY(symbol_table_entry, NumberOfAuxSymbols, 1);

    if (fwrite (&symbol_table_entry_file, sizeof (symbol_table_entry_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

static int write_struct_string_table_header (FILE *outfile, struct string_table_header_internal *string_table_header_internal) {

    struct string_table_header_file string_table_header_file;

    COPY(string_table_header, StringTableSize, 4);

    if (fwrite (&string_table_header_file, sizeof (string_table_header_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

static unsigned long translate_section_flags_to_Characteristics (unsigned int flags) {

    unsigned long Characteristics = 0;

    if (!(flags & SECTION_FLAG_READONLY)) {
        Characteristics |= IMAGE_SCN_MEM_WRITE;
    }

    if (flags & SECTION_FLAG_CODE) {
        Characteristics |= IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE;
    }

    if (flags & SECTION_FLAG_DATA) {
        Characteristics |= IMAGE_SCN_CNT_INITIALIZED_DATA;
    }

    if (flags & SECTION_FLAG_NEVER_LOAD) {
        Characteristics |= IMAGE_SCN_TYPE_NOLOAD;
    }

    if (flags & SECTION_FLAG_DEBUGGING) {
        Characteristics |= IMAGE_SCN_LNK_INFO;
    }

    if (flags & SECTION_FLAG_EXCLUDE) {
        Characteristics |= IMAGE_SCN_LNK_REMOVE;
    }

    if (!(flags & SECTION_FLAG_NOREAD)) {
        Characteristics |= IMAGE_SCN_MEM_READ;
    }

    if (flags & SECTION_FLAG_SHARED) {
        Characteristics |= IMAGE_SCN_MEM_SHARED;
    }

    /* .bss */
    if ((flags & SECTION_FLAG_ALLOC) && !(flags & SECTION_FLAG_LOAD)) {
        Characteristics |= IMAGE_SCN_CNT_UNINITIALIZED_DATA;
    }

    return Characteristics;
}

static unsigned long translate_alignment_power_to_Characteristics (int alignment_power) {

    switch (alignment_power) {

        case -1:

            /* Nothing is done, the default alignment. */
            return 0;

        case 0: return IMAGE_SCN_ALIGN_1BYTES;
        case 1: return IMAGE_SCN_ALIGN_2BYTES;
        case 2: return IMAGE_SCN_ALIGN_4BYTES;
        case 3: return IMAGE_SCN_ALIGN_8BYTES;
        case 4: return IMAGE_SCN_ALIGN_16BYTES;
        case 5: return IMAGE_SCN_ALIGN_32BYTES;
        case 6: return IMAGE_SCN_ALIGN_64BYTES;
        case 7: return IMAGE_SCN_ALIGN_128BYTES;
        case 8: return IMAGE_SCN_ALIGN_256BYTES;
        case 9: return IMAGE_SCN_ALIGN_512BYTES;

    }

    as_internal_error_at_source (__FILE__, __LINE__,
                                 "+++Unsupported section alignment power %i",
                                 alignment_power);

    return 0;

}

static int output_relocation (FILE *outfile, struct fixup *fixup)
{
    struct relocation_entry_internal reloc_entry;
    reloc_entry.VirtualAddress = fixup->frag->address + fixup->where;
    
    if (fixup->add_symbol == NULL) {
        as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "+++output relocation fixup->add_symbol is NULL");
        return 1;
    }
    
    if (symbol_is_section_symbol (fixup->add_symbol)
        && !SECTION_IS_NORMAL (symbol_get_section (fixup->add_symbol))) {
        /* It is possible the add_symbol is internal section symbol
         * such as the absolute section symbol (".L300: .long 123 - .L300").
         * In that case a special symbol table index needs to be used. */
        reloc_entry.SymbolTableIndex = ~(unsigned long)0;
    } else {
        reloc_entry.SymbolTableIndex = symbol_get_symbol_table_index (fixup->add_symbol);
    }

    if (target_Machine == IMAGE_FILE_MACHINE_AMD64) {
        switch (fixup->reloc_type) {
            
            case RELOC_TYPE_DEFAULT:
                switch (fixup->size) {
                    
                    case 4:
                        if (fixup->pcrel) reloc_entry.Type = IMAGE_REL_AMD64_REL32;
                        else reloc_entry.Type = IMAGE_REL_AMD64_ADDR32;
                        break;

                    case 8:
                        reloc_entry.Type = IMAGE_REL_AMD64_ADDR64;
                        break;

                    default:
                        as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                                        "unsupported COFF relocation size %i for reloc_type RELOC_TYPE_DEFAULT",
                                                        fixup->size);
                        break;
                    
                }
                break;

            case RELOC_TYPE_RVA:
                if (fixup->size != 4) {
                    as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                                    "unsupported COFF relocation size %i for reloc_type RELOC_TYPE_RVA",
                                                    fixup->size);
                }
                reloc_entry.Type = IMAGE_REL_AMD64_ADDR32NB;
                break;

            default:
                as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                                "output_relocation invalid reloc_type",
                                                fixup->reloc_type);
                break;

        }
    } else {
        switch (fixup->reloc_type) {
        
            case RELOC_TYPE_DEFAULT:
                switch (fixup->size) {
                
                    case 2:
                        if (fixup->pcrel) {
                            reloc_entry.Type = IMAGE_REL_I386_REL16;
                        } else {
                            reloc_entry.Type = IMAGE_REL_I386_DIR16;
                        }
                        
                        break;
                    
                    case 4:
                        if (fixup->pcrel) {
                            reloc_entry.Type = IMAGE_REL_I386_REL32;
                        } else {
                            reloc_entry.Type = IMAGE_REL_I386_DIR32;
                        }
                        
                        break;
                    
                    default:
                        as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                                        "unsupported COFF relocation size %i for reloc_type RELOC_TYPE_DEFAULT",
                                                        fixup->size);
                        break;
                
                }
                
                break;
            
            case RELOC_TYPE_RVA:
                if (fixup->size != 4) {
                    as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                                    "unsupported COFF relocation size %i for reloc_type RELOC_TYPE_RVA",
                                                    fixup->size);
                }
                reloc_entry.Type = IMAGE_REL_I386_DIR32NB;
                break;

            default:
                as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                                "output_relocation invalid reloc_type",
                                                fixup->reloc_type);
                break;
        
        }
    }
    
    if (write_struct_relocation_entry (outfile, &reloc_entry)) {
        return 1;
    }
    
    return 0;
}

void write_coff_file (void) {

    struct coff_header_internal header;
    struct string_table_header_internal string_table_header = {sizeof (struct string_table_header_file)};
    
    FILE *outfile;
    struct symbol *symbol;
    section_t section;
    
    sections_number (1);
    memset (&header, 0, sizeof (header));
    
    if ((outfile = fopen (state->outfile, "wb")) == NULL) {
    
        as_error_at (NULL, 0, "Failed to open '%s' as output file", state->outfile);
        return;
    
    }
    
    header.Machine = target_Machine;
    header.NumberOfSections = sections_get_count ();
    header.SizeOfOptionalHeader = 0;
    header.Characteristics = IMAGE_FILE_LINE_NUMS_STRIPPED;
    if (target_Machine == IMAGE_FILE_MACHINE_I386) header.Characteristics |= IMAGE_FILE_32BIT_MACHINE;
    
    if (fseek (outfile,
               (sizeof (struct coff_header_file)
                + sections_get_count () * sizeof (struct section_table_entry_file)),
               SEEK_SET)) {
    
        as_error_at (NULL, 0, "Failed to fseek");
        return;
    
    }
    
    for (section = sections; section; section = section_get_next_section (section)) {
    
        struct section_table_entry_internal *section_header = xmalloc (sizeof (*section_header));
        section_set_object_format_dependent_data (section, section_header);
        
        memset (section_header, 0, sizeof (*section_header));

        if (strlen (section_get_name (section)) <= 8) {
            memcpy (section_header->Name, section_get_name (section), strlen (section_get_name (section)));
        } else {
        
            section_header->Name[0] = '/';
            sprintf (section_header->Name + 1, "%lu", string_table_header.StringTableSize);
            
            string_table_header.StringTableSize += strlen (section_get_name (section)) + 1;

        }
        

        section_header->Characteristics = translate_section_flags_to_Characteristics (section_get_flags (section));
        section_header->Characteristics |= translate_alignment_power_to_Characteristics (section_get_alignment_power (section));
        
        if (section != bss_section) {
        
            struct frag *frag;
            section_set (section);
            
            section_header->PointerToRawData = ftell (outfile);
            section_header->SizeOfRawData = 0;
            
            for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
            
                if (frag->fixed_size == 0) {
                    continue;
                }
                
                if (fwrite (frag->buf, frag->fixed_size, 1, outfile) != 1) {
                
                    as_error_at (NULL, 0, "Failed whilst writing secton '%s'!", section_get_name (section));
                    return;
                
                }
                
                section_header->SizeOfRawData += frag->fixed_size;
            
            }
            
            if (section_header->SizeOfRawData == 0) {
                section_header->PointerToRawData = 0;
            }
        
        } else {
        
            struct frag *frag;
            
            section_set (section);
            section_header->PointerToRawData = 0;
            section_header->SizeOfRawData = 0;
            
            for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
            
                if (frag->fixed_size == 0) {
                    continue;
                }
                
                section_header->SizeOfRawData += frag->fixed_size;
            
            }
        
        }
    
    }
    
    header.PointerToSymbolTable = ftell (outfile);
    header.NumberOfSymbols = 0;
    
    for (symbol = symbols; symbol; symbol = symbol->next) {
    
        struct symbol_table_entry_internal sym_tbl_ent;
        
        if (symbol->object_format_dependent_data == NULL) {
            
            memset (&sym_tbl_ent, 0, sizeof (sym_tbl_ent));

            sym_tbl_ent.Value = symbol_get_value (symbol);
            
            if (symbol->section == undefined_section) {
                sym_tbl_ent.SectionNumber = IMAGE_SYM_UNDEFINED;
            } else {
                sym_tbl_ent.SectionNumber = section_get_number (symbol->section);
            }
        
            sym_tbl_ent.Type = ((IMAGE_SYM_DTYPE_NULL << 8) | IMAGE_SYM_TYPE_NULL);
            
            if (symbol_is_external (symbol) || symbol_is_undefined (symbol)) {
                sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
            } else if (symbol_is_section_symbol (symbol)) {
                sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_STATIC;
            } else if (symbol_get_section (symbol) == text_section) {
                sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_LABEL;
            } else {
                sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_STATIC;
            }
            
            sym_tbl_ent.NumberOfAuxSymbols = 0;

        } else {

            sym_tbl_ent = *(struct symbol_table_entry_internal *)(symbol->object_format_dependent_data);

            sym_tbl_ent.Value = symbol_get_value (symbol);

            if (!sym_tbl_ent.SectionNumber) {

                if (symbol->section == undefined_section) {
                    sym_tbl_ent.SectionNumber = IMAGE_SYM_UNDEFINED;
                } else {
                    sym_tbl_ent.SectionNumber = section_get_number (symbol->section);
                }

            }

            if (!sym_tbl_ent.Type) {
                sym_tbl_ent.Type = ((IMAGE_SYM_DTYPE_NULL << 8) | IMAGE_SYM_TYPE_NULL);
            }

            if (sym_tbl_ent.SectionNumber == IMAGE_SYM_UNDEFINED) {
                sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
            } else if (sym_tbl_ent.StorageClass == IMAGE_SYM_CLASS_STATIC && symbol_is_external (symbol)) {
                sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
            } else if (!sym_tbl_ent.StorageClass) {

                if (symbol_is_external (symbol)) {
                    sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
                } else if (symbol_is_section_symbol (symbol)) {
                    sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_STATIC;
                } else if (symbol_get_section (symbol) == text_section) {
                    sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_LABEL;
                } else {
                    sym_tbl_ent.StorageClass = IMAGE_SYM_CLASS_STATIC;
                }

            }

            free (symbol->object_format_dependent_data);
        }
        
        symbol->write_name_to_string_table = 0;
        
        if (strlen (symbol->name) <= 8) {
            memcpy (sym_tbl_ent.Name, symbol->name, strlen (symbol->name));
        } else {
        
            memset (sym_tbl_ent.Name, 0, 4);
            
            bytearray_write_4_bytes ((unsigned char *) (sym_tbl_ent.Name + 4), string_table_header.StringTableSize, LITTLE_ENDIAN);
            
            string_table_header.StringTableSize += strlen (symbol->name) + 1;
            symbol->write_name_to_string_table = 1;
        
        }
        
        if (write_struct_symbol_table_entry (outfile, &sym_tbl_ent)) {
        
            as_error_at (NULL, 0, "Error writing symbol table!");
            return;
        
        }
        
        symbol_set_symbol_table_index (symbol, header.NumberOfSymbols);
        header.NumberOfSymbols++;
    
    }
    
    if (write_struct_string_table_header (outfile, &string_table_header)) {
    
        as_error_at (NULL, 0, "Failed to write string table!");
        return;
    
    }

    for (section = sections; section; section = section_get_next_section (section)) {

        if (strlen (section_get_name (section)) > 8) {

            if (fwrite (section_get_name (section), strlen (section_get_name (section)) + 1, 1, outfile) != 1) {
            
                as_error_at (NULL, 0, "Failed to write string table!");
                return;
            
            }
        
        }

    }
    
    for (symbol = symbols; symbol; symbol = symbol->next) {
    
        if (symbol->write_name_to_string_table) {
        
            if (fwrite (symbol->name, strlen (symbol->name) + 1, 1, outfile) != 1) {
            
                as_error_at (NULL, 0, "Failed to write string table!");
                return;
            
            }
        
        }
    
    }
    
    for (section = sections; section; section = section_get_next_section (section)) {
    
        struct section_table_entry_internal *section_header = section_get_object_format_dependent_data (section);
        struct fixup *fixup;
        
        section_header->PointerToRelocations = ftell (outfile);
        section_header->NumberOfRelocations = 0;
        
        section_set (section);
        
        for (fixup = current_frag_chain->first_fixup; fixup; fixup = fixup->next) {
        
            if (fixup->done) {
                continue;
            }
            
            if (output_relocation (outfile, fixup)) {
            
                as_error_at (NULL, 0, "Failed to write relocation!");
                return;
            
            }
            
            section_header->NumberOfRelocations++;
        
        }
        
        if (section_header->NumberOfRelocations == 0) {
            section_header->PointerToRelocations = 0;
        }
    
    }
    
    rewind (outfile);
    
    if (write_struct_coff_header (outfile, &header)) {
    
        as_error_at (NULL, 0, "Failed to write header!");
        return;
    
    }
    
    for (section = sections; section; section = section_get_next_section (section)) {
        struct section_table_entry_internal *section_header = section_get_object_format_dependent_data (section);
        
        if (write_struct_section_table_entry (outfile, section_header)) {
            as_error_at (NULL, 0, "Failed to write section header!");
            return;
        }

        free (section_header);
    }
    
    if (fclose (outfile)) {
        as_error_at (NULL, 0, "Failed to close file!");
    }
}

static void handler_bss (char **pp) {

    if (is_end_of_line[(int) **pp]) {
    
        section_subsection_set (bss_section, (subsection_t) get_result_of_absolute_expression (pp));
        demand_empty_rest_of_line (pp);
    
    } else {
        handler_lcomm (pp);
    }

}

static void handler_ident (char **pp) {

    section_t saved_section = current_section;
    subsection_t saved_subsection = current_subsection;

    section_set_by_name (".comment");
    if (section_get_flags (current_section) == 0) {
        section_set_flags (current_section,
                           SECTION_FLAG_READONLY
                           | SECTION_FLAG_DEBUGGING
                           | SECTION_FLAG_NOREAD
                           /* It is not clear whether IMAGE_SCN_LNK_INFO should imply IMAGE_SCN_LNK_REMOVE
                            * or not but at least one linker cannot properly handle .comment section
                            * without IMAGE_SCN_LNK_REMOVE, so it is explictly added. */
                           | SECTION_FLAG_EXCLUDE);
    }

    handler_asciz (pp);

    section_subsection_set (saved_section, saved_subsection);

}

static void handler_section (char **pp) {

    char *name;
    char ch;
    unsigned int flags = 0, old_flags;
    int alignment = -1;
    
    *pp = skip_whitespace(*pp);
    
    name = *pp;
    ch = get_symbol_name_end (pp);
    if (name == *pp) {
        as_error ("expected section name");
        **pp = ch;
        ignore_rest_of_line (pp);
        return;
    }
    
    section_set_by_name (name);
    
    **pp = ch;
    *pp = skip_whitespace(*pp);
    
    if (**pp == ',') {
    
        *pp = skip_whitespace(++*pp);
        
        if (**pp != '"') {
            section_subsection_set (current_section, (subsection_t) get_result_of_absolute_expression (pp));
        } else {

            char attribute;
            int readonly_removed = 0;
            int load_removed = 0;

            while ((attribute = *++*pp), attribute != '"' && !is_end_of_line[(int)attribute]) {

                if (isdigit (attribute)) {

                    alignment = attribute - '0';
                    continue;

                }

                switch (attribute) {

                    case 'a':

                        /* Should be silently ignored. */
                        break;

                    case 'b':

                        flags |= SECTION_FLAG_ALLOC;
                        flags &= ~SECTION_FLAG_LOAD;
                        /*as_internal_error_at_source (__FILE__, __LINE__, "+++Creating new BSS sections is not yet supported");*/
                        break;

                    case 's':

                        flags |= SECTION_FLAG_SHARED;
                        /* Fall through. */

                    case 'd':

                        flags |= SECTION_FLAG_DATA;
                        if (!load_removed) {
                            flags |= SECTION_FLAG_LOAD;
                        }
                        flags &= ~SECTION_FLAG_READONLY;
                        break;

                    case 'e':

                        flags |= SECTION_FLAG_EXCLUDE;
                        break;

                    case 'n':

                        flags |= SECTION_FLAG_NEVER_LOAD;
                        flags &= ~SECTION_FLAG_LOAD;
                        load_removed = 1;
                        break;

                    case 'w':

                        flags &= ~SECTION_FLAG_READONLY;
                        readonly_removed = 1;
                        break;

                    case 'r':

                        readonly_removed = 0;
                        /* Fall through. */

                    case 'x':
                    
                        if ((flags & SECTION_FLAG_CODE) || attribute == 'x') {
                            flags |= SECTION_FLAG_CODE;
                        } else {
                            flags |= SECTION_FLAG_DATA;
                        }

                        if (!load_removed) {
                            flags |= SECTION_FLAG_LOAD;
                        }

                        if (!readonly_removed) {
                            flags |= SECTION_FLAG_READONLY;
                        }
                        break;

                    case 'y':

                        flags |= SECTION_FLAG_READONLY | SECTION_FLAG_NOREAD;
                        break;

                    default:

                        as_warn ("unknown section attribute '%c'", attribute);
                        break;

                }

            }

            if (attribute == '"') {
                ++*pp;
            }
            
        }
    
    }

    if (alignment >= 0) {
        section_set_alignment_power (current_section, alignment);
    }

    old_flags = section_get_flags (current_section);
    if (old_flags == 0) {

        if (flags == 0) {
            flags = COFF_DEFAULT_SECTION_FLAGS;
        }

        section_set_flags (current_section, flags);

    } else if (flags != 0) {

        if (flags ^ old_flags) {
            as_warn ("Ignoring changed section attributes for %s", section_get_name (current_section));
        }

    }
    
    demand_empty_rest_of_line (pp);

}

static struct symbol *def_symbol = NULL;

static void handler_def (char **pp) {

    char *name;
    char ch;

    if (def_symbol) {

        as_warn (".def pseudo-op used inside of .def/.endef, ignored");
        demand_empty_rest_of_line (pp);
        return;

    }

    name = *pp;
    ch = get_symbol_name_end (pp);
    if (name == *pp) {
        as_error ("expected symbol name");
        **pp = ch;
        ignore_rest_of_line (pp);
        return;
    }

    def_symbol = symbol_make (name);
    def_symbol->object_format_dependent_data = xmalloc (sizeof (struct symbol_table_entry_internal));
    memset (def_symbol->object_format_dependent_data, 0, sizeof (struct symbol_table_entry_internal));

    **pp = ch;

    demand_empty_rest_of_line (pp);

}

static void handler_endef (char **pp) {

    struct symbol_table_entry_internal *symbol_table_entry;
    struct symbol *existing_symbol;

    if (def_symbol == NULL) {

        as_warn (".endef pseudo-op used outside of .def/.endef, ignored");
        demand_empty_rest_of_line (pp);
        return;

    }

    symbol_table_entry = def_symbol->object_format_dependent_data;

    switch (symbol_table_entry->StorageClass) {

        case IMAGE_SYM_CLASS_EXTERNAL:
        case IMAGE_SYM_CLASS_STATIC:

            /* Nothing needs to be done here. */
            break;

        case IMAGE_SYM_CLASS_FILE:

            symbol_table_entry->SectionNumber = IMAGE_SYM_DEBUG;
            break;

        default:

            as_warn ("unexpected storage class %i", symbol_table_entry->StorageClass);
            break;

    }

    if ((existing_symbol = symbol_find (symbol_get_name (def_symbol))) == NULL) {
        symbol_add_to_chain (def_symbol);
    } else {

        /* Merges the new symbol into the old one. */
        if (existing_symbol->object_format_dependent_data) {
            free (existing_symbol->object_format_dependent_data);
        }
        
        existing_symbol->object_format_dependent_data = def_symbol->object_format_dependent_data;
        
    }

    def_symbol = NULL;
    demand_empty_rest_of_line (pp);

}

static void handler_scl (char **pp) {

    struct symbol_table_entry_internal *symbol_table_entry;

    if (def_symbol == NULL) {

        as_warn (".scl pseudo-op used outside of .def/.endef, ignored");
        demand_empty_rest_of_line (pp);
        return;

    }

    symbol_table_entry = def_symbol->object_format_dependent_data;

    symbol_table_entry->StorageClass = get_result_of_absolute_expression (pp);

    demand_empty_rest_of_line (pp);

}

static void handler_type (char **pp) {

    struct symbol_table_entry_internal *symbol_table_entry;

    if (def_symbol == NULL) {

        as_warn (".type pseudo-op used outside of .def/.endef, ignored");
        demand_empty_rest_of_line (pp);
        return;

    }

    symbol_table_entry = def_symbol->object_format_dependent_data;

    symbol_table_entry->Type = get_result_of_absolute_expression (pp);

    demand_empty_rest_of_line (pp);

}

static struct pseudo_op_entry pseudo_op_table[] = {

    { "bss",        &handler_bss        },
    { "def",        &handler_def        },
    { "endef",      &handler_endef      },
    { "ident",      &handler_ident      },
    { "scl",        &handler_scl        },
    { "section",    &handler_section    },
    { "type",       &handler_type       },

    /* Structured exception handling (SEH) can probably be safely ignored for now. */
    { "seh_proc",        &handler_ignore },
    { "seh_endproc",     &handler_ignore },
    { "seh_pushframe",   &handler_ignore },
    { "seh_endprologue", &handler_ignore },
    { "seh_setframe",    &handler_ignore },
    { "seh_stackalloc",  &handler_ignore },
    { "seh_pushreg",     &handler_ignore },
    { "seh_savereg",     &handler_ignore },
    { "seh_savexmm",     &handler_ignore },
    
    { 0,            0                   }

};

struct pseudo_op_entry *coff_get_pseudo_op_table (void) {
    return pseudo_op_table;
}

void coff_x86_set_bits (int bits)
{
    if (bits == 64) target_Machine = IMAGE_FILE_MACHINE_AMD64;
    else target_Machine = IMAGE_FILE_MACHINE_I386;
}
