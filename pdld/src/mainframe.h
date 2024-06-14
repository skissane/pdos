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
