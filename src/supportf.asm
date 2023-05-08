; support routines
; written by Paul Edwards
; released to the public domain

.386p
.model flat, c

        public int86
        public int86x
        public enable
        public disable
        public __setj
        public __longj
        public inp
        public inpw
        public inpd
        public outp
        public outpw
        public outpd
        public hltintgo
        public hltinthit
        public __switch
        public __brkpoint
        public __brkpoint2

        .code

; language C doesn't seem to be working for wasm

ifdef WATCOM
public _int86x
public _int86
endif

ifdef WATCOM
_int86x:
_int86:
endif

; Because of the C calling convention, and the fact that the seg
; regs are the last parameter, and they're not actually used, the
; entry point for _int86 can be reused for _int86x
int86x:
int86:
        push    ebp
        mov     ebp, esp
        push    eax
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     esi, 12[ebp]
        mov     eax, 0[esi]
        mov     ebx, 4[esi]
        mov     ecx, 8[esi]
        mov     edx, 12[esi]

        ; preserve ebp
        push    ebp
        ; next is actually ebp
        mov     edi, 32[esi]
        ; new ebp ready on stack
        push    edi

        mov     edi, 20[esi]
        mov     esi, 16[esi]
              
        cmp dword ptr    8[ebp], 010h
        jne     not10
        pop     ebp
        int     010h
        jmp     fintry
not10:

        cmp dword ptr    8[ebp], 013h
        jne     not13
        pop     ebp
        int     013h
        jmp     fintry
not13:

        cmp dword ptr    8[ebp], 014h
        jne     not14
        pop     ebp
        int     014h
        jmp     fintry
not14:

        cmp dword ptr    8[ebp], 015h
        jne     not15
        pop     ebp
        int     015h
        jmp     fintry
not15:

        cmp dword ptr    8[ebp], 016h
        jne     not16
        pop     ebp
        int     016h
        jmp     fintry
not16:

        cmp dword ptr    8[ebp], 01Ah
        jne     not1A
        pop     ebp
        int     01Ah
        jmp     fintry
not1A:

        cmp dword ptr    8[ebp], 020h
        jne     not20
        pop     ebp
        int     020h
        jmp     fintry
not20:

        cmp dword ptr    8[ebp], 021h
        jne     not21
        pop     ebp
        int     021h
        jmp     fintry
not21:

        cmp dword ptr    8[ebp], 025h
        jne     not25
        pop     ebp
        int     025h
        jmp     fintry
not25:

        cmp dword ptr    8[ebp], 026h
        jne     not26
        pop     ebp
        int     026h
        jmp     fintry
not26:

        cmp dword ptr    8[ebp], 080h
        jne     not80
        pop     ebp
        int     080h
        jmp     fintry
not80:

; Copied BIOS interrupts.
        cmp dword ptr    8[ebp], 0A0h
        jne     notA0
        pop     ebp
        int     0A0h
        jmp     fintry
notA0:

        cmp dword ptr    8[ebp], 0A3h
        jne     notA3
        pop     ebp
        int     0A3h
        jmp     fintry
notA3:

        cmp dword ptr    8[ebp], 0A4h
        jne     notA4
        pop     ebp
        int     0A4h
        jmp     fintry
notA4:

        cmp dword ptr    8[ebp], 0A5h
        jne     notA5
        pop     ebp
        int     0A5h
        jmp     fintry
notA5:

        cmp dword ptr    8[ebp], 0A6h
        jne     notA6
        pop     ebp
        int     0A6h
        jmp     fintry
notA6:

        cmp dword ptr    8[ebp], 0AAh
        jne     notAA
        pop     ebp
        int     0AAh
        jmp     fintry
notAA:

; any unknown interrupt still needs to clean up ebp
        pop     ebp

fintry:
        ; new result
        push    ebp
        push    esi
        ; this is the old value, not new result
        mov     ebp, 8[esp]
        mov     esi, 16[ebp]
        mov     0[esi], eax
        ; actually new ebp
        mov     eax, 4[esp]
        ; new ebp
        mov     32[esi], eax
        mov     4[esi], ebx
        mov     8[esi], ecx
        mov     12[esi], edx
        mov     20[esi], edi

