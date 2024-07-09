ZPBSUPA  TITLE 'Z P B S U P A  ***  ZPB VERSION OF PDPCLIB SUPPORT'
***********************************************************************
*
*  This program written by Paul Edwards.
*  Released to the public domain
*
*  Extensively modified by others
*
***********************************************************************
***********************************************************************
*
*  ZPBSUPA - Support routines for PDPCLIB for z/PDOS-generic pseudobios
*
***********************************************************************
*
         SPACE 1
         COPY  PDPTOP
         SPACE 1
         CSECT ,
         PRINT GEN,ON
         YREGS
         SPACE 1
*
         AIF ('&XSYS' NE 'ZARCH').ZVAR64
*
* These values can be found here:
* https://www.ibm.com/docs/en/zos/2.4.0
* ?topic=information-ihapsae-mapping
*
FLCEINPW EQU   496   A(X'1F0')
FLCEMNPW EQU   480   A(X'1E0')
FLCESNPW EQU   448   A(X'1C0')
FLCEPNPW EQU   464   A(X'1D0')
*
* And are distinct from the traditional values found here:
* https://www.ibm.com/docs/en/zos/2.4.0
* ?topic=information-psa-mapping
*
.ZVAR64  ANOP
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
***********************************************************************
*                                                                     *
*  SETJ - SAVE REGISTERS INTO ENV                                     *
*                                                                     *
***********************************************************************
         ENTRY @@SETJ
@@SETJ   L     R15,0(,R1)         get the env variable
         STM   R0,R14,0(R15)      save registers to be restored
         LA    R15,0              setjmp needs to return 0
         BR    R14                return to caller
         SPACE 1
***********************************************************************
*                                                                     *
*  LONGJ - RESTORE REGISTERS FROM ENV                                 *
*                                                                     *
***********************************************************************
         ENTRY @@LONGJ
@@LONGJ  L     R2,0(,R1)          get the env variable
         L     R15,60(,R2)        get the return code
         LM    R0,R14,0(R2)       restore registers
         BR    R14                return to caller
         SPACE 2
*
*
*
**********************************************************************
*                                                                    *
*  @@CONSWR - write to console                                       *
*                                                                    *
*  parameter 1 = buffer length                                       *
*  parameter 2 = buffer                                              *
*  parameter 3 = CR required flag                                    *
*                                                                    *
**********************************************************************
         ENTRY @@CONSWR
@@CONSWR DS    0H
         SAVE  (14,12),,@@CONSWR
         LR    R12,R15
         USING @@CONSWR,R12
         USING PSA,R0
*
         L     R10,=V(@@CONSDN) Device number
         L     R10,0(R10)
         L     R7,0(R1)        Bytes to write
         L     R2,4(R1)        Buffer to write
         L     R8,8(R1)        Is CR required?
         MVI   CCHAIN,X'01'    Assume no CR required
         LTR   R8,R8
         BZ    NOCRREQ
         MVI   CCHAIN,X'09'    Need a CR
* For some reason the CCW doesn't like an empty line of 0 bytes.
* Need to find out why. Until then, assume that's the way that
* it's meant to be, and force a space
         LTR   R7,R7
         BNZ   NOSPACE
         LA    R2,=C' '
         LA    R7,1
NOSPACE  DS    0H
NOCRREQ  DS    0H
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').CHN390G
         STCM  R2,B'0111',CCHAIN+1   This requires BTL buffer
         STH   R7,CCHAIN+6     Store length in WRITE CCW
         AGO   .CHN390H
.CHN390G ANOP
         ST    R2,CCHAIN+4
         STH   R7,CCHAIN+2
.CHN390H ANOP
*
* Interrupt needs to point to CCONT now. Again, I would hope for
* something more sophisticated in PDOS than this continual
* initialization.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZNEWIOA
         MVC   FLCINPSW(8),CNEWIO
         AGO   .ZNEWIOB
.ZNEWIOA ANOP
         MVC   FLCEINPW(16),CNEWIO
