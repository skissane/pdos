# rv3supa.asm - support code for C programs for RISC-V 32-bit
#
# This program written by Paul Edwards
# Released to the public domain


# Not expected to return, but better to loop than do random things
loop5: jmp loop5


.globl __setj
__setj:

ret



.globl ___longj
___longj:
.globl __longj
__longj:

ret
