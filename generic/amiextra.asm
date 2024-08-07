#######################################################################
#                                                                     #
#  This program written by Paul Edwards.                              #
#  Released to the public domain                                      #
#                                                                     #
#######################################################################
#######################################################################
#                                                                     #
#  amiextra - extra support needed to run Amiga programs              #
#                                                                     #
#  This uses the Motorola syntax                                      #
#                                                                     #
#  It puts the standard AmigaOS registers containing the command      #
#  length (d0) and command buffer (a0) as well as the AmigaPDOS       #
#  extension register (only visible if running AmigaPDOS) d7,         #
#  containing an alternative SysBase to use (only if d0 is greater    #
#  than or equal to 2 GiB, and in which case, 2 GiB should be         #
#  subtracted from d0 before use) on the stack.                       #
#                                                                     #
#  All this manipulation is left for the C startup code to handle.    #
#                                                                     #
#######################################################################

        .text
        .globl _callami2
_callami2:
        link a6, #0
        movem.l d7/a0/a1, -(sp)

        move.l 8(a6),d0
        move.l 12(a6),a0
        move.l 16(a6),d7
        move.l 20(a6),a1

        jbsr 0(a1)

        movem.l (sp)+,d7/a0/a1
        unlk    a6
        rts


.globl _Output
_Output:
        link a6, #0
        jsr _c_Output
        unlk a6
        rts


.globl _Input
_Input:
        link a6, #0
        jsr _c_Input
        unlk a6
        rts


.globl _Write
_Write:
        link a6, #0
        movem.l d1/d2/d3, -(sp)
        jsr _c_Write
        lea 12(sp), sp
        unlk a6
        rts


.globl _AllocMem
_AllocMem:
        link a6, #0
        movem.l d0/d1, -(sp)
        jsr _c_AllocMem
        lea 8(sp), sp
        unlk a6
        rts



table:
#jmp fred


# 60
dc.b 0x4e, 0xf9
dc.l _Output

dc.b 0x4e, 0xf9
dc.l _Input

# need another 48 characters

#nop
#nop
#nop

dc.b 0x4e, 0xf9
dc.l _Write

nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop

.globl _newdosbase
_newdosbase:
dc.l 0
nop
nop
nop
dc.l libname

libname: .ascii "dos.library"

# Align
dc.l 0


table2:

ds.b 180

#198
dc.b 0x4e, 0xf9
dc.l _AllocMem

ds.b 192

.globl _SysBase
_SysBase:
ds.b 378

dc.l _newdosbase
