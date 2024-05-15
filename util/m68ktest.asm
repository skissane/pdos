# Simple Motorola 68000 program for Linux

# Written by Paul Edwards
# Released the public domain

#asami -o m68ktest.o m68ktest.asm
#ldami -s -o m68ktest.exe m68ktest.o
#or
#pdld -s --oformat elf --emit-relocs -o m68ktest.exe m68ktest.o
#m68kemul m68ktest.exe


# a7=sp (stack)


        .text

        .globl  myentry
myentry:

# 4 = Linux write syscall
        move.l #4, d0

# 1 = stdout handle
        move.l #1, d1

# Don't know if there is a nicer way of doing this
        lea msg, a0
        move.l a0, -(sp)
        move.l (sp), d2

# 3 = length of data to write
        move.l #3, d3

        trap #0

zzz:    jmp zzz


        .data

msg: dc.b 'H', 'i', '\n'
