/******************************************************************************
 * @file            vesa_bytearray.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <string.h>

#include "vesa.h"
#include "bytearray.h"

#define COPY(struct_name, field_name, bytes) \
 bytearray_read_##bytes##_bytes (&struct_name##_internal->field_name, struct_name##_file->field_name, LITTLE_ENDIAN)
#define COPY_CHAR_ARRAY(struct_name, field_name) \
 memcpy (struct_name##_internal->field_name, struct_name##_file->field_name, sizeof (struct_name##_file->field_name))

void read_struct_VESAinfo (struct VESAinfo_internal *VESAinfo_internal, const void *memory)
{
    const struct VESAinfo_file *VESAinfo_file = memory;

    COPY_CHAR_ARRAY(VESAinfo, sig);
    COPY(VESAinfo, VESAver, 2);
    COPY(VESAinfo, OEMname, 4);
    COPY(VESAinfo, cap, 4);
    COPY(VESAinfo, modeptr, 4);
    COPY(VESAinfo, mem, 2);
    COPY(VESAinfo, OEMver, 2);
    COPY(VESAinfo, vendor_name, 4);
    COPY(VESAinfo, product_name, 4);
    COPY(VESAinfo, revision, 4);
    COPY(VESAinfo, AFver, 2);
    COPY(VESAinfo, acc_modes, 4);

    COPY_CHAR_ARRAY(VESAinfo, res1);
    COPY_CHAR_ARRAY(VESAinfo, res2);
}

void read_struct_VESAMinfo (struct VESAMinfo_internal *VESAMinfo_internal, const void *memory)
{
    const struct VESAMinfo_file *VESAMinfo_file = memory;

    COPY(VESAMinfo, mode_attributes, 2);
    COPY(VESAMinfo, wina_attributes, 1);
    COPY(VESAMinfo, winb_attributes, 1);
    COPY(VESAMinfo, win_granularity, 2);
    COPY(VESAMinfo, win_size, 2);
    COPY(VESAMinfo, wina_startseg, 2);
    COPY(VESAMinfo, winb_startseg, 2);
    COPY(VESAMinfo, position_func, 4);
    COPY(VESAMinfo, bytes_scan, 2);
    COPY(VESAMinfo, width, 2);
    COPY(VESAMinfo, height, 2);

    COPY(VESAMinfo, width_cell, 1);
    COPY(VESAMinfo, height_cell, 1);
    COPY(VESAMinfo, memory_planes, 1);
    COPY(VESAMinfo, bits_pixel, 1);
    COPY(VESAMinfo, banks, 1);
    COPY(VESAMinfo, memory_model, 1);
    COPY(VESAMinfo, size_bank, 1);
    COPY(VESAMinfo, image_pages, 1);
    COPY(VESAMinfo, reserved1, 1);
    COPY(VESAMinfo, red_mask, 1);
    COPY(VESAMinfo, red_pos, 1);
    COPY(VESAMinfo, green_mask, 1);
    COPY(VESAMinfo, green_pos, 1);
    COPY(VESAMinfo, blue_mask, 1);
    COPY(VESAMinfo, blue_pos, 1);
    COPY(VESAMinfo, res_mask, 1);
    COPY(VESAMinfo, res_pos, 1);
    COPY(VESAMinfo, color_info, 1);
    

    COPY(VESAMinfo, video_buffer, 4);
    COPY(VESAMinfo, offscreen, 4);
    COPY(VESAMinfo, size_offscreen, 2);
    COPY(VESAMinfo, bytes_scan_linear, 2);

    COPY(VESAMinfo, banked_num_images, 1);
    COPY(VESAMinfo, linear_num_images, 1);
    COPY(VESAMinfo, direct_red_mask, 1);
    COPY(VESAMinfo, red_mask_lsb, 1);
    COPY(VESAMinfo, direct_green_mask, 1);
    COPY(VESAMinfo, green_mask_lsb, 1);
    COPY(VESAMinfo, direct_blue_mask, 1);
    COPY(VESAMinfo, blue_mask_lsb, 1);
    COPY(VESAMinfo, direct_res_mask, 1);
    COPY(VESAMinfo, res_mask_lsb, 1);

    COPY(VESAMinfo, pixel_clock, 4);
    COPY_CHAR_ARRAY(VESAMinfo, reserved2);
}