.ZNEWIOB ANOP
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
* R3 points to CCW chain
         LA    R3,CCHAIN
         ST    R3,FLCCAW    Store in CAW
*
*
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').SIO31M
         SIO   0(R10)
*         TIO   0(R10)
         AGO   .SIO24M
.SIO31M  ANOP
         LR    R1,R10       R1 needs to contain subchannel
         LA    R9,CIRB
         TSCH  0(R9)        Clear pending interrupts
         LA    R10,CORB
         MSCH  0(R10)
         TSCH  0(R9)        Clear pending interrupts
         SSCH  0(R10)
.SIO24M  ANOP
*
*
         LPSW  CWAITNER     Wait for an interrupt
         DC    H'0'
CCONT    DS    0H           Interrupt will automatically come here
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').SIO31N
         SH    R7,FLCCSW+6  Subtract residual count to get bytes read
         LR    R15,R7
* After a successful CCW chain, CSW should be pointing to end
         CLC   FLCCSW(4),=A(CFINCHN)
         BE    CALLFIN
         AGO   .SIO24N
.SIO31N  ANOP
         TSCH  0(R9)
         SH    R7,10(R9)
         LR    R15,R7
         CLC   4(4,R9),=A(CFINCHN)
         BE    CALLFIN
.SIO24N  ANOP
         L     R15,=F'-1'   error return
CALLFIN  DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').NOT390P
         DS    0F
CIRB     DS    24F
CORB     DS    0F
         DC    F'0'
         DC    X'0080FF00'  Logical-Path Mask (enable all?) + format-1
         DC    A(CCHAIN)
         DC    5F'0'
.NOT390P ANOP
*
*
         DS    0D
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').CHN390I
* X'09' = write with automatic carriage return
CCHAIN   CCW   X'09',0,X'20',0    20 = ignore length issues
         AGO   .CHN390J
.CHN390I ANOP
CCHAIN   CCW1  X'09',0,X'20',0    20 = ignore length issues
.CHN390J ANOP
CFINCHN  EQU   *
         DS    0D
CWAITNER DC    X'060E0000'  I/O, machine check, EC, wait, DAT on
         DC    A(AMBIT)     no error
         AIF   ('&XSYS' EQ 'ZARCH').ZNEWIOC
CNEWIO   DC    X'000C0000'  machine check, EC, DAT off
         DC    A(AMBIT+CCONT)  continuation after I/O request
         AGO   .ZNEWIOD
