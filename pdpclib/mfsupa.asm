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
#if REALHLASM
R0       EQU   0
R1       EQU   1
R2       EQU   2
R3       EQU   3
R4       EQU   4
R5       EQU   5
R6       EQU   6
R7       EQU   7
R8       EQU   8
R9       EQU   9
R10      EQU   10
R11      EQU   11
R12      EQU   12
R13      EQU   13
R14      EQU   14
R15      EQU   15
#endif
*
         CSECT
         DS    0H
         USING *,R15
*        MVC   92(8,r13),=D'1.0E0'
#if REALHLASM
         ENTRY @@CRT0
#else
         .globl __crt0
#endif
@@CRT0   DS    0H
*         LA    R15,9(,0)
*         BR    R14
         B     SKIPHDR
*         DC    'PGCX'  # PDOS-generic (or compatible) extension
* Needs to be in EBCDIC
         DC    X'D7C7C3E7'
         DC    F'4'  # length of header data
#if REALHLASM
         ENTRY @@PGPARM
#else
         .globl __pgparm
#endif
* This will be zapped by z/PDOS-generic if running under it
@@PGPARM DC    F'0'
*
SKIPHDR  DS    0H
#if MVS
         STM   R14,R12,12(R13)
         LR    R11,R1
         LR    R6,R13
         L     R1,=V(@@PGPARM)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    NOTPDOS
         LA    R13,80(,R13)
         LR    R12,R15
         DROP  R15
         USING @@CRT0,R12
         B     BYPASS1
#endif
#if VSE
         L     R1,=V(@@PGPARM)
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
         L     R15,=V(@@MVSRUN)
#endif
#
#if VSE
         L     R15,=V(@@VSERUN)
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
         L     R1,=V(@@PGPARM)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    NOTPDOS2
#endif
#if MVS
         L     R1,=V(@@PGPARM)
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
#if REALHLASM
         ENTRY @@SVCRL
#else
         .globl __svcrl
#endif
@@SVCRL  DS    0H
         STM   R14,R12,12(R13)
         LR    R3,R15
         L     R7,0(,R1)
         L     R8,4(,R1)
         L     R0,0(,R8)
         L     R1,4(,R8)
         L     R15,60(,R8)
         EX    R7,SVC1
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
#if REALHLASM
         ENTRY @@RET7
#else
         .globl __ret7
#endif
@@RET7   DS    0H
         L     R15,=F'7'
         BR    R14
*
         LTORG
         DROP  R15
*
         END
