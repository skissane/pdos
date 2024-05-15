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

.globl ___pdpent
___pdpent:
#        jsr __write
        jsr ___start

        .globl  ___write
___write:

        link a6, #0
# 4 = Linux write syscall
        move.l #4, d0

# handle
        move.l 8(a6), d1

# data
        move.l 12(a6), d2

# length
        move.l 16(a6), d3

        trap #0

#zzz:    jmp zzz

        unlk a6
        rts


        .data

msg: dc.b 'H', 'i', '\n'


.text

.globl ___main
___main:
.globl ___open
___open:
.globl ___close
___close:
.globl ___read
___read:
.globl ___rename
___rename:
.globl ___seek
___seek:
.globl ___remove
___remove:
.globl ___mmap
___mmap:
.globl ___munmap
___munmap:
.globl ___clone
___clone:
.globl ___waitid
___waitid:
.globl ___execve
___execve:
.globl ___exita
___exita:
.globl ___ioctl
___ioctl:
.globl ___getpid
___getpid:
.globl ___open
___open:
.globl ___read
___read:
.globl ___close
___close:
.globl ___exita
___exita:
.globl ___exita
___exita:
.globl ___ioctl
___ioctl:
.globl ___time
___time:

rts


.endif
