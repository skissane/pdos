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

void read_struct_IMAGE_DOS_HEADER (struct IMAGE_DOS_HEADER_internal *IMAGE_DOS_HEADER_internal, const void *memory)
{
    const struct IMAGE_DOS_HEADER_file *IMAGE_DOS_HEADER_file = memory;

    COPY_CHAR_ARRAY (IMAGE_DOS_HEADER, Magic);
    
    COPY(IMAGE_DOS_HEADER, dummy1, 2);
    COPY(IMAGE_DOS_HEADER, dummy2, 2);
    COPY(IMAGE_DOS_HEADER, dummy3, 2);
    COPY(IMAGE_DOS_HEADER, SizeOfHeaderInParagraphs, 2);
    COPY(IMAGE_DOS_HEADER, MinimumExtraParagraphs, 2);
    COPY(IMAGE_DOS_HEADER, MaximumExtraParagraphs, 2);
    COPY(IMAGE_DOS_HEADER, InitialSs, 2);
    COPY(IMAGE_DOS_HEADER, InitialSp, 2);
    COPY(IMAGE_DOS_HEADER, Checksum, 2);
    COPY(IMAGE_DOS_HEADER, InitialIp, 2);
    COPY(IMAGE_DOS_HEADER, InitialCs, 2);
    COPY(IMAGE_DOS_HEADER, OffsetToRelocationTable, 2);
    COPY(IMAGE_DOS_HEADER, OverlayNumber, 2);

    COPY_CHAR_ARRAY (IMAGE_DOS_HEADER, dummy4);

    COPY(IMAGE_DOS_HEADER, OffsetToNewEXEHeader, 4);
}

void read_struct_coff_header (struct coff_header_internal *coff_header_internal, const void *memory)
{
    const struct coff_header_file *coff_header_file = memory;

    COPY(coff_header, Machine, 2);
    COPY(coff_header, NumberOfSections, 2);
    COPY(coff_header, TimeDateStamp, 4);
    COPY(coff_header, PointerToSymbolTable, 4);
    COPY(coff_header, NumberOfSymbols, 4);
    COPY(coff_header, SizeOfOptionalHeader, 2);
    COPY(coff_header, Characteristics, 2);
}

