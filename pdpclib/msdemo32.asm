; msdemo32.asm - Win32 demo program
;
; This program written by Paul Edwards
; Released to the public domain

.386
.model flat, c

.code

; When running in a 64-bit environment, there will potentially
; be 64-bit registers set to certain values, but we're not using
; any of them, so we can ignore that (and the caller doesn't
; need to do anything either).

; In a 64-bit environment, with executables running in the first
; 4 GB (deliberately constrained), rsp/rbp will have the high 32 bits
; set to 0, and we won't disturb that when we manipulate esp/ebp

; But in a 64-bit environment, we can't use "push" of a 32-bit
; value, so we need to manipulate the stack instead.

; We also need to ensure that the stack is 16-byte aligned in case
; the 64-bit environment executes SSE2 instructions. We could
; potentially require the 32 to 64-bit "glue" code do the appropriate
; alignment, but for now, we do it.

public mainCRTStartup
mainCRTStartup:
        sub     esp, 4
        mov     dword ptr [esp], ebp
	mov	ebp, esp
        add     ebp, 4
	sub	esp, 12 ; back to 16-byte alignment

; Now we are going to pass a 4-byte parameter, plus the return
; address, so that's 8 bytes. So we need an additional 8 bytes
; to get to 16-byte alignment
        sub     esp, 16
        mov     eax, offset lc0
        mov     dword ptr [esp + 4], eax
        mov     eax, offset retaddr1
        mov     dword ptr [esp], eax
        mov     eax, offset puts
        jmp     eax
retaddr1:
        add     esp, 16
        
        sub     esp, 16
	mov	dword ptr [esp], 0
        mov     eax, offset exit
	jmp	eax
retaddr2:
        add     esp, 16

lc0 db "hello, world 32", 0Ah, 00h

end
