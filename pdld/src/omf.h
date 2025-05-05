/******************************************************************************
 * @file            omf.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define RECORD_TYPE_THEADR 0x80
#define RECORD_TYPE_COMENT 0x88
#define RECORD_TYPE_MODEND 0x8A
#define RECORD_TYPE_EXTDEF 0x8C
#define RECORD_TYPE_PUBDEF 0x90
#define RECORD_TYPE_LNAMES 0x96
#define RECORD_TYPE_SEGDEF 0x98
#define RECORD_TYPE_GRPDEF 0x9A
#define RECORD_TYPE_FIXUPP 0x9C
#define RECORD_TYPE_LEDATA 0xA0
#define RECORD_TYPE_LEXTDEF 0xB4
#define RECORD_TYPE_LPUBDEF 0xB6

#define SEGMENT_ATTR_P 0x1

#define METHOD_F1_GRPDEF 1
#define METHOD_F5 5

#define METHOD_T0_SEGDEF 0
#define METHOD_T2_EXTDEF 2

#define FIXUP_LOCAT_M_SEGMENT_RELATIVE 0x40
