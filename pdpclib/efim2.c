/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  efim2.c - secondary EFI entry point                              */
/*                                                                   */
/*********************************************************************/

#include "efi.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

EFI_HANDLE *__gIH;
EFI_SYSTEM_TABLE *__gST;
EFI_BOOT_SERVICES *__gBS;

#define gST __gST

int eficall2(EFI_STATUS (EFIAPI *func)(void *p1, void *p2),
    void *p1, void *p2);

static EFI_STATUS print_string (char *str) {

    EFI_STATUS Status = EFI_SUCCESS;
    CHAR16 onechar[2] = {0, '\0'};

    while (*str) {

        if (*str == '\n') {
            
            onechar[0] = '\r';
            if ((Status = eficall2(gST->ConOut->OutputString, gST->ConOut, onechar))) {
                return Status;
            }

        }

        onechar[0] = *str;
        if ((Status = eficall2(gST->ConOut->OutputString, gST->ConOut, onechar))) {
            return Status;
        }
        str++;

    }

    return Status;

}

EFI_STATUS efimain2(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    __gIH = ImageHandle;
    __gST = SystemTable;
    __gBS = __gST->BootServices;

    print_string("Hi\n");
    return (EFI_SUCCESS);
}
