ifndef PDAS
; Written by Paul Edwards
; Released to the public domain

ifdef SASM
.amd64
else
.code
endif

; I think rcx will have env
; And it looks like we only need to save rbx, rcx, rdx, r8, r9, rsp
; plus return address
; And first location is reserved for return value
; (second is for return address)
; Seems we need to preserve more registers

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
mov [rcx + 8*8], rsi
mov [rcx + 8*9], rdi
mov [rcx + 8*10], r10
mov [rcx + 8*11], r11
mov [rcx + 8*12], r12
mov [rcx + 8*13], r13
mov [rcx + 8*14], r14
mov [rcx + 8*15], r15
mov [rcx + 8*16], rbp
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
mov rsi, [rcx + 8*8]
mov rdi, [rcx + 8*9]
mov r10, [rcx + 8*10]
mov r11, [rcx + 8*11]
mov r12, [rcx + 8*12]
mov r13, [rcx + 8*13]
mov r14, [rcx + 8*14]
mov r15, [rcx + 8*15]
mov rbp, [rcx + 8*16]
mov rax, [rcx]
mov rcx, [rcx + 8*3]
ret

ifndef PDAS
__longj endp
endif



ifndef PDAS
; suppressing main is needed if it is in p32start.c
endif

ifndef NOUNDMAIN

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

endif



ifdef CC64

ifndef PDAS
; These routines were copied (and then modified) from bcclib.asm generated
; by the public domain cc64, and are used for cc64
; (Converted to PDAS .intel_syntax noprefix by guessing.)
endif


ifndef PDAS
public m$ufloat_r64u32
else
.globl m$ufloat_r64u32
endif
m$ufloat_r64u32:

ifndef PDAS
; clear top half (already done if value just moved there)
endif
	mov ecx, ecx
	cvtsi2sd xmm15, rcx
	ret

ifndef PDAS
public m$ufloat_r32u32
else
.globl m$ufloat_r32u32
endif
m$ufloat_r32u32:
	mov ecx, ecx
	cvtsi2ss xmm15, rcx
	ret

ifndef PDAS
public m$ufloat_r64u64
else
.globl m$ufloat_r64u64
endif
m$ufloat_r64u64:

ifndef PDAS
;jjj: jmp jjj
;mov rax, 0x1234567
;mov rax, 1234567
;xorpd xmm15, xmm15
;ret
endif

	cmp ecx, 0
	jl fl1
ifndef PDAS
;number is positive, so can treat like i64
endif
	cvtsi2sd xmm15, rcx
	ret
fl1:
ifndef PDAS
;negative value
endif


ifndef PDAS
;clear top bit (subtract 2**63)
endif
	and rcx, QWORD PTR mask63[rip]
	cvtsi2sd xmm15, rcx
ifndef PDAS
;(add 2**63 back to result)
endif
	addsd xmm15, QWORD PTR offset64[rip]
	ret



ifndef PDAS
public m$ufloat_r32u64
else
.globl m$ufloat_r32u64
endif

m$ufloat_r32u64:
	cmp ecx, 0
	jl fl2
ifndef PDAS
;number is positive, so can treat like i64
endif
	cvtsi2ss xmm15, rcx
	ret
fl2:
ifndef PDAS
;negative value
endif


ifndef PDAS
;clear top bit (subtract 2**63)
endif
	and rcx, QWORD PTR mask63[rip]
	cvtsi2ss xmm15, rcx
ifndef PDAS
;(add 2**63 back to result)
endif
	addss xmm15, DWORD PTR offset32[rip]
	ret


ifndef PDAS
public m$pushcallback
else
.globl m$pushcallback
endif
m$pushcallback:
	incd __ncallbacks[rip]
        push r12
	mov r12,__ncallbacks[rip]
ifndef PDAS
;8x8 bytes is size per entry
endif
	shl r12,6
	lea r11,callbackstack[rip]
	add r11,r12
        pop r12

	mov [r11],rbx
	mov [r11+8],rsi
	mov [r11+16],rdi
	mov [r11+24],r12
	mov [r11+32],r13
	mov [r11+40],r14
	mov [r11+48],r15
	ret


ifndef PDAS
public m$popcallback
else
.globl m$popcallback
endif
m$popcallback:
        mov r12,__ncallbacks[rip]
ifndef PDAS
;8x8 bytes is size per entry
endif
	shl r12,6
	lea r11,callbackstack[rip]
	add r11,r12
	mov rbx,[r11]
	mov rsi,[r11+8]
	mov rdi,[r11+16]
	mov r12,[r11+24]
	mov r13,[r11+32]
	mov r14,[r11+40]
	mov r15,[r11+48]
	decd __ncallbacks[rip]
	ret

endif





.data

ifndef PDAS
public _fltused
_fltused dd ?
else
.globl _fltused
.p2align 2
_fltused:
.space 4
endif







ifdef CC64

.section rdata
mask63:
	.quad 0x7fffffffffffffff
offset64:

ifndef PDAS
; 2**63 as r64
endif
	.quad 9223372036854775808
offset32:
ifndef PDAS
; 2**63 as r32
endif
	.quad 9223372036854775808


.bss
.balign 8
callbackstack:
ifndef PDAS
;8-level stack
endif
	.space 576
ifndef PDAS
;	resb 5'120'000
endif

.balign 8
.globl __ncallbacks
__ncallbacks:
	.space 8

endif





ifndef PDAS
; this very odd syntax for "end" is required so that it is ignored
; by pdas, but accepted by masm. masm requires the "end" and won't
; accept the "end" within an ifdef block
endif

; /*
end
; */
