/******************************************************************************
 * @file            aout.c
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
#include <time.h>

#include "ld.h"
#include "bytearray.h"
#include "aout.h"
#include "xmalloc.h"

#include "aout_bytearray.h"

void aout_init (void)
{
    /* Force the order of sections because the output requires it.
     * This also ensures that .text is at RVA 0.
     */
    section_find_or_make (".text");
    section_find_or_make (".data");
    section_find_or_make (".bss");
}

address_type aout_get_base_address (void)
{
    return 0;
}

static size_t section_get_num_relocs (struct section *section)
{
    struct section_part *part;
    size_t num_relocs = 0;
    
    for (part = section->first_part; part; part = part->next) {
        size_t i;
        
        for (i = 0; i < part->relocation_count; i++) {
            if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_64]
                || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]
                || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_16]
                || part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_8]) {
                num_relocs++;
            }
        }
    }

    return num_relocs;
}

static unsigned char *write_relocs_for_section (unsigned char *pos,
                                                struct section *section)
{
    const struct section_part *part;

    for (part = section->first_part; part; part = part->next) {
        size_t i;
        
        for (i = 0; i < part->relocation_count; i++) {
            struct relocation_info_internal rel;
            const struct symbol *symbol;
            unsigned long size_log2;
            
            if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_64])  {
                size_log2 = 3;
            } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                size_log2 = 2;
            } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_16]) {
                size_log2 = 1;
            } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_8]) {
                size_log2 = 0;
            } else continue;

            symbol = part->relocation_array[i].symbol;
            if (symbol_is_undefined (symbol)) symbol = symbol_find (symbol->name);

            rel.r_address = part->rva - part->section->rva + part->relocation_array[i].offset;

            if (!symbol->part || strcmp (symbol->part->section->name, ".text") == 0) {
                rel.r_symbolnum = N_TEXT;
            } else if (strcmp (symbol->part->section->name, ".data") == 0) {
                rel.r_symbolnum = N_DATA;
            } else if (strcmp (symbol->part->section->name, ".bss") == 0) {
                rel.r_symbolnum = N_BSS;
            } else {
                rel.r_symbolnum = N_TEXT;
            }
            rel.r_symbolnum |= size_log2 << 25;

            write_struct_relocation_info (pos, &rel);
            pos += SIZEOF_struct_relocation_info_file;
        }
    }

    return pos;
}

void aout_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct section *text_section, *data_section, *bss_section;
    struct exec_internal exec = {0};

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    text_section = section_find (".text");
    data_section = section_find (".data");
    bss_section = section_find (".bss");

    exec.a_info = OMAGIC;

    /* Sections might not be contiguous because of RVA alignment.
     * and some might not even exist (but their order is certain).
     */
    if (data_section) {
        exec.a_text = data_section->rva;
        if (bss_section) {
            exec.a_data = bss_section->rva - data_section->rva;
        } else {
            exec.a_data = data_section->total_size;
        }
    } else {
        if (bss_section) {
            exec.a_text = bss_section->rva;
        } else {
            exec.a_text = text_section ? text_section->total_size : 0;
        }
        exec.a_data = 0;
    }
    exec.a_bss = bss_section ? bss_section->total_size : 0;

    exec.a_entry = ld_state->entry_point;
    if (text_section) {
        exec.a_trsize = section_get_num_relocs (text_section) * SIZEOF_struct_relocation_info_file;
    }
    if (data_section) {
        exec.a_drsize = section_get_num_relocs (data_section) * SIZEOF_struct_relocation_info_file;
    }

    file_size = SIZEOF_struct_exec_file;
    file_size += exec.a_text + exec.a_data;
    file_size += exec.a_trsize + exec.a_drsize;
    file_size += 4;

    file = xcalloc (file_size, 1);

    pos = file;
    write_struct_exec (pos, &exec);
    pos += SIZEOF_struct_exec_file;
    
    if (text_section) section_write (text_section, pos);
    pos += exec.a_text;
    if (data_section) section_write (data_section, pos);
    pos += exec.a_data;

    if (text_section) {
        pos = write_relocs_for_section (pos, text_section);
    }
    if (data_section) {
        pos = write_relocs_for_section (pos, data_section);
    }

    bytearray_write_4_bytes (pos, 4, LITTLE_ENDIAN); /* Empty string table. */

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }
    
    free (file);
    fclose (outfile);
}

