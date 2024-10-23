/******************************************************************************
 * @file            macho.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdint.h>

struct mach_header_64_internal {
    unsigned long magic;
    unsigned long cputype;
    unsigned long cpusubtype;
    unsigned long filetype;
    unsigned long ncmds;
    unsigned long sizeofcmds;
    unsigned long flags;
    unsigned long reserved;
};

#define SIZEOF_struct_mach_header_64_file 32
struct mach_header_64_file {
    unsigned char magic[4];
    unsigned char cputype[4];
    unsigned char cpusubtype[4];
    unsigned char filetype[4];
    unsigned char ncmds[4];
    unsigned char sizeofcmds[4];
    unsigned char flags[4];
    unsigned char reserved[4];
};

#define MH_MAGIC_64 0xfeedfacf

#define CPU_TYPE_x86_64 0x01000007
#define CPU_TYPE_ARM64  0x0100000c

#define MH_OBJECT 1

struct load_command_internal {
    unsigned long cmd;
    unsigned long cmdsize;
};

#define SIZEOF_struct_load_command_file 8
struct load_command_file {
    unsigned char cmd[4];
    unsigned char cmdsize[4];
};

#define LC_SEGMENT_64 0x19
#define LC_SYMTAB 0x2
#define LC_DYSYMTAB 0x32
#define LC_BUILD_VERSION 0xb

struct segment_command_64_internal {
    unsigned long cmd;
    unsigned long cmdsize;
    char segname[16];
    uint_fast64_t vmaddr;
    uint_fast64_t vmsize;
    uint_fast64_t fileoff;
    uint_fast64_t filesize;
    unsigned long maxprot;
    unsigned long initprot;
    unsigned long nsects;
    unsigned long flags;
};

#define SIZEOF_struct_segment_command_64_file 72
struct segment_command_64_file {
    unsigned char cmd[4];
    unsigned char cmdsize[4];
    char segname[16];
    unsigned char vmaddr[8];
    unsigned char vmsize[8];
    unsigned char fileoff[8];
    unsigned char filesize[8];
    unsigned char maxprot[4];
    unsigned char initprot[4];
    unsigned char nsects[4];
    unsigned char flags[4];
};

struct section_64_internal {
    char sectname[16];
    char segname[16];
    uint_fast64_t addr;
    uint_fast64_t size;
    unsigned long offset;
    unsigned long align;
    unsigned long reloff;
    unsigned long nreloc;
    unsigned long flags;
    unsigned long reserved1;
    unsigned long reserved2;
    unsigned long reserved3;
};

#define SIZEOF_struct_section_64_file 80
struct section_64_file {
    char sectname[16];
    char segname[16];
    unsigned char addr[8];
    unsigned char size[8];
    unsigned char offset[4];
    unsigned char align[4];
    unsigned char reloff[4];
    unsigned char nreloc[4];
    unsigned char flags[4];
    unsigned char reserved1[4];
    unsigned char reserved2[4];
    unsigned char reserved3[4];
};

/* Relocation info is the same as for a.out, only the bitfield is different.
 * Originally r_address is not unsigned
 * but supporting negative values for it does not make sense,
 * so it is changed to unsigned here.
 */
struct relocation_info_internal {
    unsigned long r_address;
    unsigned long r_symbolnum;
    /* r_symbolnum is a bitfield but for portability it is accessed manually:
     * r_symbolnum : 24,
     * r_pcrel     : 1,
     * r_length    : 2,
     * r_extern    : 1,
     * r_type      : 4;
     */
};

#define SIZEOF_struct_relocation_info_file 8
struct relocation_info_file {
    unsigned char r_address[4];
    unsigned char r_symbolnum[4];
};

#define S_ZEROFILL 0x1

struct symtab_command_internal {
    unsigned long cmd;
    unsigned long cmdsize;
    unsigned long symoff;
    unsigned long nsyms;
    unsigned long stroff;
    unsigned long strsize;
};

#define SIZEOF_struct_symtab_command_file 24
struct symtab_command_file {
    unsigned char cmd[4];
    unsigned char cmdsize[4];
    unsigned char symoff[4];
    unsigned char nsyms[4];
    unsigned char stroff[4];
    unsigned char strsize[4];
};

struct nlist_64_internal {
    unsigned long n_strx;
    unsigned char n_type;
    unsigned char n_sect;
    unsigned short n_desc;
    uint_fast64_t n_value;
};

#define SIZEOF_struct_nlist_64_file 16
struct nlist_64_file {
    unsigned char n_strx[4];
    unsigned char n_type[1];
    unsigned char n_sect[1];
    unsigned char n_desc[2];
    unsigned char n_value[8];
};

#define N_STAB 0xe0
#define N_PEXT 0x10
#define N_TYPE 0x0e
#define N_EXT 0x01

#define N_UNDF 0x0
#define N_ABS 0x2
#define N_SECT 0xe
#define N_PUBD 0xc
#define N_INDR 0xa

#define NO_SECT 0
