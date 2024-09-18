* mfsupa - mainframe support code in assembler
* Written by Paul Edwards
* Released to the public domain
*
*
* If running under z/PDOS-generic, there will be a
* stack provided already. Otherwise we need to create
* our own
*
*
         CSECT
         DS    0H
         USING *,R15
*        MVC   92(8,r13),=D'1.0E0'
*         ENTRY @@CRT0
*@@CRT0   DS    0H
         .globl __crt0
__crt0   DS    0H
*         LA    R15,9(,0)
*         BR    R14
         B     SKIPHDR
*         .byte "PGCX"  # PDOS-generic (or compatible) extension
* Needs to be in EBCDIC
         .byte 0xd7, 0xc7, 0xc3, 0xe7
*         DC    X'D7'
*         DC    F'10'
         DC    F'4'  # length of header data
         .globl __pgparm
__pgparm DC F'0'   # This will be zapped by z/PDOS-generic if running under it
*
SKIPHDR  DS    0H
#if MVS
         STM   R14,R12,12(R13)
         LR    R11,R1
         LR    R6,R13
         L     R1,=V(__pgparm)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    NOTPDOS
         LA    R13,80(,R13)
         LR    R12,R15
         DROP  R15
         USING __crt0, R12
         B     BYPASS1
#endif
#if VSE
         L     R1,=V(__pgparm)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    NOTPDOS
         STM   R14,R12,12(R13)
         LA    R13,80(,R13)
         LR    R12,R15
         DROP  R15
         USING __crt0, R12
         B     BYPASS1
#endif
*
NOTPDOS  DS    0H
         LR    R12,R15
         LR    R7,R14
         L     R3,=F'3'
         L     R4,=F'4'
         L     R5,=F'5'
         LA    R13,SAVEA
BYPASS1  DS    0H
         LA    R9,80(,R13)
         ST    R9,76(,R13)
#if MVS
         L     R15,=V(__mvsrun)
#endif
#
#if VSE
         L     R15,=V(__vserun)
#endif
*         .long 0
*         .long 0xcccccccc
*         .long 0xcccccccc
         BALR  R14,R15
*.LABC:
*         B     .LABC
#if VSE
         L     R15,=F'3'
#endif
*
#if VSE
         LR    R14,R7
#
         L     R1,=V(__pgparm)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    NOTPDOS2
#endif
#if MVS
         L     R1,=V(__pgparm)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    NOTPDOS2
#endif
         S     R13,=F'80'
*
         L     R14,12(R13)
         LM    R0,R12,20(R13)
         BR    R14
#if VSE
NOTPDOS2 DS    0H
         BR    R14
#endif
#if MVS
NOTPDOS2 DS    0H
         LR    R13,R6
         L     R14,12(R13)
         LM    R0,R12,20(R13)
         BR    R14
#endif
         DS    0D
         LTORG
         DROP  R12
         DS    0F
SAVEA    DS    4000C
*
*
*
*
*
         DS    0H
         USING *,R3
         .globl __svcreal
__svcreal:
         STM   R14,R12,12(R13)
         LR    R3,R15
         L     R7,0(,R1)
         L     R8,4(,R1)
         L     R0,0(,R8)
         L     R1,4(,R8)
         L     R15,60(,R8)
         EX    R7, SVC1
         B     SVC2
SVC1     DS    0H
         SVC   0
SVC2     DS    0H
         LM    R14,R12,12(R13)
         BR    R14
*
         LTORG
         DROP  R3
*
*
*
*
*
         DS    0H
         USING *,R15
         .globl __ret7
__ret7:
         L     R15,=F'7'
         BR    R14
*
         LTORG
         DROP  R15
