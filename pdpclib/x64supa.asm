ifndef PDAS
; Written by Paul Edwards
; Released to the public domain

.code

; I think rcx will have env
; And it looks like we only need to save rbx, rcx, rdx, r8, r9, rsp
; plus return address
; And first location is reserved for return value
; (second is for return address)

else
.code64
.intel_syntax noprefix
endif



ifndef PDAS
public __setj
__setj proc
else
.globl __setj
__setj:
endif

mov [rcx + 8*2], rbx
mov [rcx + 8*3], rcx
mov [rcx + 8*4], rdx
mov [rcx + 8*5], r8
mov [rcx + 8*6], r9
mov [rcx + 8*7], rsp
mov rax, [rsp]
mov [rcx + 8*1], rax

ifndef PDAS
; we only return int (eax), but since we're not preserving
; rax anyway (maybe we should?), may as well clear it.
endif

xor rax,rax
ret

ifndef PDAS
__setj endp
endif




ifndef PDAS
; I think rcx will have env
public __longj
__longj proc
else
.globl __longj
__longj:
endif

mov rax, [rcx + 8*7]
mov rsp, rax
mov rax, [rcx + 8*1]
mov [rsp], rax
mov rbx, [rcx + 8*2]
mov rdx, [rcx + 8*4]
mov r8, [rcx + 8*5]
mov r9, [rcx + 8*6]
mov rax, [rcx]
mov rcx, [rcx + 8*3]
ret

ifndef PDAS
__longj endp
endif




ifndef PDAS
public __main
__main proc
else
.globl __main
__main:
endif

	ret

ifndef PDAS
__main endp
endif




.data

ifndef PDAS
public _fltused
_fltused dd ?
else
_fltused:
.space 4
endif



ifndef PDAS
; this very odd syntax for "end" is required so that it is ignored
; by pdas, but accepted by masm. masm requires the "end" and won't
; accept the "end" within an ifdef block
endif


; /*
end
; */
