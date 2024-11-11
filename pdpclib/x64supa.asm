ifndef PDAS
; Written by Paul Edwards
; Released to the public domain

.code

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

# These routines were copied (and then modified) from bcclib.asm generated
# by the public domain cc64, and are used for cc64
# (Converted to PDAS .intel_syntax noprefix by guessing.)

.globl m$ufloat_r64u32
m$ufloat_r64u32:
	mov ecx, ecx					# clear top half (already done if value just moved there)
	cvtsi2sd xmm15, rcx
	ret

.globl m$ufloat_r32u32
m$ufloat_r32u32:
	mov ecx, ecx
	cvtsi2ss xmm15, rcx
	ret

.globl m$ufloat_r64u64
m$ufloat_r64u64:
#jjj: jmp jjj
#mov rax, 0x1234567
#mov rax, 1234567
#xorpd xmm15, xmm15
#ret

	cmp ecx, 0
	jl fl1
#number is positive, so can treat like i64
	cvtsi2sd xmm15, rcx
	ret
fl1:						#negative value
	and rcx, QWORD PTR mask63[rip]		#clear top bit (subtract 2**63)
	cvtsi2sd xmm15, rcx
	addsd xmm15, QWORD PTR offset64[rip]	#(add 2**63 back to result)
	ret

.globl m$ufloat_r32u64
m$ufloat_r32u64:
	cmp ecx, 0
	jl fl2
#number is positive, so can treat like i64
	cvtsi2ss xmm15, rcx
	ret
fl2:						#negative value
	and rcx, QWORD PTR mask63[rip]		#clear top bit (subtract 2**63)
	cvtsi2ss xmm15, rcx
	addss xmm15, DWORD PTR offset32[rip]	#(add 2**63 back to result)
	ret

.globl m$pushcallback
m$pushcallback:
	incd __ncallbacks[rip]
        push r12
	mov r12,__ncallbacks[rip]
	shl r12,6					#8x8 bytes is size per entry
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

.globl m$popcallback
m$popcallback:
        mov r12,__ncallbacks[rip]
	shl r12,6					#8x8 bytes is size per entry
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





ifdef CC64

.section rdata
mask63:
	.quad 0x7fffffffffffffff
offset64:
	.quad 9223372036854775808		# 2**63 as r64
offset32:
	.quad 9223372036854775808		# 2**63 as r32


.bss
.balign 8
callbackstack:
	.space 576			#8-level stack
#	resb 5'120'000

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
