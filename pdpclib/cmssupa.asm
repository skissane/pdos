**********************************************************************
*                                                                    *
*  THIS PROGRAM WRITTEN BY PAUL EDWARDS.                             *
*  RELEASED TO THE PUBLIC DOMAIN                                     *
*                                                                    *
**********************************************************************
*
* MODS BY DAVE WADE
*
*
*   1 - CHANGE REQUEST TYPE FROM RU TO R ON ALL GETMAIN/FREEMAIN
*
*   2 - REMOVE IEFJFCB AND REPLACE WITH DS 176
*
*   3 - ADD SVC 202 ROUTINE TO ALLOW CMS FUNCTIONS TO BE CALLED
*
**********************************************************************
*                                                                    *
*  CMSSUPA - SUPPORT ROUTINES FOR PDPCLIB UNDER CMS                  *
*                                                                    *
*  Note that it is necessary for these routines to stay below the    *
*  line because they are using macros like "GET" that are documented *
*  in z/VM as needing to be executed in AMODE 24. In order to lift   *
*  this 24-bit restriction it will be necessary to switch to         *
*  native CMS macros.                                                *
*                                                                    *
**********************************************************************
*
*
*
         MACRO ,
&NM      GAMOS
         GBLC  &ZSYS
.*
.*   GAMOS sets addressing mode to 24 for
.*   S380 ready for calling OS functions
.*
         AIF ('&ZSYS' NE 'S380').NOT380
         CALL  @@SETM24
.NOT380  ANOP
.MEND    MEND  ,
*
*
*
         MACRO ,
&NM      GAMAPP
         GBLC  &ZSYS
.*
.*   GAMAPP sets addressing mode to 31 for S380 which the
.*   APP was running in before
.*
         AIF ('&ZSYS' NE 'S380').NOT380
         CALL  @@SETM31
.NOT380  ANOP
.MEND    MEND  ,
*
*
*
         COPY  PDPTOP
         PRINT GEN
*
* YREGS IS NOT AVAILABLE WITH IFOX
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
**********************************************************************
*                                                                    *
*  AOPEN - Open a file                                               *
*                                                                    *
*  Parameters are:                                                   *
*  DDNAME - space-padded, 8 character DDNAME to be opened            *
*  MODE - 0 = READ, 1 = WRITE, 2 = UPDATE (update not supported)     *
*  RECFM - 0 = F, 1 = V, 2 = U. This is an output from this function *
*  LRECL - This function will determine the LRECL                    *
*  BLKSIZE - This function will determine the block size             *
*  ASMBUF - pointer to a 32K area which can be written to (only      *
*    needs to be set in move mode)                                   *
*  MEMBER - *pointer* to space-padded, 8 character member name.      *
*    If pointer is 0 (NULL), no member is requested                  *
*                                                                    *
*  Return value:                                                     *
*  An internal "handle" that allows the assembler routines to        *
*  keep track of what's what when READ etc are subsequently          *
*  called.                                                           *
*                                                                    *
*                                                                    *
*  Note - more documentation for this and other I/O functions can    *
*  be found halfway through the stdio.c file in PDPCLIB.             *
*                                                                    *
**********************************************************************
         ENTRY @@AOPEN
@@AOPEN  DS    0H
         SAVE  (14,12),,@@AOPEN
         LR    R12,R15
         USING @@AOPEN,R12
         LR    R11,R1
         GAMOS
         GETMAIN R,LV=WORKLEN,SP=SUBPOOL
         GAMAPP
         ST    R13,4(R1)
         ST    R1,8(R13)
         LR    R13,R1
         LR    R1,R11
         USING WORKAREA,R13
*
         L     R3,0(R1)         R3 POINTS TO DDNAME
         L     R4,4(R1)         R4 POINTS TO MODE
         L     R4,0(R4)         R4 now has value of mode
* 08(,R1) has RECFM
* Note that R5 is used as a scratch register
         L     R8,12(,R1)         R8 POINTS TO LRECL
