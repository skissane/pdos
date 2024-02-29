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

static size_t size_of_headers = sizeof (struct IMAGE_DOS_HEADER_file) + sizeof (struct LX_HEADER_file);

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

static void write_sections (unsigned char *file,
                            unsigned char *pos,
                            struct LX_HEADER_internal *lx_hdr_p,
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
        unsigned long i;

        obj_tab_e.VirtualSize = section->total_size;
        obj_tab_e.RelocationBaseAddress = ld_state->base_address + section->rva;
        obj_tab_e.ObjectFlags = translate_section_flags_to_ObjectFlags (section->flags);
        /* Segment descriptor setting needed for all 32-bit code. */ 
        obj_tab_e.ObjectFlags |= OBJECT_FLAGS_BIG_DEFAULT_BIT_SETTING;
        obj_tab_e.PageTableIndex = page_table_i;
        obj_tab_e.NumberOfPageTableEntries = (section->total_size / lx_hdr_p->PageSize) + !!(section->total_size % lx_hdr_p->PageSize);
        obj_tab_e.Reserved = 0;

        write_struct_object_table_entry (object_table_entry_file++,
                                         &obj_tab_e);

        for (i = 0;
             i < obj_tab_e.NumberOfPageTableEntries;
             i++, page_table_i++) {
            struct object_page_table_entry_internal obj_page_e;
            
            obj_page_e.PageDataOffset = pos - file - lx_hdr_p->DataPagesOffset;
            obj_page_e.PageDataOffset += i * lx_hdr_p->PageSize;
            if (obj_tab_e.NumberOfPageTableEntries - i == 1) {
                obj_page_e.DataSize =  section->total_size % lx_hdr_p->PageSize;
            } else {
                obj_page_e.DataSize = lx_hdr_p->PageSize;
            }

            write_struct_object_page_table_entry (object_page_table_entry_file++,
                                                  &obj_page_e);
        }

        if (!section->is_bss) {
            section_write (section, pos);
            pos += section->total_size;
        }
    }
}

void lx_write (const char *filename)
{
    FILE *outfile;
    unsigned char *file;
    size_t file_size;
    unsigned char *pos;

    struct IMAGE_DOS_HEADER_internal dos_hdr;
    struct LX_HEADER_internal lx_hdr;

    struct section *section;

    size_t lx_hdr_offset = size_of_headers - sizeof (struct LX_HEADER_file);

    if (!(outfile = fopen (filename, "wb"))) {
        ld_error ("cannot open '%s' for writing", filename);
        return;
    }

    memset (&lx_hdr, 0, sizeof lx_hdr);

    lx_hdr.Magic[0] = 'L';
    lx_hdr.Magic[1] = 'X';
    lx_hdr.BOrd = BORD_LITTLE_ENDIAN;
    lx_hdr.WOrd = WORD_LITTLE_ENDIAN;
    lx_hdr.FormatLevel = 0;
    lx_hdr.CpuType = CPU_TYPE_386;
    lx_hdr.OsType = OS_TYPE_OS2;
    lx_hdr.PageSize = PAGE_SIZE;

    {
        size_t total_section_size_to_write = 0;
        size_t object_page_table_entries = 0;

        lx_hdr.DataPagesOffset = file_size = size_of_headers;

        for (section = all_sections; section; section = section->next) {
            if (!section->is_bss) total_section_size_to_write += section->total_size;

            if (ld_state->entry_point >= section->rva
                && ld_state->entry_point < section->rva + section->total_size) {
                lx_hdr.EipObjectIndex = object_page_table_entries + 1;
                lx_hdr.Eip = ld_state->entry_point - section->rva;
            }

            object_page_table_entries += (section->total_size / lx_hdr.PageSize) + !!(section->total_size % lx_hdr.PageSize);
        }

        file_size += total_section_size_to_write;
        lx_hdr.ObjectTableOffsetHdr = file_size - lx_hdr_offset;
        lx_hdr.NumberOfObjectsInModule = section_count ();
        file_size += lx_hdr.NumberOfObjectsInModule * sizeof (struct object_table_entry_file);
        lx_hdr.ObjectPageTableOffsetHdr = file_size;
        file_size += object_page_table_entries * sizeof (struct object_page_table_entry_file);
    }

    file = xmalloc (file_size);
    memset (file, 0, file_size);

    pos = file + size_of_headers;

    write_sections (file, pos, &lx_hdr, lx_hdr_offset);

    pos = file;

    memset (&dos_hdr, 0, sizeof (dos_hdr));

    dos_hdr.Magic[0] = 'M';
    dos_hdr.Magic[1] = 'Z';

    dos_hdr.SizeOfHeaderInParagraphs = sizeof (struct IMAGE_DOS_HEADER_file) / IMAGE_DOS_HEADER_PARAGRAPH_SIZE;

    dos_hdr.OffsetToNewEXEHeader = sizeof (struct IMAGE_DOS_HEADER_file);

    write_struct_IMAGE_DOS_HEADER (pos, &dos_hdr);
    pos += sizeof (struct IMAGE_DOS_HEADER_file);

    write_struct_LX_HEADER (pos, &lx_hdr);

    if (fwrite (file, file_size, 1, outfile) != 1) {
        ld_error ("writing '%s' file failed", filename);
    }

    free (file);
    fclose (outfile);
}
