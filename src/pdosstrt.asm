; pdosstrt.asm - startup code for pdos
; 
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

extrn dstart:proc

extrn _end:byte
extrn _edata:byte

.stack 1000h

.code

top:

ifdef WATCOM
public cstart_
cstart_:
endif

public __startup
__startup proc

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
call dstart

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
