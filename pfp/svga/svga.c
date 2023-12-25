/******************************************************************************
 * @file            svga.c
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

#include <pos.h>
#include <bos.h>

#include "svga.h"
#include "vesa.h"
#include "vesa_bytearray.h"

struct vesa_state {
    struct VESAinfo_internal vi;
    struct VESAMinfo_internal mi;
    unsigned short current_mode;
    unsigned short old_mode;
};

static struct vesa_state vesa_state_s;
static struct vesa_state *vesa_state = &vesa_state_s;

static int get_VESAinfo (struct VESAinfo_internal *vi)
{
    struct VESAinfo_file *vi_file;
    
    if (!(vi_file = PosAllocMem (sizeof *vi_file, POS_LOC20))) return 1;
    
    memset (vi_file, 0, sizeof *vi_file); /* Just for sure. */
    memcpy (vi_file->sig, "VBE2", 4);
    
    if (BosVBEGetInfo (vi_file)) return 1;

    read_struct_VESAinfo (vi, vi_file);

    PosFreeMem (vi_file);
    
    return 0;
}

static int get_VESAMinfo (unsigned short mode, struct VESAMinfo_internal *mi)
{
    struct VESAMinfo_file *mi_file;

    if (!(mi_file = PosAllocMem (sizeof (*mi_file), POS_LOC20))) return 1;
    
    /* For backward compatibility because VBE 1.0 did not zero unused fields. */
    memset (mi_file, 0, sizeof *mi_file);
    if (BosVBEGetModeInfo (mode, mi_file)) return 1;

    read_struct_VESAMinfo (mi, mi_file);

    PosFreeMem (mi_file);
    
    return 0;
}

static int set_VESA_mode (unsigned short mode)
{
    struct CRTCinfo_file *ci_file;

    /* Valid pointer is provided but without the bit 11 set,
     * it should be ignored.
     */
    if (!(ci_file = PosAllocMem (sizeof (*ci_file), POS_LOC20))) return 1;
    memset (ci_file, 0, sizeof (*ci_file));
    
    BosVBESetMode (mode, ci_file);

    PosFreeMem (ci_file);
    
    return 0;
}

#if 0 /* Only for debugging. */
static void wait_for_key (void)
{
    int scancode;
    int ascii;
    
    BosReadKeyboardCharacter(&scancode, &ascii);
}
#endif

static unsigned long calculate_distance (unsigned int x,
                                         unsigned int y,
                                         unsigned int x2,
                                         unsigned int y2)
{
    long a, b;

    a = (long)x - (long)x2;
    b = (long)y - (long)y2;

    return a*a + b*b;
}