* 16(,R1) has BLKSIZE
* 20(,R1) has ASMBUF pointer
*
* Member not used on CMS
*         L     R9,24(,R1)         R9 POINTS TO MEMBER NAME (OF PDS)
*         LA    R9,00(,R9)         Strip off high-order bit or byte
*
         GAMOS
         AIF   ('&ZSYS' EQ 'S390').BELOW
* CAN'T USE "BELOW" ON MVS 3.8
         GETMAIN R,LV=ZDCBLEN,SP=SUBPOOL
         AGO   .CHKBLWE
.BELOW   ANOP
         GETMAIN R,LV=ZDCBLEN,SP=SUBPOOL,LOC=BELOW
.CHKBLWE ANOP
         GAMAPP
         LR    R2,R1
         LR    R0,R2              Load output DCB area address
         LA    R1,ZDCBLEN         Load output length of DCB area
         LR    R5,R11             Preserve parameter list
         LA    R11,0              Pad of X'00' and no input length
         MVCL  R0,R10             Clear DCB area to binary zeroes
         LR    R11,R5             Restore parameter list
* R5 free again
* THIS LINE IS FOR GCC
         LR    R6,R4
* THIS LINE IS FOR IBM C
*         L     R6,0(R4)
         LTR   R6,R6
         BNZ   WRITING
* READING
         USING IHADCB,R2
         MVC   ZDCBAREA(INDCBLN),INDCB
         LA    R10,JFCB
* EXIT TYPE 07 + 80 (END OF LIST INDICATOR)
         ICM   R10,B'1000',=X'87'
         ST    R10,JFCBPTR
         LA    R10,JFCBPTR
         LA    R4,ENDFILE
         ST    R4,DCBEODAD
         ST    R10,DCBEXLST
         MVC   DCBDDNAM,0(R3)
         MVC   OPENMB,OPENMAC
*
         GAMOS
         RDJFCB ((R2),INPUT)
         GAMAPP
*        LTR   R9,R9
* DW * DON'T SUPPORT MEMBER NAME FOR NOW
*        BZ    NOMEM
         B     NOMEM
         USING ZDCBAREA,R2
*        MVC   JFCBELNM,0(R9)
*        OI    JFCBIND1,JFCPDS
* DW * END OF MOD
NOMEM    DS    0H
*         OPEN  ((R2),INPUT),MF=(E,OPENMB),MODE=31,TYPE=J
* CAN'T USE MODE=31 ON MVS 3.8, OR WITH TYPE=J
         GAMOS
         OPEN  ((R2),INPUT),MF=(E,OPENMB),TYPE=J
         GAMAPP
* CMS is missing this flag
*         TM    DCBOFLGS,DCBOFOPN  Did OPEN work?
         TM    DCBOFLGS,OFOPN     Did OPEN work?
         BZ    BADOPEN            OPEN failed
         B     DONEOPEN
WRITING  DS    0H
         USING ZDCBAREA,R2
         MVC   ZDCBAREA(OUTDCBLN),OUTDCB
         LA    R10,JFCB
* EXIT TYPE 07 + 80 (END OF LIST INDICATOR)
         ICM   R10,B'1000',=X'87'
         ST    R10,JFCBPTR
         LA    R10,JFCBPTR
         ST    R10,DCBEXLST
         MVC   DCBDDNAM,0(R3)
         MVC   WOPENMB,WOPENMAC
*
         GAMOS
         RDJFCB ((R2),OUTPUT)
         GAMAPP
*        LTR   R9,R9
* DW * NO MEMBER ON VM/370
*        BZ    WNOMEM
         B     WNOMEM
         USING ZDCBAREA,R2
*        MVC   JFCBELNM,0(R9)
*        OI    JFCBIND1,JFCPDS
* DW * END OF MOD
WNOMEM   DS    0H
*         OPEN  ((R2),OUTPUT),MF=(E,WOPENMB),MODE=31,TYPE=J
* CAN'T USE MODE=31 ON MVS 3.8, OR WITH TYPE=J
         GAMOS
         OPEN  ((R2),OUTPUT),MF=(E,WOPENMB),TYPE=J
         GAMAPP
