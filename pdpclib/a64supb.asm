; This code was written by Paul Edwards
; Released to the public domain

; ARM64 assembler in Visual Studio syntax

; 64-bit ARM (AArch64) needs the stack 16-byte aligned


;.code
; guessing alignment
 area .text, code, readonly, align=4

; int setjmp(jmp_buf env);

        export  __Ysetjmp
__Ysetjmp proc
        str     x1,[x0,52*2]
        mov     x1,x0
        str     x2,[x1,40*2]
        str     x3,[x1,44*2]
        str     x12,[x1,48*2]
        mov     x2,sp
        str     x2,[x1]
        str     x11,[x1,4*2]
;   @ fp
        str     lr,[x1,8*2]
;    @ r14
        str     x4,[x1,12*2]
        str     x5,[x1,16*2]
        str     x6,[x1,20*2]
        str     x7,[x1,24*2]
        str     x8,[x1,28*2]
        str     x9,[x1,32*2]
;   @ rfp
        str     x10,[x1,36*2]
;  @ sl

        str     x13,[x1,14*4*2]
        str     x14,[x1,15*4*2]
        str     x15,[x1,16*4*2]
        str     x16,[x1,17*4*2]
        str     x17,[x1,18*4*2]
        str     x18,[x1,19*4*2]
        str     x19,[x1,20*4*2]
        str     x20,[x1,21*4*2]
        str     x21,[x1,22*4*2]
        str     x22,[x1,23*4*2]
        str     x23,[x1,24*4*2]
        str     x24,[x1,25*4*2]
        str     x25,[x1,26*4*2]
        str     x26,[x1,27*4*2]
        str     x27,[x1,28*4*2]
        str     x28,[x1,29*4*2]
        str     x29,[x1,30*4*2]
        str     x30,[x1,31*4*2]

; This doesn't assemble
; The distance 32 appears to be too far
; There is no sign that gcc is generating references to x31
; so it is probably fine to do nothing. No sign of references
; to x17 either.
;        str     x31,[x1,32*4*2]

        mov     x0, 0
;        ret
        endp



; void longjmp(jmp_buf env, int v);

        export  longjmp
longjmp proc

        mov     x2,x0
        mov     x0,x1

;;;; +++ need to put this line back!
;        cbnz    x0,__notzero
        cmp     x0,0
;        moveq   x0,;1
        mov     x0,1
__notzero:

        mov     x1,x2

;        ldr     sp,[x1]
        ldr     x2,[x1]
        mov     sp,x2

        ldr     x11,[x1,4*2]
;      @ fp
        ldr     lr,[x1,8*2]
        str     lr,[sp]
        ldr     x4,[x1,12*2]
        ldr     x5,[x1,16*2]
        ldr     x6,[x1,20*2]
        ldr     x7,[x1,24*2]
        ldr     x8,[x1,28*2]
        ldr     x9,[x1,32*2]
        ldr     x10,[x1,36*2]
        ldr     x2,[x1,40*2]
        ldr     x3,[x1,44*2]
        ldr     x12,[x1,48*2]
;  @ ip

        ldr     x13,[x1,14*4*2]
        ldr     x14,[x1,15*4*2]
        ldr     x15,[x1,16*4*2]
        ldr     x16,[x1,17*4*2]
        ldr     x17,[x1,18*4*2]
        ldr     x18,[x1,19*4*2]
        ldr     x19,[x1,20*4*2]
        ldr     x20,[x1,21*4*2]
        ldr     x21,[x1,22*4*2]
        ldr     x22,[x1,23*4*2]
        ldr     x23,[x1,24*4*2]
        ldr     x24,[x1,25*4*2]
        ldr     x25,[x1,26*4*2]
        ldr     x26,[x1,27*4*2]
        ldr     x27,[x1,28*4*2]
        ldr     x28,[x1,29*4*2]
        ldr     x29,[x1,30*4*2]
        ldr     x30,[x1,31*4*2]
;        ldr     x31,[x1,;32*4*2]

        ldr     x1,[x1,52*2]
;        mov     pc,lr
;        ret
        endp


        end
