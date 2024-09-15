* MVS main program
* Public domain by Paul Edwards
*
         CSECT
         PRINT GEN
* YREGS is not standard
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
*
*
*
*
         ENTRY @@MAIN
@@MAIN   DS    0H
         SAVE  (14,12),,@@MAIN
         LR    R12,R15
         USING @@MAIN,R12
*
         WTO   'In main program'
         CALL  @@MID
*
         RETURN (14,12),RC=(15)
         LTORG
*
         END
