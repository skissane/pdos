/******************************************************************************
 * @file            lx.c
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
#include "coff.h"
#include "lx.h"
#include "xmalloc.h"

#include "coff_bytearray.h"
#include "lx_bytearray.h"

static size_t size_of_headers = sizeof (struct LX_HEADER_file);
static size_t stack_size = 0x8000; /* Arbitrary. */

static struct section_part fake_lx_part_s;
static char *dll_inames;
static size_t dll_inames_size;

static unsigned long translate_section_flags_to_ObjectFlags (flag_int flags)
{
    unsigned long ObjectFlags = 0;

    if (!(flags & SECTION_FLAG_READONLY)) {
        ObjectFlags |= OBJECT_FLAGS_MEM_WRITE;
    }

    if (flags & SECTION_FLAG_CODE) {
        ObjectFlags |= OBJECT_FLAGS_MEM_EXECUTE;
    }

    if (!(flags & SECTION_FLAG_NOREAD)) {
        ObjectFlags |= OBJECT_FLAGS_MEM_READ;
    }

    if (flags & SECTION_FLAG_SHARED) {
        ObjectFlags |= OBJECT_FLAGS_SHARED;
    }

    return ObjectFlags;
}

static int reloc_compar (const void *a, const void *b)
{
    if (((struct reloc_entry *)a)->offset < ((struct reloc_entry *)b)->offset) return -1;
    if (((struct reloc_entry *)a)->offset == ((struct reloc_entry *)b)->offset) return 0;
    return 1;
}

static size_t calculate_fixup_record_table_size (void)
{
    struct section *section;
    size_t frt_size = 0;

    for (section = all_sections; section; section = section->next) {
        struct section_part *part;
        
        for (part = section->first_part; part; part = part->next) {
            size_t i;

            /* Relocations need to be sorted by RVA
             * because they need to be grouped
             * according to memory page to which they belong.
             */
            qsort (part->relocation_array,
                   part->relocation_count,
                   sizeof *part->relocation_array,
                   &reloc_compar);
            
            for (i = 0; i < part->relocation_count; i++) {
                const struct symbol *symbol;
                
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    const struct section *source_section;
                    address_type value;
                    
                    if (symbol_is_undefined ((symbol = part->relocation_array[i].symbol))) {
                        if (symbol_is_undefined ((symbol = symbol_find (symbol->name)))) continue;
                    }
                    if (!symbol->part) continue; /* Absolute symbol. */

                    source_section = symbol->part->section;
                    bytearray_read_4_bytes (&value, part->content + part->relocation_array[i].offset, LITTLE_ENDIAN);
                    value -= ld_state->base_address + source_section->rva;
                    
                    if (value > 0xffff) frt_size += 2;
                    
                    frt_size += 7;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_PC32]
                           && (symbol = symbol_find (part->relocation_array[i].symbol->name))
                           && symbol->part == &fake_lx_part_s) {
                    frt_size += 7;
                }
            }
        }
    }

    return frt_size;
}

