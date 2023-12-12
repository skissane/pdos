; dossupd.asm - support routines for C programs for DOS
;
; This program written by Paul Edwards
; Released to the public domain

; Don't specify language because if we try to use "C"
; we need the variable to be called "end" which is a
; reserved word.

% .model memodel


; These variable names pollute the C namespace,
; which is against the C90 standard. To fix this
; you need a later version of the linker than
; comes with Microsoft C 6.0. The later versions
; have redefined them to have two underscores.

extrn _end:byte
extrn _edata:byte

.code

public ___clrbss
___clrbss proc

; we are responsible for clearing our own BSS
; in Watcom at least, the BSS is at the end of the DGROUP
; which can be referenced as _end, and the end of the
; DATA section is referenced as _edata
; We can use rep stos to clear the memory, which initializes
; using the byte in al, starting at es:di, for a length of cx

push es
push cx
push di
push ax


; temporary eyecatcher
nop
nop
nop
nop
nop


mov cx, offset DGROUP:_end
mov di, offset DGROUP:_edata

sub cx, di

; move ds (data pointer, often DGROUP, into es)
push ds
pop es
mov al, 0
rep stosb


pop ax
pop di
pop cx
pop es

ret
___clrbss endp


end
