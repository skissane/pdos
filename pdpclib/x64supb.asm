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

.data

.globl _fltused
.p2align 2
_fltused:
.space 4

# These routines were copied (and then modified) from bcclib.asm generated
# by the public domain cc64, and are used for cc64

#Float routines for unsigned
#Input passed in D10
#Output in XMM15

.globl m$ufloat_r64u32
m$ufloat_r64u32:
#	mov D10,D10					# clear top half (already done if value just moved there)
#	cvtsi2sd XMM15,D10
	ret

.globl m$ufloat_r32u32
m$ufloat_r32u32:
#	mov D10,D10
#	cvtsi2ss XMM15,D10
	ret

.globl m$ufloat_r64u64
m$ufloat_r64u64:
#	cmp D10,0
	jl fl1
#number is positive, so can treat like i64
#	cvtsi2sd XMM15,D10
	ret
fl1:						#negative value
#	and D10,[mask63]		#clear top bit (subtract 2**63)
#	cvtsi2sd XMM15,D10
#	addsd XMM15,[offset64]	#(add 2**63 back to result)
	ret

.globl m$ufloat_r32u64
m$ufloat_r32u64:
#	cmp D10,0
	jl fl2
#number is positive, so can treat like i64
#	cvtsi2ss XMM15,D10
	ret
fl2:						#negative value
#	and D10,[mask63]		#clear top bit (subtract 2**63)
#	cvtsi2ss XMM15,D10
#	addss XMM15,[offset32]	#(add 2**63 back to result)
	ret

.data
mask63:
#	dq 0x7fffffffffffffff
offset64:
#	dq 9223372036854775808.0		# 2**63 as r64
offset32:
#	dd 9223372036854775808.0		# 2**63 as r32