* CMS is missing this flag
*         TM    DCBOFLGS,DCBOFOPN  Did OPEN work?
         TM    DCBOFLGS,OFOPN  Did OPEN work?
         BZ    BADOPEN            OPEN failed
*
* Handle will be returned in R7
*
         LR    R7,R2
         AIF   ('&OUTM' NE 'M').NMM4
         L     R6,=F'32768'
* Give caller an internal buffer to write to. Below the line!
*
* S/370 can't handle LOC=BELOW
*
         GAMOS
         AIF   ('&ZSYS' NE 'S370').MVT8090 If not S/370 then 380 or 390
         GETMAIN R,LV=(R6),SP=SUBPOOL  No LOC= for S/370
         AGO   .GETOENE
.MVT8090 ANOP  ,                  S/380 or S/390
         GETMAIN R,LV=(R6),SP=SUBPOOL,LOC=BELOW
.GETOENE ANOP
         GAMAPP
         ST    R1,ASMBUF
         L     R5,20(,R11)        R5 points to ASMBUF
         ST    R1,0(R5)           save the pointer
* R5 now free again
*
.NMM4    ANOP
DONEOPEN DS    0H
         LR    R7,R2
         SR    R6,R6
         LH    R6,DCBLRECL
         ST    R6,0(R8)
* DW * VM/370 IS MISSING THESE DEFS
*        TM    DCBRECFM,DCBRECF
         TM    DCBRECFM,RECF
* END
         BNO   VARIABLE
* This looks really whacky, but is correct
* We check for V, in order to split between F and U
* Because U has both F and V
         TM    DCBRECFM,RECV
         BNO   FIXED
         L     R6,=F'2'
         B     DONESET
FIXED    DS    0H
         L     R6,=F'0'
         B     DONESET
VARIABLE DS    0H
         L     R6,=F'1'
DONESET  DS    0H
         L     R5,8(,R11)         Point to RECFM
         ST    R6,0(R5)
* Finished with R5 now
         LR    R15,R7
         B     RETURNOP
BADOPEN  DS    0H
         GAMOS
         FREEMAIN R,LV=ZDCBLEN,A=(R2),SP=SUBPOOL  Free DCB area
         GAMAPP
         L     R15,=F'-1'
         B     RETURNOP           Go return to caller with negative RC
*
ENDFILE  LA    R6,1
         ST    R6,RDEOF
         BR    R14
EOFRLEN  EQU   *-ENDFILE
*
RETURNOP DS    0H
         LR    R1,R13
         L     R13,SAVEAREA+4
         LR    R7,R15
         GAMOS
         FREEMAIN R,LV=WORKLEN,A=(R1),SP=SUBPOOL
         GAMAPP
         LR    R15,R7
         RETURN (14,12),RC=(15)
         LTORG
* OPENMAC  OPEN  (,INPUT),MF=L,MODE=31
* CAN'T USE MODE=31 ON MVS 3.8
OPENMAC  OPEN  (,INPUT),MF=L,TYPE=J
OPENMLN  EQU   *-OPENMAC
* WOPENMAC OPEN  (,OUTPUT),MF=L,MODE=31
* CAN'T USE MODE=31 ON MVS 3.8
WOPENMAC OPEN  (,OUTPUT),MF=L
WOPENMLN EQU   *-WOPENMAC
*INDCB    DCB   MACRF=GL,DSORG=PS,EODAD=ENDFILE,EXLST=JPTR
* LEAVE OUT EODAD AND EXLST, FILLED IN LATER
INDCB    DCB   MACRF=GL,DSORG=PS,EODAD=ENDFILE,EXLST=JPTR
INDCBLN  EQU   *-INDCB
JPTR     DS    F
*
* OUTDCB changes depending on whether we are in LOCATE mode or
* MOVE mode
         AIF   ('&OUTM' NE 'L').NLM1
