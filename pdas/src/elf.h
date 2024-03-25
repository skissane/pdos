/******************************************************************************
 * @file            elf.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdint.h>

/* Fixed size data types. All of them except Elf32_Half must be 4 bytes. */
typedef unsigned long Elf32_Addr;
typedef unsigned long Elf32_Off;
typedef unsigned long Elf32_Word;
typedef signed long Elf32_SWord;
typedef unsigned short Elf32_Half; /* 2 bytes. */

/* Fixed size data types, short should be 2 bytes, int 4 bytes. */
typedef uint_fast64_t Elf64_Addr;
typedef uint_fast64_t Elf64_Off;
typedef unsigned short Elf64_Half;
typedef unsigned int Elf64_Word;
typedef signed int Elf64_Sword;
typedef uint_fast64_t Elf64_Xword;
typedef int_fast64_t Elf64_Sxword;

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

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry;
    Elf64_Off e_phoff;
    Elf64_Off e_shoff;
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx;
} Elf64_Ehdr;

/* e_ident index and value macros. */
#define EI_MAG0 0
#define ELFMAG0 0x7f
#define EI_MAG1 1
#define ELFMAG1 'E'
#define EI_MAG2 2
#define ELFMAG2 'L'
#define EI_MAG3 3
#define ELFMAG3 'F'
#define EI_CLASS 4
    #define ELFCLASSNONE 0 /* Invalid class. */
    #define ELFCLASS32 1 /* 32-bit object. */
    #define ELFCLASS64 2 /* 64-bit object. */
#define EI_DATA 5
    #define ELFDATANONE 0 /* Invalid data encoding. */
    #define ELFDATA2LSB 1 /* Little-endian encoding. */
    #define ELFDATA2MSB 2 /* Big-endian encoding. */
#define EI_VERSION 6 /* Must be EV_CURRENT. */
#define EI_OSABI 7
    #define ELFOSABI_NONE 0 /* No specific extensions. */
#define EI_ABIVERSION 8 /* Should be set to 0. */
#define EI_PAD 9 /* Start of pad bytes. Should be ignored. */

/* e_type */
#define ET_NONE 0
#define ET_REL 1 /* Relocatable file. */
#define ET_EXEC 2 /* Executable file. */
#define ET_DYN 3 /* Shared object file. */
#define ET_CORE 4 /* Core file. */

/* e_machine */
#define EM_NONE 0
#define EM_386 3 /* Intel 80386. */
#define EM_X86_64 62 /* x86-64. */

/* e_version */
#define EV_NONE 0
#define EV_CURRENT 1

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

typedef struct {
    Elf64_Word sh_name;
    Elf64_Word sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr sh_addr;
    Elf64_Off sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word sh_link;
    Elf64_Word sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
} Elf64_Shdr;

/* Special section indexes. */
#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff
#define SHN_XINDEX 0xffff

/* Section types. */
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11

/* Section flags. */
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20
#define SHF_INFO_LINK 0x40
#define SHF_LINK_ORDER 0x80
#define SHF_OS_NONCOMFORMING 0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_MASKOS 0x0ff00000
#define SHF_ORDERED 0x40000000
#define SHF_EXCLUDE 0x80000000
#define SHF_MASKPROC 0xf0000000

typedef struct {
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half st_shndx;
} Elf32_Sym;

typedef struct {
    Elf64_Word st_name;
    unsigned char st_info;
    unsigned char st_other;
    Elf64_Half st_shndx;
    Elf64_Addr st_value;
    Elf64_Xword st_size;
} Elf64_Sym;

#define STN_UNDEF 0

#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

/* Symbol binding. */
#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STB_LOPROC 13
#define STB_HIPROC 15

/* Symbol types. */
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_LOPROC 13
#define STT_HIPROC 15

typedef struct {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
} Elf32_Rel;

typedef struct {
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
} Elf64_Rel;

typedef struct {
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    Elf64_Sxword r_addend;
} Elf64_Rela;

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)
#define ELF64_R_INFO(s,t) (((s) << 32)+((t) & 0xffffffff))

/* Relocation types (bottom byte of r_info). */
#define R_386_NONE 0
#define R_386_32 1
#define R_386_PC32 2
#define R_386_GOT32 3
#define R_386_PLT32 4
#define R_386_COPY 5
#define R_386_GLOB_DAT 6
#define R_386_JMP_SLOT 7
#define R_386_RELATIVE 8
#define R_386_GOTOFF 9
#define R_386_GOTPC 10

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

typedef struct {
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4
#define PF_MASKPROC 0xf0000000
