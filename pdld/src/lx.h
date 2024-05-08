/******************************************************************************
 * @file            lx.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
struct LX_HEADER_internal {
    /* Some offsets are relative to the beginning of the file
     * while other offsets are relative to the beginning of the LX_HEADER
     * (the latter are marked by "Hdr" suffix).
     */
    
    unsigned char Magic[2]; /* "LX" */
    
#define BORD_LITTLE_ENDIAN  0x0
#define BORD_BIG_ENDIAN     0x1
    unsigned char BOrd;
    
#define WORD_LITTLE_ENDIAN  0x0
#define WORD_BIG_ENDIAN     0x1
    unsigned char WOrd;
    unsigned long FormatLevel;
    
#define CPU_TYPE_286  0x1
#define CPU_TYPE_386  0x2
#define CPU_TYPE_486  0x3
    unsigned short CpuType;
    
#define OS_TYPE_UNKNOWN      0x0
#define OS_TYPE_OS2          0x1
#define OS_TYPE_WINDOWS      0x2
#define OS_TYPE_DOS_4X       0x3
#define OS_TYPE_WINDOWS_386  0x4
#define OS_TYPE_IBM_MICRO    0x5
    unsigned short OsType;
    
    unsigned long ModuleVersion;
    unsigned long ModuleFlags;
    unsigned long ModuleNumberOfPages;
    unsigned long EipObjectIndex;
    unsigned long Eip;
    unsigned long EspObjectIndex;
    unsigned long Esp;
    unsigned long PageSize;
#define PAGE_SIZE  0x1000
    unsigned long PageOffsetShift;
    unsigned long FixupSectionSize;
    unsigned long FixupSectionChecksum;
    unsigned long LoaderSectionSize;
    unsigned long LoaderSectionChecksum;
    unsigned long ObjectTableOffsetHdr;
    unsigned long NumberOfObjectsInModule;
    unsigned long ObjectPageTableOffsetHdr;
    unsigned long ObjectIterPageOffset;
    unsigned long ResourceTableOffsetHdr;
    unsigned long NumberOfResourceTableEntries;
    unsigned long ResidentNameTableOffsetHdr;
    unsigned long EntryTableOffsetHdr;
    unsigned long ModuleDirectivesOffsetHdr;
    unsigned long NumberOfModuleDirectives;
    
    unsigned long FixupPageTableOffsetHdr;
    unsigned long FixupRecordTableOffsetHdr;
    
    unsigned long ImportModuleTableOffsetHdr;
    unsigned long NumberOfImportModEntries;
    unsigned long ImportProcTableOffsetHdr;
    unsigned long PerPageChecksumOffsetHdr;
    
    unsigned long DataPagesOffset;
    
    unsigned long NumberOfPreloadPages;
    unsigned long NonResNameTableOffset;
    unsigned long NonResNameTableLength;
    unsigned long NonResNameTableChecksum;
    unsigned long AutoDataSegmentObjectIndex;
    unsigned long DebugInfoOffset;
    unsigned long DebugInfoLength;
    unsigned long NumberOfInstancePreloadPages;
    unsigned long NumberOfInstanceDemandPages;
    unsigned long Heapsize;
    unsigned long Stacksize;
    
};

#define SIZEOF_struct_LX_HEADER_file 176
struct LX_HEADER_file {
    
    unsigned char Magic[2]; /* "LX" */
    
    unsigned char BOrd[1];
    unsigned char WOrd[1];
    unsigned char FormatLevel[4];

    unsigned char CpuType[2];
    unsigned char OsType[2];
    unsigned char ModuleVersion[4];
    
    unsigned char ModuleFlags[4];
    unsigned char ModuleNumberOfPages[4];
    unsigned char EipObjectIndex[4];
    unsigned char Eip[4];
    unsigned char EspObjectIndex[4];
    unsigned char Esp[4];
    unsigned char PageSize[4];
    unsigned char PageOffsetShift[4];
    unsigned char FixupSectionSize[4];
    unsigned char FixupSectionChecksum[4];
    unsigned char LoaderSectionSize[4];
    unsigned char LoaderSectionChecksum[4];
    unsigned char ObjectTableOffsetHdr[4];
    unsigned char NumberOfObjectsInModule[4];
    unsigned char ObjectPageTableOffsetHdr[4];
    unsigned char ObjectIterPageOffset[4];
    unsigned char ResourceTableOffsetHdr[4];
    unsigned char NumberOfResourceTableEntries[4];
    unsigned char ResidentNameTableOffsetHdr[4];
    unsigned char EntryTableOffsetHdr[4];
    unsigned char ModuleDirectivesOffsetHdr[4];
    unsigned char NumberOfModuleDirectives[4];
    
    unsigned char FixupPageTableOffsetHdr[4];
    unsigned char FixupRecordTableOffsetHdr[4];
    
    unsigned char ImportModuleTableOffsetHdr[4];
    unsigned char NumberOfImportModEntries[4];
    unsigned char ImportProcTableOffsetHdr[4];
    unsigned char PerPageChecksumOffsetHdr[4];
    
