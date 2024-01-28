; winsupa.asm - support code for C programs for Win32
;
; This program written by Paul Edwards
; Released to the public domain

.386
.model flat, c

.code

public __setj
public __longj
public __chkstk_ms
public __switch
public __umoddi3
public __udivdi3
public __moddi3
public __divdi3

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

        mov eax, [eax+32]    ; return value

        ret
__longj endp

; For compiling with GCC 4 we don't want to
; have to link the GCC library in
__chkstk_ms proc
        ret
__chkstk_ms endp


; From SubC, for SubC, then modified for intel syntax
; internal switch(expr) routine
; %esi = switch table, %eax = expr

__switch:
	push	esi
	mov	esi,edx
	mov	ebx,eax
	cld
	lodsd
	mov	ecx,eax
next:	lodsd
	mov	edx,eax
	lodsd
	cmp	ebx,edx
	jnz	no
	pop	esi
	jmp	eax
no:	loop	next
	lodsd
	pop	esi
	jmp	eax


# Note that the push and pop of ecx appears to
# be unnecessary but harmless. It was originally
# missing, but adding it didn't help

udivmodsi3:

    push    ebp
    mov     ebp,    esp
    
    push    edi
    push    esi
    push    ebx
#    push    ecx
    
    sub     esp,    36
    
    mov     ecx,    dword ptr [ebp + 8]
    mov     dword ptr [ebp - 40],   ecx
    
    mov     ecx,    dword ptr [ebp + 12]
    mov     dword ptr [ebp - 36],   ecx
    
    mov     ecx,    dword ptr [ebp + 16]
    mov     dword ptr [ebp - 48],   ecx
    
    mov     ecx,    dword ptr [ebp + 20]
    mov     dword ptr [ebp - 44],   ecx
    
    mov     dword ptr [ebp - 24],   1
    mov     dword ptr [ebp - 20],   0
    
    mov     dword ptr [ebp - 32],   0
    mov     dword ptr [ebp - 28],   0
    
    jmp     L2

L3:

    mov     ecx,    dword ptr [ebp - 48]
    mov     ebx,    dword ptr [ebp - 44]
    
    shld    ebx,    ecx,    1
    add     ecx,    ecx
    
    mov     dword ptr [ebp - 48],   ecx
    mov     dword ptr [ebp - 44],   ebx
    
    mov     ecx,    dword ptr [ebp - 24]
    mov     ebx,    dword ptr [ebp - 20]
    
    shld    ebx,    ecx,    1
    add     ecx,    ecx
    
    mov     dword ptr [ebp - 24],   ecx
    mov     dword ptr [ebp - 20],   ebx

L2:

    mov     ecx,    dword ptr [ebp - 48]
    mov     ebx,    dword ptr [ebp - 44]
    
    test    ebx,    ebx
    jns     L3
    
    jmp     L4

L6:

    mov     ecx,    dword ptr [ebp - 40]
    mov     ebx,    dword ptr [ebp - 36]
    
    cmp     ecx,    dword ptr [ebp - 48]
    sbb     ebx,    dword ptr [ebp - 44]
    jc      L5
    
    mov     ecx,    dword ptr [ebp - 48]
    mov     ebx,    dword ptr [ebp - 44]
    
    sub     dword ptr [ebp - 40],   ecx
    sbb     dword ptr [ebp - 36],   ebx
    
    mov     ecx,    dword ptr [ebp - 32]
    or      ecx,    dword ptr [ebp - 24]
    mov     eax,    ecx
    
    mov     ecx,    dword ptr [ebp - 28]
    or      ecx,    dword ptr [ebp - 20]
    mov     edx,    ecx
    
    mov     dword ptr [ebp - 32],   eax
    mov     dword ptr [ebp - 38],   edx

L5:

    mov     ecx,    dword ptr [ebp - 24]
    mov     ebx,    dword ptr [ebp - 20]
    
    shrd    ecx,    ebx,    1
    shr     ebx,    1
    
    mov     dword ptr [ebp - 24],   ecx
    mov     dword ptr [ebp - 20],   ebx
    
    mov     ecx,    dword ptr [ebp - 48]
    mov     ebx,    dword ptr [ebp - 44]
    
    shrd    ecx,    ebx,    1
    shr     ebx,    1
    
    mov     dword ptr [ebp - 48],   ecx
    mov     dword ptr [ebp - 44],   ebx

L4:

    mov     ecx,    dword ptr [ebp - 24]
    xor     ch,     0
    mov     esi,    ecx
    
    mov     ecx,    dword ptr [ebp - 20]
    xor     ch,     0
    mov     edi,    ecx
    
    mov     ecx,    edi
    or      ecx,    esi
    
    test    ecx,    ecx
    jne     L6
    
    cmp     dword ptr [ebp + 24],   0
    je      L7
    
    mov     eax,    dword ptr [ebp - 40]
    mov     edx,    dword ptr [ebp - 36]
    
    jmp     L8

L7:

    mov     eax,    dword ptr [ebp - 32]
    mov     edx,    dword ptr [ebp - 28]

L8:

    add     esp,    36
#    pop     ecx
    pop     ebx
    pop     esi
    pop     edi
    pop     ebp
    ret


__umoddi3:

    push    ebp
    mov     ebp,    esp
    
    push    dword ptr 1
    push    dword ptr [ebp + 20]
    push    dword ptr [ebp + 16]
    push    dword ptr [ebp + 12]
    push    dword ptr [ebp + 8]

    call    udivmodsi3
    add     esp,    20
   
    pop     ebp
    ret

__udivdi3:

    push    ebp
    mov     ebp,    esp

    push    dword ptr 0
    push    dword ptr [ebp + 20]
    push    dword ptr [ebp + 16]
    push    dword ptr [ebp + 12]
    push    dword ptr [ebp + 8]

    call    udivmodsi3
    add     esp,    20

    pop     ebp
    ret
    
__moddi3:

    push    ebp
    mov     ebp,    esp
    
    push    dword ptr 1
    push    dword ptr [ebp + 20]
    push    dword ptr [ebp + 16]
    push    dword ptr [ebp + 12]
    push    dword ptr [ebp + 8]

    call    udivmodsi3
    add     esp,    20
   
    pop     ebp
    ret

; bottom of stack has original number as 32:32, e.g. 50
; top of stack has number to divide by as 32:32, e.g. 10
; result, e.g. 5, should be in edx:eax
__divdi3:

    push    ebp
    mov     ebp,    esp

    push    dword ptr 0
    push    dword ptr [ebp + 20]
    push    dword ptr [ebp + 16]
    push    dword ptr [ebp + 12]
    push    dword ptr [ebp + 8]

    call    udivmodsi3
    add     esp,    20

    pop     ebp
    ret
    
end