OUTDCB   DCB   MACRF=PL,DSORG=PS
.NLM1    ANOP
         AIF   ('&OUTM' NE 'M').NMM1
OUTDCB   DCB   MACRF=PM,DSORG=PS
.NMM1    ANOP
OUTDCBLN EQU   *-OUTDCB
*
*
*
**********************************************************************
*                                                                    *
*  AREAD - Read from file                                            *
*                                                                    *
**********************************************************************
         ENTRY @@AREAD
@@AREAD  DS    0H
         SAVE  (14,12),,@@AREAD
         LR    R12,R15
         USING @@AREAD,R12
         LR    R11,R1
*         AIF   ('&ZSYS' NE 'S370').BELOW1
* CAN'T USE "BELOW" ON MVS 3.8
*         GETMAIN R,LV=WORKLEN,SP=SUBPOOL
*         AGO   .NOBEL1
*.BELOW1  ANOP
*         GETMAIN R,LV=WORKLEN,SP=SUBPOOL,LOC=BELOW
*.NOBEL1  ANOP
         L     R2,0(R1)         R2 CONTAINS HANDLE
         USING ZDCBAREA,R2
         LA    R1,SAVEADCB
         ST    R13,4(R1)
         ST    R1,8(R13)
         LR    R13,R1
         LR    R1,R11
         USING WORKAREA,R13
*
*        L     R2,0(R1)         R2 CONTAINS HANDLE
         L     R3,4(R1)         R3 POINTS TO BUF POINTER
         L     R4,8(R1)         R4 point to a length
         LA    R6,0
         ST    R6,RDEOF
         GAMOS
         GET   (R2)
         GAMAPP
         ST    R1,0(R3)
         LH    R5,DCBLRECL
         L     R15,RDEOF
*
RETURNAR DS    0H
         LR    R1,R13
         L     R13,SAVEAREA+4
         LR    R7,R15
*        FREEMAIN R,LV=WORKLEN,A=(R1),SP=SUBPOOL
         ST    R5,0(R4)         Tell caller the length read
         LR    R15,R7
         RETURN (14,12),RC=(15)
*
*
*
**********************************************************************
*                                                                    *
*  AWRITE - Write to file                                            *
*                                                                    *
**********************************************************************
         ENTRY @@AWRITE
@@AWRITE DS    0H
         SAVE  (14,12),,@@AWRITE
         LR    R12,R15
         USING @@AWRITE,R12
         LR    R11,R1
         L     R2,0(R1)
         USING ZDCBAREA,R2
         L     R4,8(R1)         R4 points to length to write
         L     R4,0(R4)         R4 = length to write
         STH   R4,DCBLRECL      store length to write
         L     R3,4(R1)         R3 POINTS TO BUF POINTER
*
         LA    R1,SAVEADCB
         ST    R13,4(R1)
         ST    R1,8(R13)
         LR    R13,R1
         LR    R1,R11
         USING WORKAREA,R13
*
         GAMOS
         AIF   ('&OUTM' NE 'L').NLM2
         PUT   (R2)
.NLM2    ANOP
         AIF   ('&OUTM' NE 'M').NMM2
* In move mode, always use our internal buffer. Ignore passed parm.
         L     R3,ASMBUF
         PUT   (R2),(R3)
.NMM2    ANOP
         GAMAPP
         AIF   ('&OUTM' NE 'L').NLM3
         ST    R1,0(R3)
.NLM3    ANOP
         LA    R15,0
*
RETURNAW DS    0H
         LR    R1,R13
         L     R13,SAVEAREA+4
         LR    R14,R15
*         FREEMAIN R,LV=WORKLEN,A=(R1),SP=SUBPOOL
         LR    R15,R14
         RETURN (14,12),RC=(15)
*
*
*
**********************************************************************
*                                                                    *
*  ACLOSE - Close file                                               *
*                                                                    *
**********************************************************************
         ENTRY @@ACLOSE
