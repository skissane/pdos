/******************************************************************************
 * @file            coff_bytearray.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ld.h"
#include "bytearray.h"
#include "lx.h"
#include "lx_bytearray.h"

#define COPY(struct_name, field_name, bytes) \
 bytearray_read_##bytes##_bytes (&struct_name##_internal->field_name, struct_name##_file->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_internal->field_name, struct_name##_file->field_name, sizeof (struct_name##_file->field_name))

void read_struct_LX_HEADER (struct LX_HEADER_internal *LX_HEADER_internal, const void *memory)
{
    const struct LX_HEADER_file *LX_HEADER_file = memory;

    COPY_CHAR_ARRAY (LX_HEADER, Magic);
    
    COPY(LX_HEADER, BOrd, 1);
    COPY(LX_HEADER, WOrd, 1);
    COPY(LX_HEADER, FormatLevel, 4);

    COPY(LX_HEADER, CpuType, 2);
    COPY(LX_HEADER, OsType, 2);
    COPY(LX_HEADER, ModuleVersion, 4);
    
    COPY(LX_HEADER, ModuleFlags, 4);
    COPY(LX_HEADER, ModuleNumberOfPages, 4);
    COPY(LX_HEADER, EipObjectIndex, 4);
    COPY(LX_HEADER, Eip, 4);
    COPY(LX_HEADER, EspObjectIndex, 4);
    COPY(LX_HEADER, Esp, 4);
    COPY(LX_HEADER, PageSize, 4);
    COPY(LX_HEADER, PageOffsetShift, 4);
    COPY(LX_HEADER, FixupSectionSize, 4);
    COPY(LX_HEADER, FixupSectionChecksum, 4);
    COPY(LX_HEADER, LoaderSectionSize, 4);
    COPY(LX_HEADER, LoaderSectionChecksum, 4);
    COPY(LX_HEADER, ObjectTableOffsetHdr, 4);
    COPY(LX_HEADER, NumberOfObjectsInModule, 4);
    COPY(LX_HEADER, ObjectPageTableOffsetHdr, 4);
    COPY(LX_HEADER, ObjectIterPageOffset, 4);
    COPY(LX_HEADER, ResourceTableOffsetHdr, 4);
    COPY(LX_HEADER, NumberOfResourceTableEntries, 4);
    COPY(LX_HEADER, ResidentNameTableOffsetHdr, 4);
    COPY(LX_HEADER, EntryTableOffsetHdr, 4);
    COPY(LX_HEADER, ModuleDirectivesOffsetHdr, 4);
    COPY(LX_HEADER, NumberOfModuleDirectives, 4);
    COPY(LX_HEADER, FixupPageTableOffsetHdr, 4);
    COPY(LX_HEADER, FixupRecordTableOffsetHdr, 4);
    COPY(LX_HEADER, ImportModuleTableOffsetHdr, 4);
    COPY(LX_HEADER, NumberOfImportModEntries, 4);
    COPY(LX_HEADER, ImportProcTableOffsetHdr, 4);
    COPY(LX_HEADER, PerPageChecksumOffsetHdr, 4);
    COPY(LX_HEADER, DataPagesOffset, 4);
    COPY(LX_HEADER, NumberOfPreloadPages, 4);
    COPY(LX_HEADER, NonResNameTableOffset, 4);
    COPY(LX_HEADER, NonResNameTableLength, 4);
    COPY(LX_HEADER, NonResNameTableChecksum, 4);
    COPY(LX_HEADER, AutoDataSegmentObjectIndex, 4);
    COPY(LX_HEADER, DebugInfoOffset, 4);
    COPY(LX_HEADER, DebugInfoLength, 4);
    COPY(LX_HEADER, NumberOfInstancePreloadPages, 4);
    COPY(LX_HEADER, NumberOfInstanceDemandPages, 4);
    COPY(LX_HEADER, Heapsize, 4);
    COPY(LX_HEADER, Stacksize, 4);
}

#undef COPY_CHAR_ARRAY
#undef COPY

#define COPY(struct_name, field_name, bytes) \
 bytearray_write_##bytes##_bytes (struct_name##_file->field_name, struct_name##_internal->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) memcpy (struct_name##_file->field_name, struct_name##_internal->field_name, sizeof (struct_name##_file->field_name))

void write_struct_LX_HEADER (void *memory, const struct LX_HEADER_internal *LX_HEADER_internal)
{
    struct LX_HEADER_file *LX_HEADER_file = memory;

    COPY_CHAR_ARRAY (LX_HEADER, Magic);
    
    COPY(LX_HEADER, BOrd, 1);
    COPY(LX_HEADER, WOrd, 1);
    COPY(LX_HEADER, FormatLevel, 4);

    COPY(LX_HEADER, CpuType, 2);
    COPY(LX_HEADER, OsType, 2);
    COPY(LX_HEADER, ModuleVersion, 4);
    
    COPY(LX_HEADER, ModuleFlags, 4);
    COPY(LX_HEADER, ModuleNumberOfPages, 4);
    COPY(LX_HEADER, EipObjectIndex, 4);
    COPY(LX_HEADER, Eip, 4);
    COPY(LX_HEADER, EspObjectIndex, 4);
    COPY(LX_HEADER, Esp, 4);
    COPY(LX_HEADER, PageSize, 4);
    COPY(LX_HEADER, PageOffsetShift, 4);
    COPY(LX_HEADER, FixupSectionSize, 4);
    COPY(LX_HEADER, FixupSectionChecksum, 4);
    COPY(LX_HEADER, LoaderSectionSize, 4);
    COPY(LX_HEADER, LoaderSectionChecksum, 4);
    COPY(LX_HEADER, ObjectTableOffsetHdr, 4);
    COPY(LX_HEADER, NumberOfObjectsInModule, 4);
    COPY(LX_HEADER, ObjectPageTableOffsetHdr, 4);
    COPY(LX_HEADER, ObjectIterPageOffset, 4);
    COPY(LX_HEADER, ResourceTableOffsetHdr, 4);
    COPY(LX_HEADER, NumberOfResourceTableEntries, 4);
    COPY(LX_HEADER, ResidentNameTableOffsetHdr, 4);
    COPY(LX_HEADER, EntryTableOffsetHdr, 4);
    COPY(LX_HEADER, ModuleDirectivesOffsetHdr, 4);
    COPY(LX_HEADER, NumberOfModuleDirectives, 4);
    COPY(LX_HEADER, FixupPageTableOffsetHdr, 4);
    COPY(LX_HEADER, FixupRecordTableOffsetHdr, 4);
    COPY(LX_HEADER, ImportModuleTableOffsetHdr, 4);
    COPY(LX_HEADER, NumberOfImportModEntries, 4);
    COPY(LX_HEADER, ImportProcTableOffsetHdr, 4);
    COPY(LX_HEADER, PerPageChecksumOffsetHdr, 4);
    COPY(LX_HEADER, DataPagesOffset, 4);
    COPY(LX_HEADER, NumberOfPreloadPages, 4);
    COPY(LX_HEADER, NonResNameTableOffset, 4);
    COPY(LX_HEADER, NonResNameTableLength, 4);
    COPY(LX_HEADER, NonResNameTableChecksum, 4);
    COPY(LX_HEADER, AutoDataSegmentObjectIndex, 4);
    COPY(LX_HEADER, DebugInfoOffset, 4);
    COPY(LX_HEADER, DebugInfoLength, 4);
    COPY(LX_HEADER, NumberOfInstancePreloadPages, 4);
    COPY(LX_HEADER, NumberOfInstanceDemandPages, 4);
    COPY(LX_HEADER, Heapsize, 4);
    COPY(LX_HEADER, Stacksize, 4);
}

#undef COPY_CHAR_ARRAY
#undef COPY
