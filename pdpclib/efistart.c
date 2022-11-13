/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*  startup code for EFI applications                                */
/*                                                                   */
/*********************************************************************/

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

*/


typedef struct {
    char junk1[4];
    int (*print_func)(void *x, void *y);
} EFI_SIMPLE_TEXT;

typedef struct {
    char junk1[40];
    void *stdout_handle;
    EFI_SIMPLE_TEXT *simple;
} EFI_SYSTEM;


static EFI_SYSTEM *system;

static int print_string(char *str);

int efimain(int junk, EFI_SYSTEM *sys)
{
    system = sys;

    print_string("hello, world\n");

    print_string("looping now\n");

    for (;;) ;

    return (0);
}

static int print_string(char *str)
{
    static char onechar[4];
    int x = 0;

    while (str[x] != '\0')
    {
        if (str[x] == '\n')
        {
            onechar[0] = '\r';
            system->simple->print_func(system->simple, onechar);
        }
        onechar[0] = str[x];
        system->simple->print_func(system->simple, onechar);
        x++;
    }
    return (x);
}