@@ACLOSE DS    0H
         SAVE  (14,12),,@@ACLOSE
         LR    R12,R15
         USING @@ACLOSE,R12
         LR    R11,R1
         GAMOS
         AIF   ('&ZSYS' EQ 'S390').BELOW3
* CAN'T USE "BELOW" ON MVS 3.8
         GETMAIN R,LV=WORKLEN,SP=SUBPOOL
         AGO   .NOBEL3
.BELOW3  ANOP
         GETMAIN R,LV=WORKLEN,SP=SUBPOOL,LOC=BELOW
.NOBEL3  ANOP
         GAMAPP
         ST    R13,4(R1)
         ST    R1,8(R13)
         LR    R13,R1
         LR    R1,R11
         USING WORKAREA,R13
*
         L     R2,0(R1)         R2 CONTAINS HANDLE
         USING ZDCBAREA,R2
* If we are doing move mode, free internal assembler buffer
         AIF   ('&OUTM' NE 'M').NMM6
         L     R5,ASMBUF
         LTR   R5,R5
         BZ    NFRCL
         L     R6,=F'32768'
         GAMOS
         FREEMAIN R,LV=(R6),A=(R5),SP=SUBPOOL
         GAMAPP
NFRCL    DS    0H
.NMM6    ANOP
         MVC   CLOSEMB,CLOSEMAC
*         CLOSE ((R2)),MF=(E,CLOSEMB),MODE=31
* CAN'T USE MODE=31 WITH MVS 3.8
         GAMOS
         CLOSE ((R2)),MF=(E,CLOSEMB)
         FREEPOOL ((R2))
         FREEMAIN R,LV=ZDCBLEN,A=(R2),SP=SUBPOOL
         GAMAPP
         LA    R15,0
*
RETURNAC DS    0H
         LR    R1,R13
         L     R13,SAVEAREA+4
         LR    R7,R15
         GAMOS
         FREEMAIN R,LV=WORKLEN,A=(R1),SP=SUBPOOL
         GAMAPP
         LR    R15,R7
         RETURN (14,12),RC=(15)
         LTORG
* CLOSEMAC CLOSE (),MF=L,MODE=31
* CAN'T USE MODE=31 WITH MVS 3.8
CLOSEMAC CLOSE (*-*),MF=L
CLOSEMLN EQU   *-CLOSEMAC
*
*
*
**********************************************************************
*                                                                    *
*  GETAM - get the current AMODE                                     *
*                                                                    *
*  This function returns 24 if we are running in AMODE 24 (or less), *
*  31 if we are running anything between 25-31, and 32 for anything  *
*  32 or above.                                                      *
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
*  they should really be saying is 24, 31 and 32+.                   *
*                                                                    *
**********************************************************************
         ENTRY @@GETAM
@@GETAM  DS    0H
         SAVE  (14,12),,@@GETAM
         LR    R12,R15
         USING @@GETAM,R12
*
         L     R2,=X'FF000000'
         LA    R2,0(,R2)
         CLM   R2,B'1000',=X'00'
         BE    GAIS24
         LTR   R2,R2
         BNM   GAIS31
         LA    R15,32
         B     RETURNGA
GAIS24   DS    0H
         LA    R15,24
         B     RETURNGA
GAIS31   LA    R15,31
*
RETURNGA DS    0H
         RETURN (14,12),RC=(15)
         LTORG ,
         SPACE 2
*
*
*
**********************************************************************
*                                                                    *
*  GETM - GET MEMORY                                                 *
*                                                                    *
**********************************************************************
         ENTRY @@GETM
@@GETM   DS    0H
         SAVE  (14,12),,@@GETM
         LR    R12,R15
         USING @@GETM,R12
*
         L     R2,0(R1)
* THIS LINE IS FOR GCC
         LR    R3,R2
* THIS LINE IS FOR IBM C
*         L     R3,0(R2)
         LR    R4,R3
         A     R3,=F'8'
