/******************************************************************************
 * @file            coff_bytearray.c
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
#include "coff_bytearray.h"

#define COPY(struct_name, field_name, bytes) \
 bytearray_read_##bytes##_bytes (&struct_name##_internal->field_name, struct_name##_file->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_internal->field_name, struct_name##_file->field_name, sizeof (struct_name##_file->field_name))

void read_struct_coff_header (struct coff_header_internal *coff_header_internal, void *memory)
{
    struct coff_header_file *coff_header_file = memory;

    COPY(coff_header, Machine, 2);
    COPY(coff_header, NumberOfSections, 2);
    COPY(coff_header, TimeDateStamp, 4);
    COPY(coff_header, PointerToSymbolTable, 4);
    COPY(coff_header, NumberOfSymbols, 4);
    COPY(coff_header, SizeOfOptionalHeader, 2);
    COPY(coff_header, Characteristics, 2);
}

void read_struct_section_table_entry (struct section_table_entry_internal *section_table_entry_internal, void *memory)
{
    struct section_table_entry_file *section_table_entry_file = memory;

    COPY_CHAR_ARRAY (section_table_entry, Name);

    COPY(section_table_entry, VirtualSize, 4);
    COPY(section_table_entry, VirtualAddress, 4);
    COPY(section_table_entry, SizeOfRawData, 4);
    COPY(section_table_entry, PointerToRawData, 4);
    COPY(section_table_entry, PointerToRelocations, 4);
    COPY(section_table_entry, PointerToLinenumbers, 4);
    COPY(section_table_entry, NumberOfRelocations, 2);
    COPY(section_table_entry, NumberOfLinenumbers, 2);
    COPY(section_table_entry, Characteristics, 4);
}

void read_struct_relocation_entry (struct relocation_entry_internal *relocation_entry_internal, void *memory)
{
    struct relocation_entry_file *relocation_entry_file = memory;
    
    COPY(relocation_entry, VirtualAddress, 4);
    COPY(relocation_entry, SymbolTableIndex, 4);
    COPY(relocation_entry, Type, 2);
}

void read_struct_symbol_table_entry (struct symbol_table_entry_internal *symbol_table_entry_internal, void *memory)
{
    struct symbol_table_entry_file *symbol_table_entry_file = memory;

    COPY_CHAR_ARRAY (symbol_table_entry, Name);

    COPY(symbol_table_entry, Value, 4);

    {
        /* SectionNumber is signed short using two's complement (hopefully)
         * but reading in signed numbers is a problem, so this needs to be done. */
        unsigned short SectionNumber;

        bytearray_read_2_bytes (&SectionNumber, symbol_table_entry_file->SectionNumber, LITTLE_ENDIAN);
        if (SectionNumber & 0x8000) {
            symbol_table_entry_internal->SectionNumber = -(short)(~(SectionNumber - 1));
        } else symbol_table_entry_internal->SectionNumber = SectionNumber;

    }
    
    COPY(symbol_table_entry, Type, 2);
    COPY(symbol_table_entry, StorageClass, 1);
    COPY(symbol_table_entry, NumberOfAuxSymbols, 1);
}

void read_struct_string_table_header (struct string_table_header_internal *string_table_header_internal, void *memory)
{
    struct string_table_header_file *string_table_header_file = memory;

    COPY(string_table_header, StringTableSize, 4);
}

void read_struct_IMAGE_ARCHIVE_MEMBER_HEADER (struct IMAGE_ARCHIVE_MEMBER_HEADER_internal *IMAGE_ARCHIVE_MEMBER_HEADER_internal, void *memory)
{
    struct IMAGE_ARCHIVE_MEMBER_HEADER_file *IMAGE_ARCHIVE_MEMBER_HEADER_file = memory;

    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, Name);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, Date);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, UserID);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, GroupID);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, Mode);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, Size);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, EndOfHeader);
}

#undef COPY_CHAR_ARRAY
#undef COPY

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file->field_name, struct_name##_internal->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_file->field_name, struct_name##_internal->field_name, sizeof (struct_name##_file->field_name))

void write_struct_IMAGE_DOS_HEADER (void *memory, struct IMAGE_DOS_HEADER_internal *IMAGE_DOS_HEADER_internal)
{
    struct IMAGE_DOS_HEADER_file *IMAGE_DOS_HEADER_file = memory;

    COPY_CHAR_ARRAY (IMAGE_DOS_HEADER, Magic);
    
    COPY(IMAGE_DOS_HEADER, dummy1, 2);
    COPY(IMAGE_DOS_HEADER, dummy2, 2);
    COPY(IMAGE_DOS_HEADER, dummy3, 2);
    COPY(IMAGE_DOS_HEADER, SizeOfHeaderInParagraphs, 2);

    COPY_CHAR_ARRAY (IMAGE_DOS_HEADER, dummy4);

    COPY(IMAGE_DOS_HEADER, OffsetToNewEXEHeader, 4);
}

