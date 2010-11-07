***********************************************************************
*
*  This program written by Paul Edwards.
*  Released to the public domain
*
*  Extensively modified by others
*
***********************************************************************
*
*  VSESUPA - Support routines for PDPCLIB under DOS/VSE
*
*  It is currently coded for GCC, but C/370 functionality is
*  still there, it's just not being tested after any change.
*
***********************************************************************
*
* Note that the VBS support may not be properly implemented.
* Note that this code issues WTOs. It should be changed to just
* set a return code an exit gracefully instead. I'm not talking
* about that dummy WTO. But on the subject of that dummy WTO - it
* should be made consistent with the rest of PDPCLIB which doesn't
* use that to set the RMODE/AMODE. It should be consistent one way
* or the other.
*
* Here are some of the errors reported:
*
*  OPEN input failed return code is: -37
*  OPEN output failed return code is: -39
*
* FIND input member return codes are:
* Original, before the return and reason codes had
* negative translations added refer to copyrighted:
* DFSMS Macro Instructions for Data Sets
* RC = 0 Member was found.
* RC = -1024 Member not found.
* RC = -1028 RACF allows PDSE EXECUTE, not PDSE READ.
* RC = -1032 PDSE share not available.
* RC = -1036 PDSE is OPENed output to a different member.
* RC = -2048 Directory I/O error.
* RC = -2052 Out of virtual storage.
* RC = -2056 Invalid DEB or DEB not on TCB or TCBs DEB chain.
* RC = -2060 PDSE I/O error flushing system buffers.
* RC = -2064 Invalid FIND, no DCB address.
*
***********************************************************************
*
         COPY  PDPTOP
*
@@VSESUP CSECT
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
SUBPOOL  EQU   0
*
TABDDN   DSECT
         USING     *,R9
DDN      DS        CL8
POINTER  DS        F
TABLEN   EQU       *-TABDDN
@@VSESUP CSECT
***********************************************************************
*                                                                     *
*  VSEFIL - contributed by Louis Millon                               *
*                                                                     *
*  Allows access to CIL in order to read RECFM=U binary files in a    *
*  PDS-like manner.                                                   *
*                                                                     *
*  CALL      @@VSEFIL,(OPEN,DDN)                                      *
*  CALL      @@VSEFIL,(GET,DDN,RECADDR,RECLEN)                        *
*  CALL      @@VSEFIL,(CLOSE,DDN)                                     *
*                                                                     *
*  "OPEN" etc must be CL8 with that string. DDN is CL8. Other two F   *
*                                                                     *
***********************************************************************
         ENTRY @@VSEFIL
@@VSEFIL EQU   *
         USING     *,R3
         SAVE      (14,12)
         LR        R3,R15
         LR        R10,R1
         B         DEBCODE
MAXFILE  EQU       5                             NUMBER OF FILE
*                                                WHICH MAY BE OPENED AT
*                                                THE SAME TIME
AREA     DC        (TABLEN*MAXFILE)X'00'
         DC        F'-1'                         END OF TABLE
FILENAME DS        CL8
DEBCODE  DS        0H
         L         R15,0(R10)                    FUNCTION
         CLC       =C'GET',0(R15)
         BE        GET
         CLC       =C'OPEN',0(R15)
         BE        OPEN
         CLC       =C'CLOSE',0(R15)
         BE        CLOSE
         RETURN    (14,12),RC=8                  INVALID FUNCTION
OPEN     DS        0H
         L         R15,4(R10)
         MVC       FILENAME,0(R15)               DDNAME
         LA        R9,AREA
         LA        R15,MAXFILE
LOOPOPEN DS        0H
         CLC       DDN,FILENAME
         BE        ALREADY                       THIS FILE IS ALREADY
*                                                OPENED
         LA        R9,TABLEN(R9)
         BCT       R15,LOOPOPEN                  THE FILE IS NOT OPEN
         LA        R9,AREA                       SEEK FOR A VACANT
         LA        R15,MAXFILE                   POSITION IN THE ARRAY
LOOPOPN2 DS        0H
         CLC       DDN,=8X'0'                    POSITION IS FREE?
         BE        OKOPEN                        YES
         LA        R9,TABLEN(R9)
         BCT       R15,LOOPOPEN                  NEXT OCCURENCE
         RETURN    (14,12),RC=12                 ARRAY IS FULL
ALREADY  RETURN    (14,12),RC=8                  FILE ALREADY OPENED
OKOPEN   DS        0H
         LA        R1,FILENAME
         CDLOAD    (1)
         ST        R0,POINTER
         LTR       R15,R15
         BZ        R15OK
         LNR       R15,R15
         RETURN    (14,12),RC=(15)              CDLOAD FAIL