.ZNEWIOC ANOP
CNEWIO   DC    A(X'00040000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(CCONT)  continuation after I/O request
.ZNEWIOD ANOP
*
         DROP  ,
*
*
*
*
*
**********************************************************************
*                                                                    *
*  @@CONSRD - read from console                                      *
*                                                                    *
*  parameter 1 = buffer length                                       *
*  parameter 2 = buffer                                              *
*                                                                    *
**********************************************************************
         ENTRY @@CONSRD
@@CONSRD DS    0H
         SAVE  (14,12),,@@CONSRD
         LR    R12,R15
         USING @@CONSRD,R12
         USING PSA,R0
*
         L     R10,=V(@@CONSDN) Device number
         L     R10,0(R10)
         L     R7,0(R1)        Bytes to read
         L     R2,4(R1)        Buffer to read into
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').CRD390G
         STCM  R2,B'0111',CRDCHN+1   This requires BTL buffer
         STH   R7,CRDCHN+6     Store length in READ CCW
         AGO   .CRD390H
.CRD390G ANOP
         ST    R2,CRDCHN+4
         STH   R7,CRDCHN+2
.CRD390H ANOP
*
* Interrupt needs to point to CRCONT now. Again, I would hope for
* something more sophisticated in PDOS than this continual
* initialization.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZNEWIOG
         MVC   FLCINPSW(8),CRNEWIO
         AGO   .ZNEWIOH
.ZNEWIOG ANOP
         MVC   FLCEINPW(16),CRNEWIO
.ZNEWIOH ANOP
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
* R3 points to CCW chain
         LA    R3,CRDCHN
         ST    R3,FLCCAW    Store in CAW
*
*
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').CRD31M
         SIO   0(R10)
*         TIO   0(R10)
         AGO   .CRD24M
.CRD31M  ANOP
         LR    R1,R10       R1 needs to contain subchannel
         LA    R9,CRIRB
         TSCH  0(R9)        Clear pending interrupts
         LA    R10,CRORB
         MSCH  0(R10)
         TSCH  0(R9)        Clear pending interrupts
         SSCH  0(R10)
.CRD24M  ANOP
*
*
         LPSW  CRWTNER      Wait for an interrupt
         DC    H'0'
CRCONT   DS    0H           Interrupt will automatically come here
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').CRD31N
         SH    R7,FLCCSW+6  Subtract residual count to get bytes read
         LR    R15,R7
* After a successful CCW chain, CSW should be pointing to end
         CLC   FLCCSW(4),=A(CRDFCHN)
         BE    CRALLFIN
         AGO   .CRD24N
.CRD31N  ANOP
         TSCH  0(R9)
         SH    R7,10(R9)
         LR    R15,R7
         CLC   4(4,R9),=A(CRDFCHN)
         BE    CRALLFIN
.CRD24N  ANOP
         L     R15,=F'-1'   error return
CRALLFIN DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').CRD390P
         DS    0F
CRIRB    DS    24F
CRORB    DS    0F
         DC    F'0'
         DC    X'0080FF00'  Logical-Path Mask (enable all?) + format-1
         DC    A(CRDCHN)
         DC    5F'0'
.CRD390P ANOP
*
*
         DS    0D
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').CRD390I
* X'0A' = read inquiry
CRDCHN   CCW   X'0A',0,X'20',0    20 = ignore length issues
         AGO   .CRD390J
.CRD390I ANOP
CRDCHN   CCW1  X'0A',0,X'20',0    20 = ignore length issues
.CRD390J ANOP
CRDFCHN  EQU   *
         DS    0D
         AIF   ('&XSYS' EQ 'ZARCH').ZNEWIOE
CRWTNER  DC    X'060E0000'  I/O, machine check, EC, wait, DAT on
         DC    A(AMBIT)     no error
CRNEWIO  DC    X'000C0000'  machine check, EC, DAT off
         DC    A(AMBIT+CRCONT)  continuation after I/O request
         AGO   .ZNEWIOF
.ZNEWIOE ANOP
CRWTNER  DC    A(X'060E0000'+AM64BIT)
         DC    A(AMBIT)     no error
CRNEWIO  DC    A(X'00040000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(CRCONT)  continuation after I/O request
.ZNEWIOF ANOP
*
         DROP  ,
*
*
*
*
*
**********************************************************************
*                                                                    *
*  @@C3270R - read from 3270 console                                 *
*                                                                    *
*  parameter 1 = buffer length                                       *
*  parameter 2 = buffer                                              *
*                                                                    *
**********************************************************************
         ENTRY @@C3270R
@@C3270R DS    0H
         SAVE  (14,12),,@@C3270R
         LR    R12,R15
         USING @@C3270R,R12
         USING PSA,R0
*
         L     R10,=V(@@CONSDN) Device number
         L     R10,0(R10)
         L     R7,0(R1)        Bytes to read
         L     R2,4(R1)        Buffer to read into
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').C3R390G
         STCM  R2,B'0111',C3RCHN+1   This requires BTL buffer
         STH   R7,C3RCHN+6     Store length in READ CCW
         AGO   .C3R390H
.C3R390G ANOP
         ST    R2,C3RCHN+4
         STH   R7,C3RCHN+2
.C3R390H ANOP
*
* Interrupt needs to point to CUCONT now, for an
* unsolicited interrupt.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZCUA
         MVC   FLCINPSW(8),CUNEWIO
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
         AGO   .ZCUB
.ZCUA    ANOP
         MVC   FLCEINPW(16),CUNEWIO
         STOSM FLCEINPW,X'00'  Work with DAT on or OFF
.ZCUB    ANOP
*
         LPSW  C3RWTNER     Wait for an interrupt
         DC    H'0'
CUCONT   DS    0H           Interrupt will automatically come here
*
* Interrupt needs to point to C3CONT now. Again, I would hope for
* something more sophisticated in PDOS than this continual
* initialization.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZC3A
         MVC   FLCINPSW(8),C3NEWIO
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
         AGO   .ZC3B
.ZC3A    ANOP
         MVC   FLCEINPW(16),C3NEWIO
         STOSM FLCEINPW,X'00'  Work with DAT on or OFF
.ZC3B    ANOP
* R3 points to CCW chain
         LA    R3,C3RCHN
         ST    R3,FLCCAW    Store in CAW
*
*
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').C3R31M
         SIO   0(R10)
*         TIO   0(R10)
         AGO   .C3R24M
.C3R31M  ANOP
         LR    R1,R10       R1 needs to contain subchannel
         LA    R9,C3RIRB
         TSCH  0(R9)        Clear pending interrupts
         LA    R10,C3RORB
         MSCH  0(R10)
         TSCH  0(R9)        Clear pending interrupts
         SSCH  0(R10)
.C3R24M  ANOP
*
*
         LPSW  C3RWTNER     Wait for an interrupt
         DC    H'0'
C3RCONT  DS    0H           Interrupt will automatically come here
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').C3R31N
         SH    R7,FLCCSW+6  Subtract residual count to get bytes read
         LR    R15,R7
* After a successful CCW chain, CSW should be pointing to end
         CLC   FLCCSW(4),=A(C3RFCHN)
         BE    C3RALFIN
         AGO   .C3R24N
.C3R31N  ANOP
         TSCH  0(R9)
         SH    R7,10(R9)
         LR    R15,R7
         CLC   4(4,R9),=A(C3RFCHN)
         BE    C3RALFIN
.C3R24N  ANOP
         L     R15,=F'-1'   error return
C3RALFIN DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').C3R390P
         DS    0F
C3RIRB   DS    24F
C3RORB   DS    0F
         DC    F'0'
         DC    X'0080FF00'  Logical-Path Mask (enable all?) + format-1
         DC    A(C3RCHN)
         DC    5F'0'
.C3R390P ANOP
*
*
         DS    0D
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').C3R390I
* X'06' = read modified
C3RCHN   CCW   X'06',0,X'20',0    20 = ignore length issues
         AGO   .C3R390J
.C3R390I ANOP
C3RCHN   CCW1  X'06',0,X'20',0    20 = ignore length issues
.C3R390J ANOP
C3RFCHN  EQU   *
         DS    0D
         AIF   ('&XSYS' EQ 'ZARCH').ZCCCA
C3RWTNER DC    X'060E0000'  I/O, machine check, EC, wait, DAT on
         DC    A(AMBIT)     no error
C3NEWIO  DC    X'000C0000'  machine check, EC, DAT off
         DC    A(AMBIT+C3RCONT)  continuation after I/O request
CUNEWIO  DC    X'000C0000'  machine check, EC, DAT off
         DC    A(AMBIT+CUCONT)  continuation after I/O request
         AGO   .ZCCCB
.ZCCCA   ANOP
C3RWTNER DC    X'060E0001'  I/O, machine check, EC, wait, DAT on
         DC    A(AMBIT)     no error
C3NEWIO  DC    X'00040001'  machine check, EC, DAT off
         DC    A(AMBIT)
         DC    A(0)
         DC    A(C3RCONT)  continuation after I/O request
CUNEWIO  DC    X'00040001'  machine check, EC, DAT off
         DC    A(AMBIT)
         DC    A(0)
         DC    A(CUCONT)  continuation after I/O request
.ZCCCB   ANOP
*
         DROP  ,
*
*
*
*
**********************************************************************
*                                                                    *
*  RDFBA - read a block from an FBA device                           *
*                                                                    *
*  parameter 1 = device                                              *
*  parameter 2 = blocknr                                             *
*  parameter 3 = buffer                                              *
*  parameter 4 = size of buffer                                      *
*                                                                    *
*  return = length of data read, or -1 on error                      *
*                                                                    *
**********************************************************************
         ENTRY @@RDFBA
@@RDFBA  DS    0H
         SAVE  (14,12),,@@RDFBA
         LR    R12,R15
         USING @@RDFBA,R12
         USING PSA,R0
*
         L     R10,0(R1)    Device number
         L     R2,4(R1)    Blocknr
         ST    R2,FBLKNUM
         L     R2,8(R1)    Buffer
* It is a requirement of using this routine that V=R. If it is
* ever required to support both V and R, then LRA could be used,
* and check for a 0 return, and if so, do a BNZ.
*         LRA   R2,0(R2)     Get real address
         L     R7,12(R1)    Bytes to read
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RFC390B
         STCM  R2,B'0111',RFLDCCW+1   This requires BTL buffer
         STH   R7,RFLDCCW+6  Store in READ CCW
         AGO   .RFC390C
.RFC390B ANOP
         ST    R2,RFLDCCW+4
         STH   R7,RFLDCCW+2
.RFC390C ANOP
*
* Interrupt needs to point to CONT now. Again, I would hope for
* something more sophisticated in PDOS than this continual
* initialization.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZRFNIO
         MVC   FLCINPSW(8),RFNEWIO
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
         AGO .ZRFNIOA
.ZRFNIO  ANOP
         MVC   FLCEINPW(16),RFNEWIO
         STOSM FLCEINPW,X'00'  Work with DAT on or OFF
.ZRFNIOA ANOP
*
* R3 points to CCW chain
         LA    R3,RFBEGCHN
         ST    R3,FLCCAW    Store in CAW
*
*
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RFSIO3B
         SIO   0(R10)
*         TIO   0(R10)
         AGO   .RFSIO2B
.RFSIO3B ANOP
         LR    R1,R10       R1 needs to contain subchannel
         LA    R9,RFIRB
         LA    R10,RFORB
         MSCH  0(R10)       Enable subchannel
         TSCH  0(R9)        Clear pending interrupts
         SSCH  0(R10)
.RFSIO2B ANOP
*
*
         LPSW  RFWTNOER     Wait for an interrupt
         DC    H'0'
RFCONT   DS    0H           Interrupt will automatically come here
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RFSIO3H
         SH    R7,FLCCSW+6  Subtract residual count to get bytes read
         LR    R15,R7
* After a successful CCW chain, CSW should be pointing to end
         CLC   FLCCSW(4),=A(RFFINCHN)
         BE    RFALFINE
         AGO   .RFSIO2H
.RFSIO3H ANOP
         TSCH  0(R9)
         SH    R7,10(R9)
         LR    R15,R7
         CLC   4(4,R9),=A(RFFINCHN)
         BE    RFALFINE
.RFSIO2H ANOP
         L     R15,=F'-1'   error return
RFALFINE DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').RFNOT3B
         DS    0F
RFIRB    DS    24F
RFORB    DS    0F
         DC    F'0'
         DC    X'0080FF00'  Logical-Path Mask (enable all?) + format-1
         DC    A(RFBEGCHN)
         DC    5F'0'
.RFNOT3B ANOP
*
*
RFBEGCHN DS    0D         
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RFC390
* X'63' = Define Extent
RFDFEXT  CCW   X'63',RFDEDAT,X'40',16     40 = chain command
* X'43' = locate
RFLOCATE CCW   X'43',RFLOCDAT,X'40',8    40 = chain command
* X'42' = read data
RFLDCCW  CCW   X'42',0,X'20',32767      20 = ignore length issues
         AGO   .RFC390F
.RFC390  ANOP
* X'63' = Define Extent
RFDFEXT  CCW1   X'63',RFDEDAT,X'40',16     40 = chain command
* X'43' = locate
RFLOCATE CCW1   X'43',RFLOCDAT,X'40',8    40 = chain command
* X'42' = read data
RFLDCCW  CCW1   X'42',0,X'20',32767      20 = ignore length issues
.RFC390F ANOP
RFFINCHN EQU   *
         DS    0H
*
*
         DS    0D
* Define extent data
* 2nd and 3rd byte of 0 is considered default of 512
RFDEDAT  DC    X'00000000'
FBLKNUM  DC    F'0'
         DC    X'00000000'
         DC    X'00000000'
* 6 = read
* 1 = read 1 block. At 0 offset from beginning of extent
RFLOCDAT DC    X'06000001'
         DC    X'00000000'
*
*
*
         DS    0D
* I/O, machine check, EC, wait, DAT on
RFWTNOER DC    A(X'060E0000')
         DC    A(AMBIT)  no error
*
         AIF   ('&XSYS' EQ 'ZARCH').RFZNIO
* machine check, EC, DAT off
RFNEWIO  DC    A(X'000C0000')
         DC    A(AMBIT+RFCONT)  continuation after I/O request
         AGO   .RFNZIOA
*
.RFZNIO  ANOP
RFNEWIO  DC    A(X'00040000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(RFCONT)  continuation after I/O request
.RFNZIOA ANOP
*
         DROP  ,
*
*
*
**********************************************************************
*                                                                    *
*  @@WRFBA - write a block to an FBA disk                            *
*                                                                    *
*  parameter 1 = device                                              *
*  parameter 2 = block number                                        *
*  parameter 3 = buffer                                              *
*  parameter 4 = size of buffer                                      *
*                                                                    *
*  return = length of data written, or -1 on error                   *
*                                                                    *
**********************************************************************
         ENTRY @@WRFBA
@@WRFBA   DS    0H
         SAVE  (14,12),,@@WRFBA
         LR    R12,R15
         USING @@WRFBA,R12
         USING PSA,R0
*
         L     R10,0(R1)    Device number
         L     R2,4(R1)    Block number
         ST    R2,WFBLKNUM
         L     R2,8(R1)    Buffer
* It is a requirement of using this routine that V=R. If it is
* ever required to support both V and R, then LRA could be used,
* and check for a 0 return, and if so, do a BNZ.
*         LRA   R2,0(R2)     Get real address
         L     R7,12(R1)    Bytes to write
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').WFC390B
         STCM  R2,B'0111',WFLDCCW+1   This requires BTL buffer
         STH   R7,WFLDCCW+6  Store in WRITE CCW
         AGO   .WFC390C
.WFC390B ANOP
         ST    R2,WFLDCCW+4
         STH   R7,WFLDCCW+2
.WFC390C ANOP
*
* Interrupt needs to point to CONT now. Again, I would hope for
* something more sophisticated in PDOS than this continual
* initialization.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZWFNIO
         MVC   FLCINPSW(8),WFNEWIO
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
         AGO .ZWFNIOA
.ZWFNIO  ANOP
         MVC   FLCEINPW(16),WFNEWIO
         STOSM FLCEINPW,X'00'  Work with DAT on or OFF
.ZWFNIOA ANOP
*
* R3 points to CCW chain
         LA    R3,WFBEGCHN
         ST    R3,FLCCAW    Store in CAW
*
*
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').WFSIO3B
         SIO   0(R10)
*         TIO   0(R10)
         AGO   .WFSIO2B
.WFSIO3B ANOP
         LR    R1,R10       R1 needs to contain subchannel
         LA    R9,WFIRB
         LA    R10,WFORB
         MSCH  0(R10)       Enable subchannel
         TSCH  0(R9)        Clear pending interrupts
         SSCH  0(R10)
.WFSIO2B ANOP
*
*
         LPSW  WFWTNOER     Wait for an interrupt
         DC    H'0'
WFCONT   DS    0H           Interrupt will automatically come here
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').WFSIO3H
         SH    R7,FLCCSW+6  Subtract residual count to get bytes read
         LR    R15,R7
* After a successful CCW chain, CSW should be pointing to end
         CLC   FLCCSW(4),=A(WFFINCHN)
         BE    WFALFINE
         AGO   .WFSIO2H
.WFSIO3H ANOP
         TSCH  0(R9)
         SH    R7,10(R9)
         LR    R15,R7
         CLC   4(4,R9),=A(WFFINCHN)
         BE    WFALFINE
.WFSIO2H ANOP
         L     R15,=F'-1'   error return
WFALFINE DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').WFNOT3B
         DS    0F
WFIRB    DS    24F
WFORB    DS    0F
         DC    F'0'
         DC    X'0080FF00'  Logical-Path Mask (enable all?) + format-1
         DC    A(WFBEGCHN)
         DC    5F'0'
.WFNOT3B ANOP
*
*
*
WFBEGCHN DS    0D         
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').WFC390
* X'63' = Define Extent
WFDFEXT  CCW   X'63',WFDEDAT,X'40',16     40 = chain command
* X'43' = locate
WFLOCATE CCW   X'43',WFLOCDAT,X'40',8    40 = chain command
* X'41' = write data
WFLDCCW  CCW   X'41',0,X'20',32767      20 = ignore length issues
         AGO   .WFC390F
.WFC390  ANOP
* X'63' = Define Extent
WFDFEXT  CCW1   X'63',WFDEDAT,X'40',16     40 = chain command
* X'43' = locate
WFLOCATE CCW1   X'43',WFLOCDAT,X'40',8    40 = chain command
* X'41' = write data
WFLDCCW  CCW1   X'41',0,X'20',32767      20 = ignore length issues
.WFC390F ANOP
WFFINCHN EQU   *
         DS    0H
*
*
         DS    0D
* Define extent data
* C0 = enable write
* 2nd and 3rd byte of 0 is considered default of 512
WFDEDAT  DC    X'C0000000'
WFBLKNUM DC    F'0'
         DC    X'00000000'
         DC    X'00000000'
* first 1 = write
* second 1 = write 1 block. At 0 offset from beginning of extent
WFLOCDAT DC    X'01000001'
         DC    X'00000000'
*
*
*
         DS    0D
* I/O, machine check, EC, wait, DAT on
WFWTNOER DC    A(X'060E0000')
         DC    A(AMBIT)  no error
*
         AIF   ('&XSYS' EQ 'ZARCH').WFZNIO
* machine check, EC, DAT off
WFNEWIO  DC    A(X'000C0000')
         DC    A(AMBIT+WFCONT)  continuation after I/O request
         AGO   .WFNZIOA
*
.WFZNIO  ANOP
WFNEWIO  DC    A(X'00040000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(WFCONT)  continuation after I/O request
.WFNZIOA ANOP
*
         DROP  ,
*
*
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
         PUSH  USING
         DROP  ,
         ENTRY @@GETCLK
@@GETCLK STM   R2,R5,12(R13)      save a little
         USING @@GETCLK,R15
         LA    R3,32(,R13)        use user's save area
         N     R3,=X'FFFFFFF8'      on a double word boundary
         STCK  0(R3)              stash the clock
         L     R2,0(,R1)          address may be ATL
         MVC   0(8,R2),0(R3)      copy to BTL or ATL
         L     R4,0(,R2)
         L     R5,4(,R2)
         SRDL  R4,12
         SL    R4,=X'0007D910'
         D     R4,=F'1000000'
         SL    R5,=F'1220'
         LR    R15,R5             return result
         LM    R2,R5,12(R13)      restore modified registers
         BR    R14                  return to caller
*
         LTORG ,
         POP   USING
         SPACE 2
*
*
*
         CVT   DSECT=YES
         IKJTCB
         IEZJSCB
         IHAPSA
         IHARB
         IHACDE
         END   ,