static void translate_relocation (struct reloc_entry *reloc,
                                  struct relocation_info_internal *input_reloc,
                                  struct section_part *part,
                                  const struct exec_internal *exec_p)
{
    unsigned int size, pcrel;
    
    if (input_reloc->r_symbolnum >> 28) {
        ld_internal_error_at_source (__FILE__, __LINE__,
                                     "relocation with r_symbolnum %#lx not yet supported",
                                     input_reloc->r_symbolnum);
    }

    if ((input_reloc->r_symbolnum >> 27) & 1) {
        reloc->symbol = part->of->symbol_array + (input_reloc->r_symbolnum & 0xffffff);
    } else {
        /* Local relocations always have addend relative to the start of the text section,
         * not to their target section.
         */
        if ((input_reloc->r_symbolnum & 0xffffff) == N_TEXT) {
            reloc->symbol = part->of->symbol_array + part->of->symbol_count - 3;
        } else if ((input_reloc->r_symbolnum & 0xffffff) == N_DATA) {
            reloc->symbol = part->of->symbol_array + part->of->symbol_count - 2;
            reloc->addend -= exec_p->a_text;
        } else if ((input_reloc->r_symbolnum & 0xffffff) == N_BSS) {
            reloc->symbol = part->of->symbol_array + part->of->symbol_count - 1;
            reloc->addend -= exec_p->a_text + exec_p->a_data;
        } else {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "local input_reloc->r_symbolnum %#lx is not yet supported",
                                         input_reloc->r_symbolnum & 0xffffff);
        }
    }

    reloc->offset = input_reloc->r_address;

    size = 1U << ((input_reloc->r_symbolnum >> 25) & 3);
    pcrel = (input_reloc->r_symbolnum >> 24) & 1;

    switch (size) {
        case 8:
            if (pcrel) {
                reloc->howto = &reloc_howtos[RELOC_TYPE_PC64];
                reloc->addend += reloc->offset + 8;
            } else {
                reloc->howto = &reloc_howtos[RELOC_TYPE_64];
            }
            break;
        
        case 4:
            if (pcrel) {
                reloc->howto = &reloc_howtos[RELOC_TYPE_PC32];
                /* a.out PC relative relocations are relative to the start of section,
                 * not to the end of field.
                 */
                reloc->addend += reloc->offset + 4;
            } else {
                reloc->howto = &reloc_howtos[RELOC_TYPE_32];
            }
            break;

        case 2:
            if (pcrel) {
                reloc->howto = &reloc_howtos[RELOC_TYPE_PC16];
                reloc->addend += reloc->offset + 2;
            } else {
                reloc->howto = &reloc_howtos[RELOC_TYPE_16];
            }
            break;

        case 1:
            if (pcrel) {
                reloc->howto = &reloc_howtos[RELOC_TYPE_PC8];
                reloc->addend += reloc->offset + 1;
            } else {
                reloc->howto = &reloc_howtos[RELOC_TYPE_8];
            }
            break;
    }
}

#define CHECK_READ(memory_position, size_to_read) \
    do { if (((memory_position) - file + (size_to_read) > file_size) \
             || (memory_position) < file) ld_fatal_error ("corrupted input file"); } while (0)