*
* It would be nice to allocate memory with the default
* LOC=RES. However, due to the fact that we need to be
* in AMODE 24 to use things like "GET", it is necessary
* for this program to reside below the line. As such,
* we need to use LOC=ANY to get ATL memory.
*
         AIF   ('&ZSYS' NE 'S370').ANYCHKY
* CAN'T USE "ANY" ON MVS 3.8
         GETMAIN R,LV=(R3),SP=SUBPOOL
         AGO   .ANYCHKE
.ANYCHKY ANOP
         GETMAIN RU,LV=(R3),SP=SUBPOOL,LOC=ANY
.ANYCHKE ANOP
         GAMAPP
*
* WE STORE THE AMOUNT WE REQUESTED FROM MVS INTO THIS ADDRESS
         ST    R3,0(R1)
* AND JUST BELOW THE VALUE WE RETURN TO THE CALLER, WE SAVE
* THE AMOUNT THEY REQUESTED
         ST    R4,4(R1)
         A     R1,=F'8'
         LR    R15,R1
*
RETURNGM DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
*
**********************************************************************
*                                                                    *
*  FREEM - FREE MEMORY                                               *
*                                                                    *
**********************************************************************
         ENTRY @@FREEM
@@FREEM  DS    0H
         SAVE  (14,12),,@@FREEM
         LR    R12,R15
         USING @@FREEM,R12
*
         L     R2,0(R1)
         S     R2,=F'8'
         L     R3,0(R2)
         GAMOS
         AIF   ('&ZSYS' EQ 'S370').F370
         FREEMAIN RU,LV=(R3),A=(R2),SP=SUBPOOL
         AGO   .FINFREE
.F370    ANOP
* S/370
         FREEMAIN R,LV=(R3),A=(R2),SP=SUBPOOL
.FINFREE ANOP
         GAMAPP
*
RETURNFM DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
*
**********************************************************************
*
*  @@SVC202 - ISSUES AN SVC 202 CALL
*
*  E.G. @@SVC202(PARMS,CODE,ERROR)
*
* WHERE :-
*
*  PARMS IS A POINTER TO AN SVC202 PARAMETER LIST
*
*  CODE IS A CODE TO SAY OF &CONTROL IS ON OR OFF
*
* AND ERROR IS SET TO -1
*
**********************************************************************
         ENTRY @@SVC202
@@SVC202 DS    0H
         SAVE  (14,12),,@@SVC202
         LR    R12,R15
         USING @@SVC202,R12
         LR    R11,R1           NEED TO RESTORE R1 FOR C
         GAMOS
         L     R3,0(R1)         R3 POINTS TO SVC202 PARM LIST
         L     R4,4(R1)         R4 POINTS TO CODE
         L     R5,8(R1)         R5 POINTS TO RETURN CODE
         SR    R6,R6            CLEAR R6
         ST    R6,0(R5)         AND SAVE IN RETURN CODE
         LR    R1,R3
*
         AIF   ('&ZSYS' EQ 'S390').DOCALL
         SVC   202              ISSUE COMMAND
         DC    AL4(SV202ER)     ERROR
         AGO   .FINCALL
.DOCALL  ANOP
         CMSCALL ERROR=SV202ER
.FINCALL ANOP
*
SV202RT  EQU    *
         LR    R7,R15
         GAMAPP
         LR    R15,R7
         LR    R1,R11
         RETURN (14,12),RC=(15)
SV202ER  EQU   *
         L     R3,=F'-1'
         ST    R3,0(R5)
         B     SV202RT
         LTORG
*
*
*
**********************************************************************
*
*  @@ATTN@@ - ISSUES AN SVC 202 CALL TO STACK A LINE
*
*  E.G. @@ATTN@@(LINE,LEN,ORDER)
*
* WHERE :-
*
*  LINE IS A POINTER TO LINE TO BE STACKED
*
*  LEN IS THE NUMBER OF CHARACTERS. (<256)
*
*  ORDER IS POINTER TO EITHER FIFO OR LIFO
*
*
*
*  NOTE! Currently this routine needs to reside BTL. But the AL3
*  that would enforce this has been replaced by A(), so nothing
*  will prevent the linker from creating an RMODE ANY module.
*  Which is fine because this routine is not normally used.
*  If anyone attempts to use this routine, they are obliged to
*  keep it RMODE 24. This is an internal routine so they
*  shouldn't be using it anyway.
*
**********************************************************************
         ENTRY @@ATTN@@
