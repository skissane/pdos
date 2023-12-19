; os1strt.asm - startup code for OS/2 1.x (16-bit) C programs
;
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

.stack 4000h

extrn __start:proc, __myDosExit:proc
extrn __myDosHugeShift:proc
extrn __myDosHugeIncr:proc
extrn __shift:word
extrn __incr:word

.data
banner  db  "PDPCLIB"

.code

top:

; at entry, apparently ax is the selector to
; the environment, bx is the offset of the
; commandline within the environment, and
; cx is the size of the data segment

public __main
public __intstart
__intstart proc
        mov dx,DGROUP
        mov ds,dx

; push far pointer to __start
;        push ax
;        push bx
; we are no longer doing this because the C code
; can get these values itself and the intention
; now is to retire this assembler code. The startup
; code can be written in C instead. Watch this space.
        mov ax, 0
        push ax
        push ax

; DosHugeShift is also known as _AHSHIFT
        call __myDosHugeShift
        mov __shift, ax
; DosHugeIncr is also known as _AHINCR
        call __myDosHugeIncr
        mov __incr, ax
        call __start
        add sp,4
        mov ax, 0
        push ax
        call __exita
__intstart endp


public __exita
__exita proc
        pop ax ; offset of return address
        pop ax ; segment of return address
        pop ax ; desired return code
        push ax
        mov ax, 1 ; terminate all threads
        push ax
        call __myDosExit
        add sp,4
__exita endp


__main proc
        ret
__main endp


end top
