/ fninit - initialize floating point
/ written by Paul Edwards
/ released to the public domain

        .globl start
        .text
start:
        fninit
        mov $0x4c,%ah
        int $0x21
