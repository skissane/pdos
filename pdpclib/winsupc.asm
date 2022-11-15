# winsupc.asm - support code for C programs for Win32
#
# This program written by Paul Edwards
# Released to the public domain

.text
.intel_syntax noprefix

.globl ___setj
.globl ___longj
.globl ___chkstk_ms
.globl ___switch

___setj:
        mov eax, [esp+4]
        push ebx
        mov ebx, esp
        mov [eax+20], ebx    # esp

        mov ebx, ebp
        mov [eax+24], ebx    # ebp

        mov [eax+4], ecx
        mov [eax+8], edx
        mov [eax+12], edi
        mov [eax+16], esi

        mov ebx, [esp+4]   # return address
        mov [eax+28], ebx    # return address

        pop ebx
        mov [eax+0], ebx
        mov eax, 0

        ret


___longj:
        mov eax, [esp+4]
        mov ebp, [eax+20]
        mov esp, ebp

        pop ebx                # position of old ebx
        pop ebx                # position of old return address

        mov ebx, [eax+28]    # return address
        push ebx

        mov ebx, [eax+24]
        mov ebp, ebx

        mov ebx, [eax+0]
        mov ecx, [eax+4]
        mov edx, [eax+8]
        mov edi, [eax+12]
        mov esi, [eax+16]

        mov eax, [eax+60]    # return value

        ret


# For compiling with GCC 4 we don't want to
# have to link the GCC library in
___chkstk_ms:
        ret


# From SubC, for SubC, then modified for intel syntax
# internal switch(expr) routine
# %esi = switch table, %eax = expr

___switch:
	push	esi
	mov	esi,edx
	mov	ebx,eax
	cld
	lodsd
	mov	ecx,eax
next:	lodsd
	mov	edx,eax
	lodsd
	cmp	ebx,edx
	jnz	no
	pop	esi
	jmp	eax
no:	loop	next
	lodsd
	pop	esi
	jmp	eax
