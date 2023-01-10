PDOSSUP TITLE 'P D O S S U P  ***  SUPPORT ROUTINE FOR PDOS'
***********************************************************************
*                                                                     *
*  THIS PROGRAM WRITTEN BY PAUL EDWARDS.                              *
*  RELEASED TO THE PUBLIC DOMAIN                                      *
*                                                                     *
***********************************************************************
***********************************************************************
*                                                                     *
*  PDOSSUP - assembler support routines for PDOS                      *
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
         AIF ('&XSYS' NE 'ZARCH').ZVAR64B
FLCEINPW EQU   496   A(X'1F0')
FLCEMNPW EQU   480   A(X'1E0')
FLCESNPW EQU   448   A(X'1C0')
FLCEPNPW EQU   464   A(X'1D0')
FLCESOPW EQU   320   A(X'140')
.ZVAR64B ANOP
*
*
*
         AIF ('&XSYS' EQ 'S370').AMB24A
AMBIT    EQU X'80000000'
         AGO .AMB24B
.AMB24A  ANOP
AMBIT    EQU X'00000000'
.AMB24B  ANOP
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
         AIF ('&XSYS' EQ 'ZARCH').ZSW64
         MVC   FLCINPSW(8),WAITER7
         MVC   FLCMNPSW(8),WAITER1
         MVC   FLCSNPSW(8),WAITER2
         MVC   FLCPNPSW(8),WAITER3
* Note that SVCNPSW is an alias for FLCSNPSW
         MVC   SVCNPSW(8),NEWSVC
         AGO .ZSW64B
.ZSW64   ANOP
         MVC   FLCEINPW(16),WAITER7
         MVC   FLCEMNPW(16),WAITER1
         MVC   FLCEPNPW(16),WAITER3
         MVC   FLCESNPW(16),NEWSVC
.ZSW64B  ANOP
*
*
* Prepare CR6 for interrupts
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').SIO24A
         LCTL  6,6,ALLIOINT CR6 needs to enable all interrupts
.SIO24A  ANOP
*
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
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').NOT390A
         DS    0F
ALLIOINT DC    X'FF000000'
.NOT390A ANOP
*
*
*
         DS    0D
         AIF ('&XSYS' EQ 'ZARCH').WAIT64A
WAITER7  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000777')  error 777
WAITER1  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000111')  error 111
WAITER2  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000222')  error 222
WAITER3  DC    X'040E0000'  machine check, EC, wait, dat on
         DC    A(AMBIT+X'00000333')  error 333
NEWSVC   DC    X'040C0000'  machine check, EC, DAT on
         DC    A(AMBIT+GOTSVC)  SVC handler
         AGO   .WAIT64B
