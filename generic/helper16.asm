; helper16.asm - helper code for loading 16-bit NE executables
; on an x64 system
;
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

.stack 4000h

;extrn __myDosHugeShift:proc
;extrn __myDosHugeIncr:proc
;extrn __shift:word
;extrn __incr:word

extrn hlp16st:proc


.data
banner  db  "HELPER16"

.code

top:

; at entry, bx contains the new ss which is already loaded
; and cx contains the old ss

public __intstart
__intstart proc var1: word, var2: word
;
; Microsoft C assumes that es can be used as a scratch register,
; so we are responsible for preserving that
;
        push es
        mov ax, var2
        push ss
        push ax
        mov ax, var1
        add ax, 4
        push ax
        call hlp16st
        add sp, 6

; DosHugeShift is also known as _AHSHIFT
;        call __myDosHugeShift
;        mov __shift, ax
; DosHugeIncr is also known as _AHINCR
;        call __myDosHugeIncr
;        mov __incr, ax

        pop es
        ret
__intstart endp


end top