; this is actually esi
        pop     eax
        mov     16[esi], eax
        mov     eax, 0
        mov     24[esi], eax
        jnc     nocarry
        mov     eax, 1
        mov     24[esi], eax
nocarry:        
        pushfd
        pop     eax
        mov     28[esi], eax

        ; we already popped esi, but the new ebp hasn't been popped yet
        ; we don't actually need that value anymore, so clobber eax instead
        pop     eax
        ; we already have this value loaded, but we need to
        ; get the stack back to previous state, so pop the same value
        pop     ebp

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     ebp
        ret

enable:
        sti
        ret

disable:
        cli
        ret

; read a character from a port
inp:
        push    ebp
        mov     ebp, esp
        push    edx
        mov     edx, 8[ebp]
        mov     eax, 0
        in      al, dx
        pop     edx
        pop     ebp
        ret

; read a word from a port
inpw:
        push    ebp
        mov     ebp, esp
        push    edx
        mov     edx, 8[ebp]
        mov     eax, 0
        in      ax, dx
        pop     edx
        pop     ebp
        ret

; read a dword from a port
inpd:
        push    ebp
        mov     ebp, esp
        push    edx
        mov     edx, 8[ebp]
        mov     eax, 0
        in      eax, dx
        pop     edx
        pop     ebp
        ret

; write a character to a port
outp:
        push    ebp
        mov     ebp, esp
        push    edx
        mov     edx, 8[ebp]
        mov     eax, 12[ebp]
        out     dx, al
        pop     edx
        pop     ebp
        ret

; write a word to a port
outpw:
        push    ebp
        mov     ebp, esp
        push    edx
        mov     edx, 8[ebp]
        mov     eax, 12[ebp]
        out     dx, ax
        pop     edx
        pop     ebp
        ret

; write a dword to a port
outpd:
        push    ebp
        mov     ebp, esp
        push    edx
        mov     edx, 8[ebp]
        mov     eax, 12[ebp]
        out     dx, eax
        pop     edx
        pop     ebp
        ret


; enable interrupts and then halt until interrupt hit
hltintgo:
hloop:
; I believe hlt will be interrupted by other interrupts, like
; the timer interrupt, so we need to do it in a loop
        sti
        hlt
        cli
        jmp     hloop
hltinthit:
; remove return address, segment and flags from the stack as we
; do not intend to return to the jmp following the hlt instruction
; that was likely interrupted
        add     esp, 12
; note that interrupts will be disabled again (I think) by virtue
; of the fact that an interrupt occurred. The caller would have
; disabled interrupts already, so we are returning to the same
; disabled state.
        ret


; Basically copied from linsupa.asm

public __setj
__setj:
mov eax, 4[esp]
push ebx
mov ebx, esp
mov 20[eax], ebx

mov ebx, ebp
mov 24[eax], ebx

mov 4[eax], ecx
mov 8[eax], edx
mov 12[eax], edi
mov 16[eax], esi

mov ebx, 4[esp]   ; return address
mov 28[eax], ebx  ; return address

pop ebx
mov 0[eax], ebx
mov eax, 0

ret



public __longj
__longj:
mov eax, 4[esp]
mov ebp, 20[eax]
mov esp, ebp

pop ebx    ; position of old ebx
pop ebx    ; position of old return address

mov ebx, 28[eax]  ; return address
push ebx

mov ebx, 24[eax]
mov ebp, ebx

mov ebx, 0[eax]
mov ecx, 4[eax]
mov edx, 8[eax]
mov edi, 12[eax]
mov esi, 16[eax]

mov eax, 60[eax]   ; return value

ret


; From and for SubC
; internal switch(expr) routine
; esi = switch table, eax = expr

__switch:	push esi
	mov	esi, edx
	mov	ebx, eax
	cld
	lodsd
	mov	ecx, eax
next:	lodsd
	mov	edx, eax
	lodsd
	cmp 	ebx, edx
	jnz	no
	pop	esi
	jmp	eax
no:	loop    next
	lodsd
	pop	esi
	jmp	eax


public __brkpoint
__brkpoint:
        int     03h
        ret


public __brkpoint2
__brkpoint2:
        int     03h
        ret

end