void read_struct_optional_header (struct optional_header_internal *optional_header_internal, const void *memory)
{
    const struct optional_header_file *optional_header_file = memory;

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

void read_struct_optional_header_plus (struct optional_header_plus_internal *optional_header_plus_internal, const void *memory)
{
    const struct optional_header_plus_file *optional_header_plus_file = memory;

    COPY(optional_header_plus, Magic, 2);
    COPY(optional_header_plus, MajorLinkerVersion, 1);
    COPY(optional_header_plus, MinorLinkerVersion, 1);
    COPY(optional_header_plus, SizeOfCode, 4);
    COPY(optional_header_plus, SizeOfInitializedData, 4);
    COPY(optional_header_plus, SizeOfUninitializedData, 4);
    COPY(optional_header_plus, AddressOfEntryPoint, 4);
    COPY(optional_header_plus, BaseOfCode, 4);

    COPY(optional_header_plus, ImageBase, 8);
    
    COPY(optional_header_plus, SectionAlignment, 4);
    COPY(optional_header_plus, FileAlignment, 4);
    COPY(optional_header_plus, MajorOperatingSystemVersion, 2);
    COPY(optional_header_plus, MinorOperatingSystemVersion, 2);
    COPY(optional_header_plus, MajorImageVersion, 2);
    COPY(optional_header_plus, MinorImageVersion, 2);
    COPY(optional_header_plus, MajorSubsystemVersion, 2);
    COPY(optional_header_plus, MinorSubsystemVersion, 2);
    COPY(optional_header_plus, Win32VersionValue, 4);
    COPY(optional_header_plus, SizeOfImage, 4);
    COPY(optional_header_plus, SizeOfHeaders, 4);
    COPY(optional_header_plus, CheckSum, 4);
    COPY(optional_header_plus, Subsystem, 2);
    COPY(optional_header_plus, DllCharacteristics, 2);
    
    COPY(optional_header_plus, SizeOfStackReserve, 8);
    COPY(optional_header_plus, SizeOfStackCommit, 8);
    COPY(optional_header_plus, SizeOfHeapReserve, 8);
    COPY(optional_header_plus, SizeOfHeapCommit, 8);
    
    COPY(optional_header_plus, LoaderFlags, 4);
    COPY(optional_header_plus, NumberOfRvaAndSizes, 4);
}

void read_struct_IMAGE_DATA_DIRECTORY (struct IMAGE_DATA_DIRECTORY_internal *IMAGE_DATA_DIRECTORY_internal, const void *memory)
{
    const struct IMAGE_DATA_DIRECTORY_file *IMAGE_DATA_DIRECTORY_file = memory;

    COPY(IMAGE_DATA_DIRECTORY, VirtualAddress, 4);
    COPY(IMAGE_DATA_DIRECTORY, Size, 4);
}

void read_struct_section_table_entry (struct section_table_entry_internal *section_table_entry_internal, const void *memory)
{
    const struct section_table_entry_file *section_table_entry_file = memory;

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

void read_struct_relocation_entry (struct relocation_entry_internal *relocation_entry_internal, const void *memory)
{
    const struct relocation_entry_file *relocation_entry_file = memory;
    
    COPY(relocation_entry, VirtualAddress, 4);
    COPY(relocation_entry, SymbolTableIndex, 4);
    COPY(relocation_entry, Type, 2);
}

void read_struct_symbol_table_entry (struct symbol_table_entry_internal *symbol_table_entry_internal, const void *memory)
{
    const struct symbol_table_entry_file *symbol_table_entry_file = memory;

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

void read_struct_aux_section_symbol (struct aux_section_symbol_internal *aux_section_symbol_internal, const void *memory)
{
    const struct aux_section_symbol_file *aux_section_symbol_file = memory;

    COPY(aux_section_symbol, Length, 4);
    COPY(aux_section_symbol, NumberOfRelocations, 2);
    COPY(aux_section_symbol, NumberOfLinenumbers, 2);

    COPY(aux_section_symbol, CheckSum, 4);
    COPY(aux_section_symbol, Number, 2);
    COPY(aux_section_symbol, Selection, 1);

    COPY_CHAR_ARRAY(aux_section_symbol, Unused);
}

void read_struct_string_table_header (struct string_table_header_internal *string_table_header_internal, const void *memory)
{
    const struct string_table_header_file *string_table_header_file = memory;

    COPY(string_table_header, StringTableSize, 4);
}

void read_struct_IMAGE_BASE_RELOCATION (struct IMAGE_BASE_RELOCATION_internal *IMAGE_BASE_RELOCATION_internal, const void *memory)
{
    const struct IMAGE_BASE_RELOCATION_file *IMAGE_BASE_RELOCATION_file = memory;

    COPY(IMAGE_BASE_RELOCATION, RVAOfBlock, 4);
    COPY(IMAGE_BASE_RELOCATION, SizeOfBlock, 4);
}

void read_struct_IMAGE_ARCHIVE_MEMBER_HEADER (struct IMAGE_ARCHIVE_MEMBER_HEADER_internal *IMAGE_ARCHIVE_MEMBER_HEADER_internal, const void *memory)
{
    const struct IMAGE_ARCHIVE_MEMBER_HEADER_file *IMAGE_ARCHIVE_MEMBER_HEADER_file = memory;

    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, Name);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, Date);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, UserID);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, GroupID);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, Mode);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, Size);
    COPY_CHAR_ARRAY(IMAGE_ARCHIVE_MEMBER_HEADER, EndOfHeader);
}

void read_struct_IMPORT_OBJECT_HEADER (struct IMPORT_OBJECT_HEADER_internal *IMPORT_OBJECT_HEADER_internal, const void *memory)
{
    const struct IMPORT_OBJECT_HEADER_file *IMPORT_OBJECT_HEADER_file = memory;

    COPY(IMPORT_OBJECT_HEADER, Magic1, 2);
    COPY(IMPORT_OBJECT_HEADER, Magic2, 2);
    COPY(IMPORT_OBJECT_HEADER, Version, 2);
    COPY(IMPORT_OBJECT_HEADER, Machine, 2);

    COPY(IMPORT_OBJECT_HEADER, TimeDateStamp, 4);
    COPY(IMPORT_OBJECT_HEADER, SizeOfData, 4);

    COPY(IMPORT_OBJECT_HEADER, OrdinalHint, 2);
    COPY(IMPORT_OBJECT_HEADER, Type, 2);
}

#undef COPY_CHAR_ARRAY
#undef COPY

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file->field_name, struct_name##_internal->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_file->field_name, struct_name##_internal->field_name, sizeof (struct_name##_file->field_name))

