/******************************************************************************
 * @file            a_out.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define     SEGMENT_SIZE                0x10000UL

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

#define     OMAGIC                      0407
#define     NMAGIC                      0410
#define     ZMAGIC                      0413
#define     QMAGIC                      0314

/* Relocation entry. */
struct relocation_info_internal {

    long r_address;
    unsigned long r_symbolnum;

};

struct relocation_info_file {

    unsigned char r_address[4];
    unsigned char r_symbolnum[4];

};

/* Symbol table entry. */
struct nlist_internal {

    /*union {
    
        char *n_name;
        long n_strx;
    
    } n_un;*/
    
    long n_strx;
    unsigned char n_type;
    
    char n_other;
    short n_desc;
    
    unsigned long n_value;

};

struct nlist_file {

    unsigned char n_strx[4];
    unsigned char n_type[1];
    
    unsigned char n_other[1];
    unsigned char n_desc[2];
    
    unsigned char n_value[4];

};

/* n_type values: */
#define     N_UNDF                      0x00
#define     N_ABS                       0x02
#define     N_TEXT                      0x04
#define     N_DATA                      0x06
#define     N_BSS                       0x08
#define     N_COMM                      0x12
#define     N_FN                        0x1f

#define     N_EXT                       0x01
#define     N_TYPE                      0x1e

/* Next is the string table,
 * starting with 4 bytes length including the field (so minimum is 4). */
struct string_table_header_internal {

    unsigned long s_size;

};

struct string_table_header_file {

    unsigned char s_size[4]; /* Including the size of the header itself. */

};

#define     N_TXTOFF(e)                 (0x400)
#define     N_TXTADDR(e)                (SEGMENT_SIZE)
/* this formula doesn't work when the text size is exactly 64k */
#define     N_DATADDR(e)                \
    (N_TXTADDR (e) + SEGMENT_SIZE + ((e).a_text & 0xffff0000UL))
#define     N_BSSADDR(e)                (N_DATADDR (e) + (e).a_data)

#define     N_GETMAGIC(exec_internal)            ((exec_internal).a_info & 0xffff)
