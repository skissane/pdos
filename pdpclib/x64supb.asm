# Written by Paul Edwards
# Released to the public domain


.code64
.intel_syntax noprefix

# I think rcx will have env
# And it looks like we only need to save rbx, rcx, rdx, r8, r9, rsp
# plus return address
# And first location is reserved for return value
# (second is for return address)
# After getting an issue (with EFI), added rsi and rdi to preserve
# That didn't fix the problem, so I added r10-r15
# That didn't fix the problem, so I added rbp
# That fixed the problem, and I just left everything in,
# as that is probably the best thing to do anyway

.globl __setj
__setj:
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
# we only return int (eax), but since we're not preserving
# rax anyway (maybe we should?), may as well clear it.
xor rax,rax
ret

# I think rcx will have env
.globl __longj
__longj:
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

.globl __main
__main:
	ret

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
	incd [ncallbacks]
        xor rbx, rbx
	mov ebx,[ncallbacks]
	shl ebx,6					#8x8 bytes is size per entry
	lea rbx,[rbx+callbackstack]

	mov [rbx],rbx
	mov [rbx+8],rsi
	mov [rbx+16],rdi
	mov [rbx+24],r12
	mov [rbx+32],r13
	mov [rbx+40],r14
	mov [rbx+48],r15
	ret

.globl m$popcallback
m$popcallback:
        xor rbx,rbx
	mov ebx,[ncallbacks]
	shl ebx,6					#8x8 bytes is size per entry
	lea rbx,[rbx+callbackstack]
	mov rbx,[rbx]
	mov rsi,[rbx+8]
	mov rdi,[rbx+16]
	mov r12,[rbx+24]
	mov r13,[rbx+32]
	mov r14,[rbx+40]
	mov r15,[rbx+48]
	decd [ncallbacks]
	ret

.data

.globl _fltused
.p2align 2
_fltused:
.space 4

.section rdata
mask63:
	.quad 0x7fffffffffffffff
offset64:
	.quad 9223372036854775808		# 2**63 as r64
offset32:
	.quad 9223372036854775808		# 2**63 as r32


.bss
callbackstack:
	.space 576			#8-level stack
#	resb 5'120'000

ncallbacks:
	.space 4
