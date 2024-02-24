; osstart.asm - startup code for C programs
;
; This program written by Paul Edwards
; Released to the public domain
; w/mods by Matthew Parker

; This program is meant to be used as a stub for your C programs
; It is designed for OS/2

.386p

.model flat, c

;.stack 4000h

extrn __start:near, __myDosExit:near

_DATA   segment dword public use32 'DATA'
ifdef LINK386_FIX
        org 20000h
endif
banner  db  "PDPCLIB"
_DATA   ends
_BSS    segment dword public use32 'BSS'
_BSS    ends
_STACK  segment dword stack use32 'STACK'
        db 4000h dup(?)
_STACK  ends

DGROUP  group   _DATA,_BSS
        assume cs:_TEXT,ds:DGROUP

_TEXT segment 'CODE'

top:


public __main
__intstart proc
        mov eax, [esp+16]
        push eax
        call __start
        add sp,4
        push eax
        call __exita
__intstart endp


public __exita
__exita proc
        pop eax
        pop eax
        push eax
        push 1
        call __myDosExit
        add sp,8
__exita endp


__main proc
        ret
__main endp


; Note that setjmp/longjmp are untested under OS/2 since
; I don't have that environment readily available at the
; moment

public __setj
__setj proc env:dword
        mov eax, env
        push ebx
        mov ebx, esp
        push ebx               ; esp

        mov [eax + 4], ecx
        mov [eax + 8], edx
        mov [eax + 12], edi
        mov [eax + 16], esi

        pop ebx
        mov [eax + 20], ebx    ; esp
        mov ebx, [ebp + 0]
        mov [eax + 24], ebx    ; ebp

        mov ebx, [ebp + 4]     ; return address
        mov [eax + 28], ebx    ; return address

        pop ebx
        mov [eax + 0], ebx
        mov eax, 0

        ret
__setj endp


public __longj
__longj proc env:dword
        mov eax, env
        mov ebp, [eax + 20]
        mov esp, ebp
        pop ebx                ; position of old ebx
        pop ebx                ; position of old ebp
        pop ebx                ; position of old return address

        mov ebx, [eax + 28]    ; return address
        push ebx

        mov ebx, [eax + 24]    ; ebp saved as normal
        push ebx
        mov ebp, esp

        mov ebx, [eax + 0]
        mov ecx, [eax + 4]
        mov edx, [eax + 8]
        mov edi, [eax + 12]
        mov esi, [eax + 16]

        mov eax, [eax + 32]    ; return value

        ret
__longj endp


_TEXT ends

end top
