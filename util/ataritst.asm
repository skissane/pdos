# Simple Motorola 68000 program for Atari TOS

# Written by Paul Edwards
# Released the public domain

#asami -o ataritst.o ataritst.asm
#pdld -s --oformat atari -o ataritst.exe ataritst.o
#atariemu ataritst.exe


# a7=sp (stack)


        .text

        .globl  myentry
myentry:

#        sub    sp, #4
        lea -4(sp), sp

# 2 = TOS Cconout opcode
        move.w #2, 0(sp)

        move.w 'H', 2(sp)
        trap #1

        move.w 'i', 2(sp)
        trap #1

        move.w '\r', 2(sp)
        trap #1

        move.w '\n', 2(sp)
        trap #1

#        add    sp, #4
        lea 4(sp), sp

zzz:    jmp zzz
