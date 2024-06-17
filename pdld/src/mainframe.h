/******************************************************************************
 * @file            mainframe.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define ESD_DATA_TYPE_LD 0x1
#define ESD_DATA_TYPE_ER 0x2
#define ESD_DATA_TYPE_PC 0x4

/* Bytearray structs are not currently really used
 * because only few fields need to be written.
 */
#define SIZEOF_struct_COPYR1_file 60 /* In old format, in new format it is 64. */
#define SIZEOF_struct_COPYR2_file 284
#define SIZEOF_struct_member_data_header_file 12
#define SIZEOF_struct_linkage_editor_data_file 15
#define SIZEOF_struct_translator_description_file 15

/* PDS2ATR1 flags. */
#define PDS2RENT 0x80 /* REENTERABLE */
#define DEREEN PDS2RENT
#define PDS2REUS 0x40 /* REUSABLE */
#define PDS2OVLY 0x20 /* IN OVERLAY STRUCTURE */
#define DEOVLY PDS2OVLY
#define PDS2TEST 0x10 /* MODULE TO BE TESTED - TESTRAN */
#define PDS2LOAD 0x8 /* ONLY LOADABLE */
#define DELODY PDS2LOAD
#define PDS2SCTR 0x4 /* SCATTER FORMAT */
#define DESCAT PDS2SCTR
#define PDS2EXEC 0x2 /* EXECUTABLE */
#define DEXCUT PDS2EXEC
#define PDS21BLK 0x1 /* IF ZERO, MODULE CONTAINS MULTIPLE RECORDS WITH AT LEAST ONE BLOCK OF TEXT.
                      * IF ONE, MODULE CONTAINS NO RLD ITEMS AND ONLY ONE BLOCK OF TEXT. */

/* PDS2ATR2 flags */
#define PDS2FLVL 0x80 /* IF ZERO, MODULE CAN BE PROCESSED BY ALL LEVELS OF LINKAGE EDITOR.
                       * IF ONE, MODULE CAN BE PROCESSED ONLY BY F LEVEL OF LINKAGE EDITOR. */
#define PDS2ORG0 0x40 /* LINKAGE EDITOR ASSIGNED ORIGIN OF FIRST BLOCK OF TEXT IS ZERO. */
#define PDS2EP0 0x20 /* ENTRY POINT ASSIGNED BY LINKAGE EDITOR IS ZERO */
#define PDS2NRLD 0x10 /* MODULE CONTAINS NO RLD ITEMS */
#define PDS2NREP 0x8 /* MODULE CANNOT BE REPROCESSED BY LINKAGE EDITOR */
#define PDS2TSTN 0x4 /* MODULE CONTAINS TESTRAN SYMBOL CARDS */
#define PDS2LEF 0x2 /* MODULE CREATED BY LINKAGE EDITOR F */
#define PDS2REFR 0x1 /* REFRESHABLE MODULE */
