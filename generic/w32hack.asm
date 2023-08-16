# Written by Paul Edwards
# Released to the public domain

# Glue functions for supporting Win32 executables in
# a 64-bit environment

.code64
.intel_syntax noprefix

# 
.align 8
.globl w32puts
w32puts:

push rbp
mov rbp,rsp

xor rcx, rcx
# 8 bytes for rbp followed by 4 bytes for return address
# gives the 4 bytes for parameter
mov ecx, 12[rbp]

# Ensure stack is 16-byte aligned
# by NOT padding, because we have an early push of rbp
#push rax
call puts
#pop rax

# return code 0
xor rax,rax

# Load return address
xor rcx, rcx
mov ecx, 8[rbp]
pop rbp

# Clear return address
add rsp, 8

jmp rcx

ret
