/******************************************************************************
 * @file            atari.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
struct PH_internal {
    unsigned short ph_branch; /* Branch to start of the program, must be 0x601a. */

    unsigned long ph_tlen; /* .text section length. */
    unsigned long ph_dlen; /* .data section length. */
    unsigned long ph_blen; /* .bss section length. */
    unsigned long ph_slen; /* Symbol table length. */
    unsigned long ph_res1; /* Reserved, should be 0. */

    unsigned long ph_prgflags; /* Program flags. */
    unsigned short ph_absflags; /* 0 = Relocation information present. */
};

#define SIZEOF_struct_PH_file 28
struct PH_file {
    unsigned char ph_branch[2];

    unsigned char ph_tlen[4];
    unsigned char ph_dlen[4];
    unsigned char ph_blen[4];
    unsigned char ph_slen[4];
    unsigned char ph_res1[4];

    unsigned char ph_prgflags[4];
    unsigned char ph_absflags[2];
};