static void write_relocations (unsigned char *file,
                               struct LX_HEADER_internal *lx_hdr_p,
                               size_t lx_hdr_offset)
{
    unsigned char *page_e_pos, *frt_pos;
    struct section *section;
    size_t old_rec_offset = 0;

    page_e_pos = file + lx_hdr_p->FixupPageTableOffsetHdr + lx_hdr_offset;
    frt_pos = file + lx_hdr_p->FixupRecordTableOffsetHdr + lx_hdr_offset;

    for (section = all_sections; section; section = section->next) {
        address_type cur_rva;
        struct section_part *part;

        /* BSS section has no page entries and no relocations. */
        if (section->is_bss) continue;

        cur_rva = section->rva;
        old_rec_offset = frt_pos - file - (lx_hdr_p->FixupRecordTableOffsetHdr + lx_hdr_offset);
        bytearray_write_4_bytes (page_e_pos, old_rec_offset, LITTLE_ENDIAN);
        page_e_pos += 4;

        for (part = section->first_part; part; part = part->next) {
            size_t i;
            
            for (i = 0; i < part->relocation_count; i++) {
                address_type rva;
                const struct symbol *symbol;

                rva = part->rva + part->relocation_array[i].offset;

                while (rva >= cur_rva + lx_hdr_p->PageSize) {
                    old_rec_offset = frt_pos - file - (lx_hdr_p->FixupRecordTableOffsetHdr + lx_hdr_offset);
                    bytearray_write_4_bytes (page_e_pos, old_rec_offset, LITTLE_ENDIAN);
                    page_e_pos += 4;
                    cur_rva += lx_hdr_p->PageSize;
                }
                
                if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_32]) {
                    const struct section *source_section;
                    address_type value;
                    unsigned char flags;
                    
                    if (symbol_is_undefined ((symbol = part->relocation_array[i].symbol))) {
                        if (symbol_is_undefined ((symbol = symbol_find (symbol->name)))) continue;
                    }
                    if (!symbol->part) continue; /* Absolute symbol. */

                    source_section = symbol->part->section;
                    /* The source offset is just the field value
                     * made source-section-relative.
                     */
                    bytearray_read_4_bytes (&value, part->content + part->relocation_array[i].offset, LITTLE_ENDIAN);
                    value -= ld_state->base_address + source_section->rva;

                    flags = FIXUP_TARGET_FLAGS_INTERNAL_REFERENCE;
                    if (value > 0xffff) flags |= FIXUP_TARGET_FLAGS_32_BIT_OFFSET;
                    
                    bytearray_write_1_bytes (frt_pos, 0x7, LITTLE_ENDIAN);
                    bytearray_write_1_bytes (frt_pos + 1, flags, LITTLE_ENDIAN);
                    bytearray_write_2_bytes (frt_pos + 2, rva - cur_rva, LITTLE_ENDIAN);
                    bytearray_write_1_bytes (frt_pos + 4, source_section->target_index, LITTLE_ENDIAN);
                    if (value > 0xffff) {
                        bytearray_write_4_bytes (frt_pos + 5, value, LITTLE_ENDIAN);
                        frt_pos += 2;
                    } else {
                        bytearray_write_2_bytes (frt_pos + 5, value, LITTLE_ENDIAN);
                    }
                    
                    frt_pos += 7;
                } else if (part->relocation_array[i].howto == &reloc_howtos[RELOC_TYPE_PC32]
                           && (symbol = symbol_find (part->relocation_array[i].symbol->name))
                           && symbol->part == &fake_lx_part_s) {
                    bytearray_write_1_bytes (frt_pos, 0x8, LITTLE_ENDIAN);
                    bytearray_write_1_bytes (frt_pos + 1, FIXUP_TARGET_FLAGS_IMPORT_BY_ORDINAL, LITTLE_ENDIAN);
                    bytearray_write_2_bytes (frt_pos + 2, rva - cur_rva, LITTLE_ENDIAN);
                    bytearray_write_1_bytes (frt_pos + 4, symbol->value >> 16, LITTLE_ENDIAN);
                    bytearray_write_2_bytes (frt_pos + 5, symbol->value & 0xffff, LITTLE_ENDIAN);
                    frt_pos += 7;
                }
            }
        }

        while (section->rva + section->total_size >= cur_rva + lx_hdr_p->PageSize) {
            old_rec_offset = frt_pos - file - (lx_hdr_p->FixupRecordTableOffsetHdr + lx_hdr_offset);
            bytearray_write_4_bytes (page_e_pos, old_rec_offset, LITTLE_ENDIAN);
            page_e_pos += 4;
            cur_rva += lx_hdr_p->PageSize;
        }
    }

    /* Writes the terminating fixup page table entry. */
    old_rec_offset = frt_pos - file - (lx_hdr_p->FixupRecordTableOffsetHdr + lx_hdr_offset);
    bytearray_write_4_bytes (page_e_pos, old_rec_offset, LITTLE_ENDIAN);
    page_e_pos += 4;

    if (dll_inames_size) {
        page_e_pos = file + lx_hdr_p->ImportModuleTableOffsetHdr + lx_hdr_offset;
        memcpy (page_e_pos, dll_inames, dll_inames_size);
    }
}

