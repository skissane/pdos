/******************************************************************************
 * @file            efitest.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include "efi.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;

#define return_Status_if_fail(func) do { if ((Status = (func))) { return Status; }} while (0)

static EFI_STATUS print_string (char *str) {

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

static EFI_STATUS wait_for_input (void) {

    EFI_STATUS Status;
    UINTN Index;

    return_Status_if_fail (print_string ("Press any key to continue\n"));

    return_Status_if_fail (gST->BootServices->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index));

    Status = gST->ConIn->Reset (gST->ConIn, 0);

    return Status;

}

static EFI_STATUS obtain_Root (EFI_HANDLE ImageHandle, EFI_FILE_PROTOCOL **Root) {

    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID li_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *li_protocol;
    EFI_GUID sfs_protocol_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs_protocol;

    return_Status_if_fail (gBS->HandleProtocol (ImageHandle, &li_guid, (void **)&li_protocol));
    return_Status_if_fail (gBS->HandleProtocol (li_protocol->DeviceHandle, &sfs_protocol_guid, (void **)&sfs_protocol));
    return_Status_if_fail (sfs_protocol->OpenVolume (sfs_protocol, Root));

    return Status;

}    

static EFI_STATUS file_test (EFI_HANDLE ImageHandle) {
    
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_FILE_PROTOCOL *Root;
    EFI_FILE_PROTOCOL *new_file;
    CHAR16 file_name[] = {'t','e','s','t','.','t','x','t','\0'};
    UINT64 OpenMode = {0x3, 0x80000000}; /* Read+Write+Create */
    UINT64 Attributes = {0, 0};
    char content[] = "This is a test file created by efistart.c.\n";
    UINTN bytes_to_write = sizeof (content) - 1;

    return_Status_if_fail (obtain_Root (ImageHandle, &Root));

    return_Status_if_fail (print_string ("Opening file\n"));

    return_Status_if_fail (Root->Open (Root, &new_file, file_name, OpenMode, Attributes));

    return_Status_if_fail (print_string ("Writing file\n"));

    return_Status_if_fail (new_file->Write (new_file, &bytes_to_write, content));

    return_Status_if_fail (print_string ("Closing file\n"));

    return_Status_if_fail (new_file->Close (new_file));

    return Status;

}

EFI_STATUS efimain (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

    EFI_STATUS Status, Status2;
    UINTN Index;
    UINT64 dummy_watchdog_code = {0xFFFFFFFFLU, 0xFFFFFFFFLU};

    gST = SystemTable;
    gBS = gST->BootServices;

    gST->BootServices->SetWatchdogTimer (0, dummy_watchdog_code, 0, (CHAR16 *)0);

    if ((Status = print_string ("Hello, world!\n"))) {
        return Status;
    }

    wait_for_input ();
    
    if ((Status2 = file_test (ImageHandle))) {
        
        return_Status_if_fail (print_string ("File test FAILED\n"));
        wait_for_input ();
        return Status2;
        
    }

    if ((Status = print_string("All tests succeeded\nPress any key to exit\n"))) {
        return Status;
    }
    
    if ((Status = gST->BootServices->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index))) {
        return Status;
    }

    Status = gST->ConIn->Reset (gST->ConIn, 0);

    return Status;
}



/*

For ARM you need:

rem https://aur.archlinux.org/packages/edk2-avmf
rem https://download-ib01.fedoraproject.org/pub/fedora/linux/development/rawhide/Everything/aarch64/os/Packages/e/edk2-arm-20220826gitba0e0e4c6a17-1.fc38.noarch.rpm
rem use 7z then zstd then 7z to extract QEMU_EFI-pflash.raw

qemu-system-arm -cpu cortex-a15 -M virt -pflash QEMU_EFI-pflash.raw -drive file=fat:rw:armtest -device ramfb -device qemu-xhci -device usb-tablet -device usb-kbd

Executable should be called:

armtest\efi\boot\bootarm.efi


For 80386 you need:

rem https://download.qemu.org/qemu-7.1.0.tar.xz
rem qemu-7.1.0\pc-bios\edk2-i386-code.fd.bz2
qemu-system-i386 -drive if=pflash,format=raw,file=edk2-i386-code.fd -drive file=fat:rw:386test

Executable should be called:

386test\efi\boot\bootia32.efi


For x64 you need:

copy this:
c:\Program Files\qemu\share\edk2-x86_64-code.fd

qemu-system-x86_64 -drive if=pflash,format=raw,file=edk2-x86_64-code.fd -drive file=fat:rw:x64test

Executable should be called:

x64test\efi\boot\bootx64.efi

*/

