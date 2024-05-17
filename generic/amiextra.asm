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
