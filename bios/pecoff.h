/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pecoff.h - header file for PE/COFF support                       */
/*                                                                   */
/*********************************************************************/

/* In image files there is signature "PE\0\0" at e_lfanew
 * after which is the COFF header. */
typedef struct {
    unsigned short Machine;
    unsigned short NumberOfSections;
    unsigned long TimeDateStamp;
    unsigned long PointerToSymbolTable; /* Deprecated, ignore. */
    unsigned long NumberOfSymbols; /* Deprecated, ignore. */
    unsigned short SizeOfOptionalHeader;
    unsigned short Characteristics;
} Coff_hdr;

/* Machine values. */
#define IMAGE_FILE_MACHINE_UNKNOWN 0
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_MACHINE_I386 0x14c

/* Characteristics flags. */
#define IMAGE_FILE_RELOCS_STRIPPED         0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE        0x0002
#define IMAGE_FILE_LARGE_ADDRESS_AWARE     0x0020
/* Reserved flag 0x0040. */
#define IMAGE_FILE_32BIT_MACHINE           0x0100
#define IMAGE_FILE_DEBUG_STRIPPED          0x0200
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP 0x0400
#define IMAGE_FILE_NET_RUN_FROM_SWAP       0x0800
#define IMAGE_FILE_SYSTEM                  0x1000
#define IMAGE_FILE_DLL                     0x2000
#define IMAGE_FILE_UP_SYSTEM_ONLY          0x4000

/* PE/COFF optional header (magic is MAGIC_PE32). */
typedef struct {
    unsigned short Magic;
    unsigned char MajorLinkerVersion;
    unsigned char MinorLinkerVersion;
    unsigned long SizeOfCode;
    unsigned long SizeOfInitializedData;
    unsigned long SizeOfUninitializedData;
    unsigned long AddressOfEntryPoint; /* Relative to ImageBase. */
    unsigned long BaseOfCode;
#ifndef TARGET_64BIT
    unsigned long BaseOfData;
#endif
    /* Extension fields. */
#ifdef W32HACK
    unsigned long ImageBase;
#else
    unsigned char *ImageBase;
#endif
    unsigned long SectionAlignment;
    unsigned long FileAlignment;
    unsigned short MajorOperatingSystemVersion;
    unsigned short MinorOperatingSystemVersion;
    unsigned short MajorImageVersion;
    unsigned short MinorImageVersion;
    unsigned short MajorSubsystemVersion;
    unsigned short MinorSubsystemVersion;
    unsigned long Win32VersionValue; /* Reserved, should be 0. */
    unsigned long SizeOfImage;
    unsigned long SizeOfHeaders;
    unsigned long CheckSum;
    unsigned short Subsystem;
    unsigned short DllCharacteristics;
    unsigned long SizeOfStackReserve;
#ifdef TARGET_64BIT
    unsigned long dummy1;
#endif
    unsigned long SizeOfStackCommit;
#ifdef TARGET_64BIT
    unsigned long dummy2;
#endif
    unsigned long SizeOfHeapReserve;
#ifdef TARGET_64BIT
    unsigned long dummy3;
#endif
    unsigned long SizeOfHeapCommit;
#ifdef TARGET_64BIT
    unsigned long dummy4;
#endif
    unsigned long LoaderFlags; /* Reserved, should be 0. */
    unsigned long NumberOfRvaAndSizes; /* Number of data directories. */
} Pe32_optional_hdr;

#define MAGIC_PE32     0x10B
#define MAGIC_PE32PLUS 0x20B

typedef struct {
    unsigned long VirtualAddress;
    unsigned long Size;
} IMAGE_DATA_DIRECTORY;

/* Indexes of data directories. */
#define DATA_DIRECTORY_EXPORT_TABLE 0
#define DATA_DIRECTORY_IMPORT_TABLE 1
#define DATA_DIRECTORY_REL 5

typedef struct {
    unsigned long Characteristics; /* Reserved. */
    unsigned long TimeDateStamp;
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    unsigned long Name;
    unsigned long Base; /* Subtract from ordinals from Import tables. */
    unsigned long NumberOfFunctions;
    unsigned long NumberOfNames; /* How many functions are exported by name. */
    unsigned long AddressOfFunctions;
    unsigned long AddressOfNames;
    unsigned long AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY;

typedef struct {
    unsigned long OriginalFirstThunk;
    unsigned long TimeDateStamp;
    unsigned long ForwarderChain;
    unsigned long Name; /* DLL name RVA. */
    unsigned long FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR;

typedef struct {
    unsigned long PageRva;
    unsigned long BlockSize; /* Number of all bytes in the block. */
} Base_relocation_block;

/* Relocation types. */
#define IMAGE_REL_BASED_ABSOLUTE 0 /* Skip this relocation. */
#define IMAGE_REL_BASED_HIGHLOW  3
#define IMAGE_REL_BASED_DIR64 10

typedef struct {
    unsigned char Name[8];
    unsigned long VirtualSize;
    unsigned long VirtualAddress;
    unsigned long SizeOfRawData;
    unsigned long PointerToRawData;
    unsigned long PointerToRelocations;
    unsigned long PointerToLinenumbers; /* Deprecated, ignore. */
    unsigned short NumberOfRelocations;
    unsigned short NumberOfLinenumbers; /* Deprecated, ignore. */
    unsigned long Characteristics;
} Coff_section;

/* Section Characteristics. */
#define IMAGE_SCN_CNT_CODE           0x00000020
#define IMAGE_SCN_INITIALIZED_DATA   0x00000040
#define IMAGE_SCN_UNINITIALIZED_DATA 0x00000080

