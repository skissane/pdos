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

# These need to be implemented

___setj:
        rts

___longj:
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
