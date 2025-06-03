#######################################################################
#                                                                     #
#  This program written by Paul Edwards.                              #
#  Released to the public domain                                      #
#                                                                     #
#######################################################################
#######################################################################
#                                                                     #
#  atrstart - startup code for Atari TOS.                             #
#                                                                     #
#  This uses the Motorola syntax                                      #
#                                                                     #
#  Atari provides a basepage on the stack, unless a0 is non-zero, in  #
#  which case, that is the basepage. As an extension, if a0 is -1     #
#  then we look to a1 for an extension block, and the extension       #
#  block will contain both the basepage and a PDOS-generic callback   #
#  block, which inclues a trap1 callback function                     #
#                                                                     #
#  All this manipulation is left for the C startup code to handle.    #
#                                                                     #
#######################################################################

        .text
        .globl ___atrstart
___atrstart:
        move.l  sp,savedSP

# d0 is the return value, so we can use that as a temp
# register here
        move.l  sp,d0

# Don't use movem to push the arguments because the order
# is not what you code yourself, but other rules.
        move.l  a1,-(sp)
        move.l  a0,-(sp)
        move.l  d0,-(sp)

        jsr     ___start
        lea     12(sp),sp
        rts

# This function is required by GCC but isn't used for anything
        .globl ___main
___main:
        rts

# This function receives a return code as a parameter. The stack
# then needs to be restored and the parameter placed in register d0
# prior to return to the OS.

# This is basically acting as a "poor man's longjmp". While it will
# return to the correct address, none of the registers are expected
# to be the same as what they were on entry. A normal operating
# system like AmigaDOS is probably not dependent on a user's program
# behaving nicely, but PDOS-generic is, so will get a crash on return.

        .globl ___exita
___exita:
        move.l  4(sp),d0
        move.l  savedSP,sp
        rts

        .bss
savedSP:
        .skip 4
