/******************************************************************************
 * @file            a_out.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include    <stddef.h>
#include    <stdio.h>
#include    <string.h>

#include    "as.h"
#include    "a_out.h"
#include    "bytearray.h"

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file.field_name, struct_name##_internal->field_name, LITTLE_ENDIAN)

static int write_struct_exec (FILE *outfile, struct exec_internal *exec_internal) {

    struct exec_file exec_file;

    COPY(exec, a_info, 4);
    COPY(exec, a_text, 4);
    COPY(exec, a_data, 4);
    COPY(exec, a_bss, 4);
    COPY(exec, a_syms, 4);
    COPY(exec, a_entry, 4);
    COPY(exec, a_trsize, 4);
    COPY(exec, a_drsize, 4);

    if (fwrite (&exec_file, sizeof (exec_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

static int write_struct_relocation_info (FILE *outfile, struct relocation_info_internal *relocation_info_internal) {

    struct relocation_info_file relocation_info_file;

    COPY(relocation_info, r_address, 4);
    COPY(relocation_info, r_symbolnum, 4);

    if (fwrite (&relocation_info_file, sizeof (relocation_info_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

static int write_struct_nlist (FILE *outfile, struct nlist_internal *nlist_internal) {

    struct nlist_file nlist_file;

    COPY(nlist, n_strx, 4);
    COPY(nlist, n_type, 1);
    COPY(nlist, n_other, 1);
    COPY(nlist, n_desc, 2);
    COPY(nlist, n_value, 4);

    if (fwrite (&nlist_file, sizeof (nlist_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

static int write_struct_string_table_header (FILE *outfile, struct string_table_header_internal *string_table_header_internal) {

    struct string_table_header_file string_table_header_file;

    COPY(string_table_header, s_size, 4);

    if (fwrite (&string_table_header_file, sizeof (string_table_header_file), 1, outfile) != 1) {
        return 1;
    }

    return 0;

}

#undef COPY

static int output_relocation (FILE *outfile, struct fixup *fixup, unsigned long start_address_of_section) {

    struct relocation_info_internal reloc;

    reloc.r_address = fixup->frag->address + fixup->where - start_address_of_section;
    
    if (symbol_is_section_symbol (fixup->add_symbol)) {
    
        if (symbol_get_section (fixup->add_symbol) == text_section) {
            reloc.r_symbolnum = N_TEXT;
        } else if (symbol_get_section (fixup->add_symbol) == data_section) {
            reloc.r_symbolnum = N_DATA;
        } else if (symbol_get_section (fixup->add_symbol) == bss_section) {
            reloc.r_symbolnum = N_BSS;
        } else {
            as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                            "invalid section %s",
                                            section_get_name (symbol_get_section (fixup->add_symbol)));
        }
    
    } else {
    
        struct symbol *symbol;
        int symbol_number;
        
        for (symbol = symbols, symbol_number = 0; symbol && (symbol != fixup->add_symbol); symbol = symbol->next) {
            symbol_number++;
        }
        
        reloc.r_symbolnum  = symbol_number;
        reloc.r_symbolnum |= 1LU << 27;
    
    }
    
    if (fixup->pcrel) {
        reloc.r_symbolnum |= 1LU << 24;
    }
    
    {
    
        int log2_of_size, size;
        
        for (log2_of_size = -1, size = fixup->size; size; size >>= 1, log2_of_size++);
        reloc.r_symbolnum |= ((unsigned long) log2_of_size) << 25;
    
    }
    
    if (write_struct_relocation_info (outfile, &reloc)) {
    
        as_error_at (NULL, 0, "Error writing relocation!");
        return 1;
    
    }

    return 0;

}

void write_a_out_file (void) {

    struct exec_internal header;
    struct string_table_header_internal string_table_header = {sizeof (struct string_table_header_file)};
    
    struct frag *frag;
    
    struct fixup *fixup;
    unsigned long start_address_of_data;
    
    struct symbol *symbol;
    unsigned long symbol_table_size;
    
    FILE *outfile;
    
    memset (&header, 0, sizeof (header));
    header.a_info = 0x00640000LU | OMAGIC;
    
    if ((outfile = fopen (state->outfile, "wb")) == NULL) {
    
        as_error_at (NULL, 0, "Failed to open '%s' as output file", state->outfile);
        return;
    
    }
    
    if (fseek (outfile, sizeof (struct exec_file), SEEK_SET)) {
    
        as_error_at (NULL, 0, "Failed whilst seeking past header");
        return;
    
    }
    
    section_set (text_section);
    header.a_text = 0;
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        if (fwrite (frag->buf, frag->fixed_size, 1, outfile) != 1) {
        
            as_error_at (NULL, 0, "Failed whilst writing text!");
            return;
        
        }
        
        header.a_text += frag->fixed_size;
    
    }
    
    section_set (data_section);
    header.a_data = 0;
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        if (fwrite (frag->buf, frag->fixed_size, 1, outfile) != 1) {
        
            as_error_at (NULL, 0, "Failed whilst writing data!");
            return;
        
        }
        
        header.a_data += frag->fixed_size;
    
    }
    
    section_set (bss_section);
    header.a_bss = 0;
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        header.a_bss += frag->fixed_size;
    
    }
    
    header.a_trsize = 0;
    section_set (text_section);
    
    for (fixup = current_frag_chain->first_fixup; fixup; fixup = fixup->next) {
    
        if (fixup->done) {
            continue;
        }
        
        if (output_relocation (outfile, fixup, 0)) {
            return;
        }
        
        header.a_trsize += sizeof (struct relocation_info_file);
    
    }
    
    header.a_drsize = 0;
    section_set (data_section);
    
    start_address_of_data = current_frag_chain->first_frag->address;
    
    for (fixup = current_frag_chain->first_fixup; fixup; fixup = fixup->next) {
    
        if (fixup->done) {
            continue;
        }
        
        if (output_relocation (outfile, fixup, start_address_of_data)) {
            return;
        }
        
        header.a_drsize += sizeof (struct relocation_info_file);
    
    }
    
    symbol_table_size = 0;
    
    for (symbol = symbols; symbol; symbol = symbol->next) {
    
        struct nlist_internal symbol_entry;
        memset (&symbol_entry, 0, sizeof (symbol_entry));
        
        symbol_entry.n_strx = string_table_header.s_size;
        string_table_header.s_size += strlen (symbol->name) + 1;
        
        if (symbol->section == undefined_section) {
            symbol_entry.n_type = N_UNDF;
        } else if (symbol->section == text_section) {
            symbol_entry.n_type = N_TEXT;
        } else if (symbol->section == data_section) {
            symbol_entry.n_type = N_DATA;
        } else if (symbol->section == bss_section) {
            symbol_entry.n_type = N_BSS;
        } else if (symbol->section == absolute_section) {
            symbol_entry.n_type = N_ABS;
        } else {
            as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0, "invalid section %s", section_get_name (symbol->section));
        }
        
        if (symbol_is_external (symbol) || symbol_is_undefined (symbol)) {
            symbol_entry.n_type |= N_EXT;
        }
        
        symbol_entry.n_value = symbol_get_value (symbol);
        
        if (write_struct_nlist (outfile, &symbol_entry)) {
        
            as_error_at (NULL, 0, "Error writing symbol table!");
            return;
        
        }
        
        symbol_table_size += sizeof (struct nlist_file);
    
    }
    
    header.a_syms = symbol_table_size;
    
    if (write_struct_string_table_header (outfile, &string_table_header)) {
    
        as_error_at (NULL, 0, "Failed to write string table!");
        return;
    
    }
    
    for (symbol = symbols; symbol; symbol = symbol->next) {
    
        if (fwrite (symbol->name, strlen (symbol->name) + 1, 1, outfile) != 1) {
        
            as_error_at (NULL, 0, "Failed to write string table!");
            return;
        
        }
    
    }
    
    rewind (outfile);
    
    if (write_struct_exec (outfile, &header)) {
    
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

static struct pseudo_op_entry pseudo_op_table[] = {

    { "bss",        &handler_bss        },
    { "def",        &handler_ignore     },
    { "endef",      &handler_ignore     },
    { "ident",      &handler_ignore     },
    { "scl",        &handler_ignore     },
    { "type",       &handler_ignore     },
    { 0,            0                   }

};

struct pseudo_op_entry *a_out_get_pseudo_op_table (void) {
    return pseudo_op_table;
}
