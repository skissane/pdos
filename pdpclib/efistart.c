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

#define gIH __gIH
#define gST __gST
#define gBS __gBS

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

#if 0

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

#if 0
static EFI_STATUS cursor_position_test (void)
{
    EFI_STATUS Status = EFI_SUCCESS;
    INT32 Mode;
    UINTN max_column, max_row;
    UINTN saved_column, saved_row;
    int i;

    return_Status_if_fail (gST->ConOut->ClearScreen (gST->ConOut));

    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 2, 2));
    return_Status_if_fail (print_string ("start of cursor_position_test"));

    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 2, 3));
    return_Status_if_fail (print_string ("drawing star box\n"));
    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 2, 4));

    Mode = gST->ConOut->Mode->Mode;
    saved_column = gST->ConOut->Mode->CursorColumn;
    saved_row = gST->ConOut->Mode->CursorRow;
    
    return_Status_if_fail (gST->ConOut->QueryMode (gST->ConOut, Mode, &max_column, &max_row));

    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 1, 1));
    for (i = 0; i < max_column - 2; i++) {
        return_Status_if_fail (print_string ("*"));
    }

    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 1, max_row - 2));
    for (i = 0; i < max_column - 2; i++) {
        return_Status_if_fail (print_string ("*"));
    }

    for (i = 2; i < max_row - 2; i++) {
        return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 1, i));
        return_Status_if_fail (print_string ("*"));
        return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, max_column - 2, i));
        return_Status_if_fail (print_string ("*"));
    }

    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, saved_column, saved_row));

    return_Status_if_fail (print_string ("deleting star box\n"));
    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 2, gST->ConOut->Mode->CursorRow));
    return_Status_if_fail (wait_for_input ());
    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 2, gST->ConOut->Mode->CursorRow));

    saved_column = gST->ConOut->Mode->CursorColumn;
    saved_row = gST->ConOut->Mode->CursorRow;

    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 1, 1));
    for (i = 0; i < max_column - 2; i++) {
        return_Status_if_fail (print_string (" "));
    }

    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 1, max_row - 2));
    for (i = 0; i < max_column - 2; i++) {
        return_Status_if_fail (print_string (" "));
    }

    for (i = 2; i < max_row - 2; i++) {
        return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, 1, i));
        return_Status_if_fail (print_string (" "));
        return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, max_column - 2, i));
        return_Status_if_fail (print_string (" "));
    }

    return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, saved_column, saved_row));

    {
        UINTN saved_attribute = gST->ConOut->Mode->Attribute;

        return_Status_if_fail (print_string ("writing light magenta text on cyan background\n"));

        return_Status_if_fail (gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTMAGENTA | EFI_BACKGROUND_CYAN));
        return_Status_if_fail (print_string ("The quick brown fox jumps over the lazy dog\n"));
        return_Status_if_fail (gST->ConOut->SetAttribute (gST->ConOut, saved_attribute));
        return_Status_if_fail (print_string ("done\n"));
        return_Status_if_fail (wait_for_input ());
    }

    return_Status_if_fail (gST->ConOut->ClearScreen (gST->ConOut));

    {
        return_Status_if_fail (print_string ("deleting half of line test\n"));

        saved_column = gST->ConOut->Mode->CursorColumn;
        saved_row = gST->ConOut->Mode->CursorRow;

        for (i = 0; i < max_column; i++) {
            print_string ("A");
        }

        return_Status_if_fail (print_string ("wrote the line\n"));
        return_Status_if_fail (wait_for_input ());

        /* Deletes last char of line. */
        return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, max_column - 1, saved_row));
        print_string (" \b");

        return_Status_if_fail (gST->ConOut->SetCursorPosition (gST->ConOut, max_column - 1, saved_row));
        for (i = 0; i < max_column / 2; i++) {
            print_string ("\b");
        }

        print_string ("\n");

        return_Status_if_fail (print_string ("deleted the half of line\n"));
    }
    
    return_Status_if_fail (print_string ("end of cursor_position_test\n"));
    return_Status_if_fail (wait_for_input ());

    return_Status_if_fail (gST->ConOut->ClearScreen (gST->ConOut));

    return Status;
}

#endif

#if 0
static EFI_STATUS ctrl_key_test (void)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID stiex_guid = EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *stiex;
    int i;

    return_Status_if_fail (print_string ("locating EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL\n"));

    if ((Status = gBS->LocateProtocol (&stiex_guid, NULL, (void **)&stiex))) {
        print_string ("failed to locate EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL\n");
        return Status;
    }

    return_Status_if_fail (print_string ("success\n"));

    return_Status_if_fail (stiex->Reset (stiex, 0));
    
    return_Status_if_fail (print_string ("press left-ctrl+d, you have 5 tries\n"));
    for (i = 0; i < 5; i++) {
        UINTN Index;
        EFI_KEY_DATA KeyData;
        
        if ((gBS->WaitForEvent (1, &stiex->WaitForKeyEx, &Index) != EFI_SUCCESS)
            || (stiex->ReadKeyStrokeEx (stiex, &KeyData) != EFI_SUCCESS)) {
            return print_string ("error occured\n\n");
        }

        if (!(KeyData.KeyState.KeyShiftState & EFI_SHIFT_STATE_VALID)) {
            return print_string ("device does not support KeyShiftState\n");
        }

        if (KeyData.Key.UnicodeChar == 'd') {
            return_Status_if_fail (print_string ("'d' pressed\n"));
            if (KeyData.KeyState.KeyShiftState & EFI_LEFT_CONTROL_PRESSED) {
                return print_string ("you pressed left-ctrl+d, test finished\n");
            }
        }

        return_Status_if_fail (print_string ("wrong, press left-ctrl+d\n"));
    }

    return Status;
}
#endif

