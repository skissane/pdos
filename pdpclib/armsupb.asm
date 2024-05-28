;# Support code for using Visual C for ARM32
;# Visual C generates Thumb code (an enhancement to ARM32 that
;# makes use of compact 16-bit instructions).
;# Written by Paul Edwards
;# All changes remain public domain

;# int setjmp(jmp_buf env);

; ARM32 may be:
;  0000A0E1      nop
; or
; 00F020E3 (armasm)

; Thumb may be:
; C046          nop
; or
; 00BF (armasm)

; this is not needed
; thumb

 area .text, code, readonly, align=2

        export  __Ysetjmp
__Ysetjmp proc
;___Ysetjmp:

        str     r1,[r0,#52]
        mov     r1,r0
        str     r2,[r1,#40]
        str     r3,[r1,#44]
        str     r12,[r1,#48]  ;@ ip
        mov     r2,sp
        str     r2,[r1]
        str     r11,[r1,#4]   ;@ fp
        str     lr,[r1,#8]    ;@ r14
        str     r4,[r1,#12]
        str     r5,[r1,#16]
        str     r6,[r1,#20]
        str     r7,[r1,#24]
        str     r8,[r1,#28]
        str     r9,[r1,#32]   ;@ rfp
        str     r10,[r1,#36]  ;@ sl
        mov     r0,#0
        mov     pc,lr

        endp


;# void longjmp(jmp_buf env, int v);

        export  longjmp

longjmp proc
        mov     r2,r0
        mov     r0,r1

        cmp     r0,#0
        moveq   r0,#1

        mov     r1,r2

        ldr     sp,[r1]
        ldr     r11,[r1,#4]    ;  @ fp
        ldr     lr,[r1,#8]
        ldr     r4,[r1,#12]
        ldr     r5,[r1,#16]
        ldr     r6,[r1,#20]
        ldr     r7,[r1,#24]
        ldr     r8,[r1,#28]
        ldr     r9,[r1,#32]
        ldr     r10,[r1,#36]
        ldr     r2,[r1,#40]
        ldr     r3,[r1,#44]
        ldr     r12,[r1,#48]  ; @ ip
        ldr     r1,[r1,#52]
        mov     pc,lr

        endp


;# unsigned integer divide
;# inner loop code taken from http://me.henri.net/fp-div.html
;# in:  r0 = num,  r1 = den
;# out: r0 = quot, r1 = rem

        export  __rt_udiv
;        public  __udivsi3
;        public  ___udivsi3
__rt_udiv proc
;__udivsi3:
;___udivsi3:
;__aeabi_uidiv:

        stmfd   sp!,{r2,lr}

        rsb     r2,r1,#0
        mov     r1,#0
        adds    r0,r0,r0
; +++ Need to find masm syntax for this
;        .rept   32
        adcs    r1,r2,r1,lsl #1
        subcc   r1,r1,r2
        adcs    r0,r0,r0
;        .endr
;#        mov     pc,lr
        ldmia   sp!,{r2,pc}

        endp

;# This is the traditional function that expects
;# register parameters, not on the stack - for
;# use even in a stack parameter environment
;# We may wish to eliminate this function in
;# a stack environment though

;        export  __udivsi3_trad
        export  ___udivsi3_trad
;        public  __aeabi_uidiv_trad
;__udivsi3_trad proc
___udivsi3_trad proc
;__aeabi_uidiv_trad:
        rsb     r2,r1,#0
        mov     r1,#0
        adds    r0,r0,r0
; +++
;        .rept   32
        adcs    r1,r2,r1,lsl #1
        subcc   r1,r1,r2
        adcs    r0,r0,r0
;        .endr
        mov     pc,lr

        endp


;# signed integer divide
;# in:  r0 = num,  r1 = den
;# out: r0 = quot

        export __rt_sdiv
;        import divz
;        import ___udivsi3_trad
;        public  __divsi3
;        public  ___divsi3
;        public  __aeabi_idiv
__rt_sdiv proc
;__divsi3:
;___divsi3:
;__aeabi_idiv:
;# Need to preserve r2 because the
;# function we may call, doesn't
        stmfd   sp!,{r2,r3,lr}
        eor     r3,r0,r1        ;@ r3 = sign
        mov     r3,r3,asr#31
        cmp     r1,#0
        beq     divz
        rsbmi   r1,r1,#0
        cmp     r0,#0
        rsbmi   r0,r0,#0
        bl      ___udivsi3_trad
        cmp     r3,#0
        rsbne   r0,r0,#0
        ldmia   sp!,{r2,r3,pc}
        endp

        export divz
divz proc
        mov     r0,#8           ;@ SIGFPE
; +++        stmfd   sp!,{r0}
        mov     r0,#1
; +++        stmfd   sp!,{r0}
;#        bl      Craise
        mov     r0,#0           ;@ if raise(SIGFPE) failed, return 0
        ldmia   sp!,{r2,r3,pc}

        endp
        


;# This is the traditional function that expects
;# register parameters, not on the stack - for
;# use even in a stack parameter environment
;# We may wish to eliminate this function in
;# a stack environment though

;        export __divsi3_trad
;        import divz_trad
        export ___divsi3_trad
;        export __aeabi_idiv_trad
;__divsi3_trad proc
___divsi3_trad proc
;__aeabi_idiv_trad:
; adding r5 so that there are multiple items to avoid assembler error
        stmfd   sp!,{r5,lr}
        eor     r3,r0,r1    ;    @ r3 = sign
        mov     r3,r3,asr#31
        cmp     r1,#0
        beq     divz_trad
        rsbmi   r1,r1,#0
        cmp     r0,#0
        rsbmi   r0,r0,#0
        bl      ___udivsi3_trad
        cmp     r3,#0
        rsbne   r0,r0,#0
        ldmia   sp!,{r5,pc}
        endp
        
        export divz_trad
divz_trad proc
        mov     r0,#8        ;   @ SIGFPE
; +++ don't know what to do below
;        stmfd   sp!,{r0}
        mov     r0,#1
;        stmfd   sp!,{r0}
;#        bl      Craise
        mov     r0,#0         ;  @ if raise(SIGFPE) failed, return 0
        ldmia   sp!,{r5,pc}

        endp


;# signed integer modulo
;# in:  r0 = num,  r1 = den
;# out: r0 = rem

        export __modsi3
;        export ___modsi3
;        import ___divsi3_trad
;        public  __aeabi_idivmod
__modsi3 proc
;___modsi3:
;__aeabi_idivmod:
;# Need to preserve r3, because the function
;# we call doesn't
        stmfd   sp!,{r3,r4,lr}
;#        asr     r4,r0,#31               @ r4 = sign
        mov     r4,r0,asr#31
        bl      ___divsi3_trad
        mov     r0,r1
        cmp     r4,#0
        rsbne   r0,r0,#0
        ldmia   sp!,{r3,r4,pc}

        endp


;# unsigned integer modulo
;# in:  r0 = num,  r1 = den
;# out: r0 = rem

        export __umodsi3
;        export ___umodsi3
;        public  __aeabi_uidivmod
__umodsi3 proc
___umodsi3:
;__aeabi_uidivmod:
;# Need to preserve r2, because the function
;# we call  doesn't
        stmfd   sp!,{r2,lr}
        bl      ___udivsi3_trad
        mov     r0,r1
        ldmia   sp!,{r2,pc}

        endp


        end
