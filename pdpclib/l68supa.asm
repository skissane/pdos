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


.text

.globl ___main
___main:
.globl ___open
___open:
.globl ___close
___close:
.globl ___read
___read:
.globl ___write
___write:
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
