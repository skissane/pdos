/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  Manipulate the video card                                        */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pos.h>
#include <bos.h>

typedef struct {
    unsigned short seg;
    unsigned short off;
} FARP;

#define FARP2ADDR(x) ((void *)(((unsigned long)x.seg << 4) + x.off))

typedef unsigned long U32;

typedef struct {
    char sig[4];
    unsigned short VESAver;
    FARP OEMname;
    U32 cap;
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
} VESAinfo;

typedef struct {
    unsigned short mode_attributes;
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
    U32 video_buffer;
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
    U32 pixel_clock;
    unsigned char reserved2[190];
} VESAMinfo;

typedef struct {
    char reserved[60];
} VESACRTC;

int main(int argc, char **argv)
{
    VESAinfo *vi;
    VESAMinfo *mi;
    char *vendor_name;
    unsigned short *modes;
    int x = 0;
    int y;
    unsigned char *vidbuf;
    unsigned char *palet;
    unsigned int oldmode;
    int scancode;
    int ascii;

    vi = PosAllocMem(sizeof *vi, POS_LOC20);
    printf("vi is %p\n", vi);
    mi = PosAllocMem(sizeof *mi, POS_LOC20);
    printf("mi is %p\n", mi);
    memcpy(vi->sig, "VBE2", 4);
    BosVBEGetInfo(vi);
    printf("buf starts %.4s\n", vi);
    vendor_name = FARP2ADDR(vi->vendor_name);
    printf("vendor_name is %s\n", vendor_name);
#if 0
    /* these mode numbers seem to be unreliable */
    modes = FARP2ADDR(vi->modeptr);
    while (*modes != 0xffff)
    {
        if (*modes & 0x4000)
        {
            printf("mode is %x\n", *modes);
            BosVBEGetModeInfo(*modes, mi);
            printf("width is %d, height is %d %lx %x\n",
                   mi->width, mi->height, (long)mi->video_buffer,
                   mi->wina_startseg);
            /* if (mi->width == 1920) break; */
            /* if (mi->width == 1024) break; */
            if ((*modes & 0x1ff) == 0x105) break;
            x++;
            /* if (x == 5) break; */
        }
        modes++;
    }
#endif
    BosVBEGetMode(&oldmode);
    /* 0x105 is 1024 * 768 * 256, but not guaranteed */
    BosVBEGetModeInfo(0x4105, mi); /* note sure if "4" required */
    printf("width is %d, height is %d %lx %x\n",
           mi->width, mi->height, (long)mi->video_buffer,
           mi->wina_startseg);
    vidbuf = (unsigned char *)mi->video_buffer;
    /* Palette stuff. */
    palet = PosAllocMem(12, POS_LOC20);
    BosVBESetMode(0x4105, mi); /* not sure if "4" required, 
                                  and mi is not correct mapping */
    BosVBEPaletteOps(1 /* get */, 1, 0x80, palet); /* doesn't work on my Bochs*/
    printf("red: %x green: %x blue: %x aligment: %x\n",palet[0],palet[1],
           palet[2],palet[3]);
    palet[0] = 0xff; palet[1] = 0; palet[2] = 0; palet[3] = 0;
    palet[4] = 0; palet[5] = 0xff; palet[6] = 0; palet[7] = 0;
    palet[8] = 0; palet[9] = 0; palet[10] = 0xff; palet[11] = 0;
    BosVBEPaletteOps(0 /* set */, 3, 0x80, palet); /* doesn't work on my Bochs*/
    
    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 400; x++)
        {
            vidbuf[1024 * 300 + 500 + 1024 * y + x] = 0x80;
        }
    }
    for (y = 0; y < 100; y++)
    {
        for (x = 0; x < 100; x++)
        {
            vidbuf[1024 * 300 + 500 + 50 + 1024 * 50 + 1024 * y + x] = 0x60;
        }
    }
    /* Palette test. */
    for (y = 0; y < 100; y++)
    {
        for (x = 0; x < 100; x++)
        {
            vidbuf[1024 * 300 + 1024 * y + x] = 0x82;
        }
    }
    for (y = 0; y < 100; y++)
    {
        for (x = 0; x < 100; x++)
        {
            vidbuf[1024 * 300 + 100 + 1024 * y + x] = 0x81;
        }
    }
    for (y = 0; y < 100; y++)
    {
        for (x = 0; x < 100; x++)
        {
            vidbuf[1024 * 300 + 200 + 1024 * y + x] = 0x80;
        }
    }
    BosReadKeyboardCharacter(&scancode, &ascii);
    BosVBESetMode(oldmode, mi); /* mi is not correct mapping */
    return (0);
}