static void write_sections (unsigned char *file,
                            unsigned char *pos,
                            const struct LX_HEADER_internal *lx_hdr_p,
                            size_t lx_hdr_offset)
{
    struct section *section;
    struct object_table_entry_file *object_table_entry_file;
    struct object_page_table_entry_file *object_page_table_entry_file;
    unsigned long page_table_i = 1;

    object_table_entry_file = (void *)(file + lx_hdr_p->ObjectTableOffsetHdr + lx_hdr_offset);
    object_page_table_entry_file = (void *)(file + lx_hdr_p->ObjectPageTableOffsetHdr + lx_hdr_offset);

    for (section = all_sections; section; section = section->next) {
        struct object_table_entry_internal obj_tab_e;

        obj_tab_e.VirtualSize = section->total_size;
        obj_tab_e.RelocationBaseAddress = ld_state->base_address + section->rva;
        obj_tab_e.ObjectFlags = translate_section_flags_to_ObjectFlags (section->flags);
        /* Segment descriptor setting needed for all 32-bit code. */ 
        obj_tab_e.ObjectFlags |= OBJECT_FLAGS_BIG_DEFAULT_BIT_SETTING;
        
        obj_tab_e.PageTableIndex = page_table_i;
        if (section->is_bss) {
            /* Workaround for PDOS
             * because it does not handle objects
             * without page entries properly.
             */
            obj_tab_e.PageTableIndex--;

            obj_tab_e.NumberOfPageTableEntries = 0;
        } else {
            obj_tab_e.NumberOfPageTableEntries = (section->total_size / lx_hdr_p->PageSize) + !!(section->total_size % lx_hdr_p->PageSize);
        }
        obj_tab_e.Reserved = 0;

        write_struct_object_table_entry (object_table_entry_file++,
                                         &obj_tab_e);

        if (!section->is_bss) {
            unsigned long i;
            
            for (i = 0;
                 i < obj_tab_e.NumberOfPageTableEntries;
                 i++, page_table_i++) {
                struct object_page_table_entry_internal obj_page_e;
                
                obj_page_e.PageDataOffset = pos - file - lx_hdr_p->DataPagesOffset;
                obj_page_e.PageDataOffset += i * lx_hdr_p->PageSize;
                if (obj_tab_e.NumberOfPageTableEntries - i == 1) {
                    obj_page_e.DataSize = section->total_size % lx_hdr_p->PageSize;
                } else {
                    obj_page_e.DataSize = lx_hdr_p->PageSize;
                }
                obj_page_e.Flags = 0;

                write_struct_object_page_table_entry (object_page_table_entry_file++,
                                                      &obj_page_e);
            }

            section_write (section, pos);
            pos += section->total_size;
        }
    }

    {
        /* Creates stack object which has no corresponding section. */ 
        struct object_table_entry_internal obj_tab_e;

        obj_tab_e.VirtualSize = stack_size;
        
        for (section = all_sections; section; section = section->next) {
            obj_tab_e.RelocationBaseAddress = ld_state->base_address + section->rva + section->total_size;
            obj_tab_e.RelocationBaseAddress = ALIGN (obj_tab_e.RelocationBaseAddress, section->section_alignment);
        }
        
        obj_tab_e.ObjectFlags = OBJECT_FLAGS_MEM_READ | OBJECT_FLAGS_MEM_WRITE;
        /* Segment descriptor setting needed for all 32-bit code. */ 
        obj_tab_e.ObjectFlags |= OBJECT_FLAGS_BIG_DEFAULT_BIT_SETTING;
        obj_tab_e.PageTableIndex = page_table_i;
        obj_tab_e.NumberOfPageTableEntries = 0;
        obj_tab_e.Reserved = 0;

        write_struct_object_table_entry (object_table_entry_file++,
                                         &obj_tab_e);
    }        
}

