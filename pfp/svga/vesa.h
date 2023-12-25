/******************************************************************************
 * @file            vesa.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
/* The VESA pointers are far pointers where the bottom 16 bits are offset
 * and top 16 bits are segment.
 */
typedef unsigned long FARP;
#define FARP2ADDR(x) \
((void *)((((x >> 16) & 0xffff) << 4) + (x & 0xffff)))

struct VESAinfo_internal {
    char sig[4];
    unsigned short VESAver;
    FARP OEMname;
    unsigned long cap;
    FARP modeptr;
    unsigned short mem;
    unsigned short OEMver;
    FARP vendor_name;
    FARP product_name;
    FARP revision;
    unsigned short AFver;
    FARP acc_modes;
    char res1[216];
    char res2[256];
};

struct VESAinfo_file {
    char sig[4];
    unsigned char VESAver[2];
    unsigned char OEMname[4];
    unsigned char cap[4];
    unsigned char modeptr[4];
    unsigned char mem[2];
    unsigned char OEMver[2];
    unsigned char vendor_name[4];
    unsigned char product_name[4];
    unsigned char revision[4];
    unsigned char AFver[2];
    unsigned char acc_modes[4];
    char res1[216];
    char res2[256];
};

struct VESAMinfo_internal {
    unsigned short mode_attributes;
#define MODE_ATTRIBUTE_SUPPORTED                     (1U << 0)
#define MODE_ATTRIBUTE_COLOR                         (1U << 3)
#define MODE_ATTRIBUTE_GRAPHICS                      (1U << 4)
#define MODE_ATTRIBUTE_LINEAR_FRAMEBUFFER_SUPPORTED  (1U << 7)
    unsigned char wina_attributes;
    unsigned char winb_attributes;
    unsigned short win_granularity;
    unsigned short win_size;
    unsigned short wina_startseg;
    unsigned short winb_startseg;
    FARP position_func;
    unsigned short bytes_scan;
    unsigned short width;
    unsigned short height;
    unsigned char width_cell;
    unsigned char height_cell;
    unsigned char memory_planes;
    unsigned char bits_pixel;
    unsigned char banks;
    unsigned char memory_model;
#define MEMORY_MODEL_PACKED_PIXEL  4
#define MEMORY_MODEL_DIRECT_COLOR  6
    unsigned char size_bank;
    unsigned char image_pages;
    unsigned char reserved1;
    unsigned char red_mask;
    unsigned char red_pos;
    unsigned char green_mask;
    unsigned char green_pos;
    unsigned char blue_mask;
    unsigned char blue_pos;
    unsigned char res_mask;
    unsigned char res_pos;
    unsigned char color_info;
    unsigned long video_buffer; /* Linear 32-bit pointer. */
    FARP offscreen; /* is this a far pointer? */
    unsigned short size_offscreen;
    unsigned short bytes_scan_linear;
    unsigned char banked_num_images;
    unsigned char linear_num_images;
    unsigned char direct_red_mask;
    unsigned char red_mask_lsb;
    unsigned char direct_green_mask;
    unsigned char green_mask_lsb;
    unsigned char direct_blue_mask;
    unsigned char blue_mask_lsb;
    unsigned char direct_res_mask;
    unsigned char res_mask_lsb;
    unsigned long pixel_clock;
    unsigned char reserved2[190];
};

struct VESAMinfo_file {
    unsigned char mode_attributes[2];
    unsigned char wina_attributes[1];
    unsigned char winb_attributes[1];
    unsigned char win_granularity[2];
    unsigned char win_size[2];
    unsigned char wina_startseg[2];
    unsigned char winb_startseg[2];
    unsigned char position_func[4];
    unsigned char bytes_scan[2];
    unsigned char width[2];
    unsigned char height[2];
    unsigned char width_cell[1];
    unsigned char height_cell[1];
    unsigned char memory_planes[1];
    unsigned char bits_pixel[1];
    unsigned char banks[1];
    unsigned char memory_model[1];
    unsigned char size_bank[1];
    unsigned char image_pages[1];
    unsigned char reserved1[1];
    unsigned char red_mask[1];
    unsigned char red_pos[1];
    unsigned char green_mask[1];
    unsigned char green_pos[1];
    unsigned char blue_mask[1];
    unsigned char blue_pos[1];
    unsigned char res_mask[1];
    unsigned char res_pos[1];
    unsigned char color_info[1];
    unsigned char video_buffer[4];
    unsigned char offscreen[4]; /* is this a far pointer? */
    unsigned char size_offscreen[2];
    unsigned char bytes_scan_linear[2];
    unsigned char banked_num_images[1];
    unsigned char linear_num_images[1];
    unsigned char direct_red_mask[1];
    unsigned char red_mask_lsb[1];
    unsigned char direct_green_mask[1];
    unsigned char green_mask_lsb[1];
    unsigned char direct_blue_mask[1];
    unsigned char blue_mask_lsb[1];
    unsigned char direct_res_mask[1];
    unsigned char res_mask_lsb[1];
    unsigned char pixel_clock[4];
    unsigned char reserved2[190];
};

struct CRTCinfo_internal {
    unsigned short total_pixels_horizontal;
    unsigned short horizontal_sync_start;
    unsigned short horizontal_sync_end;
    unsigned short total_scan_lines;
    unsigned short vertical_scan_start;
    unsigned short vertical_scan_end;
    unsigned char flags;
    unsigned long pixel_clock;
    unsigned short refresh_rate; /* pixel_clock / (total_pixels_horizontal * total_scan_lines) */
    unsigned char reserved[40];
};

struct CRTCinfo_file {
    unsigned char total_pixels_horizontal[2];
    unsigned char horizontal_sync_start[2];
    unsigned char horizontal_sync_end[2];
    unsigned char total_scan_lines[2];
    unsigned char vertical_scan_start[2];
    unsigned char vertical_scan_end[2];
    unsigned char flags[1];
    unsigned char pixel_clock[4];
    unsigned char refresh_rate[2]; /* pixel_clock / (total_pixels_horizontal * total_scan_lines) */
    unsigned char reserved[40];
};
