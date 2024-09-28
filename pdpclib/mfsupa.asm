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
         ENTRY @@CRT0
@@CRT0   DS    0H
#if BIGFOOT
         BALR  R15,R0
         BCTR  R15,0
         BCTR  R15,0
         NOPR  0
#endif
         B     SKIPHDR
         DC    C'PGCX'  # PDOS-generic (or compatible) extension
         DC    F'4'  # length of header data
         ENTRY @@PGPARM
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
#if VSE || BIGFOOT
         L     R1,=V(@@PGPARM)
         L     R1,0(,R1)
         LTR   R1,R1
         BZ    NOTPDOS
         STM   R14,R12,12(R13)
         LA    R13,80(,R13)
         LR    R12,R15
         DROP  R15
         USING @@CRT0, R12
         B     BYPASS1
#endif
*
NOTPDOS  DS    0H
*
         LR    R12,R15
         LR    R7,R14
         L     R3,=F'3'
         L     R4,=F'4'
         L     R5,=F'5'
*
         LA    R13,SAVEA
BYPASS1  DS    0H
         LA    R9,80(,R13)
         ST    R9,76(,R13)
*
#if MVS
         L     R15,=V(@@MVSRUN)
#endif
*
#if VSE
         L     R15,=V(@@VSERUN)
#endif
*
#if BIGFOOT
         L     R15,=V(@@BIGRUN)
#endif
*
         BALR  R14,R15
*
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
         USING *,R11
         ENTRY @@SVCRL
@@SVCRL  DS    0H
         STM   R14,R12,12(R13)
         LR    R11,R15
         L     R12,0(,R1)
         L     R10,4(,R1)
         L     R0,0(,R10)
         L     R1,4(,R10)
         L     R2,8(,R10)
         L     R3,12(,R10)
         L     R4,16(,R10)
         L     R5,20(,R10)
         L     R6,24(,R10)
         L     R7,28(,R10)
         L     R8,32(,R10)
         L     R9,36(,R10)
         L     R15,60(,R10)
         EX    R12,SVC1
         B     SVC2
SVC1     DS    0H
         SVC   0
SVC2     DS    0H
         L     R14,12(R13)
         LM    R0,R12,20(R13)
         BR    R14
*
         LTORG
         DROP  R11
*
*
*
*
*
         DS    0H
         USING *,R15
         ENTRY @@RET7
@@RET7   DS    0H
         L     R15,=F'7'
         BR    R14
*
         LTORG
         DROP  R15
*
*
*
***********************************************************************
*                                                                     *
*  SETJ - SAVE REGISTERS INTO ENV                                     *
*                                                                     *
***********************************************************************
         ENTRY @@SETJ
@@SETJ   DS    0H
         L     R15,0(,R1)         # get the env variable
         STM   R0,R14,0(R15)      # save registers to be restored
         LA    R15,0(,R0)         # setjmp needs to return 0
         BR    R14                # return to caller
*
***********************************************************************
*                                                                     *
*  LONGJ - RESTORE REGISTERS FROM ENV                                 *
*                                                                     *
***********************************************************************
         ENTRY @@LONGJ
@@LONGJ  DS    0H
         L     R2,0(,R1)          # get the env variable
         L     R15,60(,R2)        # get the return code
         LM    R0,R14,0(R2)       # restore registers
         BR    R14                # return to caller
*
***********************************************************************
*                                                                     *
*  GETCLCK - GET THE VALUE OF THE MVS CLOCK TIMER AND MOVE IT TO AN   *
*  8-BYTE FIELD.  THIS 8-BYTE FIELD DOES NOT NEED TO BE ALIGNED IN    *
*  ANY PARTICULAR WAY.                                                *
*                                                                     *
*  E.G. CALL 'GETCLCK' USING WS-CLOCK1                                *
*                                                                     *
*  THIS FUNCTION ALSO RETURNS THE NUMBER OF SECONDS SINCE 1970-01-01  *
*  BY USING SOME EMPIRICALLY-DERIVED MAGIC NUMBERS                    *
*                                                                     *
***********************************************************************
*
         ENTRY @@GETCLK
         USING @@GETCLK,R15
@@GETCLK DS    0H
         STM   R2,R5,12(R13)      # save a little
         LA    R3,32(,R13)        # /* use user's save area */
         N     R3,=X'FFFFFFF8'    # on a double word boundary
         STCK  0(R3)              # stash the clock
         L     R2,0(,R1)          # address may be ATL
         MVC   0(8,R2),0(R3)      # copy to BTL or ATL
         L     R4,0(,R2)
         L     R5,4(,R2)
         SRDL  R4,12
         SL    R4,=X'0007D910'
         D     R4,=F'1000000'
         SL    R5,=F'1220'
         LR    R15,R5             # return result
         LM    R2,R5,12(R13)      # restore modified registers
         BR    R14                # return to caller
*
         LTORG
         DROP  R15
*
*
*
**********************************************************************
*                                                                    *
*  GETAM - get the current AMODE                                     *
*                                                                    *
*  This function returns 24 if we are running in exactly AMODE 24,   *
*  31 if we are running in exactly AMODE 31, and 64 for anything     *
*  else (user-defined/infinity/16/32/64/37)                          *
*                                                                    *
*  Be aware that MVS 3.8j I/O routines require an AMODE of exactly   *
*  24 - nothing more, nothing less - so applications are required    *
*  to ensure they are in AM24 prior to executing any I/O routines,   *
*  and then they are free to return to whichever AMODE they were in  *
*  previously (ie anything from 17 to infinity), which is normally   *
*  done using a BSM to x'01', although this instruction was not      *
*  available in S/370-XA so much software does a BSM to x'80'        *
*  instead of the user-configurable x'01', which is unfortunate.     *
*                                                                    *
*  For traditional reasons, people refer to 24, 31 and 64, when what *
*  they should really be saying is 24, 31 and user-defined.          *
*                                                                    *
**********************************************************************
         ENTRY @@GETAM
@@GETAM  DS    0H
         STM   R14,R12,12(R13)
         LR    R12,R15
         USING @@GETAM,R12
*
         L     R2,=X'C1800000'
         LA    R2,0(,R2)
*         CLM   R2,B'1100',=X'0080'
         CLM   R2,12,NUM80
         BE    GAIS24
*         CLM   R2,B'1000',=X'41'
         CLM   R2,8,=X'41'
         BE    GAIS31
         LA    R15,64(0)
         B     RETURNGA
GAIS24   DS    0H
         LA    R15,24(0)
         B     RETURNGA
GAIS31   LA    R15,31(0)
*
RETURNGA DS    0H
         L     R14,12(R13)
         LM    R0,R12,20(R13)
         BR    R14
         LTORG
*
*
*
         END
