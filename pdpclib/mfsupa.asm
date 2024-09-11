# mfsupa - mainframe support code in assembler
# Written by Paul Edwards
# Released to the public domain

         .text
         .balign 2
         .extern __ret6
         .using .,r12
#        MVC   92(8,r13),=D'1.0E0'
         .globl __crt0
__crt0:
         LR    r12,r15
         LR    r7,r14
         L     r3,=F'3'
         L     r4,=F'4'
         L     r5,=F'5'
         LA    r13,savea
         ST    r13,76(,r13)
         L     r15,=V(__vserun)
#         .long 0
#         .long 0xcccccccc
#         .long 0xcccccccc
         BALR  r14,r15
#.LABC:
#         B     .LABC
         L     r15,=F'3'
         LR    r14,r7
         BR    r14
         .balign 8
         .ltorg
         .drop r12
         .balign 4
savea:
         .space 4000





         .balign 2
         .using .,r3
         .globl __svc
__svc:
         STM   r14,r12,12(r13)
         LR    R3,R15
         L     r7,0(,r1)
         L     r8,4(,r1)
         L     r0,0(,r8)
         L     r1,4(,r8)
         L     r15,60(,r8)
         EX    r7, .LSVC1
         B     .LSVC2
.LSVC1:
         SVC   0
.LSVC2:
         LM    r14,r12,12(r13)
         BR    r14

         .ltorg
         .drop r3





         .balign 2
         .using .,r15
         .globl __ret7
__ret7:
         L     r15,=F'7'
         BR    r14

         .ltorg
         .drop r15
