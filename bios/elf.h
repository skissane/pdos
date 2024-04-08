/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  elf.h - header file for ELF support                              */
/*                                                                   */
/*********************************************************************/

/* Fixed size data types. All of them except Elf32_Half must be 4 bytes. */

#ifdef __LONG64__
typedef unsigned int Elf32_Addr;
typedef unsigned int Elf32_Off;
typedef unsigned int Elf32_Word;
typedef signed int Elf32_Sword;
#else
typedef unsigned long Elf32_Addr;
typedef unsigned long Elf32_Off;
typedef unsigned long Elf32_Word;
typedef signed long Elf32_Sword;
#endif

typedef unsigned short Elf32_Half; /* 2 bytes. */

#define EI_NIDENT 16 /* Size of e_ident on all systems. */

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

/* e_ident index and value macros. */
#define EI_MAG0 0 /* 0x7f */
#define EI_MAG1 1 /* 'E' */
#define EI_MAG2 2 /* 'L' */
#define EI_MAG3 3 /* 'F' */
#define EI_CLASS 4
    #define ELFCLASSNONE 0 /* Invalid value. */
    #define ELFCLASS32 1 /* 32-bit program. */
    #define ELFCLASS64 2 /* 64-bit program. */
#define EI_DATA 5
    #define ELFDATANONE 0 /* Invalid value. */
    #define ELFDATA2LSB 1 /* Little-endian encoding. */
    #define ELFDATA2MSB 2 /* Big-endian encoding. */
#define EI_VERSION 6
#define EI_OSABI 7
    #define ELFOSABI_NONE 0 /* No specific extensions. */
    #define ELFOSABI_ARM 97
#define EI_ABIVERSION 8
#define EI_PAD 9 /* Start of pad bytes. Should be ignored. */

/* e_type value macros. */
#define ET_NONE 0
#define ET_REL 1 /* Relocatable file. */
#define ET_EXEC 2 /* Executable file. */
#define ET_DYN 3 /* Shared object file. */
#define ET_CORE 4 /* Core file. */

/* e_machine value macros. */
#define EM_386 3 /* Intel 80386. */
#define EM_ARM 40
#define EM_X86_64 62 /* x86-64. */

typedef struct {
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;

/* Segment types. */
#define PT_NULL 0
#define PT_LOAD 1

typedef struct {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

/* Special section indexes. */
#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00 /* Start of reserved indexes. */
#define SHN_ABS 0xfff1
#define SHN_XINDEX 0xffff

/* Section types. */
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_NOBITS 8
#define SHT_REL 9

/* Section flags. */
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20

typedef struct {
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half st_shndx;
} Elf32_Sym;

#define STN_UNDEF 0

typedef struct {
    Elf32_Addr r_offset;
    Elf32_Word r_info; /* Top byte is symbol index, bottom is rel. type. */
} Elf32_Rel;

typedef struct {
    Elf32_Addr r_offset;
    Elf32_Word r_info; /* Top byte is symbol index, bottom is rel. type. */
    Elf32_Sword r_addend;
} Elf32_Rela;

#define ELF32_R_SYM(r_info) ((r_info) >> 8)
#define ELF32_R_TYPE(r_info) ((r_info) & 0xff)

/* Relocation types (bottom byte of r_info). */
#define R_386_NONE 0
#define R_386_32 1
#define R_386_PC32 2

#define R_ARM_NONE 0
#define R_ARM_PC24 1
#define R_ARM_ABS32 2