void write_struct_coff_header (void *memory, struct coff_header_internal *coff_header_internal)
{
    struct coff_header_file *coff_header_file = memory;

    COPY(coff_header, Machine, 2);
    COPY(coff_header, NumberOfSections, 2);
    COPY(coff_header, TimeDateStamp, 4);
    COPY(coff_header, PointerToSymbolTable, 4);
    COPY(coff_header, NumberOfSymbols, 4);
    COPY(coff_header, SizeOfOptionalHeader, 2);
    COPY(coff_header, Characteristics, 2);
}

void write_struct_optional_header (void *memory, struct optional_header_internal *optional_header_internal)
{
    struct optional_header_file *optional_header_file = memory;

    COPY(optional_header, Magic, 2);
    COPY(optional_header, MajorLinkerVersion, 1);
    COPY(optional_header, MinorLinkerVersion, 1);
    COPY(optional_header, SizeOfCode, 4);
    COPY(optional_header, SizeOfInitializedData, 4);
    COPY(optional_header, SizeOfUninitializedData, 4);
    COPY(optional_header, AddressOfEntryPoint, 4);
    COPY(optional_header, BaseOfCode, 4);
    COPY(optional_header, BaseOfData, 4);

    COPY(optional_header, ImageBase, 4);
    COPY(optional_header, SectionAlignment, 4);
    COPY(optional_header, FileAlignment, 4);
    COPY(optional_header, MajorOperatingSystemVersion, 2);
    COPY(optional_header, MinorOperatingSystemVersion, 2);
    COPY(optional_header, MajorImageVersion, 2);
    COPY(optional_header, MinorImageVersion, 2);
    COPY(optional_header, MajorSubsystemVersion, 2);
    COPY(optional_header, MinorSubsystemVersion, 2);
    COPY(optional_header, Win32VersionValue, 4);
    COPY(optional_header, SizeOfImage, 4);
    COPY(optional_header, SizeOfHeaders, 4);
    COPY(optional_header, CheckSum, 4);
    COPY(optional_header, Subsystem, 2);
    COPY(optional_header, DllCharacteristics, 2);
    COPY(optional_header, SizeOfStackReserve, 4);
    COPY(optional_header, SizeOfStackCommit, 4);
    COPY(optional_header, SizeOfHeapReserve, 4);
    COPY(optional_header, SizeOfHeapCommit, 4);
    COPY(optional_header, LoaderFlags, 4);
    COPY(optional_header, NumberOfRvaAndSizes, 4);
}

void write_struct_IMAGE_DATA_DIRECTORY (void *memory, struct IMAGE_DATA_DIRECTORY_internal *IMAGE_DATA_DIRECTORY_internal)
{
    struct IMAGE_DATA_DIRECTORY_file *IMAGE_DATA_DIRECTORY_file = memory;

    COPY(IMAGE_DATA_DIRECTORY, VirtualAddress, 4);
    COPY(IMAGE_DATA_DIRECTORY, Size, 4);
}

void write_struct_section_table_entry (void *memory, struct section_table_entry_internal *section_table_entry_internal)
{
    struct section_table_entry_file *section_table_entry_file = memory;

    COPY_CHAR_ARRAY(section_table_entry, Name);

    COPY(section_table_entry, VirtualSize, 4);
    COPY(section_table_entry, VirtualAddress, 4);
    COPY(section_table_entry, SizeOfRawData, 4);
    COPY(section_table_entry, PointerToRawData, 4);
    COPY(section_table_entry, PointerToRelocations, 4);
    COPY(section_table_entry, PointerToLinenumbers, 4);
    COPY(section_table_entry, NumberOfRelocations, 2);
    COPY(section_table_entry, NumberOfLinenumbers, 2);
    COPY(section_table_entry, Characteristics, 4);
}

void write_struct_IMAGE_BASE_RELOCATION (void *memory, struct IMAGE_BASE_RELOCATION_internal *IMAGE_BASE_RELOCATION_internal)
{
    struct IMAGE_BASE_RELOCATION_file *IMAGE_BASE_RELOCATION_file = memory;

    COPY(IMAGE_BASE_RELOCATION, RVAOfBlock, 4);
    COPY(IMAGE_BASE_RELOCATION, SizeOfBlock, 4);
}

#undef COPY_CHAR_ARRAY
#undef COPY

