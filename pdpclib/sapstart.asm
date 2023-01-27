SAPSTART TITLE 'S A P S T A R T  ***  STARTUP ROUTINE FOR C'
***********************************************************************
*                                                                     *
*  THIS PROGRAM WRITTEN BY PAUL EDWARDS.                              *
*  RELEASED TO THE PUBLIC DOMAIN                                      *
*                                                                     *
***********************************************************************
***********************************************************************
*                                                                     *
*  SAPSTART - startup routines for standalone programs                *
*  It is currently coded to work with GCC. To activate the IBM C      *
*  version change the "&COMP" switch.                                 *
*                                                                     *
*  These routines are designed to work in conjunction with the        *
*  Hercules/380 dasdload, which will create a disk with this program  *
*  on cylinder 0, head 1, but only read the first record into low     *
*  memory.  This startup code is required to read the remaining       *
*  blocks, and looks for approximately 1 MB of them.                  *
*                                                                     *
***********************************************************************
*
         COPY  PDPTOP
*
         PRINT GEN
         YREGS
***********************************************************************
*                                                                     *
*  Equates                                                            *
*                                                                     *
***********************************************************************
STACKLOC EQU   X'080000'    The stack starts here (0.5 MiB)
HEAPLOC  EQU   X'100000'    Where malloc etc come from (1 MiB)
CHUNKSZ  EQU   18452        The executable is split into blocks
CCHUNKSZ EQU   18432        Card data is a multiple of 72
MAXBLKS  EQU   40           Maximum number of blocks to read
         AIF ('&XSYS' EQ 'ZARCH').ZVAR64
CODESTRT EQU   1024         Start of our real code
ENTSTRT  EQU   2048         Create a predictable usable entry point
         AGO .ZVAR64B
.ZVAR64  ANOP
CODESTRT EQU   8192         Start of our real code
ENTSTRT  EQU   8192+1024    Create a predictable usable entry point
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
*
* This program will be loaded by the IPL sequence to location 0
* in memory. As such, we need to zero out the lower 512 bytes of
* memory which the hardware will use. Except for the first 8
* bytes, where we need to specify the new PSW.
*
ORIGIN   DS    0D
         DC    X'000C0000' EC mode '08' + Machine Check '04' enabled
         DC    A(AMBIT+POSTIPL) First bit of "normal" memory
*
* Memory to be cleared.
*
         DC    (CODESTRT-*+ORIGIN)X'00'
*
POSTPSA  DS    0H
*
* Tape handler entry point for IPLing from tape
         DC    A(TAPEPIPL)
* Card handler entry point for IPLing from cards
         DC    A(CARDPIPL)
CMAXBLKS DC    F'0'   max number of cards to read
*
* Start of our own, somewhat normal, code. Registers are not
* defined at this point, so we need to create our own base
* register.
*
POSTIPL  DS    0H
         USING PSA,R0
         STM    R0,R15,FLCGRSAV
         BALR  R12,0
POSTIPL2 DS    0H
         LA    R12,0(R12)
*         BCTR  R12,0
*         BCTR  R12,0
         USING POSTIPL2,R12
         S     R12,=A(POSTIPL2-POSTPSA)
         DROP  R12
         USING POSTPSA,R12
         LA    R13,COMMSAVE
*
* At this point, since it is post-IPL, all further interrupts
* will occur to one of 4 locations (instead of location 0, the
* IPL newpsw). Although we are only expecting, and only need,
* the I/O interrupts, we set "dummy" values for the others in
* case something unexpected happens, to give us some visibility
* into the problem.
*
         AIF ('&XSYS' EQ 'ZARCH').ZSW64
         MVC   FLCINPSW(8),WAITER4
         MVC   FLCMNPSW(8),WAITER1
         MVC   FLCSNPSW(8),WAITER2
         MVC   FLCPNPSW(8),WAITER3
         AGO .ZSW64B
