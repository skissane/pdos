/******************************************************************************
 * @file            pdpfsvga.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pfp.h"
#include "svga.h"
#define BYTES_PER_PIXEL SVGA_BYTES_PER_PIXEL

static unsigned char s_bgr[3*HBMP*VBMP];

unsigned char *bgr = s_bgr;

int HWIN, VWIN;

void (*scale) () = scale8; 
/* 1: no scale  (center)
   2: no scale  (left upper)
   3: no scale  (right upper)
   4: no scale  (left lower)
   5: no scale  (right lower)
   6: fast  3
   7 slower but better 
   8: best 
*/

static int get_key (void);

static unsigned char *screen_buf;
static unsigned int screen_w, screen_h;

static void blit (int x, int y, int cx, int cy, const unsigned char *src)
{
    unsigned char *dst;
    int ix, iy;

    dst = screen_buf + y * screen_w * BYTES_PER_PIXEL;

    for (iy = 0; iy < cy; iy++) {
        for (ix = 0; ix < cx; ix++) {
            dst[(x + ix) * BYTES_PER_PIXEL] = src[HBMP * iy * 3 + ix * 3];
            dst[(x + ix) * BYTES_PER_PIXEL + 1] = src[HBMP * iy * 3 + ix * 3 + 1];
            dst[(x + ix) * BYTES_PER_PIXEL + 2] = src[HBMP * iy * 3 + ix * 3 + 2];
        }

        dst += screen_w * BYTES_PER_PIXEL;
    }
}

static void display_picture (void)
{
    memset (screen_buf, 0, screen_w * screen_h * BYTES_PER_PIXEL);
    blit ((HWIN-hdim)/2,(VWIN-vdim)/2, hdim, vdim, bgr);
    svga_swap_buffer (screen_buf);
}

int main (int argc, char **argv)
{
    if (argc != 2) {
        printf ("usage: %s jpg_file\n", argv[0]);
        return 1;
    }

    strcpy (filename, argv[1]);

    /* graphtst.c used 1024 * 768. */
    if (svga_vbe_init (1024, 768)) {
        printf ("failed to initialize graphics\n");
        return 1;
    }

    HWIN = screen_w = svga_get_width ();
    VWIN = screen_h = svga_get_height ();

    if (!(screen_buf = malloc (screen_w * screen_h * BYTES_PER_PIXEL))) {
        svga_vbe_restore_old_mode ();
        printf ("failed to allocate screen buffer\n");
        return 1;
    }
    memset (screen_buf, 0, screen_w * screen_h * BYTES_PER_PIXEL);

    init (); decode (); (*scale) ();

    display_picture ();

    while (1) {
        int wParam;
        int dir = 0;
        
        wParam = get_key ();

        if (wParam == ' ')  
             {if (dir<=0) {setpict(pictnr-dir); dir=1; decode(); (*scale)();}
              display_picture ();
              if (decode()) dir=0;
              (*scale)();
             }

           else if (wParam == 'b' || wParam == 'B')  
             {if (dir>=0) {setpict(pictnr-2-dir); dir= -1; decode(); (*scale)();}
              display_picture ();
              if (pictnr==1) dir=0; else {setpict(pictnr-2); decode(); (*scale)();}
             }

           else if (wParam >= '1' && wParam <= '8')  
             {if         (wParam=='1') scale = scale1;
                 else if (wParam=='2') scale = scale2;
                 else if (wParam=='3') scale = scale3;
                 else if (wParam=='4') scale = scale4;
                 else if (wParam=='5') scale = scale5;
                 else if (wParam=='6') scale = scale6;
                 else if (wParam=='7') scale = scale7;
                 else                scale = scale8;
              setpict(pictnr-1-dir); dir= 0; decode(); (*scale)();
              display_picture ();
             }


           else if (wParam == 'x' || wParam == 'X') break;
    }

    svga_vbe_restore_old_mode ();
    free (screen_buf);
    
    return 0;
}

void get_passwd()
{
    /* Dummy. */
}

#include <bos.h>

static int get_key (void)
{
    int scancode;
    int ascii;
    
    BosReadKeyboardCharacter (&scancode, &ascii);

    return ascii;
}
