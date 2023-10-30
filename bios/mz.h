/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  mz.h - header file for MZ support                                */
/*                                                                   */
/*********************************************************************/

typedef struct {
    unsigned char magic[2]; /* "MZ" or "ZM". */ /* 0 */
    unsigned short num_last_page_bytes; /* 2 */ /* page = 512 bytes */
    unsigned short num_pages; /* 4 */
    unsigned short num_reloc_entries; /* 6 */
    unsigned short header_size; /* In paragraphs (16 byte). */ /* 8 */
    unsigned short min_alloc; /* 10 A */
    unsigned short max_alloc; /* 12 C */
    unsigned short init_ss; /* 14 E */
    unsigned short init_sp; /* 16 10 */
    unsigned short checksum; /* 18 12 */
    unsigned short init_ip; /* 20 14 */
    unsigned short init_cs; /* 22 16 */
    unsigned short reloc_tab_offset; /* 24 18 */
    unsigned short overlay; /* 26 1A */
    unsigned short reserved1[4]; /* First set of reserved words. */ /* 28 1C */
    unsigned short oem_id; /* 36 24 */
    unsigned short oem_info; /* 38 26 */
    unsigned short reserved2[10]; /* Second set of reserved words */ /* 40 28 */
    unsigned long e_lfanew; /* Offset to the PE header. */ /* 60 3C */
} Mz_hdr;