.ZSW64   ANOP
*
* Activate z/Arch (code taken from UDOS)
*
         LA    R1,1   Magic number for z/Arch
         SIGP  R1,0,18   Ignore any error from this
* Adding 1 will activate AM64
         LA    R2,POSTIPLZ+1
*         LA    R2,POSTIPLZ-POSTIPL+1(R12)
         AIF ('&ZAM64' NE 'YES').NOBSM
         BSM   R0,R2
.NOBSM   ANOP
POSTIPLZ DS    0H
         MVC   FLCEINPW(16),WAITER4
         MVC   FLCEMNPW(16),WAITER1
         MVC   FLCESNPW(16),WAITER2
         MVC   FLCEPNPW(16),WAITER3
.ZSW64B  ANOP
* Save IPL address in R10
         SLR   R10,R10
         ICM   R10,B'1111',FLCIOA
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').SIO31A
         LCTL  6,6,ALLIOINT CR6 needs to enable all interrupts
.SIO31A  ANOP
         B     STAGE2
         LTORG
*
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
WAITER1  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000111')  error 111
WAITER2  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000222')  error 222
WAITER3  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000333')  error 333
WAITER4  DC    X'000E0000'  machine check, EC, wait
         DC    A(AMBIT+X'00000444')  error 444
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
WAITER4  DC    A(X'00060000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(X'00000444')  error 444
.WAIT64B ANOP
         DS    0D
*
COMMSAVE DC    18F'0'
*
STAGE2   DS    0H
         LA    R1,PARMLST2
         LA    R13,SAVEAR2
         ST    R10,READDEV
         LA    R8,0
         ST    R8,READCYL
         L     R8,=A(CHUNKSZ)
         ST    R8,READSIZE
         LA    R4,1         R4 = Number of blocks read so far
         L     R5,=A(CHUNKSZ) Current address
         LA    R6,1         R6 = head
         LA    R7,2         R7 = record
STAGE2B  DS    0H
         ST    R5,READBUF
         ST    R6,READHEAD
         ST    R7,READREC
         L     R15,=A(RDBLOCK)
         BALR  R14,R15
         LTR   R15,R15
         BM    FIRSTERR
         B     DOTESTS
FIRSTERR DS    0H
         LA    R7,1
         LA    R6,1(R6)
         ST    R6,READHEAD
         ST    R7,READREC
         L     R15,=A(RDBLOCK)
         BALR  R14,R15
DOTESTS  DS    0H
         LTR   R15,R15
         BZ    STAGE3
         BM    STAGE3
*
         A     R5,=A(CHUNKSZ)
         LA    R7,1(R7)
         LA    R4,1(R4)
* Ideally we want to read up until we have a short block, or
* an I/O error, but it's simpler to just force-read up to a
* set maximum.
         C     R4,=A(MAXBLKS)  R4=Maximum blocks to read
         BH    STAGE3
         B     STAGE2B
STAGE3   DS    0H
* Go back to the original state, with I/O disabled, so that we
* don't get any more noise unless explicitly requested
         LPSW  ST4PSW
PARMLST2 DS    0F
READDEV  DS    F
READCYL  DS    F
READHEAD DS    F
READREC  DS    F
READBUF  DS    A
READSIZE DS    F
READCC   DC    A(X'0E')  key + data command code
SAVEAR2  DS    18F
         DS    0D
         AIF   ('&XSYS' EQ 'ZARCH').ZST4
ST4PSW   DC    A(X'000C0000')
         DC    A(AMBIT+STAGE4)
         AGO   .ZST4A
*
.ZST4    ANOP
ST4PSW   DC    A(X'000C0000'+AM64BIT)
         DC    A(AMBIT+STAGE4)
.ZST4A   ANOP
*
WAITSERR DC    X'000E0000'  EC mode + Machine Check enabled + wait
         DC    A(AMBIT+X'00000444')  Severe error
*
* When IPLing from tape, we will use this routine instead
TAPEPIPL DS    0H
         BALR  R12,0
         LA    R12,0(R12)
         BCTR  R12,0
         BCTR  R12,0
         DROP  R12
         USING TAPEPIPL,R12
         S     R12,=A(TAPEPIPL-POSTPSA)
         DROP  R12
         USING POSTPSA,R12
         LA    R13,COMMSAVE
         USING PSA,R0
*         LA    R6,6
*TTLOOP   B     TTLOOP
*
*
*
* this code was copied from above, and should instead be put
* into a subroutine.
*
*
* At this point, since it is post-IPL, all further interrupts
* will occur to one of 4 locations (instead of location 0, the
* IPL newpsw). Although we are only expecting, and only need,
* the I/O interrupts, we set "dummy" values for the others in
* case something unexpected happens, to give us some visibility
* into the problem.
*
         AIF ('&XSYS' EQ 'ZARCH').TSW64
         MVC   FLCINPSW(8),WAITER4
         MVC   FLCMNPSW(8),WAITER1
         MVC   FLCSNPSW(8),WAITER2
         MVC   FLCPNPSW(8),WAITER3
         AGO .TSW64B
.TSW64   ANOP
*
* Activate z/Arch (code taken from UDOS)
*
         LA    R1,1   Magic number for z/Arch
         SIGP  R1,0,18   Ignore any error from this
* Adding 1 will activate AM64
         LA    R2,TPIPLZ+1
         AIF ('&ZAM64' NE 'YES').TNOBSM
         BSM   R0,R2
.TNOBSM  ANOP
TPIPLZ   DS    0H
         MVC   FLCEINPW(16),WAITER4
         MVC   FLCEMNPW(16),WAITER1
         MVC   FLCESNPW(16),WAITER2
         MVC   FLCEPNPW(16),WAITER3
.TSW64B  ANOP
* Save IPL address in R10
         SLR   R10,R10
         ICM   R10,B'1111',FLCIOA
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').TSIO31A
         LCTL  6,6,ALLIOINT CR6 needs to enable all interrupts
.TSIO31A ANOP
         B     TSTAGE2
         LTORG
*
*
* end of copied code
*
*
TSTAGE2  DS    0H
         LA    R1,TPARMLST
         LA    R13,SAVEAR2
         ST    R10,TRDDEV
         L     R8,=A(CHUNKSZ)
         ST    R8,TRDSIZE
         LA    R4,1         R4 = Number of blocks read so far
         L     R5,=A(CHUNKSZ) Current address
TSTAGE2B DS    0H
         ST    R5,TRDBUF
         L     R15,=A(RDTAPE)
         BALR  R14,R15
         LTR   R15,R15
         BM    TFRSTERR
         B     TDOTESTS
TFRSTERR DS    0H
         L     R15,=A(RDTAPE)
         BALR  R14,R15
TDOTESTS DS    0H
         LTR   R15,R15
         BZ    TSTAGE3
         BM    TSTAGE3
*
         A     R5,=A(CHUNKSZ)
         LA    R4,1(R4)
* We want to read up until we have a short block, or
* an I/O error.
         C     R15,=F'18452'
         BNE   TSTAGE3
*         C     R4,=A(MAXBLKS)  R4=Maximum blocks to read
*         BH    TSTAGE3
         B     TSTAGE2B
TSTAGE3  DS    0H
* Go back to the original state, with I/O disabled, so that we
* don't get any more noise unless explicitly requested
*FFLOOP   B     FFLOOP
         LA    R4,1
         ST    R4,@@ISTAPE
         LPSW  ST4PSW
TPARMLST DS    0F
TRDDEV   DS    F
TRDBUF   DS    A
TRDSIZE  DS    F
         LTORG
*
*
*
*
* When IPLing from card, we will use this routine instead
CARDPIPL DS    0H
         BALR  R12,0
         LA    R12,0(R12)
         BCTR  R12,0
         BCTR  R12,0
         DROP  R12
         USING CARDPIPL,R12
         S     R12,=A(CARDPIPL-POSTPSA)
         DROP  R12
         USING POSTPSA,R12
         LA    R13,COMMSAVE
         USING PSA,R0
*         LA    R6,6
*TTLOOP   B     TTLOOP
*
*
*
* this code was copied from above, and should instead be put
* into a subroutine.
*
*
* At this point, since it is post-IPL, all further interrupts
* will occur to one of 4 locations (instead of location 0, the
* IPL newpsw). Although we are only expecting, and only need,
* the I/O interrupts, we set "dummy" values for the others in
* case something unexpected happens, to give us some visibility
* into the problem.
*
         AIF ('&XSYS' EQ 'ZARCH').CSW64
         MVC   FLCINPSW(8),WAITER4
         MVC   FLCMNPSW(8),WAITER1
         MVC   FLCSNPSW(8),WAITER2
         MVC   FLCPNPSW(8),WAITER3
         AGO .CSW64B
.CSW64   ANOP
*
* Activate z/Arch (code taken from UDOS)
*
         LA    R1,1   Magic number for z/Arch
         SIGP  R1,0,18   Ignore any error from this
* Adding 1 will activate AM64
         LA    R2,CPIPLZ+1
         AIF ('&ZAM64' NE 'YES').CNOBSM
         BSM   R0,R2
.CNOBSM  ANOP
CPIPLZ   DS    0H
         MVC   FLCEINPW(16),WAITER4
         MVC   FLCEMNPW(16),WAITER1
         MVC   FLCESNPW(16),WAITER2
         MVC   FLCEPNPW(16),WAITER3
.CSW64B  ANOP
* Save IPL address in R10
         SLR   R10,R10
         ICM   R10,B'1111',FLCIOA
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').CSIO31A
         LCTL  6,6,ALLIOINT CR6 needs to enable all interrupts
.CSIO31A ANOP
         B     CSTAGE2
         LTORG
*
*
* end of copied code
*
*
* We share the tape read routine and parameters
CSTAGE2  DS    0H
         LA    R1,TPARMLST
         LA    R13,SAVEAR2
         ST    R10,TRDDEV
         L     R8,=F'80'
         ST    R8,TRDSIZE
         LA    R4,0         R4 = Number of blocks read so far
         L     R5,=A(CCHUNKSZ) Current address
CSTAGE2B DS    0H
         ST    R5,TRDBUF
         L     R15,=A(RDTAPE)
         BALR  R14,R15
         LTR   R15,R15
         BM    CFRSTERR
         B     CDOTESTS
CFRSTERR DS    0H
         L     R15,=A(RDTAPE)
         BALR  R14,R15
CDOTESTS DS    0H
         LTR   R15,R15
         BZ    CSTAGE3
         BM    CSTAGE3
*
         A     R5,=F'72'    we ignore the sequence numbers
         LA    R4,1(R4)
* We want to read up until we have a short block, or
* an I/O error.
         C     R15,=F'80'
         BNE   CSTAGE3
         C     R4,CMAXBLKS  R4=Maximum blocks to read
         BNL   CSTAGE3
         B     CSTAGE2B
CSTAGE3  DS    0H
* Go back to the original state, with I/O disabled, so that we
* don't get any more noise unless explicitly requested
*FFLOOP   B     FFLOOP
         LA    R4,1
         ST    R4,@@ISCARD
         LPSW  ST4PSW
         LTORG
*
*
*
* At this point, we are in a "normal" post-IPL status,
* with our bootloader loaded, and interrupts disabled,
* and low memory should be considered to be in an
* "unknown" state. We will however pass a parameter
* block to the startup routine, with various bits of information
* for it to interpret.
STAGE4   DS    0H
* Since our program is less than 0.5 MB, set the stack at
* location 0.5 MB. Note that the other thing to worry about
* is the heap, which is set here, and returned in the sapsupa 
* GETM routine.
         L     R13,=A(STACKLOC)  Stack location
         LA    R2,0
         ST    R2,4(R13)         backchain to nowhere
         LR    R2,R13
         A     R2,=F'120'        Get past save area etc
         ST    R2,76(R13)        C needs to know where we're up to
*
         LA    R1,PRMPTR         MVS-style parm block (but to struct)
         L     R15,=V(@@CRT0)
         BALR  R14,R15
* If they're dumb enough to return, load an error wait state
         LPSW  WAITSERR
         LTORG
         ENTRY @@ISTAPE
@@ISTAPE DC    F'0'
         ENTRY @@ISCARD
@@ISCARD DC    F'0'
PRMPTR   DC    A(SAPBLK)
SAPBLK   DS    0F
SAPDUM   DC    F'0'
SAPLEN   DC    F'4'              Length of following parameters
HPLOC    DC    A(HEAPLOC)        Heap location
         DROP  ,
         DC    C'PDPCLIB!'
*
* This is the "main" entry point for standalone programs.
* Control can reach here via a number of methods. It may have
* been the result of booting from the card reader, with the
* destination being location 0. Or it may have been loaded
* by a stand-alone loader, and the destination is not location 0.
* However, in either case (or other cases, e.g. the startup
* code having to complete a load itself), the invoker of this
* code will have given a somewhat MVS-style parameter list.
* You can rely on R13 being a pointer to a save area, in
* fact, an actual stack. R1 will point to a fullword of 0,
* so that it looks like an empty parameter list, but following
* that, there will also be extra data, starting with a fullword
* which contains the length of that extra data block. R15 will
* be the entry point.
*
* The intention of all this is to allow any arbitrary
* stand alone program to be either loaded by a loader, anywhere
* in memory, or to be directly loadable into location 0. Multiple
* entry points, basically, but a common executable.
*
         DC    (ENTSTRT-*+ORIGIN)X'00'
         ORG   *-12
         DC    C'ZAPCONSL'
* Just before ordinary entry point, create a zappable variable
* to store a device number for a console.
         ENTRY @@CONSDN
@@CONSDN DC    F'0'
         DS    0H
         AIF ('&COMP' NE 'IBMC').NOCEES
         ENTRY CEESTART
CEESTART DS    0H
.NOCEES  ANOP
@@CRT0   PDPPRLG CINDEX=1,FRAME=120,BASER=12,ENTRY=YES
         B     FEN1
         LTORG
FEN1     DS    0H
         DROP  12
         BALR  12,0
         USING *,12
         LR    11,1
*
* Clean base register
         LA    R12,0(R12)
*
         USING STACK,R13
*
         LA    R2,0
         ST    R2,DUMMYPTR       WHO KNOWS WHAT THIS IS USED FOR
         LA    R2,MAINSTK
         ST    R2,THEIRSTK       NEXT AVAILABLE SPOT IN STACK
         LA    R7,ANCHOR
         ST    R14,EXITADDR
         L     R3,=A(MAINLEN)
         AR    R2,R3
         ST    R2,12(R7)         TOP OF STACK POINTER
         LA    R2,0
         ST    R2,116(R7)        ADDR OF MEMORY ALLOCATION ROUTINE
*         ST    R2,ARGPTR
*
         MVC   PGMNAME,=C'SAPLOAD '
*
         ST    R1,ARGPTR         pass the R1 directly on
         L     R1,0(R1)          It's a pointer to a structure
         L     R2,8(R1)          heap is available here
         ST    R2,@@HPLOC        heap location used by GETM
         LA    R2,PGMNAME
         ST    R2,PGMNPTR
*
* FOR GCC WE NEED TO BE ABLE TO RESTORE R13
         LA    R5,SAVEAREA
         ST    R5,SAVER13
*
         CALL  @@START
*
RETURNMS DS    0H
         PDPEPIL
SAVER13  DC    F'0'
         LTORG
*
         ENTRY @@HPLOC
@@HPLOC  DS    A
         DROP  ,
         DS    0H
*         ENTRY CEESG003
*CEESG003 DS    0H
         ENTRY @@EXITA
@@EXITA  DS    0H
* SWITCH BACK TO OUR OLD SAVE AREA
         LR    R10,R15
         USING @@EXITA,R10
         L     R9,0(R1)
         L     R13,=A(SAVER13)
         L     R13,0(R13)
*
         PDPEPIL
         LTORG
*
**********************************************************************
*                                                                    *
*  RDBLOCK - read a block from disk                                  *
*                                                                    *
*  parameter 1 = device                                              *
*  parameter 2 = cylinder                                            *
*  parameter 3 = head                                                *
*  parameter 4 = record                                              *
*  parameter 5 = buffer                                              *
*  parameter 6 = size of buffer                                      *
*  parameter 7 = command code (normally you want E for key+data      *
*                but sometimes you want 6 for just data, or 1E       *
*                for count, key and data                             *
*                                                                    *
*  return = length of data read, or -1 on error                      *
*                                                                    *
**********************************************************************
         ENTRY RDBLOCK
RDBLOCK  DS    0H
         SAVE  (14,12),,RDBLOCK
         LR    R12,R15
         USING RDBLOCK,R12
         USING PSA,R0
*
         L     R10,0(R1)    Device number
         L     R2,4(R1)     Cylinder
         STCM  R2,B'0011',RBCC1
         STCM  R2,B'0011',RBCC2
         L     R2,8(R1)     Head
         STCM  R2,B'0011',RBHH1
         STCM  R2,B'0011',RBHH2
         L     R2,12(R1)    Record
         STC   R2,RBR
         L     R2,24(R1)    Command code
         STC   R2,RBLDCCW
         L     R2,16(R1)    Buffer
* It is a requirement of using this routine that V=R. If it is
* ever required to support both V and R, then LRA could be used,
* and check for a 0 return, and if so, do a BNZ.
*         LRA   R2,0(R2)     Get real address
         L     R7,20(R1)    Bytes to read
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RBC390B
         STCM  R2,B'0111',RBLDCCW+1   This requires BTL buffer
         STH   R7,RBLDCCW+6  Store in READ CCW
         AGO   .RBC390C
.RBC390B ANOP
         ST    R2,RBLDCCW+4
         STH   R7,RBLDCCW+2
.RBC390C ANOP
*
* Interrupt needs to point to CONT now. Again, I would hope for
* something more sophisticated in PDOS than this continual
* initialization.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZMVNIO
         MVC   FLCINPSW(8),RBNEWIO
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
         AGO .ZMVNIOA
.ZMVNIO  ANOP
         MVC   FLCEINPW(16),RBNEWIO
         STOSM FLCEINPW,X'00'  Work with DAT on or OFF
.ZMVNIOA ANOP
*
* R3 points to CCW chain
         LA    R3,RBSEEK
         ST    R3,FLCCAW    Store in CAW
*
*
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RBSIO3B
         SIO   0(R10)
*         TIO   0(R10)
         AGO   .RBSIO2B
.RBSIO3B ANOP
* R1 needs to contain subsystem identification word (aka SSID)
         LR    R1,R10       R10 already has SSID
         LA    R9,RBIRB
         LA    R10,RBORB
* MSCH is supposed to be pointing to a SCHIB, not a
* ORB, but I think this is working anyway because most stuff
* in the SCHIB is ignored.
         MSCH  0(R10)       Enable subchannel
         TSCH  0(R9)        Clear pending interrupts
         SSCH  0(R10)
.RBSIO2B ANOP
*
*
         LPSW  RBWTNOER     Wait for an interrupt
         DC    H'0'
RBCONT   DS    0H           Interrupt will automatically come here
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RBSIO3H
         SH    R7,FLCCSW+6  Subtract residual count to get bytes read
         LR    R15,R7
* After a successful CCW chain, CSW should be pointing to end
         CLC   FLCCSW(4),=A(RBFINCHN)
         BE    RBALFINE
         AGO   .RBSIO2H
.RBSIO3H ANOP
         TSCH  0(R9)
         SH    R7,10(R9)
         LR    R15,R7
         CLC   4(4,R9),=A(RBFINCHN)
         BE    RBALFINE
.RBSIO2H ANOP
         L     R15,=F'-1'   error return
RBALFINE DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').RBNOT3B
         DS    0F
RBIRB    DS    24F
RBORB    DS    0F
         DC    F'0'
         DC    X'0080FF00'  Logical-Path Mask (enable all?) + format-1
         DC    A(RBSEEK)
         DC    5F'0'
.RBNOT3B ANOP
*
*
         DS    0D
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RBC390
RBSEEK   CCW   7,RBBBCCHH,X'40',6       40 = chain command
RBSEARCH CCW   X'31',RBCCHHR,X'40',5    40 = chain command
         CCW   8,RBSEARCH,0,0
* X'E' = read key and data
RBLDCCW  CCW   X'E',0,X'20',32767     20 = ignore length issues
         AGO   .RBC390F
.RBC390  ANOP
RBSEEK   CCW1  7,RBBBCCHH,X'40',6       40 = chain command
RBSEARCH CCW1  X'31',RBCCHHR,X'40',5    40 = chain command
         CCW1  8,RBSEARCH,0,0
* X'E' = read key and data
RBLDCCW  CCW1  X'E',0,X'20',32767     20 = ignore length issues
.RBC390F ANOP
RBFINCHN EQU   *
         DS    0H
RBBBCCHH DC    X'000000000000'
         ORG   *-4
RBCC1    DS    CL2
RBHH1    DS    CL2
RBCCHHR  DC    X'0000000005'
         ORG   *-5
RBCC2    DS    CL2
RBHH2    DS    CL2
RBR      DS    C
         DS    0D
* I/O, machine check, EC, wait, DAT on
RBWTNOER DC    A(X'060E0000')
         DC    A(AMBIT)  no error
*
         AIF   ('&XSYS' EQ 'ZARCH').RBZNIO
* machine check, EC, DAT off
RBNEWIO  DC    A(X'000C0000')
         DC    A(AMBIT+RBCONT)  continuation after I/O request
         AGO   .RBNZIOA
*
.RBZNIO  ANOP
RBNEWIO  DC    A(X'00040000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(RBCONT)  continuation after I/O request
.RBNZIOA ANOP
*
         DROP  ,
*
*
*
**********************************************************************
*                                                                    *
*  RDTAPE - read a block from tape                                   *
*                                                                    *
*  parameter 1 = device                                              *
*  parameter 2 = buffer                                              *
*  parameter 3 = size of buffer                                      *
*                                                                    *
*  return = length of data read, or -1 on error                      *
*                                                                    *
**********************************************************************
         ENTRY RDTAPE
RDTAPE   DS    0H
         SAVE  (14,12),,RDTAPE
         LR    R12,R15
         USING RDTAPE,R12
         USING PSA,R0
*
         L     R10,0(R1)    Device number
         L     R2,4(R1)    Buffer
* It is a requirement of using this routine that V=R. If it is
* ever required to support both V and R, then LRA could be used,
* and check for a 0 return, and if so, do a BNZ.
*         LRA   R2,0(R2)     Get real address
         L     R7,8(R1)    Bytes to read
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RTC390B
         STCM  R2,B'0111',RTLDCCW+1   This requires BTL buffer
         STH   R7,RTLDCCW+6  Store in READ CCW
         AGO   .RTC390C
.RTC390B ANOP
         ST    R2,RTLDCCW+4
         STH   R7,RTLDCCW+2
.RTC390C ANOP
*
* Interrupt needs to point to CONT now. Again, I would hope for
* something more sophisticated in PDOS than this continual
* initialization.
*
         AIF   ('&XSYS' EQ 'ZARCH').ZRTNIO
         MVC   FLCINPSW(8),RTNEWIO
         STOSM FLCINPSW,X'00'  Work with DAT on or OFF
         AGO .ZRTNIOA
.ZRTNIO  ANOP
         MVC   FLCEINPW(16),RTNEWIO
         STOSM FLCEINPW,X'00'  Work with DAT on or OFF
.ZRTNIOA ANOP
*
* R3 points to CCW chain
         LA    R3,RTLDCCW
         ST    R3,FLCCAW    Store in CAW
*
*
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RTSIO3B
         SIO   0(R10)
*         TIO   0(R10)
         AGO   .RTSIO2B
.RTSIO3B ANOP
         LR    R1,R10       R1 needs to contain subchannel
         LA    R9,RTIRB
         LA    R10,RTORB
         MSCH  0(R10)       Enable subchannel
         TSCH  0(R9)        Clear pending interrupts
         SSCH  0(R10)
.RTSIO2B ANOP
*
*
         LPSW  RTWTNOER     Wait for an interrupt
         DC    H'0'
RTCONT   DS    0H           Interrupt will automatically come here
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RTSIO3H
         SH    R7,FLCCSW+6  Subtract residual count to get bytes read
         LR    R15,R7
* After a successful CCW chain, CSW should be pointing to end
         CLC   FLCCSW(4),=A(RTFINCHN)
         BE    RTALFINE
         AGO   .RTSIO2H
.RTSIO3H ANOP
         TSCH  0(R9)
         SH    R7,10(R9)
         LR    R15,R7
         CLC   4(4,R9),=A(RTFINCHN)
         BE    RTALFINE
.RTSIO2H ANOP
         L     R15,=F'-1'   error return
RTALFINE DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
         AIF   ('&XSYS' NE 'S390' AND '&XSYS' NE 'ZARCH').RTNOT3B
         DS    0F
RTIRB    DS    24F
RTORB    DS    0F
         DC    F'0'
         DC    X'0080FF00'  Logical-Path Mask (enable all?) + format-1
         DC    A(RTLDCCW)
         DC    5F'0'
.RTNOT3B ANOP
*
*
         DS    0D
         AIF   ('&XSYS' EQ 'S390' OR '&XSYS' EQ 'ZARCH').RTC390
* X'2' = read data
RTLDCCW  CCW   X'2',0,X'20',32767      20 = ignore length issues
         AGO   .RTC390F
.RTC390  ANOP
* X'2' = read data
RTLDCCW  CCW1  X'2',0,X'20',32767     20 = ignore length issues
.RTC390F ANOP
RTFINCHN EQU   *
         DS    0H
         DS    0D
* I/O, machine check, EC, wait, DAT on
RTWTNOER DC    A(X'060E0000')
         DC    A(AMBIT)  no error
*
         AIF   ('&XSYS' EQ 'ZARCH').RTZNIO
* machine check, EC, DAT off
RTNEWIO  DC    A(X'000C0000')
         DC    A(AMBIT+RTCONT)  continuation after I/O request
         AGO   .RTNZIOA
*
.RTZNIO  ANOP
RTNEWIO  DC    A(X'00040000'+AM64BIT)
         DC    A(AMBIT)
         DC    A(0)
         DC    A(RTCONT)  continuation after I/O request
.RTNZIOA ANOP
*
         DROP  ,
*
*
*
         CVT   DSECT=YES
         IKJTCB
         IEZJSCB
         IHAPSA
         IHARB
         IHACDE
STACK    DSECT
SAVEAREA DS    18F
DUMMYPTR DS    F
THEIRSTK DS    F
PARMLIST DS    0F
ARGPTR   DS    F
PGMNPTR  DS    F
TYPE     DS    F
PGMNAME  DS    CL8
PGMNAMEN DS    C                 NUL BYTE FOR C
ANCHOR   DS    0F
EXITADDR DS    F
         DS    49F         
MAINSTK  DS    65536F
MAINLEN  EQU   *-MAINSTK
STACKLEN EQU   *-STACK
         END
