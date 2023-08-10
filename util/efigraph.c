/******************************************************************************
 * @file            efigraph.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <efi.h>
#include <stddef.h>

#include "font8x8_basic.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

EFI_HANDLE *__gIH;
EFI_SYSTEM_TABLE *__gST;
EFI_BOOT_SERVICES *__gBS;

#define gIH __gIH
#define gST __gST
#define gBS __gBS

#define return_Status_if_fail(func) do { if ((Status = (func))) { return Status; }} while (0)
#define rSif return_Status_if_fail

struct logical_video_buf_info {
    unsigned long *buf;
    unsigned long max_x;
    unsigned long max_y;
};

struct physical_video_buf_info {
    void *buf;
    unsigned long horizontal_resolution;
    unsigned long vertical_resolution;
    unsigned long pixels_per_scanline;
    int blue_green_red;
};

static EFI_STATUS print_string (char *str)
{
    EFI_STATUS Status = EFI_SUCCESS;
    CHAR16 onechar[2] = {0, '\0'};

    while (*str) {
        if (*str == '\n') {
            onechar[0] = '\r';
            if ((Status = gST->ConOut->OutputString (gST->ConOut, onechar))) {
                return Status;
            }
        }

        onechar[0] = *str;
        if ((Status = gST->ConOut->OutputString (gST->ConOut, onechar))) {
            return Status;
        }
        str++;
    }

    return Status;
}

static EFI_STATUS wait_for_input (void)
{
    EFI_STATUS Status;
    UINTN Index;

    rSif (print_string ("Press any key to continue\n"));

    rSif (gST->BootServices->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index));

    Status = gST->ConIn->Reset (gST->ConIn, 0);

    return Status;
}

static EFI_STATUS wait_for_input_no_text (void)
{
    EFI_STATUS Status;
    UINTN Index;

    rSif (gST->BootServices->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index));

    Status = gST->ConIn->Reset (gST->ConIn, 0);

    return Status;
}

static EFI_STATUS gop_init (EFI_GRAPHICS_OUTPUT_PROTOCOL **gop_p)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    rSif (print_string ("locating GOP\n"));

    if ((Status = gBS->LocateProtocol (&gop_guid, NULL, (void **)&gop))) {
        print_string ("failed to locate GOP\n");
        return Status;
    }

    rSif (print_string ("success\n"));

    *gop_p = gop;

    return Status;
}

static void *tmalloc (size_t size)
{
    void *ptr;

    if (gBS->AllocatePool(EfiLoaderData, size + sizeof(size_t), (void **)&ptr))
    {
        return NULL;
    }

    return ptr;
}

static void tfree (void *ptr)
{
    if (ptr != NULL)
    {
        gBS->FreePool (ptr);
    }
}

void *tmemset (void *s, int c, size_t n)
{
    size_t x = 0;

    for (x = 0; x < n; x++)
    {
        *((unsigned char *)s + x) = (unsigned char)c;
    }
    
    return (s);
}

static void draw_char_8x8_transparent (char ch,
                                       unsigned long foreground_color,
                                       unsigned long x,
                                       unsigned long y,
                                       const struct logical_video_buf_info *lbi_p)
{
    int ch_x, ch_y;
    unsigned char *glyph;
    unsigned long *p;

    glyph = font8x8_basic[ch];

    p = lbi_p->buf;
    p += y * lbi_p->max_x + x; 

    for (ch_y = 0; ch_y < 8; ch_y++) {
        for (ch_x = 0; ch_x < 8; ch_x++) {
            if (*glyph & (1 << ch_x)) *p = foreground_color;
            p++;
        }
        glyph++;
        p += lbi_p->max_x - 8;
    }     
}

static void draw_string (const char *str,
                         unsigned long x,
                         unsigned long y,
                         const struct logical_video_buf_info *lbi_p)
{
    while (*str) {
        draw_char_8x8_transparent (*str, 255, x, y, lbi_p);
        x += 8;
        str++;
    }
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void swap_buffer_to_screen (const struct logical_video_buf_info *log_buf_info_p,
                                   const struct physical_video_buf_info *phys_buf_info_p)
{
    unsigned long *log_p = log_buf_info_p->buf;
    unsigned char *phys_p = phys_buf_info_p->buf;
    unsigned long x, y, max_x, max_y, extra_x;

    max_x = MIN (log_buf_info_p->max_x, phys_buf_info_p->horizontal_resolution);
    max_y = MIN (log_buf_info_p->max_y, phys_buf_info_p->vertical_resolution);
    extra_x = phys_buf_info_p->pixels_per_scanline - phys_buf_info_p->horizontal_resolution;

    if (phys_buf_info_p->blue_green_red) {
        for (y = 0; y < max_y; y++) {
            for (x = 0; x < max_x; x++) {
                phys_p[0] = *log_p >> 16;
                phys_p[1] = *log_p >> 8;
                phys_p[2] = *log_p;
                phys_p += 4;
                log_p++;
            }

            phys_p += extra_x * 4;
        }
    } else {
        for (y = 0; y < max_y; y++) {
            for (x = 0; x < max_x; x++) {
                phys_p[2] = *log_p >> 16;
                phys_p[1] = *log_p >> 8;
                phys_p[0] = *log_p;
                phys_p += 4;
                log_p++;
            }

            phys_p += extra_x * 4;
        }
    }
}

static void clear_log_buf (const struct logical_video_buf_info *log_buf_info_p)
{
    tmemset (log_buf_info_p->buf,
             0,
             (sizeof (*log_buf_info_p->buf)
              * log_buf_info_p->max_x
              * log_buf_info_p->max_y));
}

static EFI_STATUS gop_test (void)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    unsigned long horizontal_resolution, vertical_resolution, pixels_per_scanline;
    void *framebuffer;
    unsigned long x, y;
    unsigned long *doublebuf;
    struct physical_video_buf_info phys_buf_info;
    struct logical_video_buf_info log_buf_info;

    rSif (gop_init (&gop));

    switch (gop->Mode->Info->PixelFormat) {

        case PixelRedGreenBlueReserved8BitColor:
            rSif (print_string ("PixelFormat: PixelRedGreenBlueReserved8BitColor\n"));
            break;

        case PixelBlueGreenRedReserved8BitColor:
            rSif (print_string ("PixelFormat: PixelBlueGreenRedReserved8BitColor\n"));
            break;

        case PixelBitMask:
            rSif (print_string ("PixelFormat PixelBitMask not supported yet\n"));
            return Status;
        
        case PixelBltOnly:
            rSif (print_string ("PixelFormat PixelBltOnly not supported yet\n"));
            return Status;

    }

    horizontal_resolution = gop->Mode->Info->HorizontalResolution;
    vertical_resolution = gop->Mode->Info->VerticalResolution;
    pixels_per_scanline = gop->Mode->Info->PixelsPerScanline;
    {
        unsigned long long p = gop->Mode->FrameBufferBase.a;

        p += ((unsigned long long)(gop->Mode->FrameBufferBase.b)) << 32;
        framebuffer = (void *)p;
    }

    rSif (print_string ("setting graphics mode\n"));
    rSif (wait_for_input ());

    if ((Status = gop->SetMode (gop, gop->Mode->Mode))) {
        print_string ("failed to set mode\n");
        return Status;
    }

    rSif (print_string ("success\n"));

    rSif (print_string ("filling screen with green\n"));
    rSif (wait_for_input ());

    for (y = 0; y < vertical_resolution; y++) {
        unsigned char *p = framebuffer;

        p += y * pixels_per_scanline * 4;
        
        for (x = 0; x < horizontal_resolution; x++) {
            p[0] = 0;
            p[1] = 200;
            p[2] = 0;
            p += 4;
        }
    }

    rSif (print_string ("success\n"));
    rSif (wait_for_input ());

    for (y = 0; y < vertical_resolution; y++) {
        unsigned char *p = framebuffer;

        p += y * pixels_per_scanline * 4;
        
        for (x = 0; x < horizontal_resolution; x++) {
            p[0] = 0;
            p[1] = 0;
            p[2] = 0;
            p += 4;
        }
    }

    doublebuf = tmalloc (sizeof (*doublebuf) * horizontal_resolution * vertical_resolution);
    if (doublebuf == NULL) {
        rSif (print_string ("failed to allocate second buffer\n"));
        goto end;
    }

    log_buf_info.buf = doublebuf;
    log_buf_info.max_x = horizontal_resolution;
    log_buf_info.max_y = vertical_resolution;

    phys_buf_info.buf = framebuffer;
    phys_buf_info.horizontal_resolution = horizontal_resolution;
    phys_buf_info.vertical_resolution = vertical_resolution;
    phys_buf_info.pixels_per_scanline = pixels_per_scanline;
    phys_buf_info.blue_green_red = gop->Mode->Info->PixelFormat == PixelBlueGreenRedReserved8BitColor;

    rSif (print_string ("going to draw red text\n"));
    rSif (wait_for_input ());

    clear_log_buf (&log_buf_info);
    swap_buffer_to_screen (&log_buf_info, &phys_buf_info);

    {
        const char *p;
        const char the_text[] = "The quick brown fox jumps over the lazy dog.";

        x = 16;
        y = 16;

        for (p = the_text; *p; p++) {
            draw_char_8x8_transparent (*p, 255, x, y, &log_buf_info);
            x += 8;
            y += 1;
        }

        draw_string ("done", 0, 240, &log_buf_info);
        draw_string ("Press any key to continue", 0, 248, &log_buf_info);
        swap_buffer_to_screen (&log_buf_info, &phys_buf_info);
        rSif (wait_for_input_no_text ());
    }
    
    tfree (doublebuf);

end:
    if ((Status = gop->SetMode (gop, gop->Mode->Mode))) {
        print_string ("failed to set mode\n");
        return Status;
    }

    return Status;
}

EFI_STATUS efimain (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINT64 dummy_watchdog_code = {0, 0};
    
    gIH = ImageHandle;
    gST = SystemTable;
    gBS = gST->BootServices;

    if ((Status = __gST->BootServices->SetWatchdogTimer (0, dummy_watchdog_code, 0, (CHAR16 *)0))) {
        switch (Status & 0xff) {

            case EFI_INVALID_PARAMETER:
                print_string ("SetWatchdogTimer EFI_INVALID_PARAMETER\n");
                break;

            case EFI_UNSUPPORTED:
                print_string ("SetWatchdogTimer EFI_UNSUPPORTED\n");
                break;

            case EFI_DEVICE_ERROR:
                print_string ("SetWatchdogTimer EFI_DEVICE_ERROR\n");
                break;

            default:
                print_string ("SetWatchdogTimer other error\n");
                break;

        }

        wait_for_input ();
        return Status;
    }

    rSif (print_string ("start of test\n"));

    rSif (gop_test ());

    rSif (print_string ("end of test\n"));
    rSif (wait_for_input ());
    
    return Status;
}
