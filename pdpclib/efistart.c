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

EFI_HANDLE *__gIH;
EFI_SYSTEM_TABLE *__gST;
EFI_BOOT_SERVICES *__gBS;



#if 0

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

    return_Status_if_fail (print_string ("Opening volume\n"));

    return_Status_if_fail (obtain_Root (ImageHandle, &Root));

    return_Status_if_fail (print_string ("Opening file\n"));

    return_Status_if_fail (Root->Open (Root, &new_file, file_name, OpenMode, Attributes));

    return_Status_if_fail (print_string ("Writing file\n"));

    return_Status_if_fail (new_file->Write (new_file, &bytes_to_write, content));

    return_Status_if_fail (print_string ("Closing file\n"));

    return_Status_if_fail (new_file->Close (new_file));

    return_Status_if_fail (print_string ("Closing volume\n"));

    return_Status_if_fail (Root->Close (Root));

    return Status;

}

static EFI_STATUS block_test (EFI_HANDLE ImageHandle) {
    
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID li_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *li_protocol;
    EFI_GUID block_io_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
    EFI_BLOCK_IO_PROTOCOL *bio_protocol;
    void *buffer;
    EFI_LBA LBA = {1, 0};

    return_Status_if_fail (gBS->HandleProtocol (ImageHandle, &li_guid, (void **)&li_protocol));
    return_Status_if_fail (print_string ("Obtaining block IO protocol\n"));
    return_Status_if_fail (gBS->HandleProtocol (li_protocol->DeviceHandle, &block_io_guid, (void **)&bio_protocol));

    return_Status_if_fail (print_string ("Allocating buffer with size of a block\n"));
    return_Status_if_fail (gBS->AllocatePool (EfiLoaderData, bio_protocol->Media->BlockSize, &buffer));

    return_Status_if_fail (print_string ("Reading a block at LBA 1\n"));
    return_Status_if_fail (bio_protocol->ReadBlocks (bio_protocol,
                                                     bio_protocol->Media->MediaId,
                                                     LBA,
                                                     bio_protocol->Media->BlockSize,
                                                     buffer));

    return_Status_if_fail (print_string ("Freeing buffer\n"));
    return_Status_if_fail (gBS->FreePool (buffer));

    return Status;

}

#endif

int __start(int argc, char **argv);

void __exita(int status)
{
    return;
}

EFI_STATUS efimain (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

    EFI_STATUS Status, Status2;
    UINTN Index;
    UINT64 dummy_watchdog_code = {0, 0};
    char *argv[2] = { "prog", NULL };
    EFI_GUID sp_guid = EFI_SHELL_PARAMETERS_PROTOCOL_GUID;
    EFI_SHELL_PARAMETERS_PROTOCOL *sp_protocol;
    EFI_GUID li_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *li_protocol;
    EFI_GUID shell_guid = EFI_SHELL_PROTOCOL_GUID;
    EFI_SHELL_PROTOCOL *shell_protocol;
    int x;
    int y;
#if 0
    CHAR16 message[] = {'S','h','e','l','l',' ','t','e','s','t','\r','\n','\0'};
    UINTN message_size = sizeof (message);
#endif

    __gIH = ImageHandle;
    __gST = SystemTable;
    __gBS = __gST->BootServices;

    __gST->BootServices->SetWatchdogTimer (0, dummy_watchdog_code, 0, (CHAR16 *)0);

    if (__gBS->HandleProtocol (ImageHandle, &sp_guid, (void **)&sp_protocol) == EFI_SUCCESS)
    {
        for (x = 0; x < sp_protocol->Argc; x++)
        {
            for (y = 0; sp_protocol->Argv[x][y] != 0; y++)
            {
                *(((char *)sp_protocol->Argv[x]) + y) = (char)sp_protocol->Argv[x][y];
            }
            *(((char *)sp_protocol->Argv[x]) + y) = '\0';
        }
        __start(sp_protocol->Argc, (char **)sp_protocol->Argv);
        return (EFI_SUCCESS);
    }

#if 0
    __gBS->HandleProtocol (ImageHandle, &li_guid, (void **)&li_protocol);
    __gBS->HandleProtocol (li_protocol->ParentHandle, &shell_guid, (void **)&shell_protocol);
    shell_protocol->WriteFile (sp_protocol->StdOut, &message_size, message);
#endif

    __start(1, argv);

    return (EFI_SUCCESS);
}


#if 0
    if ((Status = print_string ("Hello, world!\n"))) {
        return Status;
    }

    wait_for_input ();
    
    return_Status_if_fail (print_string ("\nFile test started\n"));
    if ((Status2 = file_test (ImageHandle))) {
        
        return_Status_if_fail (print_string ("File test FAILED\n"));
        wait_for_input ();
        return Status2;
        
    }

    return_Status_if_fail (print_string ("\nBlock test started\n"));
    if ((Status2 = block_test (ImageHandle))) {
        
        return_Status_if_fail (print_string ("Block test FAILED\n"));
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
#endif

#if 0
    printf ("Testing EFI AllocatePages\n");
    {
        /* force allocation to be done below 2 GiB */
        EFI_PHYSICAL_ADDRESS mem = {0x80000000, 0};
        UINTN num_pages = 5;

        if (__gBS->AllocatePages (AllocateMaxAddress, EfiLoaderData, num_pages, &mem)) {
            printf ("failed to allocate pages\n");
            return (EXIT_FAILURE);
        }
        printf ("successfully allocate pages at address 0x%08x%08x\n", mem.b, mem.a);
        memset ((void *)(mem.a), '\0', num_pages * 4096);
        printf ("success memset\n");
        if (__gBS->FreePages (mem, num_pages)) {
            printf ("failed to free pages\n");
            return (EXIT_FAILURE);
        }
        printf ("success freeing\n");
    }
#endif

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
