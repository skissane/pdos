# Simple ELF program for Linux x64

# Written by Paul Edwards
# Released the public domain

# build with:
# as -o e64test.o e64test.asm
# ld e64test.o -e myentry -s -o e64test

# linux 64-bit calling convention is rdi, rsi, rdx, rcx, r8, r9

# However, the syscall convention is rdi, rsi, rdx, r10, r8, r9



.code64

.intel_syntax noprefix




.globl __write
__write:

# function code 1 = write
# handle, data pointer, length
mov rax, 1

syscall

ret




.globl __exita
__exita:

# exit/terminate
mov rax, 60

syscall

# Shouldn't get here
loop2: jmp loop2

ret






.globl myentry
myentry:

mov rdi, 1
lea rsi, message
mov rdx, 9

sub rsp, 8
call __write
add rsp, 8



mov rdi, 0

sub rsp, 8
call __exita
add rsp, 8

# Shouldn't get here
loop3: jmp loop3


.globl message
message:
.string "hi\nthere\n"
