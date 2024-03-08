; os2strt.asm - startup code for OS/2 2.0+ C programs
;
; This program written by Paul Edwards
; Released to the public domain

.386p

.model flat, c

;.stack 4000h

.code

top:


; We don't use the registers or stack at entry

public __main
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


__main proc
        ret
__main endp


; These routines copied from winsupa.asm

public __setj
__setj proc
        mov eax, [esp+4]
        push ebx
        mov ebx, esp
        mov [eax+20], ebx    ; esp

        mov ebx, ebp
        mov [eax+24], ebx    ; ebp

        mov [eax+4], ecx
        mov [eax+8], edx
        mov [eax+12], edi
        mov [eax+16], esi

        mov ebx, [esp+4]   ; return address
        mov [eax+28], ebx    ; return address

        pop ebx
        mov [eax+0], ebx
        mov eax, 0

        ret
__setj endp

public __longj
__longj proc
        mov eax, [esp+4]
        mov ebp, [eax+20]
        mov esp, ebp

        pop ebx                ; position of old ebx
        pop ebx                ; position of old return address

        mov ebx, [eax+28]    ; return address
        push ebx

        mov ebx, [eax+24]
        mov ebp, ebx

        mov ebx, [eax+0]
        mov ecx, [eax+4]
        mov edx, [eax+8]
        mov edi, [eax+12]
        mov esi, [eax+16]

; Need to account for 7 dwords of filler
        mov eax, [eax+28+7*4+4]    ; return value

        ret
__longj endp


end top