R15OK    EQU       *
         MVC       DDN,FILENAME
         RETURN    (14,12),RC=0
CLOSE    DS        0H
         L         R15,4(R10)
         MVC       FILENAME,0(R15)
         LA        R9,AREA
         LA        R15,MAXFILE
LOOPCLOS DS        0H
         CLC       DDN,FILENAME
         BE        OKCLOSE
         LA        R9,TABLEN(R9)
         BCT       R15,LOOPOPEN
         RETURN    (14,12),RC=8                  DDNAME NOTFND IN ARRAY
OKCLOSE  DS        0H
         LA        R1,FILENAME
         CDDELETE  (1)                           REMOVE PHASE FROM GETV
         XC        DDN,DDN
         XC        POINTER,POINTER
         RETURN    (14,12),RC=0
GET      DS        0H
         LA        R15,FILENAME
         MVC       FILENAME,0(R15)
         LA        R9,AREA
         LA        R15,MAXFILE
LOOPGET  DS        0H
         CLC       DDN,FILENAME
         BE        OKGET
         LA        R9,TABLEN(R9)
         BCT       R15,LOOPOPEN
         RETURN    (14,12),RC=12                 DDNAME NOTFND IN ARRAY
OKGET    DS        0H
         L         R15,POINTER
         CLC       0(4,R15),=F'0'
         BNE       NOEOF
         RETURN    (14,12),RC=8                  EOF
NOEOF    DS        0H
         L         R14,POINTER
         L         R15,12(R10)
         MVC       0(4,R15),0(R14)               LENGTH OF RECORD
         LA        R14,4(R14)                    SKIP RECLEN
         L         R15,08(R10)
         ST        R14,0(R15)                    AADR OF RECORD
         L         R14,POINTER
         AL        R14,0(R14)                    SKIP RECORD
         LA        R14,4(R14)                    AND LENGTH
         ST        R14,POINTER                   NEXT RECORD
         RETURN    (14,12),RC=0
         LTORG
***********************************************************************
*                                                                     *
*  GETTZ - Get the offset from GMT in 1.048576 seconds                *
*                                                                     *
***********************************************************************
         ENTRY @@GETTZ
@@GETTZ  LA    R15,0
         BR    R14
