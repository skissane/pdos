# arm64 code for macOS

# Written by Paul Edwards
# Released to the public domain

# 64-bit ARM (AArch64) needs the stack 16-byte aligned

# macOS needs the syscall number in x16 and uses
# interrupt 0x80. Note that the syscall numbers are
# considered private, so you may need to recompile
# your software one day. This is not a problem for
# the intended purpose of creating a pseudo-bios for
# PDOS-generic. The pdos-generic applications do not
# require recompilation.

# Standard calling convention has parameters in
# x0-x7, and these are also used by the syscall,
# so minimal code is required.


# int ___write(int fd, void *buf, int len);

        .globl  ___write
        .align  4
___write:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#4
#           @ SYS_write

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret



# void ___exit(int rc);

        .globl  ___exit
        .align  4
___exit:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#1
#           @ SYS_exit

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret
