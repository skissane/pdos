#######################################################################
#                                                                     #
#  This program written by Paul Edwards.                              #
#  Released to the public domain                                      #
#                                                                     #
#######################################################################
#######################################################################
#                                                                     #
#  amisupa - support routines for AmigaOS.                            #
#                                                                     #
#  This uses the Motorola syntax                                      #
#                                                                     #
#######################################################################
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



# These are needed for 68000 support for VBCC
# Note that just "rts" causes a crash
#        .globl __lmods
#__lmods:
#        rts
#        .globl __ldivs
#__ldivs:
#        rts
#        .globl __ldivu
#__ldivu:
#        rts
#        .globl __lmodu
#__lmodu:
#        rts


.ifdef AMIGA

# Note that when using the output of fd2pragma you should
# be careful to get rid of any leading zeroes otherwise they
# will be treated as octals when you need decimal.


        .globl  _Read
_Read:
        .globl  Read
Read:
        MOVEM.L D2/D3/A6,-(A7)
        MOVEA.L _DOSBase,A6
        MOVEM.L 16(A7),D1/D2/D3
        JSR     -42(A6)
        MOVEM.L (A7)+,D2/D3/A6
        RTS


        .globl  _Write
_Write:
        .globl  Write
Write:
        MOVEM.L D2/D3/A6,-(A7)
        MOVEA.L _DOSBase,A6
        MOVEM.L 16(A7),D1/D2/D3
        JSR     -48(A6)
        MOVEM.L (A7)+,D2/D3/A6
        RTS


        .globl  _Input
_Input:
        .globl  Input
Input:
        MOVE.L  A6,-(A7)
        MOVEA.L _DOSBase,A6
        JSR     -54(A6)
        MOVEA.L (A7)+,A6
        RTS


        .globl  _Output
_Output:
        .globl  Output
Output:
        MOVE.L  A6,-(A7)
        MOVEA.L _DOSBase,A6
        JSR     -60(A6)
        MOVEA.L (A7)+,A6
        RTS


        .globl  _AllocMem
_AllocMem:
        .globl  AllocMem
AllocMem:
        MOVE.L  A6,-(A7)
        MOVEA.L _SysBase,A6
        MOVEM.L 8(A7),D0/D1
        JSR     -198(A6)
        MOVEA.L (A7)+,A6
        RTS


        .globl  _FreeMem
_FreeMem:
        .globl  FreeMem
FreeMem:
        MOVE.L  A6,-(A7)
        MOVEA.L _SysBase,A6
        MOVEA.L 8(A7),A1
        MOVE.L  12(A7),D0
        JSR     -210(A6)
        MOVEA.L (A7)+,A6
        RTS


        .globl  _Open
_Open:
        .globl  Open
Open:
        MOVEM.L D2/A6,-(A7)
        MOVEA.L _DOSBase,A6
        MOVEM.L 12(A7),D1/D2
        JSR     -30(A6)
        MOVEM.L (A7)+,D2/A6
        RTS


        .globl  _Close
_Close:
        .globl  Close
Close:
        MOVE.L  A6,-(A7)
        MOVEA.L _DOSBase,A6
        MOVE.L  8(A7),D1
        JSR     -36(A6)
        MOVEA.L (A7)+,A6
        RTS


        .globl  _Seek
_Seek:
        .globl  Seek
Seek:
        MOVEM.L D2/D3/A6,-(A7)
        MOVEA.L _DOSBase,A6
        MOVEM.L 16(A7),D1/D2/D3
        JSR     -66(A6)
        MOVEM.L (A7)+,D2/D3/A6
        RTS


        .globl  _DateStamp
_DateStamp:
        .globl  DateStamp
DateStamp:
        MOVE.L  A6,-(A7)
        MOVEA.L _DOSBase,A6
        MOVE.L  8(A7),D1
        JSR     -192(A6)
        MOVEA.L (A7)+,A6
        RTS


        .globl  _DeleteFile
_DeleteFile:
        .globl  DeleteFile
DeleteFile:
        MOVE.L  A6,-(A7)
        MOVEA.L _DOSBase,A6
        MOVE.L  8(A7),D1
        JSR     -72(A6)
        MOVEA.L (A7)+,A6
        RTS


        .globl  _Rename
_Rename:
        .globl  Rename
Rename:
        MOVEM.L D2/A6,-(A7)
        MOVEA.L _DOSBase,A6
        MOVEM.L 12(A7),D1/D2
        JSR     -78(A6)
        MOVEM.L (A7)+,D2/A6
        RTS

.endif



.ifdef ATARI

# I don't know how to do a nice block move, so instead
# I assume that 20 bytes are valid (atrsupc has 24 in fact)
# and just ignore the count

        .globl ___asmt1

___asmt1:
        link a6, #0

# preserve d1
        move.l d1, -(sp)
# preserve a5
        move.l a5, -(sp)

        move.l 8(a6),d1
        move.l 12(a6),a5

        lea -20(sp),sp
        move.l 0(a5), d0
        move.l d0, 0(sp)
        move.l 4(a5), d0
        move.l d0, 4(sp)
        move.l 8(a5), d0
        move.l d0, 8(sp)
        move.l 12(a5), d0
        move.l d0, 12(sp)
        move.l 16(a5), d0
        move.l d0, 16(sp)
        trap #1
        lea 20(sp),sp

        move.l (sp)+, a5
        move.l (sp)+, d1
        unlk a6
        rts
.endif
