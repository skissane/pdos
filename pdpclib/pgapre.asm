; pgapre.asm - prefix code for PDOS-generic 16-bit programs
;
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

extrn __crt0: proc

.stack 4000h

.data

; eyecatcher and padding
public __eyepad
__eyepad db 'XXXXXXXXXXXXXXXX'

.code

top:

public __intstart2
__intstart2 proc
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
jmp __crt0
__intstart2 endp

end top