int svga_vbe_init (unsigned int wanted_width, unsigned int wanted_height)
{
    struct VESAinfo_internal vi;
    unsigned short *modes;

    if (get_VESAinfo (&vi)) return 1;
    if (memcmp (vi.sig, "VESA", 4)) return 1;
    
    vesa_state->vi = vi;

    {
        /* The list of modes might reside in some temporary memory
         * that gets overwritten by other VBE calls. */
        int i;
        unsigned short *orig_modes = FARP2ADDR (vi.modeptr);

        for (i = 0; orig_modes[i] != 0xffff; i++) ;
        i++;
        modes = malloc (sizeof (*modes) * i);
        if (!modes) return 1;
        memcpy (modes, orig_modes, sizeof (*modes) * i);
    }

#if 0
    /* Print all information, so it is certain all works properly. */
    printf ("buf starts '%.4s'\n", vi.sig);
    printf ("VESAver is %#x\n", vi.VESAver);
    printf ("OEMname is '%s'\n", FARP2ADDR (vi.OEMname));
    printf ("capabilities: %0#10lx\n", vi.cap);
    printf ("mem (number of 64 KiB blocks) is %u\n", vi.mem);
    printf ("OEMver is %#x\n", vi.OEMver);
    printf ("vendor_name is '%s'\n", FARP2ADDR (vi.vendor_name));
    printf ("product_name is '%s'\n", FARP2ADDR (vi.product_name));
    printf ("revision is '%s'\n", FARP2ADDR (vi.revision));

    printf ("available regular modes: \n");
    {
        int i;

        for (i = 0; modes[i] != 0xffff; i++) {
            printf ("%x ", modes[i]);
            if (i % 8 == 7) printf ("\n");
        }
        if (i % 8 != 7) printf ("\n");
    }
#endif
    
    {
        unsigned int old_mode;

        BosVBEGetMode (&old_mode);
        vesa_state->old_mode = old_mode;
    }

    {
        struct VESAMinfo_internal mi, best_mi;
        int i;
        unsigned short best_mode = 0xffff;

        for (i = 0; modes[i] != 0xffff; i++) {
            /* 0x4000 means to use linear framebuffer. */
            if (get_VESAMinfo (0x4000 | modes[i], &mi)) continue;

            if (!(mi.mode_attributes & MODE_ATTRIBUTE_SUPPORTED)
                || !(mi.mode_attributes & MODE_ATTRIBUTE_COLOR)
                || !(mi.mode_attributes & MODE_ATTRIBUTE_GRAPHICS)
                || !(mi.mode_attributes & MODE_ATTRIBUTE_LINEAR_FRAMEBUFFER_SUPPORTED))
                continue;

            /* Packed pixel graphics or direct color are what is wanted. */
            if (mi.memory_model != MEMORY_MODEL_PACKED_PIXEL
                && mi.memory_model != MEMORY_MODEL_DIRECT_COLOR) continue;

            if (mi.memory_model == MEMORY_MODEL_PACKED_PIXEL) {
                /* Only 8-8-8-bit modes are desired. */ 
                if (mi.red_mask != 8
                    || mi.green_mask != 8
                    || mi.blue_mask != 8) continue;

                /* The reserved 8 bits are good. */
                if (mi.res_mask != 8) continue;

                /* The memory should be in byte order BGRX. */
                if (mi.res_pos != 24
                    || mi.red_pos != 16
                    || mi.green_pos != 8
                    || mi.blue_pos != 0) continue;
            } else /* MEMORY_MODEL_DIRECT_COLOR */ {
                /* Only 8-8-8-bit modes are desired. */ 
                if (mi.direct_red_mask != 8
                    || mi.direct_green_mask != 8
                    || mi.direct_blue_mask != 8) continue;
                
                /* The reserved 8 bits are good. */
                if (mi.direct_res_mask != 8) continue;

                /* The memory should be in byte order BGRX. */
                if (mi.res_mask_lsb != 24
                    || mi.red_mask_lsb != 16
                    || mi.green_mask_lsb != 8
                    || mi.blue_mask_lsb != 0) continue;
            }

            /* Note that monitor might not actually support the mode
             * but dealing with that is complicated.
             * Options are:
             * 1. Get EDID using BIOS Int 10/AX=4F15h/BL=01h, compare timings here,
             *    set the good timing when setting mode using CRTCinfo (VBE3 only...)
             *    and hope that video card follows it correctly.
             * 2. Pick a mode and have the user confirm it is working.
             * 3. Use only modes that are likely to be safe (likely to have standard VGA timing)
             *    what are 720*480, 640*480 etc.
             * 4. Do nothing and hope it works. (Current solution)
             */

            /* Finally select the best resolution possible
             * or the closest one to the one wanted if it was specified.
             */
            if (best_mode == 0xffff) {
                best_mode = modes[i];
                best_mi = mi;
            } else {
                if (wanted_width && wanted_height) {
                    if (calculate_distance (mi.width, mi.height, wanted_width, wanted_height)
                        < calculate_distance (best_mi.width, best_mi.height, wanted_width, wanted_height)) {
                        best_mode = modes[i];
                        best_mi = mi;
                    }                    
                } else {
                    if (best_mi.width * best_mi.height < mi.width * mi.height) {
                        best_mode = modes[i];
                        best_mi = mi;
                    }
                }
            }
        }

        free (modes);

        /* No suitable mode exists. */
        if (best_mode == 0xffff) return 2;

#if 0
        printf ("mode num: %#x attri: %#x w: %u h: %u\n",
                best_mode, best_mi.mode_attributes, best_mi.width, best_mi.height);
#endif

        vesa_state->mi = best_mi;
        vesa_state->current_mode = best_mode;
    }

    if (set_VESA_mode (0x4000 | vesa_state->current_mode)) return 1;
    
    return 0;
}

int svga_vbe_restore_old_mode (void)
{
    return set_VESA_mode (vesa_state->old_mode);
}

unsigned int svga_get_width (void)
{
    return vesa_state->mi.width;
}

unsigned int svga_get_height (void)
{
    return vesa_state->mi.height;
}

/* BGRX to BGRcX (cX means it is not written) copy. */
void svga_swap_buffer (const void *double_buffer)
{
    unsigned char *vbuf;
    const unsigned char *dbuf;
    unsigned int x, y;

    vbuf = (void *)vesa_state->mi.video_buffer;
    dbuf = double_buffer;

    for (y = 0; y < vesa_state->mi.height; y++) {
        for (x = 0; x < vesa_state->mi.width; x++) {
            vbuf[x * 4] = dbuf[x * SVGA_BYTES_PER_PIXEL];
            vbuf[x * 4 + 1] = dbuf[x * SVGA_BYTES_PER_PIXEL + 1];
            vbuf[x * 4 + 2] = dbuf[x * SVGA_BYTES_PER_PIXEL + 2];
        }

        vbuf += vesa_state->mi.bytes_scan;
        dbuf += vesa_state->mi.width * SVGA_BYTES_PER_PIXEL;
    }
}
