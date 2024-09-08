* VSE main test program
* Public domain by Paul Edwards
* This was butchered from PDPCLIB and never cleaned up/rationalized
* An exercise for the reader, perhaps
*
         PRINT GEN
* REGEQU IS NOT AVAILABLE ON DOS/VS
*         YREGS
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
SUBPOOL  EQU   0
         CSECT
*
* Put an eyecatcher here to ensure program has been linked
* correctly.
         DC    C'PDPCLIB!'
         ENTRY @@CRT0
@@CRT0   DS    0H
*
* DOS/VS doesn't require or allow saving of registers
*         SAVE  (14,12),,@@CRT0
*
         BALR  R10,R0
         LA    R10,0(R10)        clean address
         BCTR  R10,0
         BCTR  R10,0
         USING @@CRT0,R10
*
         LR    R8,R15            save R15 so that we can get the PARM
         LR    R11,R1            save R1 so we can get the PARM
         GETVIS LENGTH=STACKLEN
         LTR   R15,R15
         BNZ   RETURNMS
         ST    R13,4(R1)
         LR    R13,R1
         USING STACK,R13
*
         LA    R2,0
         ST    R2,DUMMYPTR       WHO KNOWS WHAT THIS IS USED FOR
         LA    R2,MAINSTK
         ST    R2,THEIRSTK       NEXT AVAILABLE SPOT IN STACK
         LA    R12,ANCHOR
         ST    R14,EXITADDR
         L     R3,=A(MAINLEN)
         AR    R2,R3
         ST    R2,12(R12)        TOP OF STACK POINTER
         LA    R2,0
         ST    R2,116(R12)       ADDR OF MEMORY ALLOCATION ROUTINE
*
* Now let's get the parameter list
*
         COMRG                   get address of common region in R1
         LR    R5,R1             use R5 to map common region
         USING COMREG,R5         address common region
         L     R2,SYSPAR         get access to SYSPARM
         LA    R2,0(R2)          clean the address, just in case
         ST    R2,ARGPTR         store SYSPARM
         MVC   XUPSI,UPSI        move the UPSI switches in
         LA    R2,0              default no VSE-style PARM
         CR    R11,R8            compare original R15 and original R1
         BE    CONTPARM          no difference = no VSE-style PARM
         LR    R2,R11            R11 has PARM, now R2 does too
* Note - do not clean the PARM, as we need access to the top bit
CONTPARM DS    0H
         ST    R2,ARGPTRE        store VSE-style PARM
         L     R2,JAPART         address of job accounting table
         DROP  R5                no longer need common region
         LA    R2,PGMNAME        address our program name
         ST    R2,PGMNPTR        store program name
*
* Set R4 to true if we were called in 31-bit mode
*
         LA    R4,0
         ST    R4,SAVER4
*
* FOR GCC WE NEED TO BE ABLE TO RESTORE R13
         LA    R5,SAVEAREA
         ST    R5,SAVER13
*
         LA    R1,PARMLIST
*
*
NOMEM    DS    0H
         EXCP  CCB
         WAIT  CCB
         B     BYPASS1
ERRMSG   DC    C'PDPTEST STARTED'
CCB      CCB   SYSLOG,CCW
CCW      CCW   X'09',ERRMSG,0,L'ERRMSG
BYPASS1  DS    0H
         CALL  @@MID
NOMEM2   DS    0H
         EXCP  CCB2
         WAIT  CCB2
         B     BYPASS2
ERRMSG2  DC    C'PDPTEST ENDED'
CCB2     CCB   SYSLOG,CCW2
CCW2     CCW   X'09',ERRMSG2,0,L'ERRMSG2
BYPASS2  DS    0H
         LR    R9,R15
*
*
RETURNMS DS    0H
         LR    R1,R13
         L     R13,SAVEAREA+4
         LR    R14,R9
         FREEVIS LENGTH=STACKLEN
         LR    R15,R14
         EOJ
SAVER4   DS    F
SAVER13  DS    F
         LTORG
*         ENTRY CEESG003
*CEESG003 DS    0H
         DS    0H
         ENTRY @@EXITA
@@EXITA  DS    0H
* SWITCH BACK TO OUR OLD SAVE AREA
         LR    R10,R15
         USING @@EXITA,R10
         L     R9,0(R1)
         L     R13,=A(SAVER13)
         L     R13,0(R13)
*
*
         LR    R1,R13
         L     R13,4(R13)
         LR    R14,R9
         FREEVIS LENGTH=STACKLEN
         LR    R15,R14
*         RETURN (14,12),RC=(15)
         EOJ
         LTORG
STACKLEN DC    A(STKLTMP)
*
STACK    DSECT
SAVEAREA DS    18F
DUMMYPTR DS    F
THEIRSTK DS    F
PARMLIST DS    0F
ARGPTR   DS    F
PGMNPTR  DS    F
ARGPTRE  DS    F
TYPE     DS    F
PGMNAME  DS    CL8
PGMNAMEN DS    C                 NUL BYTE FOR C
XUPSI    DS    C                 UPSI switches
ANCHOR   DS    0F
EXITADDR DS    F
         DS    49F
MAINSTK  DS    32000F
MAINLEN  EQU   *-MAINSTK
STKLTMP  EQU   *-STACK
*
COMREG   MAPCOMR
         END   @@CRT0
