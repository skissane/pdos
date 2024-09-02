# mfsupa - mainframe support code in assembler
# Written by Paul Edwards
# Released to the public domain

         .text
         .balign 2
         .using .,r15
#        MVC   92(8,r13),=D'1.0E0'
         .globl __crt0
__crt0:
         L     r15,=F'3'
         BR    r14
         .balign 8
         .ltorg
         .drop r15





         .balign 2
         .using .,r15
         .globl __svc
__svc:
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
         BR    r14

         .ltorg
         .drop r15
