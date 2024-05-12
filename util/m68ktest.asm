# Simple Motorola 68000 program for Linux

# Written by Paul Edwards
# Released the public domain

#asami -o m68ktest.o m68ktest.asm
#ldami -s -o m68ktest.exe m68ktest.o

#m68kemul m68ktest.exe


# a7=sp (stack)
# not sure if order of stack pushes is correct


        .text

        .globl  myentry
myentry:

# 1 = stdout handle
        move.l 1, -(sp)

        lea msg, a0
        move.l a0, -(sp)

# 3 = length
        move.l 3, -(sp)

# 4 = write
        move.l 4, d0
        trap #0

zzz:    jmp zzz



        .data

msg:    dc.b 'H', 'i', '\n'
