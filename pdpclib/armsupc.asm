;# Written by Paul Edwards
;# Release to the public domain
;# Note that although we don't really need to save r0, I am
;# saving it anyway as a means to keep the stack 8-byte aligned
;# as otherwise when we go to call msvcrt.dll we can't print
;# doubles

 area .text, code, readonly, align=4

 import mainCRTStartup
 export __pdpent
__pdpent proc
        stmfd   sp!,{r0,lr}
        ldr     r0,=mainCRTStartup
;# Activate Thumb mode by adding 1 (should probaby use OR instead,
;# in case the above ldr has already compensated for that)
;        add     r0, r0, #1
;# mainCRTStartup returns by moving lr into pc, and will restore
;# our original ARM mode (not Thumb) due to that, I think
        blx     r0
        ldmia   sp!,{r0,pc}

 endp

 end