void lx_import_generate_import_with_dll_name (const char *import_name,
                                              short OrdinalHint,
                                              short ImportType,
                                              short ImportNameType,
                                              const char *filename,
                                              const char *dll_name)
{
    struct object_file *of;
    struct symbol *symbol;
    unsigned short dll_index = 1;

    if (ImportNameType != IMPORT_ORDINAL) {
        ld_internal_error_at_source (__FILE__, __LINE__,
                                     "ImportNameType other than IMPORT_ORDINAL is not yet supported for LX");
    }
    if (ImportType != IMPORT_CODE) {
        ld_fatal_error ("ImportType other than IMPORT_CODE not supported for LX");
    }

    {
        size_t i;
        size_t name_len = strlen (dll_name);

        for (i = 0; i < dll_inames_size; i += dll_inames[i] + 1, dll_index++) {
            if (name_len == dll_inames[i]
                && memcmp (dll_name, dll_inames + i + 1, name_len) == 0) {
                break;
            }
        }

        if (i == dll_inames_size) {
            dll_inames_size += 1 + name_len;
            dll_inames = xrealloc (dll_inames, dll_inames_size);
            dll_inames[i] = name_len;
            memcpy (dll_inames + i + 1, dll_name, name_len);
        }
    }
    
    of = object_file_make (1, filename);
    symbol = of->symbol_array;

    symbol->name = xstrdup (import_name);
    symbol->value = (dll_index << 16) | OrdinalHint;
    symbol->part = &fake_lx_part_s;
    symbol->section_number = ABSOLUTE_SECTION_NUMBER;
    symbol_record_external_symbol (symbol);
}

