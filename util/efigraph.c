/******************************************************************************
 * @file            efi_graphtest.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <efi.h>

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

static EFI_STATUS gop_test (void)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    unsigned long horizontal_resolution, vertical_resolution, pixels_per_scanline;
    unsigned char *framebuffer;
    unsigned long x, y;

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
        unsigned char *p = framebuffer + y * pixels_per_scanline * 4;
        
        for (x = 0; x < horizontal_resolution; x++) {
            p[0] = 0;
            p[1] = 200;
            p[2] = 0;
            p += 4;
        }
    }

    rSif (print_string ("success\n"));
    rSif (wait_for_input ());

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
