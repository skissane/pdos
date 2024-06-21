/******************************************************************************
 * @file            aout.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define SEGMENT_SIZE 0x10000UL

struct exec_internal {
    unsigned long a_info;
    unsigned long a_text;
    unsigned long a_data;
    unsigned long a_bss;
    unsigned long a_syms;
    unsigned long a_entry;
    unsigned long a_trsize;
    unsigned long a_drsize;
};

#define SIZEOF_struct_exec_file 32
struct exec_file {
    unsigned char a_info[4];
    unsigned char a_text[4];
    unsigned char a_data[4];
    unsigned char a_bss[4];
    unsigned char a_syms[4];
    unsigned char a_entry[4];
    unsigned char a_trsize[4];
    unsigned char a_drsize[4];
};

#define OMAGIC 0407
#define NMAGIC 0410
#define ZMAGIC 0413
#define QMAGIC 0314

/* Originally some of the fields are not unsigned
 * but supporting negative values for them does not make sense,
 * so they are changed to unsigned here.
 */
struct relocation_info_internal {
    unsigned long r_address;
    unsigned long r_symbolnum;
    /* r_symbolnum is a bitfield but for portability it is accessed manually:
     * r_symbolnum : 24,
     * r_pcrel     : 1,
     * r_length    : 2,
     * r_extern    : 1,
     * r_baserel   : 1,
     * r_jmptable  : 1,
     * r_relative  : 1,
     * r_copy      : 1;
     */
};

#define SIZEOF_struct_relocation_info_file 8
struct relocation_info_file {
    unsigned char r_address[4];
    unsigned char r_symbolnum[4];
};

struct nlist_internal {
    unsigned long n_strx;
    unsigned char n_type;
    
    unsigned char n_other;
    unsigned short n_desc;
    
    unsigned long n_value;
};

#define SIZEOF_struct_nlist_file 12
struct nlist_file {
    unsigned char n_strx[4];
    unsigned char n_type[1];
    
    unsigned char n_other[1];
    unsigned char n_desc[2];
    
    unsigned char n_value[4];
};

/* n_type values: */
#define N_UNDF 0x00
#define N_ABS  0x02
#define N_TEXT 0x04
#define N_DATA 0x06
#define N_BSS  0x08
#define N_COMM 0x12
#define N_FN   0x1f

#define N_EXT  0x01
#define N_TYPE 0x1e
