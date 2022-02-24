/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

struct COFF_file_header {
    unsigned short Machine;
    unsigned short NumberOfSections;
    unsigned long TimeDateStamp;
    unsigned long PointerToSymbolTable;
    unsigned long NumberOfSymbols;
    unsigned short SizeOfOptionalHeader;
    unsigned short Characteristics;
};

#define IMAGE_FILE_MACHINE_I386 0x14c

#define IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004

struct Section_Table_entry {
    char Name[8];
    unsigned long VirtualSize;
    unsigned long VirtualAddress;
    unsigned long SizeOfRawData;
    unsigned long PointerToRawData;
    unsigned long PointerToRelocations;
    unsigned long PointerToLinenumbers;
    unsigned short NumberOfRelocations;
    unsigned short NumberOfLinenumbers;
    unsigned long Characteristics;
};

#define IMAGE_SCN_CNT_CODE               0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA   0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_EXECUTE            0x20000000
#define IMAGE_SCN_MEM_READ               0x40000000
#define IMAGE_SCN_MEM_WRITE              0x80000000

struct Relocation_entry {
    unsigned long VirtualAddress;
    unsigned long SymbolTableIndex;
    unsigned short Type;
};
#define Relocation_entry_size 10

#define IMAGE_REL_I386_ABSOLUTE 0x0000
#define IMAGE_REL_I386_DIR32 0x0006
#define IMAGE_REL_I386_DIR32NB 0x0007
#define IMAGE_REL_I386_REL32 0x0014

struct Symbol_Table_entry {
    char Name[8];
    unsigned long Value;
    signed short SectionNumber;
    unsigned short Type;
    unsigned char StorageClass;
    unsigned char NumberOfAuxSymbols;
};
#define Symbol_Table_entry_size 18

#define IMAGE_SYM_UNDEFINED 0
#define IMAGE_SYM_ABSOLUTE -1
#define IMAGE_SYM_DEBUG -2

#define IMAGE_SYM_TYPE_NULL 0

#define IMAGE_SYM_DTYPE_NULL 0

#define IMAGE_SYM_CLASS_EXTERNAL 2
#define IMAGE_SYM_CLASS_STATIC 3
#define IMAGE_SYM_CLASS_FILE 103
