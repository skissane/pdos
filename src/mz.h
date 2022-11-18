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
    unsigned char magic[2]; /* "MZ" or "ZM". */
    unsigned short num_last_page_bytes;
    unsigned short num_pages;
    unsigned short num_reloc_entries;
    unsigned short header_size; /* In paragraphs (16 byte). */
    unsigned short min_alloc;
    unsigned short max_alloc;
    unsigned short init_ss;
    unsigned short init_sp;
    unsigned short checksum;
    unsigned short init_ip;
    unsigned short init_cs;
    unsigned short reloc_tab_offset;
    unsigned short overlay;
    unsigned short reserved1[4]; /* First set of reserved words. */
    unsigned short oem_id;
    unsigned short oem_info;
    unsigned short reserved2[10]; /* Second set of reserved words. */
    unsigned long e_lfanew; /* Offset to the PE header. */
} Mz_hdr;
