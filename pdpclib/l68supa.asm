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

# I suspect the function calling convention doesn't require D1 to be
# preserved, but I am currently preserving it. Can be changed.


#
        .text
        .globl ___setj
        .globl ___longj



# These need to be implemented

___setj:
        move.l #0, d0
        rts



___longj:
# not read for this yet
loop0:  jmp loop0
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

        movem.l (sp)+, d1-d3
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

        move.l (sp)+, d1
        unlk a6
        rts



.globl ___getpid
___getpid:

# 20 = Linux exit syscall
        move.l #20, d0
        trap #0
        rts




.globl ___open
___open:

        link a6, #0
        movem.l d1-d3, -(sp)

# 5 = Linux open syscall
        move.l #5, d0

# filename
        move.l 8(a6), d1

# flag
        move.l 12(a6), d2

# mode
        move.l 16(a6), d3

        trap #0

        movem.l (sp)+, d1-d3
        unlk a6
        rts




.globl ___read
___read:

        link a6, #0
        movem.l d1-d3, -(sp)

# 3 = Linux read syscall
        move.l #3, d0

# handle
        move.l 8(a6), d1

# data
        move.l 12(a6), d2

# length
        move.l 16(a6), d3

        trap #0

        movem.l (sp)+, d1-d3
        unlk a6
        rts




.globl ___close
___close:

        link a6, #0
        move.l d1, -(sp)

# 6 = Linux exit syscall
        move.l #6, d0

# handle
        move.l 8(a6), d1

        trap #0

        move.l (sp)+, d1
        unlk a6
        rts




.globl ___mmap
___mmap:

        link a6, #0
        move.l d1, -(sp)

# 90 = Linux mmap syscall
        move.l #90, d0

# struct
        move.l 8(a6), d1

        trap #0

        move.l (sp)+, d1
        unlk a6
        rts





.globl ___munmap
___munmap:

        link a6, #0
        movem.l d1-d2, -(sp)

# 91 = Linux munmap syscall
        move.l #91, d0

# addr
        move.l 8(a6), d1

# len
        move.l 12(a6), d2

        trap #0

        movem.l (sp)+, d1-d2
        unlk a6
        rts




.globl ___rename
___rename:

        link a6, #0
        movem.l d1-d2, -(sp)

# 38 = Linux rename syscall
        move.l #38, d0

# old file
        move.l 8(a6), d1

# new file
        move.l 12(a6), d2

        trap #0

        movem.l (sp)+, d1-d2
        unlk a6
        rts


.globl ___seek
___seek:

        link a6, #0
        movem.l d1-d3, -(sp)

# 19 = Linux lseek syscall
        move.l #19, d0

# handle
        move.l 8(a6), d1

# offset
        move.l 12(a6), d2

# whence
        move.l 16(a6), d3

        trap #0

        movem.l (sp)+, d1-d3
        unlk a6
        rts


.globl ___remove
___remove:

        link a6, #0
        move.l d1, -(sp)

# 10 = Linux unlink syscall
        move.l #10, d0

# filename
        move.l 8(a6), d1

        trap #0

        move.l (sp)+, d1
        unlk a6
        rts




.globl ___time
___time:

        link a6, #0
        move.l d1, -(sp)

# 13 = Linux syscall to retrieve current time
        move.l #13, d0

# pointer to time_t
        move.l 8(a6), d1

        trap #0

        move.l (sp)+, d1
        unlk a6
        rts



.globl ___ioctl
___ioctl:

        link a6, #0
        movem.l d1-d3, -(sp)

# 54 = Linux ioctl syscall
        move.l #54, d0

# file descriptor
        move.l 8(a6), d1

# command
        move.l 12(a6), d2

# parameter
        move.l 16(a6), d3

        trap #0

        movem.l (sp)+, d1-d3
        unlk a6
        rts


.globl ___main
___main:
.globl ___clone
___clone:
.globl ___waitid
___waitid:
.globl ___execve
___execve:

move.l #0, d0
rts


.endif
