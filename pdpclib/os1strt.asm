; os1strt.asm - startup code for OS/2 1.x (16-bit) C programs
;
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

.stack 4000h

extrn __start:proc, __myDosExit:proc
extrn DosHugeShift:proc
extrn DosHugeIncr:proc
extrn __shift:word
extrn __incr:word

.data
banner  db  "PDPCLIB"

parm db "X Y"

.code

top:


public __main
public __intstart
__intstart proc
;        mov ax, [sp+16]
;        push ax
        mov dx,DGROUP
        mov ds,dx
        mov ax, offset DosHugeShift ; aka AHSHIFT
        mov __shift, ax
        mov ax, offset DosHugeIncr ; aka AHINCR
        mov __incr, ax
        mov ax, seg parm
        push ax
        mov ax, offset parm
        push ax
        call __start
        sub sp,4
        mov ax, 3
        push ax
        call __exita
__intstart endp


public __exita
__exita proc
        pop ax
        pop ax
        push ax
        mov cx, 1
        push cx
        call __myDosExit
        sub sp,4
__exita endp


__main proc
        ret
__main endp


end top