@@ATTN@@ DS    0H
         SAVE  (14,12),,@@ATTN@@
         LR    R12,R15
         USING @@ATTN@@,R12
         LR    R11,R1           NEED TO RESTORE R1 FOR C
         L     R3,0(R1)         R3 POINTS TO LINE TO STACK
         ST    R3,ATTNLN        SAVE IN 202 PLIST
         L     R4,4(R1)         R4 POINTS TO LENGTH OF LINE
         MVC   ATTNLN,3(R4)     FIDDLE
         L     R5,8(R1)         R5 POINTS TO LIFO OR FIFO
         MVC   ATTNOD,0(R5)
         SR    R6,R6            CLEAR R6
*        ST    R6,0(R5)         AND SAVE IN RETURN CODE
         LA    R1,ATTNPL
         GAMOS
         SVC   202              ISSUE COMMAND
         GAMAPP
         DC    AL4(ATTNER)      ERROR
ATTNRT   EQU    *
         LR     R1,R11
         RETURN (14,12),RC=(15)
ATTNER   EQU    *
*        L      R3,=F'-1'
*        ST     R3,0(R5)
         B      ATTNRT
         LTORG
*
ATTNPL   DS   0D
         DC   CL8'ATTN'
ATTNOD   DC   CL4'XXXX'     WHERE ORDER MAY BE LIFO OR FIFO.
*                            FIFO IS THE DEFAULT
* See note above about why this is commented out.
*ATTNLN   DC   AL1(0)         LENGTH OF LINE TO BE STACKED
*ATTNAD   DC   AL3(ATTNAD)    ADDRESS OF LINE TO BE STACKED
ATTNLN   DC   A(0)    LENGTH OF LINE TO BE STACKED (1 byte)
*                     followed by address of line to stack
*                     which must be BTL
*                     and is provided by the caller.
*
*
**********************************************************************
*
*  @@STACKN - RETURNS THE NUMBER OF LINES ON THE CONSOLE STACK
*
*  E.G. @@STACKN(COUNT)
*
* WHERE :-
*
*  COUNT IS A POINTER TO AN INT - NUMBER OF LINES TETURNED
*
*
**********************************************************************
         ENTRY @@STACKN
@@STACKN DS    0H
         SAVE  (14,12),,@@STACKN
         LR    R12,R15
         USING @@STACKN,R12
         USING NUCON,R0
         LR    R11,R1           NEED TO RESTORE R1 FOR C
         L     R3,0(R1)         R3 POINTS TO COUNT
         LH    R2,NUMFINRD      R2 HAS COUNT OF LINES ON STACK
         ST    R2,0(R3)         R2 TO COUNT
         LR    R1,R11
         RETURN (14,12),RC=(15)
         LTORG
*
**********************************************************************
*                                                                    *
*  GETCLCK - GET THE VALUE OF THE MVS CLOCK TIMER AND MOVE IT TO AN  *
*  8-BYTE FIELD.  THIS 8-BYTE FIELD DOES NOT NEED TO BE ALIGNED IN   *
*  ANY PARTICULAR WAY.                                               *
*                                                                    *
*  E.G. CALL 'GETCLCK' USING WS-CLOCK1                               *
*                                                                    *
*  THIS FUNCTION ALSO RETURNS THE NUMBER OF SECONDS SINCE 1970-01-01 *
*  BY USING SOME EMPERICALLY-DERIVED MAGIC NUMBERS                   *
*                                                                    *
**********************************************************************
         ENTRY @@GETCLK
@@GETCLK DS    0H
         SAVE  (14,12),,@@GETCLK
         LR    R12,R15
         USING @@GETCLK,R12