#if 0

#define STATUS_IS_ERROR(a) (!!((a) >> (sizeof (a) * 8 - 1)))
#define STATUS_GET_CODE(a) (((a) << 1) >> 1)

static EFI_STATUS dir_list (EFI_FILE_PROTOCOL *dir)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN Read, buf_size;
    EFI_FILE_INFO *Buffer;

    buf_size = sizeof (*Buffer) + 8 /* Just enough space for 5 CHAR16 FileName for demonstration. */;

    return_Status_if_fail (gBS->AllocatePool (EfiLoaderData, buf_size, (void **)&Buffer));

    while (1) {
        Read = buf_size;
        Status = dir->Read (dir, &Read, Buffer);
        if (STATUS_IS_ERROR (Status) && STATUS_GET_CODE (Status) == EFI_BUFFER_TOO_SMALL) {
            gBS->FreePool (Buffer);
            buf_size = Read;
            return_Status_if_fail (print_string ("HAD to increase size of buffer\n"));
            return_Status_if_fail (gBS->AllocatePool (EfiLoaderData, buf_size, (void **)&Buffer));
            return_Status_if_fail (dir->Read (dir, &buf_size, Buffer));
        }

        if (!Read) break;

        return_Status_if_fail (print_string ("found directory entry: '"));
        return_Status_if_fail (gST->ConOut->OutputString (gST->ConOut, Buffer->FileName));
        return_Status_if_fail (print_string ("'\n"));
    };

    gBS->FreePool (Buffer);

    return Status;
}

static EFI_STATUS directory_test (void)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID li_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *li_protocol;
    EFI_GUID sfs_protocol_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs_protocol;
    
    EFI_FILE_PROTOCOL *EfiRoot;
    EFI_FILE_PROTOCOL *dir, *dir2;
    static UINT64 OpenModeDirReadWrite = {0x3, 0};
    static UINT64 OpenModeDirCreateReadWrite = {0x3, 0x80000000};
    static UINT64 Attributes = {0, 0};
    static UINT64 DirCreateAttributes = {0x10, 0};

    CHAR16 dir_name[] = {'a', 'b', 'c', '\0'};
    CHAR16 dir2_name[] = {'n', 'e', 'w', 'd', '\0'};
    CHAR16 dir3_name[] = {'d', 'e', 'l', 'd', '\0'};

    return_Status_if_fail (print_string ("start of directory test\n"));

    return_Status_if_fail (__gBS->HandleProtocol (__gIH, &li_guid, (void **)&li_protocol));
    return_Status_if_fail (__gBS->HandleProtocol (li_protocol->DeviceHandle, &sfs_protocol_guid, (void **)&sfs_protocol));
    return_Status_if_fail (sfs_protocol->OpenVolume (sfs_protocol, &EfiRoot));

    return_Status_if_fail (print_string ("changing to directory 'abc'\n"));

    Status = EfiRoot->Open (EfiRoot, &dir, dir_name, OpenModeDirReadWrite, Attributes);
    if (STATUS_IS_ERROR (Status) && STATUS_GET_CODE (Status) == EFI_NOT_FOUND) {
        return_Status_if_fail (print_string ("directory 'abc' not found, create it and restart test\n"));
        goto end;
    }
    if (Status) return Status;

    return_Status_if_fail (print_string ("success\n"));

    return_Status_if_fail (print_string ("creating/opening new directory 'newd'\n"));

    return_Status_if_fail (dir->Open (dir, &dir2, dir2_name, OpenModeDirCreateReadWrite, DirCreateAttributes));
    
    return_Status_if_fail (print_string ("success\n"));

    dir2->Close (dir2);

    return_Status_if_fail (print_string ("trying to delete directory 'deld' in 'abc'\n"));

    Status = dir->Open (dir, &dir2, dir3_name, OpenModeDirReadWrite, Attributes);
    if (STATUS_IS_ERROR (Status) && STATUS_GET_CODE (Status) == EFI_NOT_FOUND) {
        return_Status_if_fail (print_string ("directory 'deld' not found in 'abc', create it and restart test\n"));
        goto skip;
    }
    if (Status) return Status;

    dir2->Delete (dir2);

    return_Status_if_fail (print_string ("success\n"));

skip:
    return_Status_if_fail (print_string ("listing files in EfiRoot\n"));
    return_Status_if_fail (dir_list (EfiRoot));
    return_Status_if_fail (print_string ("success\n"));

    return_Status_if_fail (print_string ("listing files in 'abc'\n"));
    return_Status_if_fail (dir_list (dir));
    return_Status_if_fail (print_string ("success\n"));

    dir->Close (dir);
end:
    EfiRoot->Close (EfiRoot);

    return_Status_if_fail (print_string ("end of directory test\n"));

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
    static EFI_GUID sp_guid = EFI_SHELL_PARAMETERS_PROTOCOL_GUID;
    EFI_SHELL_PARAMETERS_PROTOCOL *sp_protocol;
    static EFI_GUID li_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *li_protocol;
    static EFI_GUID shell_guid = EFI_SHELL_PROTOCOL_GUID;
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
    }

#if 0
    return_Status_if_fail (cursor_position_test ());
    return_Status_if_fail (ctrl_key_test ());
#endif
#if 0
    return_Status_if_fail (directory_test ());
#endif

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
