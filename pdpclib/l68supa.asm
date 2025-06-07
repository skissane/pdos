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




# stack will have
# jmp_buf
# ret address
# old a6
# old a5

# and a6 will be pointing to old a6
# and a5 will point to the jmp_buf

___setj:
        link a6, #0
        move.l a5, -(sp)
# get jmp_buf into a5
        move.l 8(a6),a5

        move.l a0, 0(a5)
        move.l a1, 4(a5)
        move.l a2, 8(a5)
        move.l a3, 12(a5)
        move.l a4, 16(a5)
        move.l a5, 20(a5)
# Defer storing original a6 into location 24

# Stack (a7) that we want (basically) is in a6 now
        move.l a6, 28(a5)
        move.l d0, 32(a5)
        move.l d1, 36(a5)
        move.l d2, 40(a5)
        move.l d3, 44(a5)
        move.l d4, 48(a5)
        move.l d5, 52(a5)
        move.l d6, 56(a5)
        move.l d7, 60(a5)

# copy return address
        move.l 4(a6), d0
        move.l d0, 64(a5)

# original a6 is here
        move.l 0(a6), d0
        move.l d0, 24(a5)

        move.l #0, d0
        move.l (sp)+, a5
        unlk a6
        rts


# stack is
# retval (won't be 0 and won't be used)
# (actually, retval isn't on the stack)
# jmp_buf
# ret address (won't be used)
# old a6 (don't care)

# a6 points to old a6
# a5 will point to jmp_buf

___longj:
        link a6, #0

        move.l 8(a6), a5

# retrieve return value before it is lost
# save return value in jmpbuf for now
#        move.l 12(a6), d0
#        move.l d0, 32(a5)
# actually, longjmp stores it directly into the jmpbuf
# (our implementation)

# switch to old stack (a7 = sp)
        move.l 28(a5), sp

# put old a6 on stack as it originally was, at the original offset
# don't adjust the stack.
#        move.l 24(a5), d0
#        move.l d0, (sp)
# Actually don't do that, as it is more effort to get unlk to work

# put return address on stack as it originally was,
# and at the original offset
# don't adjust the stack as it is already set correctly
        move.l 64(a5), d0
        move.l d0, 4(sp)

# we no longer care about the current a6, and a5 will be
# last thing restored

        move.l 0(a5), a0
        move.l 4(a5), a1
        move.l 8(a5), a2
        move.l 12(a5), a3
        move.l 16(a5), a4
# Not ready for this yet
#        move.l 20(a5), a5
# This is required as we won't be using unlk
        move.l 24(a5), a6
# This has already been done
#        move.l 28(a5), a7
# Return value is not stored here
#        move.l 32(a5), d0
# It's actually here
        move.l 68(a5), d0

        move.l 36(a5), d1
        move.l 40(a5), d2
        move.l 44(a5), d3
        move.l 48(a5), d4
        move.l 52(a5), d5
        move.l 56(a5), d6
        move.l 60(a5), d7

# Finally ready for this
        move.l 20(a5), a5

# a6 doesn't have the stack pointer, so don't do unlk
# manually adjust stack instead, as a6 is already set
#        unlk a6
        lea 4(sp), sp
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
