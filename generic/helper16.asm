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
__intstart proc var1: word
        mov ax, var1
        add ax, 4
        push ax
        call hlp16st
        add sp, 2

; DosHugeShift is also known as _AHSHIFT
;        call __myDosHugeShift
;        mov __shift, ax
; DosHugeIncr is also known as _AHINCR
;        call __myDosHugeIncr
;        mov __incr, ax

        ret
__intstart endp


end top