***********************************************************************
*
*  AOPEN - Open a dataset
*
*  Note that under MUSIC, RECFM=F is the only reliable thing. It is
*  possible to use RECFM=V like this:
*  /file myin tape osrecfm(v) lrecl(32756) vol(PCTOMF) old
*  but it is being used outside the normal MVS interface. All this
*  stuff really needs to be rewritten per normal MUSIC coding.
*
*
*  Note - more documentation for this and other I/O functions can
*  be found halfway through the stdio.c file in PDPCLIB.
*
***********************************************************************
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
*                                                                    *
*                                                                    *
*  VSE notes - in the general case of an open of a disk file, the    *
*  OPEN should allocate its storage area (ZDCBAREA - what "handle"   *
*  points to, and then it should copy the DTFSD into part of that    *
*  "DCB area" (it is called that for historical reasons and will     *
*  probably be renamed). The OPEN macro, using register notation,    *
*  points to that area, which will have first been modified to put   *
*  in the DDNAME (DLBL) being opened. This way we only need a        *
*  single DTFSD in the main code, which is reused any number of      *
*  times.                                                            *
*                                                                    *
*  The stdin/stdout/stderr are treated differently - each of those   *
*  has its own DTF, because they are special files (not disks).      *
*  The special files are SYSIPT, SYSLST and SYSLOG respectively.     *
*                                                                    *
*  Note that as of time of writing, only the special files have      *
*  been implemented.                                                 *
*                                                                    *
**********************************************************************
         ENTRY @@AOPEN
@@AOPEN  EQU   *
         SAVE  (14,12),,@@AOPEN
         LR    R12,R15
         USING @@AOPEN,R12
         LR    R11,R1
         L     R0,=A(ZDCBLEN)
         AIF   ('&SYS' EQ 'S390').BELOW
* USE DEFAULT LOC=RES for S/370 and S/380
         GETVIS
         AGO   .CHKBLWE
.BELOW   ANOP
         GETVIS LOC=BELOW
.CHKBLWE ANOP
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
         L     R9,24(,R1)         R9 POINTS TO MEMBER NAME (OF PDS)
         LA    R9,0(,R9)          Strip off high-order bit or byte
*
         LR    R2,R13             Access DCB
         LA    R2,WORKLEN(R2)     Point past save area
         LR    R0,R2              Load output DCB area address
         LA    R1,ZDCBLEN         Load output length of DCB area
         S     R1,=A(WORKLEN)     Adjust for save area
         LR    R5,R11             Preserve parameter list
         LA    R11,0              Pad of X'00' and no input length
         MVCL  R0,R10             Clear DCB area to binary zeroes
         LR    R11,R5             Restore parameter list
* R5 free again
* THIS LINE IS FOR GCC
         LR    R6,R4
* THIS LINE IS FOR C/370
*         L     R6,0(R4)
         LTR   R6,R6
         BNZ   WRITING
* READING
*         USING IHADCB,R2
*         MVC   ZDCBAREA(INDCBLN),INDCB
*         LA    R10,JFCB
* EXIT TYPE 07 + 80 (END OF LIST INDICATOR)
*         ICM   R10,B'1000',=X'87'
*         ST    R10,JFCBPTR
*         LA    R10,JFCBPTR
         LA    R4,ENDFILE
*         ST    R4,DCBEODAD
*         ST    R10,DCBEXLST
*         MVC   DCBDDNAM,0(R3)
*         MVC   OPENMB,OPENMAC
*
*         RDJFCB ((R2),INPUT)
         LTR   R9,R9
         BZ    NOMEM
*         USING ZDCBAREA,R2
*         MVC   JFCBELNM,0(R9)
*         OI    JFCBIND1,JFCPDS
NOMEM    DS    0H
*         OPEN  ((R2),INPUT),MF=(E,OPENMB),MODE=31,TYPE=J
* CAN'T USE MODE=31 ON MVS 3.8, OR WITH TYPE=J
*         OPEN  ((R2),INPUT),MF=(E,OPENMB),TYPE=J
*         TM    DCBOFLGS,DCBOFOPN  Did OPEN work?
*         BZ    BADOPEN            OPEN failed
         CLC   0(8,R3),=C'SYSIN   '
         BNE   NOTSYSI
*
* We use the register notation, because other than the standard
* files, all files will have their data stored in ZDCBAREA, not
* a local variable.
         LA    R6,80   +++ hardcode to 80
         ST    R6,DCBLRECL
         LA    R6,0    +++ hardcode to fixed
         ST    R6,DCBRECFM
         LA    R5,SYSIN
         ST    R5,PTRDTF
         OPEN  (R5)
* R5 is free again
         B     DONEOPEN
*
NOTSYSI  DS    0H
         L     R6,=F'19069'   +++ hardcode to 19069
         ST    R6,DCBLRECL
         LA    R6,2    +++ hardcode to recfm=U
         ST    R6,DCBRECFM
*
* Here we need to choose tape or disk
* There's probably a better way than looking at the name of
* the DD, to see if it starts with "MT", as a convention, 
* but of course it would be better if this was
* transparent to the programmer in the first place!
*
         CLC   0(2,R3),=C'MT'
         BNE   NOTTAP
         LA    R5,MTIN
         ST    R5,PTRDTF
         OPEN  (R5)
         B     DONEOPEN
*
NOTTAP   DS    0H
         LA    R5,SDIN
         ST    R5,PTRDTF
         OPEN  (R5)
         B     DONEOPEN
*
         B     BADOPEN
WRITING  DS    0H
*         USING ZDCBAREA,R2
*         MVC   ZDCBAREA(OUTDCBLN),OUTDCB
*         LA    R10,JFCB
* EXIT TYPE 07 + 80 (END OF LIST INDICATOR)
*         ICM   R10,B'1000',=X'87'
*         ST    R10,JFCBPTR
*         LA    R10,JFCBPTR
*         ST    R10,DCBEXLST
*         MVC   DCBDDNAM,0(R3)
*         MVC   WOPENMB,WOPENMAC
*
*         RDJFCB ((R2),OUTPUT)
*        LTR   R9,R9
         BZ    WNOMEM
*         USING ZDCBAREA,R2
*         MVC   JFCBELNM,0(R9)
*         OI    JFCBIND1,JFCPDS
WNOMEM   DS    0H
*         OPEN  ((R2),OUTPUT),MF=(E,WOPENMB),MODE=31,TYPE=J
* CAN'T USE MODE=31 ON MVS 3.8, OR WITH TYPE=J
*         OPEN  ((R2),OUTPUT),MF=(E,WOPENMB),TYPE=J
*         TM    DCBOFLGS,DCBOFOPN  Did OPEN work?
*         BZ    BADOPEN            OPEN failed
*
         CLC   0(8,R3),=C'SYSPRINT'
         BNE   NOTSYS
*
* We use the register notation, because other than the standard
* files, all files will have their data stored in ZDCBAREA, not
* a local variable.
         LA    R6,80   +++ hardcode to 80
         ST    R6,DCBLRECL
         LA    R6,0    +++ hardcode to fixed
         ST    R6,DCBRECFM
*
         L     R6,DCBLRECL
         LA    R5,SYSPRT
         ST    R5,PTRDTF
         OPEN  (R5)
         B     FINO1
*
*         CNTRL SYSPRT,SP,1
NOTSYS   DS    0H
         CLC   0(8,R3),=C'SYSTERM '
         BNE   NOTSYST
*
* We use the register notation, because other than the standard
* files, all files will have their data stored in ZDCBAREA, not
* a local variable.
         LA    R6,80   +++ hardcode to 80
         ST    R6,DCBLRECL
         LA    R6,0    +++ hardcode to fixed
         ST    R6,DCBRECFM
*
         L     R6,DCBLRECL
         LA    R5,SYSTRM
         ST    R5,PTRDTF
         OPEN  (R5)
         B     FINO1
*
*         CNTRL SYSPRT,SP,1
NOTSYST  DS    0H
* Assume RECFM=U
         LA    R6,80   +++ hardcode to 80
         ST    R6,DCBLRECL
         LA    R6,2    +++ hardcode to undefined
         ST    R6,DCBRECFM
         L     R6,DCBLRECL
         LA    R5,SDOUT
         ST    R5,PTRDTF
         OPEN  (R5)
         B     FINO1
*
FINO1    DS    0H
*         ST    R5,PTRDTF
* R5 is free again
*
* Handle will be returned in R7
*
         LR    R7,R13
         AIF   ('&OUTM' NE 'M').NMM4
         L     R6,=F'32768'
* Give caller an internal buffer to write to. Below the line!
*
* S/370 can't handle LOC=BELOW
*
         AIF   ('&SYS' EQ 'S390').MVT8090  If not 390
         GETVIS LENGTH=(R6)  Use default LOC=RES for S/370 and S/380
         AGO   .GETOENE
.MVT8090 ANOP  ,                  S/390
         GETVIS LENGTH=(R6),LOC=BELOW
.GETOENE ANOP
         ST    R1,ASMBUF
         L     R5,20(,R11)        R5 points to ASMBUF
         ST    R1,0(R5)           save the pointer
* R5 now free again         
*
.NMM4    ANOP
DONEOPEN DS    0H
         LR    R7,R13
*         SR    R6,R6
         L     R6,DCBLRECL
*         L     R6,=F'80'  hardcoded to lrecl=80
         ST    R6,0(R8)
*         TM    DCBRECFM,DCBRECF
*         BNO   VARIABLE
* This looks really whacky, but is correct
* We check for V, in order to split between F and U
* Because U has both F and V
*         TM    DCBRECFM,DCBRECV
*         B     FIXED  +++ hardcoded to recfm=f
         L     R6,DCBRECFM
         B     DONESET
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
         L     R13,SAVEAREA+4
         L     R0,=A(ZDCBLEN)
         FREEVIS
         L     R15,=F'-1'
         RETURN (14,12),RC=(15)
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
         L     R0,=A(ZDCBLEN)
* If all goes according to plan, keep the allocated memory
*         FREEVIS
         LR    R15,R7
         RETURN (14,12),RC=(15)
         LTORG
* OPENMAC  OPEN  (,INPUT),MF=L,MODE=31
* CAN'T USE MODE=31 ON MVS 3.8
*OPENMAC  OPEN  (,INPUT),MF=L,TYPE=J
*OPENMLN  EQU   *-OPENMAC
* WOPENMAC OPEN  (,OUTPUT),MF=L,MODE=31
* CAN'T USE MODE=31 ON MVS 3.8
*WOPENMAC OPEN  (,OUTPUT),MF=L
*WOPENMLN EQU   *-WOPENMAC
*INDCB    DCB   MACRF=GL,DSORG=PS,EODAD=ENDFILE,EXLST=JPTR
* LEAVE OUT EODAD AND EXLST, FILLED IN LATER
*INDCB    DCB   MACRF=GL,DSORG=PS,EODAD=ENDFILE,EXLST=JPTR
*INDCBLN  EQU   *-INDCB
JPTR     DS    F
*
* OUTDCB changes depending on whether we are in LOCATE mode or
* MOVE mode
         AIF   ('&OUTM' NE 'L').NLM1
*OUTDCB   DCB   MACRF=PL,DSORG=PS
.NLM1    ANOP
         AIF   ('&OUTM' NE 'M').NMM1
*OUTDCB   DCB   MACRF=PM,DSORG=PS
.NMM1    ANOP
*OUTDCBLN EQU   *-OUTDCB
*
*
*
**********************************************************************
*                                                                    *
*  AREAD - Read from file                                            *
*                                                                    *
**********************************************************************
         ENTRY @@AREAD
@@AREAD  EQU   *
         SAVE  (14,12),,@@AREAD
         LR    R12,R15
         USING @@AREAD,R12
         LR    R11,R1
         AIF ('&SYS' EQ 'S370').NOMOD1
         CALL  @@SETM24
.NOMOD1  ANOP
*         AIF   ('&SYS' NE 'S370').BELOW1
* CAN'T USE "BELOW" ON MVS 3.8
*         GETMAIN R,LV=WORKLEN,SP=SUBPOOL
*         AGO   .NOBEL1
*.BELOW1  ANOP
*         GETMAIN R,LV=WORKLEN,SP=SUBPOOL,LOC=BELOW
*.NOBEL1  ANOP
         L     R1,0(R1)         R1 CONTAINS HANDLE
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
*         ST    R6,RDEOF         
         LA    R5,INTSTOR         
         ST    R5,0(R3)
         L     R8,DCBLRECL
         L     R7,PTRDTF         
         GET   (R7),(R5)
         LTR   R6,R6
         BNE   GOTEOF
         LA    R15,0             SUCCESS
         B     FINFIL
GOTEOF   DS    0H
         LA    R15,1             FAIL
*         ST    R6,0(R4)
FINFIL   DS    0H
*
*
         ST    R8,0(R4)          store length read
*         L     R5,DCBLRECL
*         L     R15,RDEOF
*
RETURNAR DS    0H
         LR    R1,R13
         L     R13,SAVEAREA+4
         LR    R7,R15
*        FREEVIS LENGTH=WORKLEN
         AIF ('&SYS' EQ 'S370').NOMOD2
         CALL  @@SETM31
.NOMOD2  ANOP
*         ST    R5,0(R4)         Tell caller the length read
         LR    R15,R7
         RETURN (14,12),RC=(15)
         LTORG
MYLINE   DS    CL80
*
*
*
***********************************************************************
*
*  AWRITE - Write to an open dataset
*
***********************************************************************
         ENTRY @@AWRITE
@@AWRITE EQU   *
         SAVE  (14,12),,@@AWRITE
         LR    R12,R15
         USING @@AWRITE,R12
         L     R2,0(,R1)          R2 contains GETMAINed address
         L     R3,4(,R1)          R3 points to the record address
         L     R4,8(,R1)          R4 points to the length
         L     R4,0(,R4)          R4 now has actual length
*         USING ZDCBAREA,R2
*        GETMAIN RU,LV=WORKLEN,SP=SUBPOOL
         LR    R11,R1             SAVE
         LR    R1,R2              R1 IS NOW HANDLE
         ST    R13,4(,R1)
         ST    R1,8(,R13)
         LR    R13,R1
         LR    R1,R11             RESTORE
*        USING WORKAREA,R13
*
         AIF   ('&SYS' NE 'S380').N380WR1
         CALL  @@SETM24
.N380WR1 ANOP
*
*         STCM  R4,B'0011',DCBLRECL
*
         AIF   ('&OUTM' NE 'L').NLM2
*        PUT   (R2)
.NLM2    ANOP
         AIF   ('&OUTM' NE 'M').NMM2
* In move mode, always use our internal buffer. Ignore passed parm.
         L     R3,ASMBUF
*         PUT   (R2),(R3)
         L     R5,PTRDTF
         LR    R8,R4           Length of write expected in R8
         PUT   (R5),(R3)
.NMM2    ANOP
         AIF   ('&OUTM' NE 'L').NLM3
         ST    R1,0(R3)
.NLM3    ANOP
*
         AIF   ('&SYS' NE 'S380').N380WR2
         CALL  @@SETM31
.N380WR2 ANOP
*
*        LR    R1,R13
*        L     R13,SAVEAREA+4
         L     R13,4(R13)
*        FREEVIS LENGTH=WORKLEN
         LA    R15,0
         RETURN (14,12),RC=(15)
*
         LTORG
*
**********************************************************************
*                                                                    *
*  ACLOSE - Close file                                               *
*                                                                    *
**********************************************************************
         ENTRY @@ACLOSE
@@ACLOSE EQU   *
         SAVE  (14,12),,@@ACLOSE
         LR    R12,R15
         USING @@ACLOSE,R12
         LR    R11,R1
         L     R1,0(R1)         R1 CONTAINS HANDLE
         ST    R13,4(R1)
         ST    R1,8(R13)
         LR    R13,R1
         LR    R1,R11
         USING WORKAREA,R13
*
* If we are doing move mode, free internal assembler buffer
         AIF   ('&OUTM' NE 'M').NMM6
         L     R5,ASMBUF
         LTR   R5,R5
         BZ    NFRCL
         L     R6,=F'32768'
         FREEVIS LENGTH=(R6),ADDRESS=(R5)
NFRCL    DS    0H
.NMM6    ANOP
*         MVC   CLOSEMB,CLOSEMAC
*         CLOSE ((R2)),MF=(E,CLOSEMB),MODE=31
* CAN'T USE MODE=31 WITH MVS 3.8
*         CLOSE ((R2)),MF=(E,CLOSEMB)
*         FREEPOOL ((R2))
*         FREEVIS LENGTH=ZDCBLEN,ADDRESS=(R2)
*         CLOSE SYSPRT
         L     R5,PTRDTF
         LTR   R5,R5
         BZ    NOTOP1
* The CLOSE appears to be abending when called in 31-bit mode,
* despite it being an SVC. So we need to switch to 24-bit mode
         AIF   ('&SYS' NE 'S380').N380CL1
         CALL  @@SETM24
.N380CL1 ANOP
         CLOSE (R5)
         AIF   ('&SYS' NE 'S380').N380CL2
         CALL  @@SETM31
.N380CL2 ANOP
NOTOP1   DS    0H
         LA    R15,0
*
RETURNAC DS    0H
         LR    R1,R13
         L     R13,SAVEAREA+4
         LR    R7,R15
         L     R0,=A(ZDCBLEN)
         FREEVIS
         LR    R15,R7
         RETURN (14,12),RC=(15)
         LTORG
* CLOSEMAC CLOSE (),MF=L,MODE=31
* CAN'T USE MODE=31 WITH MVS 3.8
*CLOSEMAC CLOSE (),MF=L
*CLOSEMLN EQU   *-CLOSEMAC
*
*
*
*
* This macro is only for reading from stdin
SYSIN    DTFCD DEVADDR=SYSIPT,IOAREA1=IO1,BLKSIZE=80,RECFORM=FIXUNB,   X
               WORKA=YES,EOFADDR=GOTEOF,MODNAME=CARDMOD
CARDMOD  CDMOD RECFORM=FIXUNB,WORKA=YES             
* These macros are only used for writing to stdout and stderr
SYSPRT   DTFPR CONTROL=YES,BLKSIZE=80,DEVADDR=SYSLST,MODNAME=PRINTMOD, X
               IOAREA1=IO1,RECFORM=FIXUNB,WORKA=YES
SYSTRM   DTFPR CONTROL=YES,BLKSIZE=80,DEVADDR=SYS005,MODNAME=PRINTMOD, X
               IOAREA1=IO1,RECFORM=FIXUNB,WORKA=YES
SDOUT    DTFSD BLKSIZE=80,DEVADDR=SYS010,DEVICE=3350,                  X
               IOAREA1=IO1,RECFORM=UNDEF,WORKA=YES,                    X
               TYPEFLE=OUTPUT,RECSIZE=(8)
PRINTMOD PRMOD CONTROL=YES,RECFORM=FIXUNB,WORKA=YES
*
SDIN     DTFSD BLKSIZE=19069,DEVADDR=SYS010,DEVICE=3350,               X
               IOAREA1=WORKI1,RECFORM=UNDEF,WORKA=YES,                 X
               TYPEFLE=INPUT,RECSIZE=(8),EOFADDR=GOTEOF
MTIN     DTFMT BLKSIZE=19069,DEVADDR=SYS011,MODNAME=MTINMOD,           X
               IOAREA1=WORKI1,RECFORM=UNDEF,WORKA=YES,FILABL=NO,       X
               TYPEFLE=INPUT,RECSIZE=(8),EOFADDR=GOTEOF
MTINMOD  MTMOD WORKA=YES,RECFORM=UNDEF
IO1      DS    CL200
*
***********************************************************************
*
*  GETM - GET MEMORY
*
***********************************************************************
         ENTRY @@GETM
@@GETM   EQU   *
         SAVE  (14,12),,@@GETM
         LR    R12,R15
         USING @@GETM,R12
*
         L     R2,0(,R1)
         AIF ('&COMP' NE 'GCC').GETMC
* THIS LINE IS FOR GCC
         LR    R3,R2
         AGO   .GETMEND
.GETMC   ANOP
* THIS LINE IS FOR C/370
         L     R3,0(,R2)
.GETMEND ANOP
         LR    R4,R3
         LA    R3,8(,R3)
*
* To avoid fragmentation, round up size to 64 byte multiple
*
         A     R3,=A(64-1)
         N     R3,=X'FFFFFFC0'
*
         AIF   ('&SYS' NE 'S380').N380GM1
*         GETMAIN RU,LV=(R3),SP=SUBPOOL,LOC=ANY
*
* When in 380 mode, we need to keep the program below the
* line, but we have the ability to use storage above the
* line, and this is where we get it, with the LOC=ANY parameter.
* For other environments, the default LOC=RES is fine.
*
         GETVIS LENGTH=(R3),LOC=ANY
         AGO   .N380GM2
.N380GM1 ANOP
         GETVIS LENGTH=(R3)
.N380GM2 ANOP
*
         LTR   R15,R15
         BZ    GOODGM
         LA    R15,0
         B     RETURNGM
GOODGM   DS    0H
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
***********************************************************************
*
*  FREEM - FREE MEMORY
*
***********************************************************************
         ENTRY @@FREEM
@@FREEM  EQU   *
         SAVE  (14,12),,@@FREEM
         LR    R12,R15
         USING @@FREEM,R12
*
         L     R2,0(,R1)
         S     R2,=F'8'
         L     R3,0(,R2)
*
         AIF   ('&SYS' NE 'S380').N380FM1
* On S/380, nothing to free - using preallocated memory block
*         FREEMAIN LENGTH=(R3),ADDRESS=(R2)
         AGO   .N380FM2
.N380FM1 ANOP
         FREEVIS LENGTH=(R3),ADDRESS=(R2)
.N380FM2 ANOP
*
RETURNFM DS    0H
         RETURN (14,12),RC=(15)
         LTORG
***********************************************************************
*
*  GETCLCK - GET THE VALUE OF THE MVS CLOCK TIMER AND MOVE IT TO AN
*  8-BYTE FIELD.  THIS 8-BYTE FIELD DOES NOT NEED TO BE ALIGNED IN
*  ANY PARTICULAR WAY.
*
*  E.G. CALL 'GETCLCK' USING WS-CLOCK1
*
*  THIS FUNCTION ALSO RETURNS THE NUMBER OF SECONDS SINCE 1970-01-01
*  BY USING SOME EMPERICALLY-DERIVED MAGIC NUMBERS
*
***********************************************************************
         ENTRY @@GETCLK
@@GETCLK EQU   *
         SAVE  (14,12),,@@GETCLK
         LR    R12,R15
         USING @@GETCLK,R12
*
         L     R2,0(,R1)
         STCK  0(R2)
         L     R4,0(,R2)
         L     R5,4(,R2)
         SRDL  R4,12
         SL    R4,=X'0007D910'
         D     R4,=F'1000000'
         SL    R5,=F'1220'
         LR    R15,R5
*
RETURNGC DS    0H
         RETURN (14,12),RC=(15)
         LTORG
***********************************************************************
*
*  SYSTEM - execute another command
*
***********************************************************************
         ENTRY @@SYSTEM
@@SYSTEM EQU   *
         SAVE  (14,12),,@@SYSTEM
         LR    R12,R15
         USING @@SYSTEM,R12
         LR    R11,R1
*
*         GETVIS LENGTH=SYSTEMLN,SP=SUBPOOL
         ST    R13,4(,R1)
         ST    R1,8(,R13)
         LR    R13,R1
         LR    R1,R11
         USING SYSTMWRK,R13
*
         MVC   CMDPREF,FIXEDPRF
         L     R2,0(R1)
         CL    R2,=F'200'
         BL    LENOK
         L     R2,=F'200'
LENOK    DS    0H
         STH   R2,CMDLEN
         LA    R4,CMDTEXT
         LR    R5,R2
         L     R6,4(R1)
         LR    R7,R2
         MVCL  R4,R6
         LA    R1,CMDPREF
*         SVC   $EXREQ
*
RETURNSY DS    0H
         LR    R1,R13
         L     R13,SYSTMWRK+4
*         FREEMAIN RU,LV=SYSTEMLN,A=(1),SP=SUBPOOL
*
         LA    R15,0
         RETURN (14,12),RC=(15)
* For documentation on this fixed prefix, see SVC 221
* documentation.
FIXEDPRF DC    X'7F01E000000000'
         LTORG
SYSTMWRK DSECT ,             MAP STORAGE
         DS    18A           OUR OS SAVE AREA
CMDPREF  DS    CL8           FIXED PREFIX
CMDLEN   DS    H             LENGTH OF COMMAND
CMDTEXT  DS    CL200         COMMAND ITSELF
SYSTEMLN EQU   *-SYSTMWRK    LENGTH OF DYNAMIC STORAGE
@@VSESUP CSECT ,
***********************************************************************
*
*  IDCAMS - dummy function to keep MVS happy
*
***********************************************************************
         ENTRY @@IDCAMS
@@IDCAMS EQU   *
         SAVE  (14,12),,@@IDCAMS
         LR    R12,R15
         USING @@IDCAMS,R12
*
         LA    R15,0
*
         RETURN (14,12),RC=(15)
         LTORG
*
***********************************************************************
*
*  DYNAL - dummy function to keep MVS happy
*
***********************************************************************
         ENTRY @@DYNAL
@@DYNAL EQU   *
         SAVE  (14,12),,@@DYNAL
         LR    R12,R15
         USING @@DYNAL,R12
*
         LA    R15,0
*
         RETURN (14,12),RC=(15)
         LTORG
*
*
*
* Keep this code last because it uses a different base register
*
         DROP  R12
***********************************************************************
*
*  SETJ - SAVE REGISTERS INTO ENV
*
***********************************************************************
         ENTRY @@SETJ
         USING @@SETJ,R15
@@SETJ   L     R15,0(R1)          get the env variable
         STM   R0,R14,0(R15)      save registers to be restored
         LA    R15,0              setjmp needs to return 0
         BR    R14                return to caller
         LTORG ,
*
***********************************************************************
*
*  LONGJ - RESTORE REGISTERS FROM ENV
*
***********************************************************************
         ENTRY @@LONGJ
         USING @@LONGJ,R15
@@LONGJ  L     R2,0(R1)           get the env variable
         L     R15,60(R2)         get the return code
         LM    R0,R14,0(R2)       restore registers
         BR    R14                return to caller
         LTORG ,
*
* S/370 doesn't support switching modes so this code is useless,
* and won't compile anyway because "BSM" is not known.
*
         AIF   ('&SYS' EQ 'S370').NOMODE  If S/370 we can't switch mode
***********************************************************************
*
*  SETM24 - Set AMODE to 24
*
***********************************************************************
         ENTRY @@SETM24
         USING @@SETM24,R15
@@SETM24 ICM   R14,8,=X'00'       Sure hope caller is below the line
         BSM   0,R14              Return in amode 24
*
***********************************************************************
*
*  SETM31 - Set AMODE to 31
*
***********************************************************************
         ENTRY @@SETM31
         USING @@SETM31,R15
@@SETM31 ICM   R14,8,=X'80'       Set to switch mode
         BSM   0,R14              Return in amode 31
         LTORG ,
*
.NOMODE  ANOP  ,                  S/370 doesn't support MODE switching
*
*         IEZIOB                   Input/Output Block
*
*ZDCBLEN  EQU   =A(TDCBLEN)
*WORKLEN  EQU   =A(TWRKLEN)
*
* Not sure why this needs to be kept way down here.
* But it should be temporary anyway, as the I/O buffers
* should be dynamically allocated for each handle. It looks
* to me like this is used to store an entire track rather
* than a single block, so can't be shared.
WORKI1    DS    CL19069
*
WORKAREA DSECT
SAVEAREA DS    18F
WORKLEN  EQU   *-WORKAREA
*
*         DCBD  DSORG=PS,DEVD=DA   Map Data Control Block
*         ORG   IHADCB             Overlay the DCB DSECT
ZDCBAREA DS    0H
*         DS    CL(INDCBLN)
*         DS    CL(OUTDCBLN)
OPENCLOS DS    F                  OPEN/CLOSE parameter list
         DS    0H
*EOFR24   DS    CL(EOFRLEN)
*         IHADECB DSECT=NO         Data Event Control Block
BLKSIZE  DS    F                  Save area for input DCB BLKSIZE
LRECL    DS    F                  Save area for input DCB LRECL
BUFFADDR DS    F                  Location of the BLOCK Buffer
BUFFEND  DS    F                  Address after end of current block
BUFFCURR DS    F                  Current record in the buffer
VBSADDR  DS    F                  Location of the VBS record build area
VBSEND   DS    F                  Addr. after end VBS record build area
VBSCURR  DS    F                  Location to store next byte
RDRECPTR DS    F                  Where to store record pointer
RDLENPTR DS    F                  Where to store read length
PTRDTF   DS    F                  Pointer to the DTF in use
DCBLRECL DS    F                  Logical record length
DCBRECFM DS    F                  Record format
JFCBPTR  DS    F
JFCB     DS    0F
*         IEFJFCBN LIST=YES        SYS1.AMODGEN JOB File Control Block
* Format 1 Data Set Control Block
DSCB     DS    0F
*         IECSDSL1 (1)             Map the Format 1 DSCB
DSCBCCHH DS    CL5                CCHHR of DSCB returned by OBTAIN
         DS    CL47               Rest of OBTAIN's 148 byte work area
*CLOSEMB  DS    CL(CLOSEMLN)
         DS    0F
*OPENMB   DS    CL(OPENMLN)
         DS    0F
*WOPENMB  DS    CL(WOPENMLN)
RDEOF    DS    1F
ASMBUF   DS    A                  Pointer to an area for PUTing data
MEMBER24 DS    CL8
INTSTOR  DS    CL200              Internal storage for GET
ZDCBLEN  EQU   *-WORKAREA
*
         END
