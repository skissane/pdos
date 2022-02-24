/* written by Paul Edwards */
/* released to the public domain */
/* poor man's version of a.out.h - contains everything required by
   pdos at time of writing */
/* Documentation for the a.out format can be found here:
   http://man.cat-v.org/unix_8th/5/a.out */

#define SEGMENT_SIZE 0x10000UL

struct exec {
    unsigned long a_info;
    unsigned long a_text;
    unsigned long a_data;
    unsigned long a_bss;
    unsigned long a_syms;
    unsigned long a_entry;
    unsigned long a_trsize;
    unsigned long a_drsize;
};

/* First 2 bytes of a_info are magic number identifying the format.
 * Mask 0xffff can be used on the a_info for checking the numbers.
 * Top word should be 0x0064. */
#define OMAGIC 0407
#define NMAGIC 0410
#define ZMAGIC 0413
#define QMAGIC 0314

/* Relocation entry. */
struct relocation_info {
    long r_address;
    unsigned long r_symbolnum;
    /* First 24 bits are r_symbolnum,
    next 1 bit is r_pcrel,
    next 2 bits are r_length (0 = byte, 1 = word, 2 = long),
    next 1 bit is r_extern:
        if it is set r_symbolnum is a symbol number,
        otherwise r_symbolnum is n_type (segment)
        and the relocation is local
    (https://www.freebsd.org/cgi/man.cgi?query=a.out&sektion=5)
    next 4 are nothing. */
    
};

/* Symbol table entry. */
struct nlist {
    union {
        char *n_name;
        long n_strx; /* Index into string table. */
    } n_un;
    unsigned char n_type;
    char n_other;
    short n_desc;
    unsigned long n_value;
};

/* n_type values: */
#define N_UNDF 0x0
#define N_ABS 0x2
#define N_TEXT 0x4
#define N_DATA 0x6
#define N_BSS 0x8
#define N_COMM 0x12
#define N_FN 0x1f

#define N_EXT 0x1
#define N_TYPE 0x1e

/* Next is the string table,
 * starting with 4 bytes length including the field (so minimum is 4). */

#define N_TXTOFF(e) (0x400)
#define N_TXTADDR(e) (SEGMENT_SIZE)
/* this formula doesn't work when the text size is exactly 64k */
#define N_DATADDR(e) \
    (N_TXTADDR(e) + SEGMENT_SIZE + ((e).a_text & 0xffff0000UL))
#define N_BSSADDR(e) (N_DATADDR(e) + (e).a_data)
