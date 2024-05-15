#######################################################################
#                                                                     #
#  This program written by Paul Edwards.                              #
#  Released to the public domain                                      #
#                                                                     #
#######################################################################
#######################################################################
#                                                                     #
#  l68supa - support routines for Linux 68000                         #
#                                                                     #
#  This uses the Motorola syntax                                      #
#                                                                     #
#######################################################################
#
        .text
        .globl ___setj
        .globl ___longj

# These need to be implemented

___setj:
        rts

___longj:
        rts

.ifdef LINUX

.globl __pdpent
__pdpent:

        .globl  __write
__write:

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


.endif
