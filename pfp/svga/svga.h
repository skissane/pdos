/******************************************************************************
 * @file            svga.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
/* It is expected that OS keeps a screen double buffer of size w*h*SVGA_BYTES_PER_PIXEL
 * in format BGRX (because little-endian RGB) and does all drawing there.
 * The X byte is just for alignment and OS can use it as seen fit.
 */
#define SVGA_BYTES_PER_PIXEL 4

/* These two functions use BIOS
 * and ideally should be used only when preparing environment for OS.
 */
/* Sets mode closest to the wanted resolution or the biggest resolution if 0. */
int svga_vbe_init (unsigned int wanted_width, unsigned int wanted_height);
int svga_vbe_restore_old_mode (void);

/* These functions do not use BIOS and can be used at any time after svga_vbe_init(). */
unsigned int svga_get_width (void);
unsigned int svga_get_height (void);
/* Once OS finishes drawing in double buffer and wants to display it,
 * svga_swap_buffer() handles translation to video memory format whatever it is.
 */
void svga_swap_buffer (const void *double_buffer);
