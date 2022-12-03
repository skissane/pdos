/******************************************************************************
 * @file            coff.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
struct coff_header {

    unsigned short Machine;
    unsigned short NumberOfSections;
    
    unsigned int TimeDateStamp;
    unsigned int PointerToSymbolTable;
    unsigned int NumberOfSymbols;
    
    unsigned short SizeOfOptionalHeader;
    unsigned short Characteristics;

};

#define     IMAGE_FILE_MACHINE_I386                         0x014C
#define     IMAGE_FILE_LINE_NUMS_STRIPPED                   0x0004

struct section_table_entry {

    char Name[8];
    
    unsigned int VirtualSize;
    unsigned int VirtualAddress;
    
    unsigned int SizeOfRawData;
    unsigned int PointerToRawData;
    unsigned int PointerToRelocations;
    unsigned int PointerToLinenumbers;
    
    unsigned short NumberOfRelocations;
    unsigned short NumberOfLinenumbers;
    
    unsigned int Characteristics;

};

#define     IMAGE_SCN_CNT_CODE                              0x00000020
#define     IMAGE_SCN_CNT_INITIALIZED_DATA                  0x00000040
#define     IMAGE_SCN_CNT_UNINITIALIZED_DATA                0x00000080
#define     IMAGE_SCN_LNK_INFO                              0x00000200
#define     IMAGE_SCN_MEM_EXECUTE                           0x20000000
#define     IMAGE_SCN_MEM_READ                              0x40000000
#define     IMAGE_SCN_MEM_WRITE                             0x80000000

struct relocation_entry {

    unsigned int VirtualAddress;
    unsigned int SymbolTableIndex;
    
    unsigned short Type;

};

#define     RELOCATION_ENTRY_SIZE                           10

#define     IMAGE_REL_I386_ABSOLUTE                         0x0000
#define     IMAGE_REL_I386_DIR16                            0x0001
#define     IMAGE_REL_I386_REL16                            0x0002
#define     IMAGE_REL_I386_DIR32                            0x0006
#define     IMAGE_REL_I386_DIR32NB                          0x0007
#define     IMAGE_REL_I386_REL32                            0x0014

struct symbol_table_entry {

    char Name[8];
    unsigned int Value;
    
    signed short SectionNumber;
    unsigned short Type;
    
    unsigned char StorageClass;
    unsigned char NumberOfAuxSymbols;

};

#define     SYMBOL_TABLE_ENTRY_SIZE                         18

#define     IMAGE_SYM_UNDEFINED                             0
#define     IMAGE_SYM_ABSOLUTE                              -1
#define     IMAGE_SYM_DEBUG                                 -2

#define     IMAGE_SYM_TYPE_NULL                             0
#define     IMAGE_SYM_DTYPE_NULL                            0

#define     IMAGE_SYM_CLASS_EXTERNAL                        2
#define     IMAGE_SYM_CLASS_STATIC                          3
#define     IMAGE_SYM_CLASS_LABEL                           6
#define     IMAGE_SYM_CLASS_FILE                            103
