; pdosstrt.asm - startup code for pdos
; 
; This program written by Paul Edwards
; Released to the public domain

% .model memodel

ifdef WATCOM
extrn main_:proc
else
extrn main:proc
endif

extrn _end:byte
extrn _edata:byte

_DATA   segment word public 'DATA'
_DATA   ends
_BSS    segment word public 'BSS'
_BSS    ends
_STACK  segment word stack 'STACK'
        db 1000h dup(?)
_STACK  ends

DGROUP  group   _DATA,_BSS
        assume cs:_TEXT,ds:DGROUP

_TEXT segment word public 'CODE'

top:

ifdef WATCOM
public _cstart_
_cstart_:
endif

public ___startup
___startup proc

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


ifdef WATCOM
call far ptr main_
else
call far ptr main
endif

;call far ptr _displayc

sub sp,2
mov ax, 0
push ax
call far ptr ___exita
___startup endp

;display a 'C' just to let us know that it's working!
;public _displayc
;_displayc proc
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
;_displayc endp

public ___exita
___exita proc
pop ax
pop ax
mov ah,4ch
int 21h ; terminate
___exita endp

_TEXT ends
          
end top
