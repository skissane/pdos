* VSE Subroutine
* Public domain by Paul Edwards
*
         CSECT
         PRINT GEN
* REGEQU is not standard
*         REGEQU
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
*
*
*
*
         ENTRY @@MID
@@MID    DS    0H
         SAVE  (14,12),,@@MID
         LR    R12,R15
         USING @@MID,R12
*
         EXCP  CCB
         WAIT  CCB
         B     BYPASS1
ERRMSG   DC    C'PDPTEST MIDDLE'
CCB      CCB   SYSLOG,CCW
CCW      CCW   X'09',ERRMSG,0,L'ERRMSG
BYPASS1  DS    0H
         LA    R15,0
*
         RETURN (14,12),RC=(15)
         LTORG
*
         END