    unsigned char DataPagesOffset[4];
    
    unsigned char NumberOfPreloadPages[4];
    unsigned char NonResNameTableOffset[4];
    unsigned char NonResNameTableLength[4];
    unsigned char NonResNameTableChecksum[4];
    unsigned char AutoDataSegmentObjectIndex[4];
    unsigned char DebugInfoOffset[4];
    unsigned char DebugInfoLength[4];
    unsigned char NumberOfInstancePreloadPages[4];
    unsigned char NumberOfInstanceDemandPages[4];
    unsigned char Heapsize[4];
    unsigned char Stacksize[4];
    
};

#define MODULE_FLAGS_PER_PROCESS_LIBRARY_INITIALIZATION  0x00000004
#define MODULE_FLAGS_INTERNAL_FIXUPS_REMOVED             0x00000010
#define MODULE_FLAGS_EXTERNAL_FIXUPS_REMOVED             0x00000020
#define MODULE_FLAGS_PM_WINDOWING_INCOMPATIBLE           0x00000100
#define MODULE_FLAGS_PM_WINDOWING_COMPATIBLE             0x00000200
#define MODULE_FLAGS_PM_WINDOWING_USES_API               0x00000300
#define MODULE_FLAGS_NOT_LOADABLE                        0x00002000
#define MODULE_FLAGS_EXECUTABLE_MODULE                   0x00000000
#define MODULE_FLAGS_LIBRARY_MODULE                      0x00008000
#define MODULE_FLAGS_PDD_MODULE                          0x00020000
#define MODULE_FLAGS_VDD_MODULE                          0x00028000
#define MODULE_FLAGS_DLD_MODULE                          0x00030000
#define MODULE_FLAGS_MP_UNSAFE                           0x00080000
#define MODULE_FLAGS_PER_PROCESS_LIBRARY_TERMINATION     0x40000000

struct object_table_entry_internal {

    unsigned long VirtualSize;
    unsigned long RelocationBaseAddress;
    unsigned long ObjectFlags;
    unsigned long PageTableIndex; /* 1-based. */
    unsigned long NumberOfPageTableEntries;
    unsigned long Reserved;

};

#define SIZEOF_struct_object_table_entry_file 24
struct object_table_entry_file {

    unsigned char VirtualSize[4];
    unsigned char RelocationBaseAddress[4];
    unsigned char ObjectFlags[4];
    unsigned char PageTableIndex[4];
    unsigned char NumberOfPageTableEntries[4];
    unsigned char Reserved[4];

};

#define OBJECT_FLAGS_MEM_READ                 0x0001
#define OBJECT_FLAGS_MEM_WRITE                0x0002
#define OBJECT_FLAGS_MEM_EXECUTE              0x0004
#define OBJECT_FLAGS_RESOURCE                 0x0008
#define OBJECT_FLAGS_DISCARDABLE              0x0010
#define OBJECT_FLAGS_SHARED                   0x0020
#define OBJECT_FLAGS_HAS_PRELOAD_PAGES        0x0040
#define OBJECT_FLAGS_HAS_INVALID_PAGES        0x0080
#define OBJECT_FLAGS_RESIDENT                 0x0100
#define OBJECT_FLAGS_RESIDENT_CONTIGUOUS      0x0300
#define OBJECT_FLAGS_RESIDENT_LONG_LOCKABLE   0x0400
#define OBJECT_FLAGS_IBM_MICRO_EXTENSION      0x0800
#define OBJECT_FLAGS_16_16_ALIAS_NEEDED       0x1000
#define OBJECT_FLAGS_BIG_DEFAULT_BIT_SETTING  0x2000
#define OBJECT_FLAGS_CONFORMING_FOR_CODE      0x4000
#define OBJECT_FLAGS_IO_PRIVILEGE_LEVEL       0x8000

struct object_page_table_entry_internal {

    unsigned long PageDataOffset;
    unsigned short DataSize;
    unsigned short Flags;

};

#define SIZEOF_struct_object_page_table_entry_file 8
struct object_page_table_entry_file {

    unsigned char PageDataOffset[4];
    unsigned char DataSize[2];
    unsigned char Flags[2];

};

#define FIXUP_TARGET_FLAGS_INTERNAL_REFERENCE    0x00
#define FIXUP_TARGET_FLAGS_IMPORT_BY_ORDINAL     0x01
#define FIXUP_TARGET_FLAGS_IMPORT_BY_NAME        0x02
#define FIXUP_TARGET_FLAGS_INTERNAL_ENTRY_TABLE  0x03
#define FIXUP_TARGET_FLAGS_ADDITIVE              0x04
#define FIXUP_TARGET_FLAGS_INTERNAL_CHAINING     0x08
#define FIXUP_TARGET_FLAGS_32_BIT_OFFSET         0x10
#define FIXUP_TARGET_FLAGS_32_BIT_ADDITIVE       0x20
#define FIXUP_TARGET_FLAGS_16_BIT_OBJECT_NUMBER  0x40
#define FIXUP_TARGET_FLAGS_8_BIT_ORDINAL         0x80
