# Written by Paul Edwards
# Released to the public domain

# Windows calling convention passes the first 4
# parameters in rcx, rdx, r8 and r9. The rest are
# put on the stack.

# SubC puts everything on the stack.

# eficall2 will convert SubC format into Windows (and EFI)
# format for any function with 2 parameters.

# Not sure if we need the subtraction of 40 from rsp, but
# that's what mingw64 generates.



# Here is the original C code used to give a basis for the
# assembler:

# #include "efi.h"

# int eficall2(EFI_STATUS (EFIAPI *func)(void *p1, void *p2),
#     void *p1, void *p2)
# {
#     EFI_STATUS Status;
    
#     Status = func(p1, p2);

#     return Status;

# }

# EFI_STATUS efimain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
# {
#     EFI_STATUS Status;
    
#     Status = efimain2(ImageHandle, SystemTable);
#     return (EFI_SUCCESS);
# }




.code64
.intel_syntax noprefix

.text


# Three parameters are:
# 1. Function to call.
# 2. First parameter to that function
# 3. Second parmater to that function

# It is intended that this should be generalized into a
# function that takes a count of number of parameters and
# then the function and parameters

.globl	eficall2
eficall2:
	sub	rsp, 40
	mov	rax, rcx
	mov	rcx, rdx
	mov	rdx, r8
	call	rax
	add	rsp, 40
	ret

.globl efimain
efimain:
	sub	rsp, 40
	call	_efimain2
	xor	eax, eax
	add	rsp, 40
	ret
