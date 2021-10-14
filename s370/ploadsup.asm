PLOADSUP TITLE 'P L O A D S U P  ***  SUPPORT ROUTINE FOR PLOAD'
***********************************************************************
*                                                                     *
*  THIS PROGRAM WRITTEN BY PAUL EDWARDS.                              *
*  RELEASED TO THE PUBLIC DOMAIN                                      *
*                                                                     *
***********************************************************************
***********************************************************************
*                                                                     *
*  PLOADSUP - assembler support routines for PLOAD                    *
*  It is currently coded to work with GCC. To activate the C/370      *
*  version change the "&COMP" switch.                                 *
*                                                                     *
***********************************************************************
*
         COPY  PDPTOP
*
         PRINT GEN
         YREGS
SUBPOOL  EQU   0
*
         AIF ('&ZSYS' NE 'ZARCH').ZVAR64B
FLCEINPW EQU   496   A(X'1F0')
FLCEMNPW EQU   480   A(X'1E0')
FLCESNPW EQU   448   A(X'1C0')
FLCEPNPW EQU   464   A(X'1D0')
.ZVAR64B ANOP
*
*
*
         AIF ('&ZSYS' EQ 'S370').AMB24A
AMBIT    EQU X'80000000'
         AGO .AMB24B
.AMB24A  ANOP
AMBIT    EQU X'00000000'
.AMB24B  ANOP
*
*
*
         AIF ('&ZAM64' NE 'YES').AMZB24A
AM64BIT  EQU X'00000001'
         AGO .AMZB24B
.AMZB24A ANOP
AM64BIT  EQU X'00000000'
.AMZB24B ANOP
*
*
*
         CSECT
**********************************************************************
*                                                                    *
*  INITSYS - initialize system                                       *
*                                                                    *
*  Note that at this point we can't assume what the status of the    *
*  interrupt vectors are, so we need to set them all to something    *
*  sensible ourselves. I/O will only be enabled when we want to do   *
*  an I/O.                                                           *
*                                                                    *
**********************************************************************
         ENTRY INITSYS
INITSYS  DS    0H
         SAVE  (14,12),,INITSYS
         LR    R12,R15
         USING INITSYS,R12
         USING PSA,R0
*
* At this stage we don't want any interrupts, but we need
* to set "dummy" values for all of them, to give us
* visibility into any problem.
*
         AIF ('&ZSYS' EQ 'ZARCH').ZSW64
         MVC   FLCINPSW(8),WAITER7
         MVC   FLCMNPSW(8),WAITER1
         MVC   FLCSNPSW(8),WAITER2
         MVC   FLCPNPSW(8),WAITER3
         AGO .ZSW64B
.ZSW64   ANOP
         MVC   FLCEINPW(16),WAITER7
         MVC   FLCEMNPW(16),WAITER1
         MVC   FLCESNPW(16),WAITER2
         MVC   FLCEPNPW(16),WAITER3
.ZSW64B  ANOP
*
*
* Prepare CR6 for interrupts
         AIF   ('&ZSYS' NE 'S390' AND '&ZSYS' NE 'ZARCH').SIO24A
         LCTL  6,6,ALLIOINT CR6 needs to enable all interrupts
.SIO24A  ANOP
*
* Save IPL address in R10
* We should really obtain this from a parameter passed by
* sapstart.
*
         SLR   R10,R10
         ICM   R10,B'1111',FLCIOA
         LR    R15,R10
*
         RETURN (14,12),RC=(15)
         LTORG
*
*
*
         AIF   ('&ZSYS' NE 'S390' AND '&ZSYS' NE 'ZARCH').NOT390A
         DS    0F
ALLIOINT DC    X'FF000000'
.NOT390A ANOP
*
*
*
         DS    0D
         AIF ('&ZSYS' EQ 'ZARCH').WAIT64A
WAITER7  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000777')  error 777
WAITER1  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000111')  error 111
WAITER2  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000222')  error 222
WAITER3  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000333')  error 333
         AGO   .WAIT64B
.WAIT64A ANOP
* Below values somewhat obtained from UDOS
WAITER1  DC    A(X'00060000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(X'00000111')  error 111
WAITER2  DC    A(X'00060000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(X'00000222')  error 222
WAITER3  DC    A(X'00060000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(X'00000333')  error 333
WAITER7  DC    A(X'00060000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(X'00000777')  error 777
.WAIT64B ANOP
*
*
*
         CVT   DSECT=YES
         IKJTCB
         IEZJSCB
         IHAPSA
         IHARB
         IHACDE
         END
