; msdemo32.asm - Win32 demo program
;
; This program written by Paul Edwards
; Released to the public domain

.386
.model flat, c

.code

LC0 db "hello, world 32", 0Ah, 00h

public mainCRTStartup
mainCRTStartup:
	push	ebp
	mov	ebp, esp
	sub	esp, 20
	push	OFFSET LC0
	call	puts
	mov	DWORD PTR [esp], 0
	call	exit

end
