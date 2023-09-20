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


udivmodsi3:

    push    ebp
    mov     ebp,    esp
    
    push    edi
    push    esi
    push    ebx
    
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
    
    jmp     L2

L4:

    mov     ecx,    dword ptr [ebp - 48]
    mov     ebx,    dword ptr [ebp - 44]
    
    shld    ebx,    ecx,    1
    add     ecx,    ecx
    
    mov     dword ptr [ebp - 48],   ecx
    mov     dword ptr [ebp - 44],   ebx
    
    sal     dword ptr [ebp - 24]

L2:

    mov     ecx,    dword ptr [ebp - 48]
    mov     ebx,    dword ptr [ebp - 44]
    
    cmp     ecx,    dword ptr [ebp - 40]
    sbb     ebx,    dword ptr [ebp - 36]
    jnc     L5
    
    cmp     dword ptr [ebp - 24],   0
    jnb     L5
    
    mov     ecx,    dword ptr [ebp - 48]
    and     ecx,    -2147483648
    
    mov     eax,    ecx
    mov     ecx,    dword ptr [ebp - 44]
    and     ch,     -1
    
    mov     edi,    ecx
    mov     ebx,    eax
    xor     bh,     0
    
    mov     edi,    ebx
    mov     ebx,    edx
    xor     bh,     0
    
    mov     edi,    ebx
    mov     ecx,    edi
    or      ecx,    esi
    
    test    ecx,    ecx
    jna     L4
    
    jmp     L5

L7:

    mov     eax,    dword ptr [ebp - 40]
    mov     edx,    dword ptr [ebp - 36]
    
    cmp     eax,    dword ptr [ebp - 48]
    mov     eax,    edx
    sbb     eax,    dword ptr [ebp - 44]
    jc      L6
    
    mov     eax,    dword ptr [ebp - 48]
    mov     edx,    dword ptr [ebp - 44]
    
    sub     dword ptr [ebp - 40],   eax
    sbb     dword ptr [ebp - 36],   edx
    
    mov     eax,    dword ptr [ebp - 24]
    or      dword ptr [ebp - 20],   eax

L6:

    shr     dword ptr [ebp - 24]
    
    mov     eax,    dword ptr [ebp - 48]
    mov     edx,    dword ptr [ebp - 44]
    
    shrd    eax,    edx,    1
    shr     edx
    
    mov     dword ptr [ebp - 48],   eax
    mov     dword ptr [ebp - 44],   edx

L5:

    cmp     dword ptr [ebp - 24],   0
    jne     L7
    
    cmp     dword ptr [ebp + 24],   0
    je      L8
    
    mov     eax,    dword ptr [ebp - 40]
    mov     edx,    dword ptr [ebp - 36]
    
    jmp     L9

L8:

    mov     eax,    dword ptr [ebp - 20]
    xor     edx,    edx

L9:

    add     esp,    36
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
