; fninit.asm - initialize floating point
; 
; This program written by Paul Edwards
; Released to the public domain

ifndef AS86
.model tiny

.stack 400h

.code
endif

top:

fninit
mov al, 0
mov ah, 4ch
int 21h

end top
