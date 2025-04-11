# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

.code64
.intel_syntax noprefix


.globl get_cs
get_cs:
    mov rax, cs
    ret

.globl disable_interrupts
disable_interrupts:
    cli
    ret

.globl enable_interrupts
enable_interrupts:
    sti
    ret

.globl save_gdt
save_gdt:
    sgdt [rcx]
    ret

.globl load_gdt
load_gdt:
    sub rsp, 16
    mov rax, rsp
    dec rdx
    mov [rax], rdx
    mov [rax+2], rcx
    lgdt [rax]
    add rsp, 16
    ret

.global call_cm32
call_cm32:
    sub rsp, 8
    mov rax, cs
    mov [rsp+4], eax
    lea rax, call_cm32_end[rip]
    mov [rsp], eax
    push rcx
    push rdx
    retfq
call_cm32_end:
    ret



# This requires the test16 to be in the same 64k
# segment. We need another routine after the test16
# and use that instead, if this one is not suitable

# Note that this creates a gap of 4 bytes on the stack
# that will need to be compensated for by test16
# Note that this is to keep the stack 8-byte aligned
# which I think is a requirement

.global call_cm16
call_cm16:
    sub rsp, 8
    mov rax, cs
    mov [rsp+6], ax
    lea rax, call_cm16_end[rip]
    mov [rsp+4], ax
    push rcx
    push rdx
    retfq
call_cm16_end:
    ret



.code32

.globl test32
test32:
#    cli
#    hlt
    retf



.code16

.globl test16
test16:
#    cli
#    hlt
# adding "-debugcon stdio" to qemu allows this to appear
# Although I got a different character than 'X' for some reason
#    mov al, 'X'
#    out 0xe9, al
#    mov al, '\n'
#    out 0xe9, al
#
# Need to compensate as described above
# Still doesn't work though
    add sp, 4
    retf



.code64