void lx_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    unsigned char *stub_file;
    size_t stub_size;

    struct IMAGE_DOS_HEADER_internal dos_hdr;
    struct LX_HEADER_internal lx_hdr;
    size_t lx_hdr_offset;
    size_t module_name_len;

    struct section *section;

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    coff_get_stub_file (&stub_file, &stub_size);
    {
        size_t new_stack_size = coff_get_SizeOfStackCommit ();
        if (new_stack_size > stack_size) stack_size = new_stack_size;
    }

    if (stub_file) {
        size_of_headers += stub_size;
    } else {
        size_of_headers += sizeof (struct IMAGE_DOS_HEADER_file);
    }
    lx_hdr_offset = size_of_headers - sizeof (struct LX_HEADER_file);

    memset (&lx_hdr, 0, sizeof lx_hdr);

    lx_hdr.Magic[0] = 'L';
    lx_hdr.Magic[1] = 'X';
    lx_hdr.BOrd = BORD_LITTLE_ENDIAN;
    lx_hdr.WOrd = WORD_LITTLE_ENDIAN;
    lx_hdr.FormatLevel = 0;
    lx_hdr.CpuType = CPU_TYPE_386;
    lx_hdr.OsType = OS_TYPE_OS2;
    lx_hdr.ModuleFlags = MODULE_FLAGS_PM_WINDOWING_COMPATIBLE;
    lx_hdr.PageSize = PAGE_SIZE;

    {
        size_t total_section_size_to_write = 0;
        size_t object_page_table_entries = 0;
        int object_index = 1;

        lx_hdr.DataPagesOffset = file_size = size_of_headers;

        for (section = all_sections; section; section = section->next) {
            section->target_index = object_index++;
            
            if (!section->is_bss) {
                total_section_size_to_write += section->total_size;
                object_page_table_entries += (section->total_size / lx_hdr.PageSize) + !!(section->total_size % lx_hdr.PageSize);
            }

            if (ld_state->entry_point >= section->rva
                && ld_state->entry_point < section->rva + section->total_size) {
                lx_hdr.EipObjectIndex = section->target_index;
                lx_hdr.Eip = ld_state->entry_point - section->rva;
            }
        }

        lx_hdr.EspObjectIndex = object_index;
        lx_hdr.Esp = stack_size - 16;

        file_size += total_section_size_to_write;
        lx_hdr.ObjectTableOffsetHdr = file_size - lx_hdr_offset;
        lx_hdr.NumberOfObjectsInModule = object_index;
        file_size += lx_hdr.NumberOfObjectsInModule * sizeof (struct object_table_entry_file);
        lx_hdr.ObjectPageTableOffsetHdr = file_size - lx_hdr_offset;
        file_size += object_page_table_entries * sizeof (struct object_page_table_entry_file);
        lx_hdr.ModuleNumberOfPages = object_page_table_entries;

        lx_hdr.ResidentNameTableOffsetHdr = file_size - lx_hdr_offset;
        {
            const char *p = strrchr (ld_state->output_filename, '.');

            if (p && p != ld_state->output_filename) {
                module_name_len = p - ld_state->output_filename;
            } else {
                module_name_len = strlen (ld_state->output_filename);
            }
        }
        /* Length byte + the name + 2 byte ordinal + empty entry (length byte with 0). */
        file_size += 1 + module_name_len + 2 + 1;

        lx_hdr.EntryTableOffsetHdr = file_size - lx_hdr_offset;
        file_size += 2; /* For dummy entry. */
        lx_hdr.LoaderSectionSize = file_size - lx_hdr_offset - lx_hdr.ObjectTableOffsetHdr;

        lx_hdr.FixupPageTableOffsetHdr = file_size - lx_hdr_offset;
        file_size += 4 * (lx_hdr.ModuleNumberOfPages + 1);
        lx_hdr.FixupRecordTableOffsetHdr = file_size - lx_hdr_offset;
        file_size += calculate_fixup_record_table_size ();

        lx_hdr.ImportModuleTableOffsetHdr = file_size - lx_hdr_offset;
        {
            size_t i;

            for (i = 0; i < dll_inames_size; i += dll_inames[i] + 1) {
                lx_hdr.NumberOfImportModEntries++;
            }
        }
        file_size += dll_inames_size;
        lx_hdr.ImportProcTableOffsetHdr = file_size - lx_hdr_offset;
        /* Empty Import Procedure Table entry (length byte with 0). */
        file_size += 1;
        lx_hdr.FixupSectionSize = file_size - (lx_hdr.FixupPageTableOffsetHdr + lx_hdr_offset);
    }

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    pos = file + size_of_headers;

    write_sections (file, pos, &lx_hdr, lx_hdr_offset);
    write_relocations (file, &lx_hdr, lx_hdr_offset);

    pos = file + lx_hdr_offset + lx_hdr.ResidentNameTableOffsetHdr;
    pos[0] = module_name_len;
    memcpy (pos + 1, ld_state->output_filename, pos[0]);

    pos = file;

    if (stub_file) {
        read_struct_IMAGE_DOS_HEADER (&dos_hdr, stub_file);
        
        dos_hdr.OffsetToRelocationTable = 0x40; /* LX specification requirement. */
        dos_hdr.OffsetToNewEXEHeader = stub_size;

        write_struct_IMAGE_DOS_HEADER (stub_file, &dos_hdr);
        memcpy (pos, stub_file, stub_size);
        pos += stub_size;
        free (stub_file);
    } else {
        memset (&dos_hdr, 0, sizeof (dos_hdr));

        dos_hdr.Magic[0] = 'M';
        dos_hdr.Magic[1] = 'Z';

        dos_hdr.SizeOfHeaderInParagraphs = sizeof (struct IMAGE_DOS_HEADER_file) / IMAGE_DOS_HEADER_PARAGRAPH_SIZE;

        dos_hdr.OffsetToRelocationTable = 0x40; /* LX specification requirement. */
        dos_hdr.OffsetToNewEXEHeader = sizeof (struct IMAGE_DOS_HEADER_file);

        write_struct_IMAGE_DOS_HEADER (pos, &dos_hdr);
        pos += sizeof (struct IMAGE_DOS_HEADER_file);
    }

    write_struct_LX_HEADER (pos, &lx_hdr);

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }

    free (dll_inames);

    free (file);
    fclose (outfile);
}