void write_struct_IMAGE_DOS_HEADER (void *memory, const struct IMAGE_DOS_HEADER_internal *IMAGE_DOS_HEADER_internal)
{
    struct IMAGE_DOS_HEADER_file *IMAGE_DOS_HEADER_file = memory;

    COPY_CHAR_ARRAY (IMAGE_DOS_HEADER, Magic);
    
    COPY(IMAGE_DOS_HEADER, dummy1, 2);
    COPY(IMAGE_DOS_HEADER, dummy2, 2);
    COPY(IMAGE_DOS_HEADER, dummy3, 2);
    COPY(IMAGE_DOS_HEADER, SizeOfHeaderInParagraphs, 2);
    COPY(IMAGE_DOS_HEADER, MinimumExtraParagraphs, 2);
    COPY(IMAGE_DOS_HEADER, MaximumExtraParagraphs, 2);
    COPY(IMAGE_DOS_HEADER, InitialSs, 2);
    COPY(IMAGE_DOS_HEADER, InitialSp, 2);
    COPY(IMAGE_DOS_HEADER, Checksum, 2);
    COPY(IMAGE_DOS_HEADER, InitialIp, 2);
    COPY(IMAGE_DOS_HEADER, InitialCs, 2);
    COPY(IMAGE_DOS_HEADER, OffsetToRelocationTable, 2);
    COPY(IMAGE_DOS_HEADER, OverlayNumber, 2);

    COPY_CHAR_ARRAY (IMAGE_DOS_HEADER, dummy4);

    COPY(IMAGE_DOS_HEADER, OffsetToNewEXEHeader, 4);
}

void write_struct_coff_header (void *memory, const struct coff_header_internal *coff_header_internal)
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

void write_struct_optional_header (void *memory, const struct optional_header_internal *optional_header_internal)
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

void write_struct_optional_header_plus (void *memory, const struct optional_header_plus_internal *optional_header_plus_internal)
{
    struct optional_header_plus_file *optional_header_plus_file = memory;

    COPY(optional_header_plus, Magic, 2);
    COPY(optional_header_plus, MajorLinkerVersion, 1);
    COPY(optional_header_plus, MinorLinkerVersion, 1);
    COPY(optional_header_plus, SizeOfCode, 4);
    COPY(optional_header_plus, SizeOfInitializedData, 4);
    COPY(optional_header_plus, SizeOfUninitializedData, 4);
    COPY(optional_header_plus, AddressOfEntryPoint, 4);
    COPY(optional_header_plus, BaseOfCode, 4);

    COPY(optional_header_plus, ImageBase, 8);
    
    COPY(optional_header_plus, SectionAlignment, 4);
    COPY(optional_header_plus, FileAlignment, 4);
    COPY(optional_header_plus, MajorOperatingSystemVersion, 2);
    COPY(optional_header_plus, MinorOperatingSystemVersion, 2);
    COPY(optional_header_plus, MajorImageVersion, 2);
    COPY(optional_header_plus, MinorImageVersion, 2);
    COPY(optional_header_plus, MajorSubsystemVersion, 2);
    COPY(optional_header_plus, MinorSubsystemVersion, 2);
    COPY(optional_header_plus, Win32VersionValue, 4);
    COPY(optional_header_plus, SizeOfImage, 4);
    COPY(optional_header_plus, SizeOfHeaders, 4);
    COPY(optional_header_plus, CheckSum, 4);
    COPY(optional_header_plus, Subsystem, 2);
    COPY(optional_header_plus, DllCharacteristics, 2);
    
    COPY(optional_header_plus, SizeOfStackReserve, 8);
    COPY(optional_header_plus, SizeOfStackCommit, 8);
    COPY(optional_header_plus, SizeOfHeapReserve, 8);
    COPY(optional_header_plus, SizeOfHeapCommit, 8);
    
    COPY(optional_header_plus, LoaderFlags, 4);
    COPY(optional_header_plus, NumberOfRvaAndSizes, 4);
}

void write_struct_IMAGE_DATA_DIRECTORY (void *memory, const struct IMAGE_DATA_DIRECTORY_internal *IMAGE_DATA_DIRECTORY_internal)
{
    struct IMAGE_DATA_DIRECTORY_file *IMAGE_DATA_DIRECTORY_file = memory;

    COPY(IMAGE_DATA_DIRECTORY, VirtualAddress, 4);
    COPY(IMAGE_DATA_DIRECTORY, Size, 4);
}

void write_struct_section_table_entry (void *memory, const struct section_table_entry_internal *section_table_entry_internal)
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