*
         L     R2,0(R1)
         STCK  0(R2)
         L     R4,0(R2)
         L     R5,4(R2)
         SRDL  R4,12
         SL    R4,=X'0007D910'
         D     R4,=F'1000000'
         SL    R5,=F'1220'
         LR    R15,R5
*
RETURNGC DS    0H
         RETURN (14,12),RC=(15)
         LTORG
*
*
*
* Keep this code last because it uses a different base register
*
         DROP  R12
**********************************************************************
*                                                                    *
*  SETJ - SAVE REGISTERS INTO ENV                                    *
*                                                                    *
**********************************************************************
         ENTRY @@SETJ
         USING @@SETJ,R15
@@SETJ   L     R15,0(R1)        get the env variable
         STM   R0,R14,0(R15)    save registers to be restored
         LA    R15,0            setjmp needs to return 0
         BR    R14              return to caller
         LTORG ,
*
*
*
**********************************************************************
*                                                                    *
*  LONGJ - RESTORE REGISTERS FROM ENV                                *
*                                                                    *
**********************************************************************
         ENTRY @@LONGJ
         USING @@LONGJ,R15
@@LONGJ  L     R2,0(R1)         get the env variable
         L     R15,60(R2)       get the return code
         LM    R0,R14,0(R2)     restore registers
         BR    R14              return to caller
         LTORG ,
*
*
*
* S/370 doesn't support switching modes so this code is useless,
* and won't compile anyway because "BSM" is not known.
*
         AIF   ('&ZSYS' EQ 'S370').NOMODE2 If S/370 can't switch mode
**********************************************************************
*                                                                    *
*  SETM24 - Set AMODE to 24                                          *
*                                                                    *
**********************************************************************
         ENTRY @@SETM24
         USING @@SETM24,R15
@@SETM24 ICM   R14,8,=X'00'       Sure hope caller is below the line
         BSM   0,R14              Return in amode 24
*
**********************************************************************
*                                                                    *
*  SETM31 - Set AMODE to 31                                          *
*                                                                    *
**********************************************************************
         ENTRY @@SETM31
         USING @@SETM31,R15
@@SETM31 DS    0H
         ICM   R14,8,=X'80'       Set to switch mode
*         LA    R14,0(,R14)        Clear junk from high byte
*         O     R14,=X'00000001'
         BSM   0,R14              Return in amode 31
         LTORG ,
*
.NOMODE2 ANOP  ,                  S/370 doesn't support MODE switching
*
*
*
WORKAREA DSECT
SAVEAREA DS    18F
WORKLEN  EQU   *-WORKAREA
         DCBD  DSORG=PS
         ORG   IHADCB
ZDCBAREA DS    0H
         DS    CL(INDCBLN)
         ORG   IHADCB
         DS    CL(OUTDCBLN)
         DS    0H
EOFR24   DS    CL(EOFRLEN)
JFCBPTR  DS    F
JFCB     DS    0F
*        IEFJFCBN
* z/VM manual says to use 176 characters
         DS    CL176
SAVEADCB DS    18F                Register save area for PUT
         DS    0F
CLOSEMB  DS    CL(CLOSEMLN)
         DS    0F
OPENMB   DS    CL(OPENMLN)
         DS    0F
WOPENMB  DS    CL(WOPENMLN)
RDEOF    DS    1F
ASMBUF   DS    A                  Pointer to an area for PUTting data
*
ZDCBLEN  EQU   *-ZDCBAREA
RECF     EQU   X'80'                   FIXED RECORD FORMAT
RECV     EQU   X'40'                   VARYING RECORD FORMAT
RECU     EQU   X'C0'                   UNDEFINED RECORD FORMAT
RECUV    EQU   X'40'                   U OR V RECORD FORMAT
RECUF    EQU   X'80'                   U OR F RECORD FORMAT
OFOPN    EQU   X'10'                   OPEN SUCCESSFUL
         NUCON
         END
