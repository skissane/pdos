; ploadst.asm - startup code for pload
; 
; This program written by Paul Edwards
; Released to the public domain

ifdef WATCOM
% .model memodel
extrn main_:proc
else
% .model memodel, c
extrn main:proc
endif


_DATA   segment word public 'DATA'
_DATA   ends
_BSS    segment word public 'BSS'
_BSS    ends

_TEXT segment word public 'CODE'

org 0100h
top:

ifdef WATCOM
public _cstart_
_cstart_:
endif

public __startup
__startup proc

; signify MSDOS version 5 ("sys.com" looks for this byte at offset 3
; in the io.sys file).
nop
nop
mov ax, 0005h

; for standard startup, comment out this line
jmp bootstrap

standard:
; put this in in case we're running as an executable
push cs
push cs
pop ds
pop es
; free initially allocated memory
mov bx, 01111h
mov ah, 4ah
int 21h
jmp bypass

; for bootstrap
bootstrap:
push cs
pop ax
sub ax, 010h
lea bx, newstart
push ax
push bx
retf ; should return to next instruction
newstart:
push ax
push ax
pop es
pop ds
mov ss, ax
mov sp, 0fffeh

bypass:

ifdef WATCOM
call near ptr main_
else
call near ptr main
endif
sub sp,2
mov ax, 0
push ax
call near ptr __exita
__startup endp

ifdef NEED_DISPLAYC
;display a 'C' just to let us know that it's working!
public displayc
displayc proc
push ax
push bx
mov ah, 0eh
mov al, 043h
mov bl, 0
mov bh, 0
int 10h
pop bx
pop ax
ret
displayc endp
endif

public __exita
__exita proc
;myloop:
;jmp myloop
pop ax
pop ax
mov ah,4ch
int 21h ; terminate
__exita endp

_TEXT ends

end top
