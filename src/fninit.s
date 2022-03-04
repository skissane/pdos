/ fninit - initialize floating point
/ written by Paul Edwards
/ released to the public domain

        .globl start
        .text
start:
        fninit
        mov $0,%al
        mov $0x4c,%ah
        int $0x21
/ This instruction should not be needed, but there seems
/ to be a bug in ld386 that makes the executable do an
/ int 0 instead of int 21H
        nop