static int read_aout_object (unsigned char *file, size_t file_size, const char *filename)
{
    struct exec_internal exec;
    unsigned long strtab_size;
    const char *strtab;

    unsigned char *pos;

    size_t num_symbols;
    struct object_file *of;
    struct section *section, *bss_section;
    struct section_part *part;
    struct section_part *part_p_array[4] = {NULL};

    unsigned long i;

    pos = file;
    CHECK_READ (pos, SIZEOF_struct_exec_file);
    read_struct_exec (&exec, pos);
    pos += SIZEOF_struct_exec_file;

    num_symbols = exec.a_syms / SIZEOF_struct_nlist_file;
    of = object_file_make (num_symbols + 4, filename);

    section = section_find_or_make (".text");
    section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_READONLY | SECTION_FLAG_CODE;
    
    part = section_part_new (section, of);
    part->content_size = exec.a_text;
    part->content = xmalloc (part->content_size);
    CHECK_READ (pos, part->content_size);
    memcpy (part->content, pos, part->content_size);
    pos += part->content_size;
    section_append_section_part (section, part);
    part_p_array[1] = part;
    

    section = section_find_or_make (".data");
    section->flags = SECTION_FLAG_ALLOC | SECTION_FLAG_LOAD | SECTION_FLAG_DATA;
    
    part = section_part_new (section, of);
    part->content_size = exec.a_data;
    part->content = xmalloc (part->content_size);
    CHECK_READ (pos, part->content_size);
    memcpy (part->content, pos, part->content_size);
    pos += part->content_size;
    section_append_section_part (section, part);
    part_p_array[2] = part;
    

    bss_section = section = section_find_or_make (".bss");
    section->flags = SECTION_FLAG_ALLOC;
    section->is_bss = 1;
    
    part = section_part_new (section, of);
    part->content_size = exec.a_bss;
    section_append_section_part (section, part);
    part_p_array[3] = part;


    pos += exec.a_trsize + exec.a_drsize + exec.a_syms;
    CHECK_READ (pos, 4);
    bytearray_read_4_bytes (&strtab_size, pos, LITTLE_ENDIAN);
    if (strtab_size < 4) {
        ld_error ("%s: invalid string table size: %lu", filename, strtab_size);
        return 1;
    } else {
        CHECK_READ (pos, strtab_size);
        strtab = (char *)pos;
    }

    pos -= exec.a_syms;
    for (i = 0; i < num_symbols; i++) {
        struct nlist_internal nlist;
        struct symbol *symbol = of->symbol_array + i;

        read_struct_nlist (&nlist, pos + i * SIZEOF_struct_nlist_file);

        if (nlist.n_strx < strtab_size) {
            symbol->name = xstrdup (strtab + nlist.n_strx);
        } else ld_fatal_error ("%s: invalid offset into string table", filename);

        symbol->value = nlist.n_value;
        symbol->size = 0;

        if ((nlist.n_type & N_TYPE) == N_UNDF
            || (nlist.n_type & N_TYPE) == N_COMM) {
            if (symbol->value) {
                /* It is a common symbol. */
                struct symbol *old_symbol = symbol_find (symbol->name);

                if (!old_symbol || symbol_is_undefined (old_symbol)) {
                    struct section_part *bss_part;
                    
                    bss_part = section_part_new (bss_section, of);
                    section_append_section_part (bss_section, bss_part);

                    bss_part->content_size = symbol->size = symbol->value;
                    symbol->part = bss_part;
                    symbol->value = 0;
                    symbol->section_number = 3;
                } else {
                    if (symbol->value > old_symbol->size) {
                        old_symbol->part->content_size = old_symbol->size = symbol->value;
                    }
                    
                    symbol->value = 0;
                    symbol->part = NULL;
                    symbol->section_number = UNDEFINED_SECTION_NUMBER;
                }
            } else {
                symbol->part = NULL;
                symbol->section_number = UNDEFINED_SECTION_NUMBER;
            }
        } else if ((nlist.n_type & N_TYPE) == N_ABS) {
            symbol->section_number = ABSOLUTE_SECTION_NUMBER;
            symbol->part = NULL;
        } else if ((nlist.n_type & N_TYPE) == N_TEXT) {
            symbol->section_number = 1;
            symbol->part = part_p_array[1];
        } else if ((nlist.n_type & N_TYPE) == N_DATA) {
            symbol->section_number = 2;
            symbol->part = part_p_array[2];
            symbol->value -= exec.a_text;
        } else if ((nlist.n_type & N_TYPE) == N_BSS) {
            symbol->section_number = 3;
            symbol->part = part_p_array[3];
            symbol->value -= exec.a_text + exec.a_data;
        } else if (nlist.n_type == N_FN) {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "nlist.n_type N_FN is not yet supported");
        } else {
            ld_internal_error_at_source (__FILE__, __LINE__,
                                         "unsupported nlist.n_type: %#x",
                                         nlist.n_type);
        }

        if ((nlist.n_type & N_EXT)
            || (nlist.n_type & N_TYPE) == N_UNDF
            || (nlist.n_type & N_TYPE) == N_COMM) {
            symbol_record_external_symbol (symbol);
        }
    }

    /* a.out relocations use implicit section symbols
     * which do not exist in the symbol table,
     * so section symbols need to be automatically generated by the linker.
     */
    for (i = 1; i < 4; i++) {
        struct symbol *symbol = of->symbol_array + num_symbols + i;

        part = part_p_array[i];
        symbol->name = xstrdup (part->section->name);
        symbol->value = 0;
        symbol->size = 0;
        symbol->part = part;
        symbol->section_number = i;
    }

    pos -= exec.a_trsize + exec.a_drsize;
    part = part_p_array[1];
    part->relocation_count = exec.a_trsize / SIZEOF_struct_relocation_info_file;
    part->relocation_array = xcalloc (part->relocation_count, sizeof *part->relocation_array);
    for (i = 0; i < part->relocation_count; i++) {
        struct relocation_info_internal relocation_info;
        read_struct_relocation_info (&relocation_info,
                                      pos + SIZEOF_struct_relocation_info_file * i);
        translate_relocation (part->relocation_array + i, &relocation_info, part, &exec);
    }

    pos += exec.a_trsize;
    part = part_p_array[2];
    part->relocation_count = exec.a_drsize / SIZEOF_struct_relocation_info_file;
    part->relocation_array = xcalloc (part->relocation_count, sizeof *part->relocation_array);
    for (i = 0; i < part->relocation_count; i++) {
        struct relocation_info_internal relocation_info;
        read_struct_relocation_info (&relocation_info,
                                      pos + SIZEOF_struct_relocation_info_file * i);
        translate_relocation (part->relocation_array + i, &relocation_info, part, &exec);
    }
    
    return 0;
}

int aout_read (unsigned char *file, size_t file_size, const char *filename)
{
    unsigned long a_info;
    
    CHECK_READ (file, 4);

    bytearray_read_4_bytes (&a_info, file, LITTLE_ENDIAN);

    if ((a_info & 0xffff) == OMAGIC) {
        if (read_aout_object (file, file_size, filename)) return INPUT_FILE_ERROR;
        return INPUT_FILE_FINISHED;
    }
    
    return INPUT_FILE_UNRECOGNIZED;
}
