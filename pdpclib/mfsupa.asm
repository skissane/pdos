# mfsupa - mainframe support code in assembler
# Written by Paul Edwards
# Released to the public domain


# If running under z/PDOS-generic, there will be a
# stack provided already. Otherwise we need to create
# our own


.ifndef MVS
.set MVS,0
.endif

.ifndef VSE
.set VSE,0
.endif

         .text
         .balign 2
         .extern __ret6
         .using .,r15
#        MVC   92(8,r13),=D'1.0E0'
         .globl __crt0
__crt0:
#         LA    R15,9(,0)
#         BR    R14
         B     skiphdr
#         .byte "PGCX"  # PDOS-generic (or compatible) extension
# Needs to be in EBCDIC
         .byte 0xd7, 0xc7, 0xc3, 0xe7
#         .long 4   # length of header data
         .DC   F'4'
         .globl __pgparm
__pgparm: .long 0   # This will be zapped by z/PDOS-generic if running under it
skiphdr:
.if MVS
         STM   r14,r12,12(r13)
         LR    R11,R1
         LR    R6,R13
         L     R1,=V(__pgparm)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    notpdos
         LA    r13,80(,r13)
         LR    r12,r15
         .drop r15
         .using __crt0, r12
         B     BYPASS1
.endif
.if VSE
         L     R1,=V(__pgparm)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    notpdos
         STM   r14,r12,12(r13)
         LA    r13,80(,r13)
         LR    r12,r15
         .drop r15
         .using __crt0, r12
         B     BYPASS1
.endif
notpdos:
         LR    r12,r15
         LR    r7,r14
         L     r3,=F'3'
         L     r4,=F'4'
         L     r5,=F'5'
         LA    r13,savea
BYPASS1:
         LA    r9,80(,r13)
         ST    r9,76(,r13)
.if MVS
         L     r15,=V(__mvsrun)
.endif
#
.if VSE
         L     r15,=V(__vserun)
.endif
#         .long 0
#         .long 0xcccccccc
#         .long 0xcccccccc
         BALR  r14,r15
#.LABC:
#         B     .LABC
.if VSE
         L     r15,=F'3'
.endif
#
.if VSE
         LR    r14,r7
#
         L     r1,=V(__pgparm)
         L     r1,0(,r1)
         LTR   r1,r1
         BZ    notpdos2
.endif
.if MVS
         L     r1,=V(__pgparm)
         L     r1,0(,r1)
         LTR   r1,r1
         BZ    notpdos2
.endif
         S     r13,=F'80'
#
         L     r14,12(r13)
         LM    r0,r12,20(r13)
         BR    R14
.if VSE
notpdos2:
         BR    r14
.endif
.if MVS
notpdos2:
         LR    R13,R6
         L     r14,12(r13)
         LM    r0,r12,20(r13)
         BR    R14
.endif
         .balign 8
         .ltorg
         .drop r12
         .balign 4
savea:
         .space 4000





         .balign 2
         .using .,r3
         .globl __svcreal
__svcreal:
         STM   r14,r12,12(r13)
         LR    R3,R15
         L     r7,0(,r1)
         L     r8,4(,r1)
         L     r0,0(,r8)
         L     r1,4(,r8)
         L     r15,60(,r8)
         EX    r7, SVC1
         B     SVC2
SVC1:
         SVC   0
SVC2:
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
