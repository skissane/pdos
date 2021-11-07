#######################################################################
#                                                                     #
#  This program written by Paul Edwards.                              #
#  Released to the public domain                                      #
#                                                                     #
#######################################################################
#######################################################################
#                                                                     #
#  amistart - startup code for AmigaOS.                               #
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
        .globl ___amistart
___amistart:
        move.l  sp,savedSP

# Don't use movem to push the arguments because the order
# is not what you code yourself, but other rules.
        move.l  d7,-(sp)
        move.l  a0,-(sp)
        move.l  d0,-(sp)

        jsr     ___start
        lea     12(sp),sp
        rts

# This function receives a return code as a parameter. The stack
# then needs to be restored and the parameter placed in register d0
# prior to return to the OS.

        .globl ___exita
___exita:
        move.l  4(sp),d0
        move.l  savedSP,sp
        rts

        .bss
savedSP:
        .skip 4
