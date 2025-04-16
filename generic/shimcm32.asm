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

.global call_cm16
call_cm16:
.global call_cm32
call_cm32:
    sub rsp, 8
    mov rax, cs
    mov [rsp+4], eax
    lea rax, call_cm32_end[rip]
    mov [rsp], eax
    xor rax, rax
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
    mov eax, 7
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
    mov ax, 5
    push ax
    call main16
    add sp, 2
# Using 32-bit retf is simpler than preparing for 16-bit retf
#    mov ax, 5
    data32 retf


# Note that bp operations don't work, full ebp is presumably needed
# But that is invalid syntax
main16:
    push bp
    mov bp, sp
    mov ax, [bp+4]
    add ax, 1
    pop bp
    ret


.code64
