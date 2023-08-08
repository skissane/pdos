/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  efidummy.c - simply return 0, which will hopefully cause an      */
/*  EFI shell to be started, on a system where that normally isn't   */
/*  an option. Note that there are systems where this trick doesn't  */
/*  work.                                                            */
/*                                                                   */
/*  After building this, you will need to name the executable        */
/*  \EFI\BOOT\BOOTX64.EFI                                            */
/*                                                                   */
/*********************************************************************/

#include "efi.h"

EFI_STATUS efimain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    return (EFI_SUCCESS);
}
