# Written by Paul Edwards
# Released to the public domain

# Note that although we don't really need to save r0, I am
# saving it anyway as a means to keep the stack 8-byte aligned
# as otherwise when we go to call msvcrt.dll we can't print
# doubles

# It seems that when msvcrt.dll calls an ARM entry point, the
# stack is not 8-byte aligned, so we are adjusting it here.
# We should really check to see if it is already properly aligned

.globl __pdpent
__pdpent:
        stmfd   sp!,{r0,lr}
        sub     sp, sp, #4
#        ldr     r0,=mainCRTStartup
# Activate Thumb mode by adding 1 (should probaby use OR instead,
# in case the above ldr has already compensated for that)
#        add     r0, r0, #1
# mainCRTStartup returns by moving lr into pc, and will restore
# our original ARM mode (not Thumb) due to that, I think
        bl      mainCRTStartup
        add     sp, sp, #4
        ldmia   sp!,{r0,pc}
