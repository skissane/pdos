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
