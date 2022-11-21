; pdosstrt.asm - startup code for pdos
; 
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

extrn __start:proc

extrn _end:byte
extrn _edata:byte

; Although we don't use this stack, it needs to be at least 200h
; so that wlink doesn't make it 1000h when it appears in the map
.stack 200h

.data

public __psp
public __envptr
public __osver

__psp   dd  0
__envptr dd 0
__osver dw 0

.code

top:

; This is required here, but not in PDPCLIB, because
; here we are using -ecc, but we can't do that in PDPCLIB
; because we can't put an intrusive dstart in every main
; and we can't handle the unusual parameter passing either
; Here it is void
ifdef WATCOM
public cstart_
cstart_:
endif

public __startup
__startup proc

; eyecatcher
nop
nop
nop
nop

push cs
push cs
pop ds
pop es
mov ax, cs
; Keep using the pload stack until we figure
; out something better. The below code prevents
; a module from being more than 64k in size
;mov ss, ax
;mov sp, 0fffeh

mov ax, DGROUP
mov ds, ax
;call far ptr _displayc

; we are responsible for clearing our own BSS
; in Watcom at least, the BSS is at the end of the DGROUP
; which can be referenced as _end, and the end of the
; DATA section is referenced as _edata
; We can use rep stos to clear the memory, which initializes
; using the byte in al, starting at es:di, for a length of cx

mov cx, offset DGROUP:_end
mov di, offset DGROUP:_edata
sub cx, di
mov es, ax ; we still have that set from above
mov al, 0
rep stosb


; Because main is effectively a reserved word, and gets
; mangled by Watcom when we can't handle it being mangled,
; we call dstart instead, which calls main.
;call dstart
; Now that we're using the C library, let it handle it
xor ax,ax
push ax
push ax
call __start
add sp,4

;call displayc

sub sp,2
mov ax, 0
push ax
call __exita
__startup endp

;display a 'C' just to let us know that it's working!
;public displayc
;displayc proc
;push ax
;push bx
;mov ah, 0eh
;mov al, 043h
;mov bl, 0
;mov bh, 0
;int 10h
;pop bx
;pop ax
;ret
;displayc endp

public __exita
__exita proc
pop ax
pop ax
mov ah,4ch
int 21h ; terminate
__exita endp


end top
