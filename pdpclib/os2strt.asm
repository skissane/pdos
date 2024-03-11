; os2strt.asm - startup code for OS/2 2.0+ C programs
;
; This program written by Paul Edwards
; Released to the public domain

.386p

.model flat, c

;.stack 4000h

.code

public __top
__top:


; We don't use the registers or stack at entry

public __intstart
__intstart proc
        mov eax, 0
        push eax
        call __start
        add esp,4
        push eax
        call __exita
        add esp, 4
        ret
__intstart endp


public __exita
__exita proc
        pop eax
        pop eax
        push eax
        push 1
        call __myDosExit
        add esp,8
zloop:  jmp zloop
__exita endp


public __main
__main proc
        ret
__main endp


end __top
