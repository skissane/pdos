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
    push rdi
    push rbx

    mov rdi, r8
    sub rsp, 16
    mov rax, cs
    mov [rsp+12], eax

# new ds/es is stored at unused ss location
# we preserve that in bx
    mov rbx, [rdi + 36]

# and preserve current ds/es too
    mov [rdi+40], ds
    mov [rdi+44], es

# and cs
    mov [rdi+24], cs

    lea rax, call_cm32_end[rip]
    mov [rsp+8], eax

# We will need access to the parameter block
    mov [rsp + 4], edi
# We don't need this for 16-bit because we are taking advantage
# of the fact that helper16 will return by a retf
# Anyway, we need helper32 to return to this address first
    lea rax, test32_end[rip]
    mov [rsp], eax

# Carry the address of helper32 in edi/rdi
# This accesses more memory than it really should
    mov rdi, [rdi]

    xor rax, rax

    push rcx
    push rdx
    retfq
call_cm32_end:

# gain access to parms again
    mov rdi, r8
# restore ds and es
    mov ds, [rdi+40]
    mov es, [rdi+44]

    pop rbx
    pop rdi
    pop rbp
    ret


# Note that ideally we would subtract FFE0 bytes
# from the UEFI-provided stack and then max the sp
# out to FFC0 so that our 16-bit apps start with a
# nearly full 64k stack. The odd numbers are to
# avoid any possible edge cases

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

# preserve old ss
    mov ax, ss
    mov [rdi+36], ax
# and old ds
    mov ax, ds
    mov [rdi+40], ax
# and old es
    mov ax, es
    mov [rdi+44], ax

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
# complete switch to standard stack
    mov ss, ax
#    push rbp
#    mov rbp, rsp
#
#    mov rax, 0x60
#    pop rbp
#    mov al, 'X'
#    out 0xe9, al
#    mov al, '\n'
#    out 0xe9, al

#    mov dx, 0x1234
#    mov ax, ss

# rbx appears to be volatile
# or maybe it is the rcx and rdx
# or others?
# i suspect rbp

# emperical testing shows that i need some of
# rdx, rbp, r8, r9
# don't need rdx
# don't need r9
# so need rbp and r8
# no idea how it could be demanding that r8 still be intact
# when I haven't yet returned to the caller of call_cm16
# Maybe it is related to calling functions with variable arguments
# Or maybe it is UEFI that expects it to be intact on the
# second call
# More experimentation suggests that I need more than just
# those 2 registers. Also the order potentially matters
# Maybe it is expecting rbp to be in a particular spot

    push rbx
    push rcx
    push rbp
    push r8
    push r10
    push r11
    call shimcm32_callback
    pop r11
    pop r10
    pop r8
    pop rbp
    pop rcx
    pop rbx

# return code is in rax
# needs to be split into dx:ax before returning to 16-bit code
    mov rdx, rax
    shr rdx, 16

    retfq



# 64-bit code to support 32-bit callback
.global callb32m
callb32m:

# Not sure which of these are required

# Note that we don't need to provide a parameter - that will
# be resurrected from the global variable

    push rbx
    push rcx
    push rbp
    push r8
    push r10
    push r11
    call shimcm32_callback
    pop r11
    pop r10
    pop r8
    pop rbp
    pop rcx
    pop rbx

# return code is in eax

    retfq



.code32

.globl test32
test32:
#    cli
#    hlt
#    mov eax, 4
#    push eax

# point to new segment registers
# things seem to work even without this
    mov ds, bx
    mov es, bx

    push edi
# call helper32
    ret
test32_end:
#    mov al, 'X'
#    out 0xe9, al
#    mov al, '\n'
#    out 0xe9, al
    add esp, 4
    retf



# callback function
.globl callb32
callb32:
    push ebp
    mov ebp, esp
    push ecx
    push ds
    push es
    push ebx

    mov ebx,[ebp+12]

# get the stack aligned on a 16-byte boundary
    mov ebp, esp
    mov eax, ebp
    sub eax, 18
    and eax, 0xFFFFFFF0
    add eax, 4
    mov esp, eax
    push ebp

# this is callb32r
# we need to prepare it for a 64-bit far return
    mov eax, 0
    push eax
    mov ax, cs
    push eax
    mov eax, 0
    push eax
    mov edx, [ebx + 32]
    push edx

# and now we do our 32-bit return to execute callb32m (64-bit)
# we need the old cs (expanded to 32 bits) first
    xor eax, eax
    mov ax, [ebx + 24]
    push eax
# and now the 32-bit offset
    mov eax, [ebx + 28]
    push eax

# Note that this is probably superfluous
    mov eax, [ebx + 40]
    mov ds, ax

# This is also probably superfluous
    mov eax, [ebx + 44]
    mov es, ax

    retf



#
.globl callb32r
callb32r:

#    mov al, 'X'
#    out 0xe9, al
#    mov al, '\n'
#    out 0xe9, al

# return code is in eax already
# but need to reinstate ds/es
# the pops should be good enough for that

# restore our 32-bit stack alignment

    pop ebp
    mov esp, ebp

    pop ebx
    pop es
    pop ds
    pop ecx
    pop ebp
    ret



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
    push cx
    push ds
    push es
    push bx
#    mov ax, 0x30
    les bx, 8[bp]
#    mov ax, bx
#    mov dx, es


# The stack will presumably need to be 8-byte or 16-byte
# aligned to have any chance of working. Try to see if
# something works
#    push cx
#    push cx
#    push cx
#    push cx
#    push cx
#    push cx
#    push cx

# get the stack aligned on a 16-byte boundary
    mov bp, sp
    mov ax, bp
    sub ax, 18
    and ax, 0xFFF0
    add ax, 2
    mov sp, ax
    push bp

# this is callb16r
# we need to prepare it for a 64-bit far return
    mov ax, 0
    push ax
    push ax
    push ax
    mov dx, es:[bx + 34]
    push dx

    push ax
    push ax
    push ax
    mov ax, es:[bx + 32]
    push ax

# and now we do our 16-bit return to execute callb16m (64-bit)
# we need the old cs (expanded to 32 bits) first
    mov ax, 0
    push ax
    mov ax, es:[bx + 24]
    push ax
# and now the 32-bit offset
    mov dx, es:[bx + 30]
    push dx
    mov ax, es:[bx + 28]
    push ax

# Note that this is probably superfluous
    mov ax, es:[bx + 40]
    mov ds, ax

# prepare to switch back to standard stack
    mov ax, es:[bx + 36]
    mov cx, ax

# This is also probably superfluous
    mov ax, es:[bx + 44]
    mov es, ax

# save our 16-bit stack in bx
    mov ax, ss
    mov bx, ax

# and now ax will carry the 64-bit ss
    mov ax, cx

    data32 retf



# I don't know if there is a better way of doing this
# Even assuming the code isn't allowed to span a 64k segment
#
.globl callb16r
callb16r:

#    mov al, 'X'
#    out 0xe9, al
#    mov al, '\n'
#    out 0xe9, al

# restore our 16-bit stack, while preserving ax
    mov cx, ax
    mov ax, bx
    mov ss, ax
    mov ax, cx

    pop bp
    mov sp, bp

# For stack alignment
#    pop cx
#    pop cx
#    pop cx
#    pop cx
#    pop cx
#    pop cx
#    pop cx

    pop bx
    pop es
    pop ds
    pop cx
    pop bp
    retf


.code64
