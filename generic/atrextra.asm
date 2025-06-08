#######################################################################
#                                                                     #
#  This program written by Paul Edwards.                              #
#  Released to the public domain                                      #
#                                                                     #
#######################################################################
#######################################################################
#                                                                     #
#  atrextra - extra support needed to run Atari programs              #
#                                                                     #
#  This uses the Motorola syntax                                      #
#                                                                     #
#  Normally the basepage would be on the stack, and a0 would be       #
#  set to 0. A non-zero a0 means that a0 contains the basepage        #
#  However, as an extension, we have a rule that if a0 is             #
#  x'ffffffff' then a1 points to the PDOS-generic parm block,         #
#  and the stack contains the basepage after all                      #
#  If more extensions are required, simply make a1 x'ffffffff' too    #
#  and add a more elaborate structure.                                #
#                                                                     #
#######################################################################

        .text
        .globl _callatr2
_callatr2:
        link a6, #0
        movem.l a0/a1/a2, -(sp)

        move.l 8(a6),d0
        move.l d0,-(sp)

        move.l 12(a6),a0
        move.l 16(a6),a1
        move.l 20(a6),a2

        jbsr 0(a2)

        lea 4(sp), sp

        movem.l (sp)+,a0/a1/a2
        unlk    a6
        rts
