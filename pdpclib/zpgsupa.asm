* This program written by Paul Edwards.
* Released to the public domain
*
         CSECT
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
         LA    R3,32(,R13)        # use user's save area
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
         END
