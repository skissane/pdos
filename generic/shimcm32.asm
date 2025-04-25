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
    push rbp
    mov rbp, rsp
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
    pop rbp
    ret


.global call_cm16
call_cm16:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx
    push rdi
    push rsi
    push rdx
    mov rdi, r8
# both of these moves access above their boundaries
# and pollute the upper bits - but that doesn't matter
# should probably be changed though
    mov rbx, [rdi+4]
    mov rdi, [rdi]
    mov rsi, rdi
    shr rsi, 16
    sub rsp, 8
    mov rax, cs
    mov [rsp+4], eax
    lea rax, call_cm16_end[rip]
    mov [rsp], eax
    xor rax, rax
    push rcx
    push rdx
# rdx is now free, so load it with the parameter pointer
    mov  rdx, r8
    retfq
call_cm16_end:
# return value is dx:ax - need to put in rax
# rcx can be used as a scratch register
    xor rcx, rcx
    mov rcx, dx
    shl rcx, 16
    mov cx, ax
    mov rax, rcx

    pop rdx
    pop rsi
    pop rdi
    pop rcx
    pop rbx
    pop rbp
    ret



# 64-bit code to support 16-bit callback
.global callb16m
callb16m:
#    push rbp
#    mov rbp, rsp
#
    mov rax, 0x60
#    pop rbp
    retfq



.code32

.globl test32
test32:
#    cli
#    hlt
    mov eax, 4
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
    mov ax, ss
# cx is preserved in previous function
# and is now used to preserve the current ss value
# bx contains the new ss value to be used
    mov cx, ax
    mov ax, bx
    mov ss, ax
# dx contains parameter block
    push dx
    mov ax, 3
    push ax
    push cs
    call main16
    add sp, 4
# restore old ss, while preserving ax return value
    mov bx, ax
    mov ax, cx
    mov ss, ax
    mov ax, bx
# Using 32-bit retf is simpler than preparing for 16-bit retf
    data32 retf


main16:
    push si
    push di
    retf
#    push bp
#    mov bp, sp
#    mov ax, [bp + 4]
#    add ax, 2
#    pop bp
#    ret



# callback function
.globl callb16
callb16:
    push bp
    mov bp, sp
    push es
    push bx
#    mov ax, 0x30
    les bx, 8[bp]
#    mov ax, bx
#    mov dx, es
    mov ax, es:[bx + 28]
    mov dx, es:[bx + 30]
    mov ax, es:[bx + 32]
    mov dx, es:[bx + 34]
    push dx
    push ax
    retf

# I don't know if there is a better way of doing this
# Even assuming the code isn't allowed to span a 64k segment
#
.globl callb16r
callb16r:
    pop bx
    pop es
    pop bp
    retf


.code64