.WAIT64A ANOP
WAITER1  DC    A(X'00060000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(X'00000111')  error 111
WAITER3  DC    A(X'00060000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(X'00000333')  error 333
WAITER7  DC    A(X'00060000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(X'00000777')  error 777
NEWSVC   DC    A(X'00040000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(GOTSVC)
.WAIT64B ANOP
*
*
*
**********************************************************************
*                                                                    *
*  WRBLOCK - write a block to disk                                   *
*                                                                    *
*  parameter 1 = device                                              *
*  parameter 2 = cylinder                                            *
*  parameter 3 = head                                                *
*  parameter 4 = record                                              *
*  parameter 5 = buffer                                              *
*  parameter 6 = size of buffer                                      *
*  parameter 7 = command code (x'5' for data, x'd' for key + data,   *
*                x'1d' for count, key, data. Normally x'1d' is       *
*                required (but the 8 byte counter field does not     *
*                include the length of the counter itself; also, the *
*                record number passed to the routine must be one     *
*                less than in the counter field).                    *
*                                                                    *
*  return = length of data written, or -1 on error                   *
*                                                                    *
**********************************************************************
         ENTRY WRBLOCK
WRBLOCK  DS    0H
         SAVE  (14,12),,WRBLOCK
         LR    R12,R15
         USING WRBLOCK,R12
         USING PSA,R0
*
         L     R10,0(R1)    Device number
         L     R2,4(R1)     Cylinder
         STCM  R2,B'0011',WRCC1
         STCM  R2,B'0011',WRCC2
         L     R2,8(R1)     Head
         STCM  R2,B'0011',WRHH1
         STCM  R2,B'0011',WRHH2
         L     R2,12(R1)    Record
         STC   R2,WRR
         L     R2,24(R1)    Command code
         STC   R2,WRLDCCW
         L     R2,16(R1)    Buffer
* It is a requirement of using this routine that V=R. If it is
* ever required to support both V and R, then LRA could be used,
* and check for a 0 return, and if so, do a BNZ.
*         LRA   R2,0(R2)     Get real address
         L     R7,20(R1)    Bytes to write
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').WR390B
         STCM  R2,B'0111',WRLDCCW+1   This requires BTL buffer
         STH   R7,WRLDCCW+6  Store in WRITE CCW
         AGO   .WR390C
.WR390B  ANOP
         ST    R2,WRLDCCW+4
         STH   R7,WRLDCCW+2
.WR390C  ANOP
*
* Interrupt needs to point to CONT now. Again, I would hope for
* something more sophisticated in PDOS than this continual
* initialization.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZWRNIO
         MVC   FLCINPSW(8),WRNEWIO
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
         AGO .ZWRNIOA
.ZWRNIO  ANOP
         MVC   FLCEINPW(16),WRNEWIO
         STOSM FLCEINPW,X'00'  Work with DAT on or OFF
.ZWRNIOA ANOP
*
* R3 points to CCW chain
         LA    R3,WRSEEK
         ST    R3,FLCCAW    Store in CAW
*
*
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').WR31B
         SIO   0(R10)
*         TIO   0(R10)
         AGO   .WR24B
.WR31B   ANOP
         LR    R1,R10       R1 needs to contain subchannel
         LA    R9,WRIRB
         LA    R10,WRORB
         MSCH  0(R10)
         TSCH  0(R9)        Clear pending interrupts
         SSCH  0(R10)
.WR24B   ANOP
*
*
         LPSW  WRWTNOER     Wait for an interrupt
         DC    H'0'
WRCONT   DS    0H           Interrupt will automatically come here
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').WR31H
         SH    R7,FLCCSW+6  Subtract residual count to get bytes written
         LR    R15,R7
* After a successful CCW chain, CSW should be pointing to end
         CLC   FLCCSW(4),=A(WRFINCHN)
         BE    WRALLFIN
         AGO   .WR24H
.WR31H   ANOP
         TSCH  0(R9)
         SH    R7,10(R9)
         LR    R15,R7
         CLC   4(4,R9),=A(WRFINCHN)
         BE    WRALLFIN
.WR24H   ANOP
         L     R15,=F'-1'   error return
WRALLFIN DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').WR390G
         DS    0F
WRIRB    DS    24F
WRORB    DS    0F
         DC    F'0'
         DC    X'0080FF00'  Logical-Path Mask (enable all?) + format-1
         DC    A(WRSEEK)
         DC    5F'0'
.WR390G  ANOP
*
*
         DS    0D
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').WR390
WRSEEK   CCW   7,WRBBCCHH,X'40',6       40 = chain command
WRSRCH   CCW   X'31',WRCCHHR,X'40',5    40 = chain command
         CCW   8,WRSRCH,0,0
* X'1D' = write count, key and data
WRLDCCW  CCW   X'1D',0,X'00',32767     not 20 = ignore length issues
         AGO   .WR390F
.WR390   ANOP
WRSEEK   CCW1  7,WRBBCCHH,X'40',6       40 = chain command
WRSRCH   CCW1  X'31',WRCCHHR,X'40',5    40 = chain command
         CCW1  8,WRSRCH,0,0
* X'1D' = write count, key and data
WRLDCCW  CCW1  X'1D',0,X'00',32767     not 20 = ignore length issues
.WR390F  ANOP
WRFINCHN EQU   *
         DS    0H
WRBBCCHH DC    X'000000000000'
         ORG   *-4
WRCC1    DS    CL2
WRHH1    DS    CL2
WRCCHHR  DC    X'0000000005'
         ORG   *-5
WRCC2    DS    CL2
WRHH2    DS    CL2
WRR      DS    C
         DS    0D
WRWTNOER DC    X'060E0000'  I/O, machine check, EC, wait, DAT on
         DC    A(AMBIT)  no error
*
         AIF   ('&XSYS' EQ 'ZARCH').WRZNIO
* machine check, EC, DAT off
WRNEWIO  DC    A(X'000C0000')
         DC    A(AMBIT+WRCONT)  continuation after I/O request
         AGO   .WRNZIOA
*
.WRZNIO  ANOP
WRNEWIO  DC    A(X'00040000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(WRCONT)  continuation after I/O request
.WRNZIOA ANOP
*
         DROP  ,
*
*
*
*
*
**********************************************************************
*                                                                    *
*  ADISP - dispatch a bit of code                                    *
*                                                                    *
**********************************************************************
         ENTRY ADISP
ADISP    DS    0H
         SAVE  (14,12),,ADISP
         LR    R12,R15
         USING ADISP,R12
         USING PSA,R0
*
         STM   R0,R15,FLCFLA        Save our OS registers
         LM    R0,R15,FLCGRSAV        Load application registers
         LPSW  SVCOPSW                App returns to old PSW
         DC    H'0'
ADISPRET DS    0H
         LA    R15,0
ADISPRT2 DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
*
**********************************************************************
*                                                                    *
*  GOTSVC - got an SVC interrupt                                     *
*                                                                    *
*  Need to go back through the dispatcher, which is waiting for this *
*                                                                    *
**********************************************************************
*
GOTSVC   DS    0H
         STM   R0,R15,FLCGRSAV        Save application registers
         LM    R0,R15,FLCFLA        Load OS registers
         B     ADISPRET
*
*
*
**********************************************************************
*                                                                    *
*  GOTRET - the application has simply returned with BR R14          *
*                                                                    *
*  Need to go back through the dispatcher. The easiest way to do     *
*  this is simply execute SVC 3, which the dispatcher has special    *
*  processing for.                                                   *
*                                                                    *
**********************************************************************
*
         ENTRY GOTRET
GOTRET   DS    0H
* force an SVC - it will take care of the rest
         SVC   3
         DC    H'0'                   PDOS should not return here
*
*
*
**********************************************************************
*                                                                    *
*  DREAD - DCB read routine (for when people do READ)                *
*                                                                    *
*  for now - go back via the same path as an SVC                     *
*                                                                    *
*  Relies on R12 being restored to its former glory, since we saved  *
*  it before running the application                                 *
*                                                                    *
**********************************************************************
*
         ENTRY DREAD
DREAD    DS    0H
         STM   R0,R15,FLCGRSAV        Save application registers
         AIF   ('&XSYS' EQ 'ZARCH').ZRDA
         ST    R14,SVCOPSW+4
         NI    SVCOPSW+4,X'80'
         AGO   .ZRDB
.ZRDA    ANOP
         ST    R14,FLCESOPW+12
*         NI    FLCESOPW+12,X'00'
         ST    R14,FLCESOPW+4
         AIF   ('&ZAM64' NE 'YES').STAY24A
         OI    FLCESOPW+4,X'80'
         AGO   .STAY24B
.STAY24A ANOP
         NI    FLCESOPW+4,X'00'
.STAY24B ANOP
*         NI    FLCESOPW+3,X'FE'
.ZRDB    ANOP
         LM    R0,R15,FLCFLA          Load OS registers
*
* We need to return to 31-bit mode, which PDOS may be operating in.
         AIF   ('&XSYS' EQ 'S370' OR                                   +
                ('&XSYS' EQ 'ZARCH' AND '&ZAM64' EQ 'YES')).MOD24G
         CALL  @@SETM31
.MOD24G  ANOP
         LA    R15,3
         B     ADISPRT2
*
*
*
**********************************************************************
*                                                                    *
*  DWRITE - DCB write routine (for when people do WRITE)             *
*                                                                    *
*  for now - go back via the same path as an SVC                     *
*                                                                    *
**********************************************************************
*
         ENTRY DWRITE
DWRITE   DS    0H
         STM   R0,R15,FLCGRSAV        Save application registers
         AIF   ('&XSYS' EQ 'ZARCH').ZWRA
         ST    R14,SVCOPSW+4
         NI    SVCOPSW+4,X'80'
         AGO   .ZWRB
.ZWRA    ANOP
         ST    R14,FLCESOPW+12
*         NI    FLCESOPW+12,X'00'
         ST    R14,FLCESOPW+4
         AIF   ('&ZAM64' NE 'YES').STAY24C
         OI    FLCESOPW+4,X'80'
         AGO   .STAY24D
.STAY24C ANOP
         NI    FLCESOPW+4,X'00'
.STAY24D ANOP
*         NI    FLCESOPW+3,X'FE'
.ZWRB    ANOP
         LM    R0,R15,FLCFLA          Load OS registers
*
* We need to return to 31-bit mode, which PDOS may be operating in.
         AIF   ('&XSYS' EQ 'S370' OR                                   +
                ('&XSYS' EQ 'ZARCH' AND '&ZAM64' EQ 'YES')).MOD24D
         CALL  @@SETM31
.MOD24D  ANOP
         LA    R15,2
         B     ADISPRT2
         DROP  ,
*
*
*
**********************************************************************
*                                                                    *
*  DCHECK - DCB check routine (for when people do CHECK)             *
*                                                                    *
*  for now, do nothing, since writes are executed synchronously      *
*                                                                    *
**********************************************************************
*
         ENTRY DCHECK
DCHECK   DS    0H
         BR    R14
*
*
*
**********************************************************************
*                                                                    *
*  DNOTPNT - DCB note/point routine (for when people do NOTE/POINT)  *
*                                                                    *
*  for now is pretty much a dummy function                           *
*                                                                    *
**********************************************************************
*
         ENTRY DNOTPNT
DNOTPNT  DS    0H
* NOTE entry point is 0
         LA    R1,0
* POINT entry point is 4
         LA    R1,0
         BR    R14
*
*
*
**********************************************************************
*                                                                    *
*  DEXIT - DCB exit                                                  *
*                                                                    *
*  This is for when very annoying people have used a DCB exit which  *
*  needs to be called in the middle of doing an OPEN                 *
*                                                                    *
*  They are expecting a DCB in R1                                    *
*                                                                    *
*  This routine is expecting the address of their exit as the first  *
*  parameter, and the address of the DCB as the second.              *
*                                                                    *
**********************************************************************
         ENTRY DEXIT
DEXIT    DS    0H
         SAVE  (14,12),,DEXIT
         LR    R12,R15
         USING DEXIT,R12
*
         LR    R4,R13                 save old save area
         LA    R13,76(R13)            new save area
*
         L     R2,0(R1)               their exit
         L     R3,4(R1)               actual DCB for them
         AIF   ('&XSYS' EQ 'S370' OR                                   +
                ('&XSYS' EQ 'ZARCH' AND '&ZAM64' EQ 'YES')).MOD24E
         CALL  @@SETM24
.MOD24E  ANOP
*
         LR    R15,R2
         LR    R1,R3
         STM   R14,R12,12(R13)         
         BALR  R14,R15
         LM    R0,R12,20(R13)
*
         AIF   ('&XSYS' EQ 'S370' OR                                   +
                ('&XSYS' EQ 'ZARCH' AND '&ZAM64' EQ 'YES')).MOD24F
         CALL  @@SETM31
.MOD24F  ANOP
*
DEXITRET DS    0H
         LR    R13,R4                 restore save area
         RETURN (14,12),RC=(15)
         LTORG
*
*
*
**********************************************************************
*                                                                    *
*  TRKCLC - TRKCALC CVT function                                     *
*                                                                    *
*  just return 3 tracks per block                                    *
*                                                                    *
**********************************************************************
*
         ENTRY TRKCLC
TRKCLC   DS    0H
* Main entry point is offset 12
         DC    12X'00'
         LA    R0,3
         LA    R15,0
         BR    R14
*
*
*
**********************************************************************
*                                                                    *
*  LCREG0 - load control register 0                                  *
*                                                                    *
*  parameter 1 = value                                               *
*                                                                    *
**********************************************************************
         ENTRY LCREG0
LCREG0   DS    0H
         SAVE  (14,12),,LCREG0
         LR    R12,R15
         USING LCREG0,R12
*
         LCTL  0,0,0(R1)
*
         LA    R15,0
         RETURN (14,12),RC=(15)
         LTORG
**********************************************************************
*                                                                    *
*  LCREG1 - load control register 1                                  *
*                                                                    *
*  parameter 1 = value                                               *
*                                                                    *
**********************************************************************
         ENTRY LCREG1
LCREG1   DS    0H
         SAVE  (14,12),,LCREG1
         LR    R12,R15
         USING LCREG1,R12
*
         LCTL  1,1,0(R1)
*
         LA    R15,0
         RETURN (14,12),RC=(15)
         LTORG
**********************************************************************
*                                                                    *
*  LCREG13 - load control register 13                                *
*                                                                    *
*  parameter 1 = value                                               *
*                                                                    *
**********************************************************************
         ENTRY LCREG13
LCREG13  DS    0H
         SAVE  (14,12),,LCREG13
         LR    R12,R15
         USING LCREG13,R12
*
         LCTL  13,13,0(R1)
*
         LA    R15,0
         RETURN (14,12),RC=(15)
         LTORG
**********************************************************************
*                                                                    *
*  DATON - switch on DAT                                             *
*                                                                    *
**********************************************************************
         ENTRY DATON
DATON    DS    0H
         SAVE  (14,12),,DATON
         LR    R12,R15
         USING DATON,R12
*
         STOSM CURRMASK,X'04'
*
         LA    R15,0
         RETURN (14,12),RC=(15)
         LTORG
**********************************************************************
*                                                                    *
*  DATOFF - switch off DAT                                           *
*                                                                    *
**********************************************************************
         ENTRY DATOFF
DATOFF   DS    0H
         SAVE  (14,12),,DATOFF
         LR    R12,R15
         USING DATOFF,R12
*
         STNSM CURRMASK,X'FB'
*
         LA    R15,0
         RETURN (14,12),RC=(15)
         LTORG
CURRMASK DS    C
*
*
*
**********************************************************************
*                                                                    *
*  DSECTS                                                            *
*                                                                    *
**********************************************************************
         CVT   DSECT=YES
         IKJTCB
         IEZJSCB
         IHAPSA
         IHARB
         IHACDE
         IHASVC
         END