void write_struct_IMAGE_EXPORT_DIRECTORY (void *memory, const struct IMAGE_EXPORT_DIRECTORY_internal *IMAGE_EXPORT_DIRECTORY_internal)
{
    struct IMAGE_EXPORT_DIRECTORY_file *IMAGE_EXPORT_DIRECTORY_file = memory;

    COPY(IMAGE_EXPORT_DIRECTORY, ExportFlags, 4);
    COPY(IMAGE_EXPORT_DIRECTORY, TimeDateStamp, 4);
    COPY(IMAGE_EXPORT_DIRECTORY, MajorVersion, 2);
    COPY(IMAGE_EXPORT_DIRECTORY, MinorVersion, 2);
    COPY(IMAGE_EXPORT_DIRECTORY, NameRVA, 4);
    COPY(IMAGE_EXPORT_DIRECTORY, OrdinalBase, 4);
    COPY(IMAGE_EXPORT_DIRECTORY, AddressTableEntries, 4);
    COPY(IMAGE_EXPORT_DIRECTORY, NumberOfNamePointers, 4);
    COPY(IMAGE_EXPORT_DIRECTORY, ExportAddressTableRVA, 4);
    COPY(IMAGE_EXPORT_DIRECTORY, NamePointerRVA, 4);
    COPY(IMAGE_EXPORT_DIRECTORY, OrdinalTableRVA, 4);
}

void write_struct_EXPORT_Name_Pointer_Table (void *memory, const struct EXPORT_Name_Pointer_Table_internal *EXPORT_Name_Pointer_Table_internal)
{
    struct EXPORT_Name_Pointer_Table_file *EXPORT_Name_Pointer_Table_file = memory;

    COPY(EXPORT_Name_Pointer_Table, FunctionNameRVA, 4);
}

void write_struct_EXPORT_Ordinal_Table (void *memory, const struct EXPORT_Ordinal_Table_internal *EXPORT_Ordinal_Table_internal)
{
    struct EXPORT_Ordinal_Table_file *EXPORT_Ordinal_Table_file = memory;

    COPY(EXPORT_Ordinal_Table, FunctionOrdinal, 2);
}

void write_struct_IMPORT_Directory_Table (void *memory, const struct IMPORT_Directory_Table_internal *IMPORT_Directory_Table_internal)
{
    struct IMPORT_Directory_Table_file *IMPORT_Directory_Table_file = memory;

    COPY(IMPORT_Directory_Table, ImportNameTableRVA, 4);
    COPY(IMPORT_Directory_Table, TimeDateStamp, 4);
    COPY(IMPORT_Directory_Table, ForwarderChain, 4);
    COPY(IMPORT_Directory_Table, NameRVA, 4);
    COPY(IMPORT_Directory_Table, ImportAddressTableRVA, 4);
}

void write_struct_IMAGE_BASE_RELOCATION (void *memory, const struct IMAGE_BASE_RELOCATION_internal *IMAGE_BASE_RELOCATION_internal)
{
    struct IMAGE_BASE_RELOCATION_file *IMAGE_BASE_RELOCATION_file = memory;

    COPY(IMAGE_BASE_RELOCATION, RVAOfBlock, 4);
    COPY(IMAGE_BASE_RELOCATION, SizeOfBlock, 4);
}

void write_struct_IMAGE_ARCHIVE_MEMBER_HEADER (void *memory, const struct IMAGE_ARCHIVE_MEMBER_HEADER_internal *IMAGE_ARCHIVE_MEMBER_HEADER_internal)
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

void write_struct_IMPORT_OBJECT_HEADER (void *memory, const struct IMPORT_OBJECT_HEADER_internal *IMPORT_OBJECT_HEADER_internal)
{
    struct IMPORT_OBJECT_HEADER_file *IMPORT_OBJECT_HEADER_file = memory;

    COPY(IMPORT_OBJECT_HEADER, Magic1, 2);
    COPY(IMPORT_OBJECT_HEADER, Magic2, 2);
    COPY(IMPORT_OBJECT_HEADER, Version, 2);
    COPY(IMPORT_OBJECT_HEADER, Machine, 2);

    COPY(IMPORT_OBJECT_HEADER, TimeDateStamp, 4);
    COPY(IMPORT_OBJECT_HEADER, SizeOfData, 4);

    COPY(IMPORT_OBJECT_HEADER, OrdinalHint, 2);
    COPY(IMPORT_OBJECT_HEADER, Type, 2);
}

#undef COPY_CHAR_ARRAY
#undef COPY

