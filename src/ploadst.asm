; ploadst.asm - startup code for pload
; 
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

extrn dstart:proc

ifndef MAKECOM
.stack 1000h
endif

.code

ifdef MAKECOM
org 0100h
endif

top:

public _startup
_startup proc

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

ifdef NEWMODEL
mov ax, 0

else
push cs
pop ax
sub ax, 010h
endif

lea bx, newstart
push ax
push bx
retf ; should return to next instruction
newstart:

ifdef NEWMODEL
mov dx, DGROUP
mov ds,dx
mov es,dx
mov ss, dx
mov sp, 02580h

else
push ax
push ax
pop es
pop ds
mov ss, ax
mov sp, 0fffeh
endif


bypass:

; see "nop" further down
;call displayc

; We need to call dstart instead of __start because we're
; not linking in the C library. And dstart needs to go into
; the main C code so that it gets the name mangling correct.
; It can be put into another source file if main is declared
; with __watcall
call dstart

mov ax, 0
push ax
call _exita
_startup endp

; There appears to be a bug in wasm in Open Watcom 1.6
; where references to displayc actually hit the byte
; before. At least with current alignment. But adding
; a "nop" safeguards against that.
nop

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

public _exita
_exita proc
;myloop:
;jmp myloop
pop ax
mov ah,4ch
int 21h ; terminate
_exita endp

ifdef WATCOM
ifndef NEWMODEL
public cstart_
cstart_:
ret
endif
endif


end top
