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
        jsr ___start

# Not expecting to return here
loop1:  jmp loop1



        .globl  ___write
___write:

        link a6, #0
        movem.l d1-d3, -(sp)
# 4 = Linux write syscall
        move.l #4, d0

# handle
        move.l 8(a6), d1

# data
        move.l 12(a6), d2

# length
        move.l 16(a6), d3

        trap #0

        movem.l (sp), d1-d3
        unlk a6
        rts



.globl ___exita
___exita:

        link a6, #0
        move.l d1, -(sp)

# 1 = Linux exit syscall
        move.l #1, d0

# return code
        move.l 8(a6), d1

        trap #0

# Not expecting to return here
loop2:  jmp loop2

        move.l (sp), d1
        unlk a6
        rts


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
.globl ___ioctl
___ioctl:
.globl ___getpid
___getpid:
.globl ___time
___time:

rts


.endif
