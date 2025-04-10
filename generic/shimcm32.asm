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
    retf



.code64
