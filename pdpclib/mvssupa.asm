MVSSUPA  TITLE 'M V S S U P A  ***  MVS VERSION OF PDPCLIB SUPPORT'
***********************************************************************
*                                                                     *
*  This program written by Paul Edwards.                              *
*  Released to the public domain                                      *
*                                                                     *
*  Extensively modified by others                                     *
*                                                                     *
***********************************************************************
*                                                                     *
*  MVSSUPA - Support routines for PDPCLIB under MVS                   *
*    Additional macros in (EDWARDS.)PDPCLIB.MACLIB                    *
*  It is currently coded for GCC, but IBM C functionality is          *
*  still there, it's just not being tested after each change.         *
*                                                                     *
***********************************************************************
*                                                                     *
* Note that some of the functionality in here has not been exercised  *
* to any great extent, since it is dependent on whether the C code    *
* invokes it or not.                                                  *
*                                                                     *
* Note that this code issues WTOs. It should be changed to just       *
* set a return code and exit gracefully instead.                      *
*                                                                     *
***********************************************************************
*   Changes by Gerhard Postpischil:
*     EQU * for entry points deleted (placed labels on SAVE) to avoid
*       0C6 abends when EQU follows a LTORG
*     Fixed 0C4 abend in RECFM=Vxxx processing; fixed PUT length error.
*     Deleted unnecessary and duplicated instructions
*     Added @@SYSTEM and @@DYNAL routines                2008-06-10
*     Added @@IDCAMS non-reentrant, non-refreshable      2008-06-17
*     Modified I/O for BSAM, EXCP, and terminal I/O
*     Added checks to AOPEN to support unlike PDS BLDL   2014-03-03
*     Caller may use @@ADCBA any time to get current DCB attributes.
*     Added support for unlike PDS concatenation; requires member.
*     Fixed problems with sequential unlike concatenation:
*       When next DD is entered, AREAD returns with R15=4, no data.
*       Use @@ADCBA to get attributes for next DD.
*     Largest blocksize will be used for buffer.         2014-07-24
*     The program now supports reading the VTOC of a disk pack;
*     use @@AOPEN, @@ACLOSE, @@AREAD normally for a sequential data
*       set with record length 140 (44 key, 96 data). The DD card:
*       //ddname DD DISP=OLD,DSN=FORMAT4.DSCB,UNIT=SYSDA,
*       //           VOL=SER=serial                      2014-08-01
*
***********************************************************************
*
*   To facilitate cross-assembly (S390 on S370/380 system), some
*   OS/390 & zOS macros replaced. Affected old statements are flagged
*   *COMP*                                               2015-01-06
*
***********************************************************************
*
*
* Internal macros (these replace external macro library)
*
*
*
         MACRO ,
&NM      AMUSE &WRK1=R14,&WRK2=R15
         GBLC  &ZSYS
.*
.*   AMUSE sets addressing mode back to the caller's
.*         Expands nothing or label for S370 or S390
.*         Required after SAM24 call to return data to caller
.*
         AIF   ('&ZSYS' NE 'S380' AND '&ZSYS' NE 'ZARCH').OTHSYS
&NM      L     &WRK1,4(,R13)      Old save area
         L     &WRK1,12(,&WRK1)   Caller's mode in high bit
         N     &WRK1,=X'80000000'   Kill address
         LA    &WRK2,*+4+2+2      Get new mode and address
         OR    &WRK1,&WRK2
         BSM   R0,&WRK1           CONTINUE IN USER MODE
         MEXIT ,
.OTHSYS  AIF   ('&NM' EQ '').MEND
&NM      DS    0H            DEFINE LABEL ONLY
.MEND    MEND  ,
*
*
*
         MACRO ,
&NM      GAMOS
         GBLC  &STEPD
.*
.*   GAMOS sets addressing mode to 24 or 31 or
.*   potentially bypasses the BSM
.*
         AIF   ('&STEPD' NE 'YES').OTHSYS2
&NM      DS    0H
         L     R15,=A(NEEDBF)
         TM    0(R15),NEEDBANY   Need AM switching?
*         TM    NEEDBF,NEEDBANY   Need AM switching?
         BZ    ZZ&SYSNDX.X
         LA    R14,ZZ&SYSNDX.X
         L     R15,=A(NEEDBOO)
         L     R15,0(,R15)
         OR    R14,R15    whatever mode OS requires
*         O     R14,NEEDBOO whatever mode OS requires
         BSM   0,R14
ZZ&SYSNDX.X DS 0H
         MEXIT ,
.OTHSYS2 AIF   ('&NM' EQ '').MEND
&NM      DS    0H            DEFINE LABEL ONLY
.MEND    MEND  ,
*
*
*
         MACRO ,
&NM      GAMAPP
         GBLC  &STEPD
.*
.*   GAMAPP sets addressing mode to 31 or 64 or
.*   potentially bypasses the BSM
.*
         AIF   ('&STEPD' NE 'YES').OTHSYS3
&NM      DS    0H
         L     R15,=A(NEEDBF)
         TM    0(R15),NEEDBANY   Did we previously need a switch?
*         TM    NEEDBF,NEEDBANY   Did we previously need a switch?
         BZ    ZZ&SYSNDX.X       No, no switching required
.* We don't know whether we need to switch to 31 or 64
         LA    R14,ZZ&SYSNDX.X
         L     R15,=A(NEEDBOA)
         L     R15,0(,R15)
         OR    R14,R15      This decides 31/64
*         O     R14,NEEDBOA  This decides 31/64
         BSM   R0,R14
ZZ&SYSNDX.X DS 0H
         MEXIT ,
.OTHSYS3 AIF   ('&NM' EQ '').MEND
&NM      DS    0H            DEFINE LABEL ONLY
.MEND    MEND  ,
*
*
*
         MACRO ,                  FIXED 2010.293
&NM      FUNEXIT &RC=
         GBLC  &ZSYS,&ZZSETSA,&ZZSETSL,&ZZSETSP
         GBLB  &ZZSETAM
         LCLC  &LBL
&LBL     SETC  '&NM'
         AIF   ('&ZZSETSL' NE '' AND '&RC' EQ '').JUSTF
         AIF   ('&ZZSETSA' EQ '').SAMESA
         AIF   ('&ZZSETSL' NE '').SAMESA
&LBL     L     R13,4(,R13)        RESTORE HIGHER SA
&LBL     SETC  ''
.SAMESA  AIF   ('&RC' EQ '').LMALL
         AIF   ('&RC' EQ '(15)' OR '&RC' EQ '(R15)').NORC
         AIF   (K'&RC LT 3).LA
         AIF   ('&RC'(1,1) NE '(' OR '&RC'(2,1) EQ '(').LA
         AIF   ('&RC'(K'&RC,1) NE ')' OR '&RC'(K'&RC-1,1) EQ ')').LA
&LBL     LR    R15,&RC(1)
&LBL     SETC  ''
         AGO   .NORC
.LA      ANOP  ,
&LBL     LA    R15,&RC            SET RETURN CODE
&LBL     SETC  ''
.NORC    AIF   ('&ZZSETSL' EQ '').NOFRM
         LR    R1,R13             SAVE CURRENT SA
         L     R13,4(,R13)        REGAIN CALLER'S SA
         ST    R15,16(,R13)       SAVE RETURN CODE
         FREEMAIN R,A=(1),LV=&ZZSETSL,SP=&ZZSETSP
         AGO   .LMALL             GOTTA LOVE SPAGHETTI CODE
.NOFRM   ANOP  ,
&LBL     L     R14,12(,R13)
         LM    R0,R12,20(R13)
         AGO   .EXMODE
.JUSTF   ANOP  ,
&LBL     LR    R1,R13             SAVE CURRENT SA
&LBL     SETC  ''
         L     R13,4(,R13)        REGAIN CALLER'S SA
         FREEMAIN R,A=(1),LV=&ZZSETSL,SP=&ZZSETSP
.LMALL   ANOP  ,
&LBL     LM    R14,R12,12(R13)    RELOAD ALL
.EXMODE  AIF   (&ZZSETAM).BSM
         BR    R14
         MEXIT ,
.BSM     BSM   R0,R14
         MEND  ,
*
*
*
         MACRO ,             UPDATED 2010.293
&NM      FUNHEAD &ID=YES,&IO=NO,&AM=NO,&SAVE=,&US=YES
.*
.*   MACRO TO BEGIN EACH FUNCTION
.*     HANDLES STANDARD OS ENTRY CONVENTIONS
.*   ID=  YES | NO      YES GENERATES DC WITH FUNCTION NAME
.*   IO=  YES | NO      YES GENERATES LOAD / USING FOR ZDCBAREA
.*   AM=  YES | NO      YES USES BSM TO PRESERVE CALLER'S AMODE
.*   SAVE=name          USES STATIC SAVE AREA OF THAT NAME,
.*                           SETS R13, AND DECLARES ON USING
.*   SAVE=(name,len{,subpool})   CREATES SAVE AREA WITH GETMAIN,
.*                           SETS R13, AND DECLARES ON USING
.*   US=  YES | NO      YES - want a USING for R13
.*   Options used here are remembered and handled properly by
.*     subsequent FUNEXIT macros
.*
         GBLC  &ZSYS,&ZZSETSA,&ZZSETSL,&ZZSETSP
         GBLB  &ZZSETAM
         LCLC  &LBL
         LCLA  &I
&I       SETA  K'&NM
&I       SETA  ((&I)/2*2+1)       NEED ODD LENGTH FOR STM ALIGN
&LBL     SETC  '&NM'
&ZZSETAM SETB  ('&AM' NE 'NO')
&ZZSETAM SETB  (&ZZSETAM AND ('&ZSYS' EQ 'S380' OR '&ZSYS' EQ 'ZARCH'))
&ZZSETSA SETC  ''
&ZZSETSL SETC  ''
&ZZSETSP SETC  ''
         ENTRY &NM
         DROP  ,                  Isolate from other code
         AIF   ('&ID' EQ 'NO').SKIPID
&LBL     B     *+4+1+&I-&NM.(,R15)    SKIP LABEL
         DC    AL1(&I),CL(&I)'&NM'    EXPAND LABEL
&LBL     SETC  ''
.SKIPID  AIF   (NOT &ZZSETAM).SKIPAM
&LBL     BSM   R14,R0                 PRESERVE AMODE
&LBL     SETC  ''
.SKIPAM  ANOP  ,
&LBL     STM   R14,R12,12(R13)    SAVE CALLER'S REGISTERS
         LR    R12,R15
         USING &NM,R12
         AIF   ('&IO' EQ 'NO').SAVE
         L     R10,0(,R1)         LOAD FILE WORK AREA
         USING IHADCB,R10
.SAVE    AIF   ('&SAVE' EQ '').MEND
         AIF   (N'&SAVE EQ 1).STATIC
         AIF   (N'&SAVE EQ 2).DYNAM
&ZZSETSP SETC  '&SAVE(3)'
.DYNAM   ANOP  ,
&ZZSETSL SETC  '&SAVE(2)'
&ZZSETSA SETC  '&SAVE(1)'
         AIF ('&ZSYS' EQ 'S370').NOBEL2
         GETMAIN RU,LV=&ZZSETSL,SP=&ZZSETSP,LOC=BELOW
         AGO .GETFIN2
.NOBEL2  ANOP  ,
         GETMAIN RU,LV=&ZZSETSL,SP=&ZZSETSP
.GETFIN2 ANOP  ,
         LR    R14,R1             START OF NEW AREA
         LA    R15,&ZZSETSL       LENGTH
         SR    R3,R3              ZERO FILL
         MVCL  R14,R2             CLEAR GOTTEN STORAGE
         ST    R1,8(,R13)         POINT DOWN
         ST    R13,4(,R1)         POINT UP
         LR    R2,R13             SAVE OLD SAVE
         LR    R13,R1             NEW SAVE AREA
         USING &SAVE(1),R13       DECLARE IT
         LM    R14,R3,12(R2)      RESTORE FROM ENTRY
         MEXIT ,
.STATIC  LA    R15,&SAVE(1)
         ST    R15,8(,R13)
         ST    R13,4(,R15)
         LR    R13,R15
&ZZSETSA SETC  '&SAVE(1)'
         AIF   ('&US' EQ 'NO').MEND
         USING &SAVE(1),R13       DECLARE IT
.MEND    MEND  ,
*
*
*
         MACRO ,
&NM      GAM24 &WORK=R15
         GBLC  &ZSYS
.*
.*   GAM24 sets addressing mode to 24 for S380
.*         expands nothing or label for S370 AND S390
.*
         AIF   ('&ZSYS' NE 'S380' AND '&ZSYS' NE 'ZARCH').OLDSYS
&NM      LA    &WORK,*+6     GET PAST BSM WITH BIT 0 OFF
         BSM   R0,&WORK      CONTINUE IN 24-BIT MODE
         MEXIT ,
.OLDSYS  AIF   ('&NM' EQ '').MEND
&NM      DS    0H            DEFINE LABEL ONLY
.MEND    MEND  ,
*
*
*
         MACRO ,
&NM      GAM31 &WORK=R15
         GBLC  &ZSYS
.*
.*   GAM31 sets addressing mode to 31 for S380.
.*         expands nothing or label for S370  AND S390
.*
         AIF   ('&ZSYS' NE 'S380' AND '&ZSYS' NE 'ZARCH').OLDSYS
&NM      LA    &WORK,*+10    GET PAST BSM WITH BIT 0 ON
         O     &WORK,=X'80000000'  SET MODE BIT
         BSM   R0,&WORK            CONTINUE IN 31-BIT MODE
         MEXIT ,
.OLDSYS  AIF   ('&NM' EQ '').MEND
&NM      DS    0H            DEFINE LABEL ONLY
.MEND    MEND  ,
*
*
*
         MACRO ,             COMPILER DEPENDENT LOAD INTEGER
&NM      LDVAL &R,&A         LOAD VALUE FROM PARM LIST
&NM      L     &R,&A         LOAD PARM VALUE
         L     &R,0(,&R)     LOAD VALUE
.MEND    MEND  ,
*
*
*
         MACRO ,             COMPILER DEPENDENT LOAD PARM ADDRESS
&NM      LDADD &R,&A         GET ADDRESS FROM PARM LIST
&NM      L     &R,&A         LOAD PARM ADDRESS
.MEND    MEND  ,
*
*
*
         MACRO ,             COMPILER DEPENDENT LOAD INT ONLY
&NM      LDINT &R,&A         LOAD INTEGER FROM PARM LIST
         GBLC  &COMP         COMPILER GCC OR IBM C
&NM      L     &R,&A         LOAD PARM VALUE
         AIF   ('&COMP' EQ 'GCC').MEND
.* THIS LINE IS FOR ANYTHING NOT GCC: IBM C
         L     &R,0(,&R)     LOAD VALUE
.MEND    MEND  ,
*
*
*
         MACRO ,
&NM      QBSM  &F1,&F2
         GBLC  &ZSYS
.*
.*   QBSM expands as BSM on environments that require such
.*   mode switch (S380-only)
.*   Otherwise it expands as BALR r1,r2 (instead of BSM r1,r2)
.*   Unless r1 = 0, in which case, a simple BR r2 is done instead
.*
         AIF   ('&ZSYS' NE 'S380' AND '&ZSYS' NE 'ZARCH').OTHSYS
&NM      BSM   &F1,&F2
         MEXIT ,
.OTHSYS  AIF   ('&F1' EQ '0' OR '&F1' EQ 'R0').BR
&NM      BALR  &F1,&F2
         MEXIT ,
.BR      ANOP  ,
&NM      BR    &F2
.MEND    MEND  ,
*
*
*
         MACRO ,
&NM      MAPSUPRM &PFX=ZP,&DSECT=
.*  THIS MACRO DESCRIBES/DEFINES THE OPEN I/O MODE AND ASSOCIATED WORK
.*  AREA USED BY THE MVSSUPA SERVICE ROUTINE.
.*  MODE IS INPUT AND RETURNED, POSSIBLY MODIFIED.
.*  ID IS INPUT AND UPDATED.
.*  REST ARE OUTPUT ONLY, AND MAY DIFFER FROM THE @@AOPEN REQUEST.
         LCLC  &P,&N
&P       SETC  '&PFX'
&N       SETC  '&NM'
         AIF   ('&N' NE '').HAVSECT
&N       SETC  'MAP'.'&P'
.HAVSECT AIF   ('&DSECT' EQ 'NO').NOSEC
&N       DSECT ,
         AGO   .COMSEC
.NOSEC   AIF   ('&NM' EQ '').COMSEC
&NM      DS    0F
.COMSEC  ANOP  ,
&P.MODE  DC    F'0' I/O MODE (0-IN,1-OUT,2-UPD,3-APP,4-INOUT,5-OUTIN)
.*                  +8-USE EXCP FOR TAPE
.*                  +10-USE BLOCK MODE (BSAM RATHER THAN QSAM MODE)
.*                  +80-TERMINAL GETLINE  +81-TERMINAL PUTLINE
.*                  RETURNS 40-VSAM; 20-BPAM UNLIKE CONCAT
&P.MIN   EQU   0    I/O MODE quick definitions
&P.MOUT  EQU   1
&P.MUPD  EQU   2
&P.MAPP  EQU   3
&P.MINO  EQU   4
&P.MOIN  EQU   5
&P.MBLK  EQU   16
&P.MTRM  EQU   128
.*
&P.DVTYP EQU   &P.MODE,1     DEVICE TYPE OF FIRST/ONLY DD
.*
&P.RECFM EQU   &P.MODE+1,1   EQUIVALENT RECFM BITS
&P.RFU   EQU   X'C0'           UNDEFINED
&P.RFF   EQU   X'80'           FIXED
&P.RFV   EQU   X'40'           VARIABLE (WITH BDW/RDW)
&P.RFD   EQU   X'20'           ASCII VARIABLE (WITH BIT 0-1 OFF)
&P.RFT   EQU   X'20'           TRACK OVERFLOW (WITH 0-1 NOT OFF)
&P.RFB   EQU   X'10'           BLOCKED
&P.RFS   EQU   X'08'           STANDARD(F), SPANNED(V)
&P.RFA   EQU   X'04'           ANSI CONTROL CHARACTERS
&P.RFM   EQU   X'02'           MACHINE CONTROL CHARACTERS (1403 CCW)
.*
&P.SFLGS EQU   &P.MODE+2,1   ADDITIONAL MODE RELATED FLAGS
&P.FUPDT EQU   X'80'           UPDATE MODE (BSAM/VSAM)
&P.FUPIN EQU   X'40'           LAST WAS INPUT (GET/READ)
&P.FUPOU EQU   X'20'           LAST WAS OUTPUT (PUT/WRITE)
&P.FVSAM EQU   X'08'           USE VSAM
&P.FVSRR EQU   X'04'           RRDS
&P.FVSES EQU   X'02'           ESDS
&P.FVSKS EQU   X'01'           KSDS
.*
&P.MFLGS EQU   &P.MODE+3,1   REMEMBER OPEN MODE
&P.FTERM EQU   X'80'           USING GETLINE/PUTLINE
&P.FBPAM EQU   X'20'           UNLIKE BPAM CONCAT - SPECIAL HANDL
&P.FBLOK EQU   X'10'           USING BSAM READ/WRITE MODE
&P.FEXCP EQU   X'08'           USE EXCP FOR TAPE
.*.FUPD  EQU   X'06'           UPDATE IN PLACE (XSAM, VSAM)
.*.      EQU                   (RESERVED)
&P.FOUT  EQU   X'01'           OUTPUT MODE
.*
.*
&P.LEN   DC    AL2(&P.SIZE)  CONTROL BLOCK LENGTH
&P.ID    DC    H'42'         BLOCK IDENTIFIER (0 ON RETURN)
.*
&P.FLAGS DC    X'0'          DD SCAN FLAG
&P.FDD   EQU   X'80'           FOUND A DD - LATER CONCAT FLAG
&P.FSEQ  EQU   X'40'           USE IS SEQUENTIAL
&P.FPDQ  EQU   X'20'           DS IS PDS WITH MEMBER NAME
&P.FPDS  EQU   X'10'           DS IS PDS (OR PDS/E WITH S390)
&P.FVSM  EQU   X'08'           DS IS VSAM (LIMITED SUPPORT)
&P.FVTOC EQU   X'04'           DS IS VTOC (LIMITED SUPPORT)
&P.FBLK  EQU   X'02'           DD HAS FORCED BLKSIZE
.*
&P.PIX   DC    X'0'          PROCESSING INDEX
&P.IXSAM EQU   0               BSAM/BPAM - DEFAULT
&P.IXQSM EQU   4               QSAM - SAME AS BSAM, WITH DEBLOCKING
&P.IXVSM EQU   8               VSAM DATA SET
&P.IXVTC EQU   12              VTOC READER
&P.IXTRM EQU   16              TSO TERMINAL
.*.IXVSK EQU   20              (RESERVED) VSAM KEYED I/O
.*.IXVSU EQU   24              (RESERVED) VSAM UPDATE (GET/PUT/RELEASE)
.*
&P.BLKPT DC    X'00'         BLOCKS PER TRACK (MAX BLKSI)
.*
&P.OPC   DC    X'0'          DCB OPTCD
.*
&P.DEVT  DC    XL2'0'        UCBTBYT3/4
&P.ORG   DC    XL2'0'        DSORG
&P.KEYL  DC    F'0'          KEY LENGTH
&P.KEYP  DC    F'0'          KEY POSITION
&P.LRECL DC    F'0'          RECORD LENGTH
&P.BLKSZ DC    F'0'          BLOCK SIZE
&P.MAXRC DC    F'0'          MAXIMUM RECORD NUMBER
.*
.*
&P.FMOD  DC    X'00'         CALLER'S MODE: 0-F  1-V  2-U
&P.FMFIX EQU   0               FIXED RECFM (BLOCKED)
&P.FMVAR EQU   1               VARIABLE (BLOCKED)
&P.FMUND EQU   2               UNDEFINED
.*
&P.TTR   DC    XL3'00'       (MISC. USE)  TTR
.*
&P.SIZE  EQU   *-&P.MODE     SIZE TO CLEAR
         MEND  ,
*
*
*
         MACRO ,             COMPILER DEPENDENT LOAD INTEGER
&NM      STVAL &R,&A,&S=R14  STORE VALUE FROM PARM LIST
&NM      L     &S,&A         LOAD PARM VALUE
         ST    &R,0(,&S)     RETURN VALUE
.MEND    MEND  ,
*
*
*
         MACRO ,             PATTERN FOR @@DYNAL'S DYNAMIC WORK AREA
&NM      DYNPAT &P=MISSING-PFX
.*   NOTE THAT EXTRA FIELDS ARE DEFINED FOR FUTURE EXPANSION
.*
&NM      DS    0D            ALLOCATION FIELDS
&P.ARBP  DC    0F'0',A(X'80000000'+&P.ARB) RB POINTER
&P.ARB   DC    0F'0',AL1(20,S99VRBAL,0,0)
         DC    A(0,&P.ATXTP,0,0)       SVC 99 REQUEST BLOCK
&P.ATXTP DC    10A(0)
&P.AXVOL DC    Y(DALVLSER,1,6)
&P.AVOL  DC    CL6' '
&P.AXDSN DC    Y(DALDSNAM,1,44)
&P.ADSN  DC    CL44' '
&P.AXMEM DC    Y(DALMEMBR,1,8)
&P.AMEM  DC    CL8' '
&P.AXDSP DC    Y(DALSTATS,1,1)
&P.ADSP  DC    X'08'         DISP=SHR
&P.AXFRE DC    Y(DALCLOSE,0)   FREE=CLOSE
&P.AXDDN DC    Y(DALDDNAM,1,8)    DALDDNAM OR DALRTDDN
&P.ADDN  DC    CL8' '        SUPPLIED OR RETURNED DDNAME
&P.ALEN  EQU   *-&P.ARBP       LENGTH OF REQUEST BLOCK
         SPACE 1
&P.URBP  DC    0F'0',A(X'80000000'+&P.URB) RB POINTER
&P.URB   DC    0F'0',AL1(20,S99VRBUN,0,0)
         DC    A(0,&P.UTXTP,0,0)       SVC 99 REQUEST BLOCK
&P.UTXTP DC    A(X'80000000'+&P.UXDDN)
&P.UXDDN DC    Y(DUNDDNAM,1,8)
&P.UDDN  DC    CL8' '        RETURNED DDNAME
&P.ULEN  EQU   *-&P.URBP       LENGTH OF REQUEST BLOCK
&P.DYNLN EQU   *-&P.ARBP     LENGTH OF ALL DATA
         MEND  ,
*
*
*
         MACRO ,
&NM      FIXWRITE ,
&NM      L     R15,=A(TRUNCOUT)
         BALR  R14,R15       TRUNCATE CURRENT WRITE BLOCK
         MEND  ,
*
*
*
         MACRO ,
&NM      OSUBHEAD ,
         PUSH  USING
         DROP  ,
         USING WORKAREA,R13
         USING ZDCBAREA,R10
&NM      STM   R10,R15,SAVOSUB    Save registers
         BALR  R12,0
         USING *,R12
         MEND  ,
         SPACE 1
*
*
*
         MACRO ,
&NM      OSUBRET &ROUTE=
         LCLC  &T
&T       SETC  '&NM'
         AIF   (T'&ROUTE EQ 'O').BACK
         AIF   ('&ROUTE' EQ '(14)' OR '&ROUTE' EQ '(R14)').ROUT14
&T       LA    R14,=A(&ROUTE)     Return point in AOPEN
         AGO   .BACK
&T       SETC  ''                 Set label used
.ROUT14  ANOP  ,
&T       ST    R14,SAVOSUB+4*4    Update R14
&T       SETC  ''                 Set label used
.BACK    ANOP  ,
&T       LM    R10,R15,SAVOSUB    Load registers
         BR    R14                Return to caller
         MEND  ,
         SPACE 1
*
*
*
         MACRO ,
&NM      OPENCALL &WHOM
&NM      L     R15,=A(&WHOM)      Extension routine
         BALR  R14,R15            Invoke it
         MEXIT ,
         MEND  ,
         SPACE 1
         MACRO ,
&NM      OBRAN &WHERE,&OP=B,&EXIT=VECTOR
&NM      L     R14,=A(&WHERE)     Return point
         &OP   &EXIT              Branch to alternate return
         MEND  ,
         SPACE 1
*
*
*
         SPACE 1
         COPY  PDPTOP
         SPACE 1
* For non-S/370 we need to deliberately request LOC=BELOW storage
* in most places. We can't use GETMAIN R because that is not
* AM32/AM64 clean. For the main storage we deliberately request
* LOC=ANY storage. Fortunately those flags are ignored for S/370.
*
         CSECT ,
         PRINT GEN,ON
         SPACE 1
*-----------------------ASSEMBLY OPTIONS------------------------------*
SUBPOOL  EQU   0                                                      *
*---------------------------------------------------------------------*
         SPACE 1
*                                                                     *
*   Note: Variable @@BUGF controls various debugging options, and is  *
*   externally accessible. Low order 1 bit requests SNAPs of failed   *
*   OPEN information and bad records.                                 *
         ENTRY @@BUGF
*                                                                     *
*---------------------------------------------------------------------*
*                                                                     *
* Start of functions                                                  *
*                                                                     *
*                                                                     *
***********************************************************************
*                                                                     *
*  AOPEN - Open a data set                                            *
*                                                                     *
***********************************************************************
*                                                                     *
*  Parameters are:                                                    *
*1 DDNAME - space-padded, 8 character DDNAME to be opened             *
*                                                                     *
*2 MODE =  0 INPUT  1 OUTPUT  2 UPDAT   3 APPEND      Record mode     *
*  MODE =  4 INOUT  5 OUTIN     (6-7 reserved)                        *
*  MODE = 8/9 Use EXCP for tape, BSAM otherwise (or 32<=JFCPNCP<=65)  *
*  MODE + 10 = Use BLOCK mode (valid hex 10-15)                       *
*  MODE = 80 = GETLINE, 81 = PUTLINE (other bits ignored)             *
*    N.B.: see comments under Return value                            *
*                                                                     *
*3 RECFM - 0 = F, 1 = V, 2 = U. Default/preference set by caller;     *
*                               actual value returned from open.      *
*                                                                     *
*4 LRECL   - Default/preference set by caller; OPEN value returned.   *
*                                                                     *
*5 BLKSIZE - Default/preference set by caller; OPEN value returned.   *
*                                                                     *
* August 2009 revision - caller will pass preferred RECFM (coded 0-2) *
*    LRECL, and BLKSIZE values. DCB OPEN exit OCDCBEX will use these  *
*    defaults when not specified on JCL or via DSCB merge.            *
*                                                                     *
*6 ZBUFF2 - pointer to an area that may be written to (size is LRECL) *
*                                                                     *
*7 MEMBER - *pointer* to space-padded, 8 character member name.       *
*    A member name beginning with blank or hex zero is ignored.       *
*    If pointer is 0 (NULL), no member is requested                   *
*    For a DD card specifying a PDS with a member name, this parameter*
*    will replace the JCL member, unless the DD is concatenated, then *
*    all DDs are treated as sequential and a member operand will be   *
*    an error.                                                        *
*                                                                     *
*                                                                     *
*  Return value: In R15                                               *
*  An internal "handle" that allows the assembler routines to         *
*  keep track of what's what, when READ etc are subsequently          *
*  called.                                                            *
*                                                                     *
*                                                                     *
*  Return value: PARM2 MODE:                                          *
*    Byte 0 - major device type (defined by UCBTBYT3; JES sets 01)    *
*    Byte 1 - true RECFM as used in the DCB                           *
*    Byte 2 - processing mode (see IOSFLAGS)                          *
*    Byte 3 - modified user's MODE                                    *
*                                                                     *
*  All passed parameters are subject to overrides based on device     *
*  capabilities and capacities, e.g., blocking may be turned off.     *
*                                                                     *
*                                                                     *
*  Note - more documentation for this and other I/O functions can     *
*  be found halfway through the stdio.c file in (EDWARDS.)PDPCLIB.    *
*                                                                     *
* Here are some of the errors reported:                               *
*                                                                     *
*  Input  OPEN (SVC) failed; return code is: -37                      *
*  Output OPEN (SVC) failed; return code is: -39                      *
*    Also used for VSAM failure in OPEN, SHOWCB, or TESTCB            *
*                                                                     *
* FIND input member return codes are:                                 *
* Original, before the return and reason codes had                    *
* negative translations added refer to copyrighted:                   *
* DFSMS Macro Instructions for Data Sets.                             *
* RC = 0 Member was found.                                            *
*                                                                     *
*     The 1nnn group has not been implemented.                        *
* RC = -1024 Member not found. (replaced by 2068)                     *
* RC = -1028 RACF allows PDSE EXECUTE, not PDSE READ.                 *
* RC = -1032 PDSE share not available.                                *
* RC = -1036 PDSE is OPENed output to a different member.             *
*---------------------------------------------------------------------*
*   New OPEN validity checking added; return codes are:               *
* RC = -2004 DDname starts with blank or null                         *
* RC = -2008 DDname not found                                         *
* RC = -2012 Error in system control block (JFCB, JSCB, PSCB)         *
* RC = -2016 Error reading DSCB1                                      *
* RC = -2020 Invalid TIOT entry                                       *
* RC = -2024 Invalid or unsupported DSORG                             *
* RC = -2028 Invalid DCB parameters                                   *
* RC = -2032 Invalid unit type (Graphics, Communications...)          *
* RC = -2036 Invalid concatenation (not input; mixed sequential & PDS)*
* RC = -2040 Invalid MODE request for DD                              *
* RC = -2044 PDS has no directory blocks                              *
* RC = -2048 Directory I/O error.                                     *
* RC = -2052 Out of virtual storage.                                  *
* RC = -2056 Invalid DEB or DEB not on TCB or TCBs DEB chain.         *
* RC = -2060 PDSE I/O error flushing system buffers.                  *
* RC = -2064 Invalid FIND or BLDL.                                    *
* RC = -2068 Member not found                                         *
* RC = -2072 Member not allowed                                       *
* RC = -2096 Unable to extend data (>64KiB tracks)                    *
* RC = -3nnn VSAM OPEN failed with ACBERF=nn                          *
*                                                                     *
***********************************************************************
@@AOPEN  FUNHEAD SAVE=(WORKAREA,OPENLEN,SUBPOOL)
         SR    R10,R10            Indicate no ZDCB area gotten
         LA    R11,2048(R12)
         LA    R11,2048(R11)
         USING @@AOPEN+4096,R11
         MVC   PARM1(4*7),0(R1)   Move parameters to work area
         LDADD R3,PARM1           R3 POINTS TO DDNAME
         MVC   DWDDNAM,0(R3)      Move below the line
         PUSH  USING
***********************************************************************
**                                                                   **
**  Code added to support unlike concatenation for both sequential   **
**  and partitioned access (one member from FB or VB library DDs).   **
**  Determines maximum buffer size need for both cases (except tape) **
**                                                                   **
**  Added validity checking and error codes.                         **
**                                                                   **
**  Does not use R3, R11-R13                                         **
**                                                                   **
***********************************************************************
         MVI   OPERF,ORFBADNM     PRESET FOR BAD DD NAME
         CLI   DWDDNAM,C' '       VALID NAME ?
         BNH   OPSERR               NO; REJECT IT
         MVI   OPERF,ORFNODD      PRESET FOR MISSING DD NAME
         LA    R8,DWDDNAM         COPY DDNAME POINTER TO SCRATCH REG.
* If running on MVS/XA or above, this code must be executed in AM31.
* This means that the module must be marked AM31. AM24 is only
* supported on MVS 3.8j. Hopefully one day via some mechanism
* such as dynamic allocation, this code can be executed in AM24
* on MVS/XA+, but until then, this restriction is in place.
* We do not force AM31, as that is contrary to the AMODE
* switching doctrine used by PDPCLIB.
*         GAM31 ,                 AM31 FOR S380
         LA    R4,DDWATTR         POINTER TO DD ATTRIBUTES
         USING DDATTRIB,R4        DECLARE TABLE
         L     R14,PSATOLD-PSA    GET MY TCB
         L     R9,TCBTIO-TCB(,R14) GET TIOT
         USING TIOT1,R9           DECLARE IT
         LA    R0,TIOENTRY-TIOT1  INCREMENT TO FIRST ENTRY
*---------------------------------------------------------------------*
*   LOOK FOR FIRST (OR ONLY) DD                                       *
*---------------------------------------------------------------------*
DDCFDD1  AR    R9,R0              NEXT ENTRY
         MVI   OPERF,ORFNODD      PRESET FOR NO TIOT ENTRY
         USING TIOENTRY,R9        DECLARE IT
         ICM   R0,1,TIOELNGH      GET ENTRY LENGTH
         BZ    DDCTDONE             TOO BAD
         TM    TIOESTTA,TIOSLTYP  SCRATCHED ENTRY?
         BNZ   DDCFDD1              YES; IGNORE
         CLC   TIOEDDNM,0(R8)     MATCHES USER REQUEST?
         BNE   DDCFDD1              NO; TRY AGAIN
         SR    R7,R7
         ICM   R7,7,TIOEFSRT      LOAD UCB ADDRESS (COULD BE ZERO)
         USING UCBOB,R7
         MVI   OPERF,ORFBATIO     SET FOR INVALID TIOT
         CLI   TIOELNGH,20        SINGLE UCB ?
         BL    OPSERR               NOT EVEN
*---------------------------------------------------------------------*
* EXAMINE ONE DD ENTRY, AND SET FLAGS AND BUFFER SIZE HIGH-WATER MARK *
*---------------------------------------------------------------------*
         SPACE 1
DDCHECK  MVI   OPERF,ORFNOJFC     PRESET FOR BAD JFCB
         ICM   R1,7,TIOEJFCB      GET JFCB ADDRESS OR TOKEN
         BZ    OPSERR               NO JFCB ?
         L     R15,=A(LOOKSWA)    GET TOKEN CONVERSION
         BALR  R14,R15            INVOKE IT
         LTR   R6,R15             LOAD AND TEST ADDRESS
         BNP   OPSERR               NO JFCB ?
*COMP*   AIF   ('&ZSYS' NE 'S390').MVSJFCB
*COMP*   XC    DDWSWA(DDWSWAL),DDWSWA  CLEAR SWA LIST FORM
*COMP*   LA    R1,DDWSVA          ADDRESS OF JFCB TOKEN
*COMP*   ST    R1,DDWEPA
*COMP*   MVC   DDWSVA+4(3),TIOEJFCB    JFCB TOKEN
*COMP*   SWAREQ FCODE=RL,EPA=DDWEPA,MF=(E,DDWSWA),UNAUTH=YES
*COMP*   BXH   R15,R15,OPSERR
*COMP*   ICM   R6,15,DDWSVA       LOAD JFCB ADDRESS
*COMP*   BZ    OPSERR               NO; SERIOUS PROBLEM
*COMP*   AGO   .COMJFCB
*MVSJFCB SR    R6,R6              FOR AM31
*COMP*   ICM   R6,7,TIOEJFCB      SHOULD NEVER BE ZERO
*COMP*   BZ    OPSERR               NO; SERIOUS PROBLEM
*COMP*   LA    R6,16(,R6)         SKIP QUEUE HEADER
.COMJFCB MVC   MYJFCB(JFCBLGTH),0(R6)   MOVE TO MY STORAGE
         OI    DDWFLAG2,CWFDD     DD FOUND
         MVC   DDADSORG,JFCDSORG  SAVE
         MVC   DDARECFM,JFCRECFM    DCB
         MVC   DDALRECL,JFCLRECL      ATTRIBUTES
         MVC   DDABLKSI,JFCBUFSI
         CLC   JFCDSORG,ZEROES    ANY DSORG HERE?
         BE    DDCNOORG             NO
         TM    JFCDSRG2,JFCORGAM  VSAM ?
         BNZ   DDCNOORG             YES; SPECIAL HANDLING
         CLI   JFCDSRG2,0         ANYTHING UNWANTED?
         BNE   BADDSORG             YES; CAN'T USE
         TM    JFCDSRG1,254-JFCORGPS-JFCORGPO  UNSUPPORTED ?
         BNZ   BADDSORG             YES; FAIL
DDCNOORG SR    R5,R5
         ICM   R5,3,JFCBUFSI      ANY BLOCK/BUFFER SIZE ?
         C     R5,DDWBLOCK        COMPARE TO PRIOR VALUE
         BNH   DDCNJBLK             NOT LARGER
         ST    R5,DDWBLOCK        SAVE FOR RETURN
DDCNJBLK LTR   R7,R7              IS THERE A UCB (OR TOKEN)?
         BZ    DDCSEQ               NO; MUST NOT BE A PDS
         MVI   OPERF,ORFBATY3     PRESET UNSUPPORTED DEVTYPE
         CLI   UCBTBYT3,UCB3DACC  DASD ?
         BNE   DDCNRPS              NO
         MVC   DDAATTR,UCBTBYT2   COPY ATTRIBUTES
         NI    DDAATTR,UCBRPS     KEEP ONLY RPS
DDCNRPS  TM    UCBTBYT3,255-(UCB3DACC+UCB3TAPE+UCB3UREC)
         BNZ   OPSERR               UNSUPPORTED DEVICE TYPE
         CLI   UCBTBYT3,UCB3DACC  DASD VOLUME ?
         BNE   DDCSEQ               NO; NOT PDS
         CLC   =C'FORMAT4.DSCB ',JFCBDSNM    VTOC READ?
         BNE   NOTVTOC
         LA    R0,ORFBDMOD        PRESET FOR BAD MODE
         LDVAL R14,PARM2          GET THE MODE
         TM    3(R14),X'07'       ANYTHING OTHER THAN INPUT?
         BNZ   OPRERR             VTOC WRITE NOT SUPPORTED
         MVI   JFCBDSNM,X'04'     MAKE VTOC 'NAME'
         MVC   JFCBDSNM+1(L'JFCBDSNM-1),JFCBDSNM   44X'04'
         MVI   DDADSORG,X'40'     SEQUENTIAL
         MVI   DDARECFM,X'80'     RECFM=F
         MVC   DDALRECL,=AL2(44+96)    KEY+DATA
         MVC   DDABLKSI,=AL2(44+96)    KEY+DATA
         OI    DDWFLAG2,CWFVTOC   SET FOR VTOC ACCESS
         MVC   DDWBLOCK,=A(DS1END-IECSDSL1+5)    SET BUFFER SZ
         B     DDCSEQ             SKIP AROUND
         SPACE 1
*   For a DASD data set, obtain the format 1 DSCB to get DCB parameters
*   prior to OPEN. If the data set is not found, the DD may have
*   used an alias name for the data set. If so, we look it up in the
*   catalog, and use the true name to try again.
*
NOTVTOC  L     R14,CAMDUM         GET FLAGS IN FIRST WORD
         LA    R15,JFCBDSNM       POINT TO DSN
         LA    R0,UCBVOLI         POINT TO SERIAL
         LA    R1,DS1FMTID        POINT TO RETURN
         STM   R14,R1,CAMLST
         MVI   OPERF,ORFNODS1     PRESET FOR BAD DSCB 1
         OBTAIN CAMLST       READ DSCB1
         BXLE  R15,R15,OBTGOOD
         SPACE 1
         TM    JFCBTSDM,JFCCAT    Cataloged DS ?
         BZ    TESTORGA             No; fail unless VSAM
         MVC   TRUENAME,JFCBDSNM  Copy name in case replaced
         L     R14,CAMLOC         GET FLAGS IN FIRST WORD
         LA    R15,TRUENAME       POINT TO DSN
         LA    R0,0                 CVOL pointer
         LA    R1,CATWORK         POINT TO RETURN
         STM   R14,R1,CAMLST
         LOCATE CAMLST       CHECK CATALOG ENTRY
         BXH   R15,R15,OPSERR
         L     R14,CAMDUM         GET FLAGS IN FIRST WORD
         LA    R15,TRUENAME       POINT TO DSN
         LA    R0,UCBVOLI         POINT TO SERIAL
         LA    R1,DS1FMTID        POINT TO RETURN
         STM   R14,R1,CAMLST
         OBTAIN CAMLST       READ DSCB1
         BXLE  R15,R15,OBTGOOD
TESTORGA TM    JFCDSRG2,JFCORGAM  VSAM?
         BNZ   DDCVSAM              Yes; let open handle it
         B     OPSERR
OBTGOOD  CLI   DS1FMTID,C'1'      SUCCESSFUL READ ?
         BNE   OPSERR               NO; OOPS
         CLC   =C'IBM',FM1SMSFG   Old format ?
         BNE   *+4+6                No; keep intact
         MVC   FM1SMSFG(2),ZEROES      Fake it out
         SPACE 1
         CLC   DDADSORG,ZEROES
         BNE   *+4+6
         MVC   DDADSORG,DS1DSORG  SAVE
         CLI   DDARECFM,0
         BNE   *+4+6
         MVC   DDARECFM,DS1RECFM    DCB
         CLC   DDALRECL,ZEROES
         BNE   *+4+6
         MVC   DDALRECL,DS1LRECL      ATTRIBUTES
         CLC   DDABLKSI,ZEROES
         BNE   *+4+6
         MVC   DDABLKSI,DS1BLKL
         SPACE 1
         LTR   R5,R5              DID JFCB HAVE OVERRIDING BUFFER SIZE
         BNZ   DDCUJBLK             YES; IGNORE DSCB
         LH    R5,DS1BLKL         GET BLOCK SIZE
         C     R5,DDWBLOCK        COMPARE TO PRIOR VALUE
         BNH   DDCUJBLK             NOT LARGER
         ST    R5,DDWBLOCK        SAVE FOR RETURN
DDCUJBLK TM    DS1DSORG+1,DS1ACBM VSAM ?
         BNZ   DDCVSAM              YES; HOP FOR THE BEST
         CLI   DS1DSORG+1,0       ANYTHING UNPROCESSABLE?
         BNE   BADDSORG
         TM    DS1DSORG,255-DS1DSGPS-DS1DSGPO-DS1DSGU NOT GOOD
         BNZ   BADDSORG             YES; TOO BAD
         TM    DS1DSORG,DS1DSGPO
         BZ    DDCSEQ             (CHECK JFCB OVERRIDE DSORG?)
         TM    JFCBIND1,JFCPDS    MEMBER NAME ON DD ?
         BNZ   DDCPMEM              YES; SHOW
*DEFER*  LTR   R8,R8              First DD of possible concat?
*DEFER*  BZ    DDCPPDS              No; ignore member parameter
*DEFER*  LDADD R14,PARM7          R14 POINTS TO MEMBER NAME (OF PDS)
*DEFER*  LA    R14,0(,R14)        Strip off high-order bit or byte
*DEFER*  LTR   R14,R14            Zero address passed ?
*DEFER*  BZ    DDCPPDS              Yes; not member name
*DEFER*  TM    0(R14),255-X'40'   Either blank or zero?
*DEFER*  BNZ   DDCPMEM              No; sequential
DDCPPDS  OI    DDWFLAG2,CWFPDS    SET PDS ONLY
         B     DDCKPDS              Test LSTAR & SMS
DDCVSAM  OI    DDWFLAG2,CWFVSM    SHOW VSAM MODE
         B     DDCX1DD              NEXT DD
DDCPMEM  OI    DDWFLAG2,CWFPDQ    SHOW SEQUENTIAL PDS USE
DDCKPDS  DS    0H
*  Note that this test may fail for data sets written under DOS
         TM    FM1SMSFG,FM1STRP+FM1PDSEX+FM1DSAE  Usable
         BNZ   BADDSORG             No; too bad
         LA    R0,ORFBDPDS        Preset - not initialized
         ICM   R15,7,DS1LSTAR     Any directory blocks?
         BZ    OPRERR               No; fail
         B     DDCX1DD
         SPACE 1
*---------------------------------------------------------------------*
*   FOR A CONCATENATION, PROCESS THE NEXT DD                          *
*---------------------------------------------------------------------*
DDCSEQ   OI    DDWFLAG2,CWFSEQ    SET FOR SEQUENTIAL ACCESS
DDCX1DD  SR    R0,R0              RESET
         LTR   R8,R8              FIRST TIME FOR DD ?
         BZ    FIND1DD              NO
         MVC   DDWFLAG1,DDWFLAG2  SAVE FLAGS FOR FIRST DD
         MVI   DDWFLAG2,0         RESET DD
         SR    R8,R8              SIGNAL FIRST DD DONE
FIND1DD  IC    R0,TIOELNGH        GET ENTRY LENGTH BACK
         AR    R9,R0              NEXT ENTRY
         ICM   R0,1,TIOELNGH      GET NEW ENTRY LENGTH
         BZ    DDCTDONE             ZERO; ALL DONE
         TM    TIOESTTA,TIOSLTYP  SCRATCHED ENTRY?
         BNZ   DDCTDONE             YES; DONE (?)
         CLI   TIOEDDNM,C' '      CONCATENATION?
         BH    DDCTDONE             NO; DONE WITH DD
         LA    R4,DDASIZE(,R4)    NEXT DD ATTRIBUTE ENTRY
         SR    R7,R7
         ICM   R7,7,TIOEFSRT      LOAD UCB ADDRESS (COULD BE ZERO)
         USING UCBOB,R7
         MVI   OPERF,ORFBATIO     SET FOR INVALID TIOT
         CLI   TIOELNGH,20        SINGLE UCB ?
         BL    OPSERR               NOT EVEN
         B     DDCHECK            AND PROCESS IT
         SPACE 1
BADDSORG LA    R0,ORFBADSO        BAD DSORG
         B     OPRERR
         SPACE 2
***********************************************************************
         POP   USING
         SPACE 1
DDCTDONE MVC   DDWFLAGS,DDWFLAG1  COPY FIRST DD'S FLAGS
         NI    DDWFLAGS,255-CWFDD BUT RESET FIRST DD PRESENT
         OC    DDWFLAGS,DDWFLAG2  OR TOGETHER
         GAMAPP ,                 RESTORE AMODE FROM ENTRY
* Note that R5 is used as a scratch register
         L     R8,PARM4           R8 POINTS TO LRECL
* PARM5    has BLKSIZE
* PARM6    has ZBUFF2 pointer
         SPACE 1
         LDVAL R4,PARM2           MODE.  0=input 1=output, etc.
         SPACE 1
*   Conditional forms of storage acquisition are reentrant unless
*     they pass values that vary between uses, which ours don't,
*     or require storaage alteration (ditto).
*   Note that PAGE alignment makes for easier dump reading
*   but wastes storage - so we use it for debugging only.
*   Using RC to get a return code if memory unavailable.
*DEBUG*  GETMAIN RC,LV=ZDCBLEN,SP=SUBPOOL,LOC=BELOW,BNDRY=PAGE **DEBUG
*not usd LA    R0,ORFNOSTO        Preset for no storage
*not set BXH   R15,R15,OPRERR       Return error code
         AIF ('&ZSYS' EQ 'S370').NOBEL3
         GETMAIN RU,LV=ZDCBLEN,SP=SUBPOOL,LOC=BELOW  I/O work area BTL
         AGO .GETFIN3
.NOBEL3  ANOP  ,
         GETMAIN RU,LV=ZDCBLEN,SP=SUBPOOL  I/O work area BTL
.GETFIN3 ANOP  ,
*
         LR    R10,R1             Addr.of storage obtained to its base
         USING IHADCB,R10         Give assembler DCB area base register
         LR    R0,R10             Load output DCB area address
         LA    R1,ZDCBLEN         Load output length of DCB area
         LA    R15,0              Pad of X'00' and no input length
         MVCL  R0,R14             Clear DCB area to binary zeroes
         MVC   ZDDN,DWDDNAM       DDN for debugging
         XC    ZMEM,ZMEM          Preset for no member
         MVC   ZPFLAGS,DDWFLAGS   SAVE FLAGS
         LDADD R14,PARM7          R14 POINTS TO MEMBER NAME (OF PDS)
         LA    R14,0(,R14)        Strip off high-order bit or byte
         LTR   R14,R14            Zero address passed ?
         BZ    OPENMPRM             Yes; skip
         TM    0(R14),255-X'40'   Either blank or zero?
         BZ    OPENMPRM             Yes; no member
         MVC   ZMEM,0(R14)        Save member name
*---------------------------------------------------------------------*
*   GET USER'S DEFAULTS HERE, BECAUSE THEY MAY GET CHANGED
*---------------------------------------------------------------------*
OPENMPRM L     R5,PARM3    HAS RECFM code (0-FB 1-VB 2-U)
         L     R14,0(,R5)         LOAD RECFM VALUE
         STC   R14,FILEMODE       PASS TO OPEN
         L     R14,0(,R8)         GET LRECL VALUE
         ST    R14,ZPLRECL        PASS TO OPEN
         L     R14,PARM5          R14 POINTS TO BLKSIZE
         L     R14,0(,R14)        GET BLOCK SIZE
         ST    R14,ZPBLKSZ        PASS TO OPEN
         MVC   ZDDFLAGS,DDWFLAGS  PRESERVE FLAGS (OPEN EXIT)
         SPACE 1
*---------------------------------------------------------------------*
*   DO THE DEVICE TYPE NOW TO CHECK WHETHER EXCP IS POSSIBLE
*     ALSO BYPASS STUFF IF USER REQUESTED TERMINAL I/O
*---------------------------------------------------------------------*
OPCURSE  STC   R4,WWORK           Save to storage
         STC   R4,WWORK+1         Save to storage
         NI    WWORK+1,7          Retain only open mode bits
         TM    WWORK,IOFTERM      Terminal I/O ?
         BNZ   TERMOPEN           Yes; do completely different
***> Consider forcing terminal mode if DD is a terminal?
         DEVTYPE DWDDNAM,DWORK    Check device type
         MVC   ZDVTYPE,DWORK+2    return device type
         MVC   ZPDEVT,DWORK+2     return device type
         LA    R0,ORFNODD         Missing DD
         BXH   R15,R15,OPRERR     DD missing
         ICM   R0,15,DWORK+4      Any device size ?
         BNZ   OPHVMAXS
         MVC   DWORK+6(2),=H'32760'    Set default max
         SPACE 1
OPHVMAXS CLI   WWORK+1,3          Append (AKA Extend) requested ?
         BNE   OPNOTAP              No
         TM    DWORK+2,UCB3TAPE+UCB3DACC    TAPE or DISK ?
         BM    OPNOTAP              Yes; supported
         NI    WWORK,255-2        Change to plain output
*OR-FAIL BNM   FAILDCB              No, not supported
         SPACE 1
OPNOTAP  CLI   WWORK+1,2          UPDAT request?
         BNE   OPNOTUP              No
         LA    R0,ORFBDMOD        Preset for bad mode
         CLI   DWORK+2,UCB3DACC   DASD ?
         BNE   OPRERR               No, not supported
         SPACE 1
OPNOTUP  CLI   WWORK+1,4          INOUT or OUTIN ?
         BL    OPNOTIO              No
         TM    DWORK+2,UCB3TAPE+UCB3DACC    TAPE or DISK ?
         LA    R0,ORFBDMOD        Preset for bad mode
         BNM   OPRERR               No; not supported
         SPACE 1
OPNOTIO  TM    WWORK,IOFEXCP      EXCP requested ?
         BZ    OPFIXMD2             No
         CLI   DWORK+2,UCB3TAPE   TAPE/CARTRIDGE device?
         BE    OPFIXMD1             Yes; wonderful ?
OPFIXMD0 NI    WWORK,255-IOFEXCP  Cancel EXCP request
         B     OPFIXMD2
OPFIXMD1 L     R0,ZPBLKSZ         GET USER'S SIZE
         CH    R0,=H'32760'       NEED EXCP ?
         BNH   OPFIXMD0             No; use BSAM
         ST    R0,DWORK+4         Increase max size
         ST    R0,ZPLRECL         ALSO RECORD LENGTH
         MVI   FILEMODE,2         FORCE RECFM=U
         SPACE 1
OPFIXMD2 IC    R4,WWORK           Fix up
OPFIXMOD STC   R4,WWORK           Save to storage
         MVC   IOMFLAGS,WWORK     Save for duration
         SPACE 1
*---------------------------------------------------------------------*
*   Do as much common code for input and output before splitting
*   Set mode flag in Open/Close list
*   Move BSAM, QSAM, or EXCP DCB to work area
*---------------------------------------------------------------------*
         STC   R4,OPENCLOS        Initialize MODE=24 OPEN/CLOSE list
         NI    OPENCLOS,X'07'        For now
*                  OPEN mode: IN OU UP AP IO OI
         TR    OPENCLOS(1),=X'80,8F,84,8E,83,86,0,0'
         CLI   OPENCLOS,0         Unsupported ?
         LA    R0,ORFBDMOD        Preset bad mode
         BE    OPRERR               Yes; fail
         SPACE 1
         TM    WWORK,IOFEXCP      (TAPE) EXCP mode ?
         BZ    OPQRYBSM             No
         L     R15,=A(EXCPDCB)    Point to DCB/IOB/CCW
         MVC   ZDCBAREA(EXCPDCBL),0(R15)   Move DCB/IOB/CCW
         LA    R15,TAPEIOB   FOR EASIER SETTINGS
         USING IOBSTDRD,R15
         MVI   IOBFLAG1,IOBDATCH+IOBCMDCH   COMMAND CHAINING IN USE
         MVI   IOBFLAG2,IOBRRT2
         LA    R1,TAPEECB
         ST    R1,IOBECBPT
         LA    R1,TAPECCW
         ST    R1,IOBSTART   CCW ADDRESS
         ST    R1,IOBRESTR   CCW ADDRESS
         LA    R1,TAPEDCB
         ST    R1,IOBDCBPT   DCB
         LA    R1,TAPEIOB
         STCM  R1,7,DCBIOBAA LINK IOB TO DCB FOR DUMP FORM.ING
         LA    R0,1          SET BLOCK COUNT INCREMENT
         STH   R0,IOBINCAM
         DROP  R15
         B     OPREPCOM
         SPACE 1
OPQRYBSM TM    WWORK,IOFBLOCK     Block mode ?
*DEFUNCT BNZ   OPREPBSM
*DEFUNCT TM    WWORK,X'01'        In or Out
*DEFUNCT BNZ   OPREPQSM
OPREPBSM CLI   DDWFLAGS,CWFPDS    PDS ONLY (NO CONCAT?)
         BE    OPREPBSQ             Yes; use for directory read
         TM    DDWFLAGS,CWFCONC+CWFSEQ   Seq. Concatenation?
         BNZ   OPREPBSQ             Yes; plain BSAM
         TM    DDWFLAGS,CWFCONC+CWFPDQ   Concatenation pds(MEM)
         BNZ   OPREPBSQ             YES; plain BSAM
         CLI   DDWFLAGS,CWFPDQ    Single PDS with member?
         BNE   OPREPBAM           Test PDS concatenation
         CLI   WWORK+1,0          Plain input ?
         BNE   OPREPBSQ             No; leave it
         B     OPREPBCM             Yes; will verify member
OPREPBAM CLI   DDWFLAGS,X'90'     PDS CONCATENATION?
         BNE   OPREPBSQ             NO
         OI    IOMFLAGS,IOFBPAM   Use BPAM logic
         MVC   ZDCBAREA(BPAMDCBL),BPAMDCB  Move DCB template +
         MVC   ZDCBAREA+BPAMDCBL(READDUML),READDUM    DECB
         MVI   DCBRECFM,X'C0'     FORCE xSAM TO IGNORE
         B     OPREPCOM
OPREPBCM MVC   ZDCBAREA(BPAMDCBL),BPAMDCB  Move DCB template +
         MVC   ZDCBAREA+BPAMDCBL(READDUML),READDUM    DECB
         B     OPREPCOM
OPREPBSQ MVC   ZDCBAREA(BSAMDCBL),BSAMDCB  Move DCB template to work
         MVC   ZDCBAREA+BPAMDCBL(READDUML),READDUM    DECB
         TM    DWORK+2,UCB3DACC+UCB3TAPE    Tape or Disk ?
         BM    OPREPCOM           Either; keep RP,WP
         NC    DCBMACR(2),=AL1(DCBMRRD,DCBMRWRT) Strip Point
         B     OPREPCOM
         SPACE 1
*PREPQSM MVC   ZDCBAREA(QSAMDCBL),QSAMDCB   *> UNUSED <*
OPREPCOM MVC   DCBDDNAM,ZDDN          WAS 0(R3)
         MVC   DEVINFO(8),DWORK   Check device type
         ICM   R14,15,DEVINFO+4   Any ?
         LA    R0,ORFNODD         Set for no DD
         BZ    OPRERR             No DD card or ?
         N     R4,=X'000000EF'    Reset block mode
         TM    DDWFLAGS,CWFVSM    VSAM ACCESS ?
         BNZ   OPDOVSAM
         TM    DDWFLAGS,CWFVTOC   VTOC ACCESS ?
         BNZ   OPDOVTOC             USE STORAGE ONE
         TM    WWORK,IOFTERM      Terminal I/O?
         BNZ   OPFIXMOD
         TM    WWORK,IOFBLOCK           Blocked I/O?
         BZ    OPREPJFC
         CLI   DEVINFO+2,UCB3UREC Unit record?
         BE    OPFIXMOD           Yes, may not block
         SPACE 1
OPREPJFC LA    R14,MYJFCB
* EXIT TYPE 07 + 80 (END OF LIST INDICATOR)
         ST    R14,DCBXLST+4
         MVI   DCBXLST+4,X'87'    JFCB address; end of list
* While the code is meant to be assembled in S370, it may also get
* assembled in S390. Thus the EODAD must be below the line.
* For end-file processing we place a small stub in the ZDCB work area.
         LA    R1,EOFR24
         STCM  R1,B'0111',DCBEODA
         MVC   EOFR24(EOFRLEN),ENDFILE   Put EOF code below the line
* The DCB exit (OCDCBEX) is coded to work in any AMODE, and only
* needs a simple branch, UNLESS this module is loaded ATL.
         LA    R14,OCDCBEX        POINT TO DCB EXIT for BTL
         AIF   ('&ZSYS' EQ 'S370').NOSTB   Only S/380+90 needs a stub
         TM    @OCDCBEX,X'7F'     Loaded above the line?
         BZ    EXBTL                No; invoke directly
         MVC   A24STUB,PATSTUB    Stub code for DCB exit
         LA    R14,A24STUB        Switch to 24-bit stub
.NOSTB   ANOP  ,                  Only S/390 needs a stub
EXBTL    ST    R14,DCBXLST        AND SET IT BACK
         MVI   DCBXLST,X'05'      Identify Open exit address
         LA    R14,DCBXLST
         STCM  R14,B'0111',DCBEXLSA
         RDJFCB ((R10)),MF=(E,OPENCLOS)  Read JOB File Control Blk
*---------------------------------------------------------------------*
*   If the caller did not request EXCP mode, but the user has BLKSIZE
*   greater than 32760 on TAPE, then we set the EXCP bit in R4 and
*   restart the OPEN. Otherwise MVS should fail?
*   The system fails explicit BLKSIZE in excess of 32760, so we cheat.
*   The NCP field is not otherwise honored, so if the value is 32 to
*   64 inclusive, we use that times 1024 as a value (max 65535)
*---------------------------------------------------------------------*
         CLI   DEVINFO+2,UCB3TAPE TAPE DEVICE?
         BNE   OPNOTBIG           NO
         TM    WWORK,IOFEXCP      USER REQUESTED EXCP ?
         BNZ   OPVOLCNT           NOTHING TO DO
         CLI   JFCNCP,32          LESS THAN MIN ?
         BL    OPNOTBIG           YES; IGNORE
         CLI   JFCNCP,65          NOT TOO HIGH ?
         BH    OPNOTBIG           TOO BAD
*---------------------------------------------------------------------*
*   Clear DCB work area and force RECFM=U,BLKSIZE>32K
*     and restart the OPEN processing
*---------------------------------------------------------------------*
         LR    R0,R10             Load output DCB area address
         LA    R1,ZDCBLEN         Load output length
         LA    R15,0              Pad of X'00'
         MVCL  R0,R14             Clear DCB area to zeroes
         SR    R0,R0
         ICM   R0,1,JFCNCP        NUMBER OF CHANNEL PROGRAMS
         SLL   R0,10              *1024
         C     R0,=F'65535'       LARGER THAN CCW SUPPORTS?
         BL    *+8                NO
         L     R0,=F'65535'       LOAD MAX SUPPORTED
         ST    R0,ZPBLKSZ         MAKE NEW VALUES THE DEFAULT
         ST    R0,ZPLRECL         MAKE NEW VALUES THE DEFAULT
         MVI   FILEMODE,2         USE RECFM=U
         LA    R0,IOFEXCP         GET EXCP OPTION
         OR    R4,R0              ADD TO USER'S REQUEST
         B     OPCURSE            AND RESTART THE OPEN
         SPACE 1
OPVOLCNT SR    R1,R1
         ICM   R1,1,JFCBVLCT      GET VOLUME COUNT FROM DD
         BNZ   *+8                OK
         LA    R1,1               SET FOR ONE
         ST    R1,ZXCPVOLS        SAVE FOR EOV
         SPACE 1
OPNOTBIG CLI   DEVINFO+2,UCB3DACC   Is it a DASD device?
         BNE   OPNODSCB           No; no member name supported
*---------------------------------------------------------------------*
*   For a DASD resident file, get the format 1 DSCB
*---------------------------------------------------------------------*
*
         CLI   DS1FMTID,C'1'      Already done?
         BE    OPNODSCB             Yes; save time
* CAMLST CAMLST SEARCH,DSNAME,VOLSER,DSCB+44
         L     R14,CAMDUM         Get CAMLST flags
         LA    R15,JFCBDSNM       Load address of output data set name
         LA    R0,JFCBVOLS        Load addr. of output data set volser
         LA    R1,DS1FMTID        Load address of where to put DSCB
         STM   R14,R1,CAMLST      Complete CAMLST addresses
         OBTAIN CAMLST            Read the VTOC record
         MVI   OPERF,ORFNODS1     PRESET FOR BAD DSCB 1
         LTR   R15,R15            Check return
         BNZ   OPSERR               Bad; fail
         B     OPNODSCB
         SPACE 1
OPDOVTOC OPENCALL OPENVTOC   Invoke VTOC open code
         B     GETBUFF       Normal return
         SPACE 1
*---------------------------------------------------------------------*
*   VSAM:  OPEN ACB; SET RPL; SET FLAGS FOR SEQUENTIAL READ OR WRITE  *
*---------------------------------------------------------------------*
OPDOVSAM OPENCALL OPENVSAM        Transfer to the extension
         B     GETBUFF            Join common code
         SPACE 1
*---------------------------------------------------------------------*
*   Split READ and WRITE paths
*     Note that all references to DCBRECFM, DCBLRECL, and DCBBLKSI
*     have been replaced by ZPRECFM, ZPLRECL, and ZPBLKSZ for EXCP use.
*---------------------------------------------------------------------*
OPNODSCB TM    WWORK,1            See if OPEN input or output
         BNZ   OPENWRIT
         MVI   OPERF,ORFBACON     Preset invalid concatenation
         TM    DDWFLAGS,CWFDD     Concatenation ?
         BZ    READNCON             No
         TM    OPENCLOS,X'07'     Other than simple open?
         BNZ   OPSERR               Yes, fail
*---------------------------------------------------------------------*
*
* READING
*   N.B. moved RDJFCB prior to member test to allow uniform OPEN and
*        other code. Makes debugging and maintenance easier
*
*---------------------------------------------------------------------*
READNCON OI    JFCBTSDM,JFCNWRIT  Don't mess with control block
         CLI   DEVINFO+2,UCB3DACC   Is it a DASD device?
         BNE   OPENVSEQ           No; no member name supported
*---------------------------------------------------------------------*
* See if DSORG=PO but no member; use member from JFCB if one
*---------------------------------------------------------------------*
         TM    DDWFLAG2,CWFDD     Concatenation?
         BZ    OPENITST             No
         TM    DDWFLAGS,CWFSEQ+CWFPDQ  Sequential ?
         BNZ   OPENVSEQ             Yes; bypass member check
         B     OPENBCOM
         SPACE 1
OPENITST TM    DDWFLAGS,CWFSEQ    Single DD non-PDS?
         BNZ   OPENVSEQ             Yes; skip PDS stuff
         TM    DDWFLAGS,CWFPDQ    PDS with member ?
         BZ    OPENBCOM             No
         TM    WWORK,X'01'        Output?
         BNZ   OPENVSEQ             Yes; skip PDS stuff
         B     OPENBINP             Yes; check input mode
OPENBCOM TM    DS1DSORG,DS1DSGPO  See if DSORG=PO
         BZ    OPENVSEQ           Not PDS, don't read PDS directory
OPENBINP TM    WWORK,X'07'   ANY NON-READ OPTION ?
         LA    R0,ORFBDMOD          Yes; not supported
         BNZ   OPRERR               No, fail
         TM    ZMEM,255-X'40'     See if a member name
         BNZ   ZEROMEM            User member - reset JFCB
         TM    JFCBIND1,JFCPDS    See if a member name in JCL
         BZ    OPENDIR            No; read directory
         MVC   ZMEM,JFCBELNM      Save the member name
ZEROMEM  NI    JFCBIND1,255-JFCPDS    Reset it
         XC    JFCBELNM,JFCBELNM  Delete it in JFCB
         B     OPENMEM            Change DCB to BPAM PO
*---------------------------------------------------------------------*
* At this point, we have a PDS but no member name requested.
* Request must be to read the PDS directory
*---------------------------------------------------------------------*
OPENDIR  TM    OPENCLOS,X'0F'     Other than plain OPEN ?
         LA    R0,ORFBDMOD          Yes; not supported
         BNZ   OPRERR               No, fail (allow UPDAT later?)
         LA    R0,256             Set size for Directory BLock
         STH   R0,DCBBLKSI        Set DCB BLKSIZE to 256
         STH   R0,DCBLRECL        Set DCB LRECL to 256
         ST    R0,ZPLRECL
         ST    R0,ZPBLKSZ
         MVI   DCBRECFM,DCBRECF   Set DCB RECFM to RECFM=F (notU?)
         B     OPENIN
OPENMEM  TM    DDWFLAGS,CWFDD     Concatenation ?
         BZ    OPENBPAM             No; use BPAM
         TM    DDWFLAGS,CWFSEQ CWFPDQ  Rest sequential ?
         BNZ   OPENVSEQ             Must use BSAM
OPENBPAM OI    JFCBTSDM,JFCVSL    Force OPEN analysis of JFCB
         MVI   DCBDSRG1,DCBDSGPO  Replace DCB DSORG=PS with PO
         B     OPENIN
         SPACE 1
OPENVSEQ TM    ZMEM,255-X'40'     Member name for sequential?
         LA    R0,ORFBDMEM        Member not allowed
         BNZ   OPRERR             Yes, fail
         TM    DDWFLAGS,CWFSEQ+CWFPDQ  SEQUENTIAL ACCESS ?
         BZ    OPENIN               NO; SKIP CONCAT
         MVI   DCBMACR+1,0        Remove Write
         OI    DCBOFLGS,DCBOFPPC  Allow unlike concatenation
         SPACE 1
OPENIN   OPEN  MF=(E,OPENCLOS),TYPE=J  OPEN THE DATA SET
         TM    DCBOFLGS,DCBOFOPN  Did OPEN work?
         BZ    FAILDCB            OPEN failed, go return error code -37
         TM    ZMEM,255-X'40'     Member name?
         BZ    GETBUFF            No member name, no find
*  N.B. BLDL provides the concatenation number.
*
         MVC   BLDLLIST(4),=AL2(1,12+2+31*2)
         MVC   BLDLNAME,ZMEM      Copy member name
         BLDL  (R10),BLDLLIST     Try to find it
         BXH   R15,R15,OPNOMEM    See if member found
         TM    DDWFLAGS,CWFSEQ+CWFPDQ  SEQUENTIAL ACCESS ?
         BNZ   SETMTTR              YES; LEAVE DCB INTACT
*  SET USER'S DCB PARAMETERS FOR MEMBER'S PDS
*    PHYSICAL DCB WILL BE SET TO RECFM=U, MAX BLKSIZE
*
         SR    R15,R15
         IC    R15,PDS2CNCT-PDS2+BLDLNAME
         MH    R15,=AL2(DDASIZE)  Position to entry
         LA    R15,DDWATTR(R15)   Point to start of table
         USING DDATTRIB,R15       Declare table entry
         LH    R0,DDALRECL
         STH   R0,DCBLRECL
         ST    R0,ZPLRECL
         LH    R0,DDABLKSI
         ST    R0,ZPBLKSZ
         CH    R0,=H'256'         At least enough for a PDS/DIR
         BNL   *+8                  OK
         LH    R0,=H'256'         set to minimum
         STH   R0,DCBBLKSI
         SLR   R0,R0
         IC    R0,DDARECFM        Load RECFM
         STC   R0,ZRECFM
         SRL   R0,6               Keep format only
         STC   R0,FILEMODE        Store
         TR    FILEMODE,=AL1(1,1,0,2)  D,V,F,U
         MVC   RECFMIX,FILEMODE
         TR    RECFMIX,=AL1(0,4,8,8)   F,V,U,U
         MVI   DCBRECFM,X'C0'     FORCE xSAM TO IGNORE REAL RCF
         DROP  R15
         SPACE 1
SETMTTR  FIND  (R10),ZMEM,D       Point to the requested member
         BXLE  R15,R15,GETBUFF    See if member found
         SPACE 1
* If FIND return code not zero, process return and reason codes and
* return to caller with a negative return code.
OPNOMEM  LR    R2,R15             Save error code
         CLOSE MF=(E,OPENCLOS)    Close, FREEPOOL not needed
         LA    R7,2068            Set for member not found
         CH    R2,=H'8'           FIND/BLDL RC=4 ?
         BE    FREEDCB              Yes
         LA    R7,2064            Set for error
         B     FREEDCB
         SPACE 1
FAILDCB  LDVAL R4,PARM2           Reload mode
         N     R4,=F'1'           Mask other option bits
         LA    R7,37(R4,R4)       Preset OPEN error code
FREEDCB  LR    R14,R13            COPY WORK AREA ADDRESS
         LA    R15,OPENLEN-1(,R14)    LAST BYTE
         STM   R14,R15,OSNLIST
         OI    OSNLIST+4,X'80'    SET END OF LIST
         LTR   R14,R10            HAVE A ZDCBAREA ?
         BZ    OSNDONE
         LA    R15,ZDCBLEN-1(R14)    LAST BYTE
         STM   R14,R15,OSNLIST+8
         OI    OSNLIST+12,X'80'   SET END OF LIST
         NI    OSNLIST+4,X'7F'    RESET SHORT END
OSNDONE  L     R15,=A(@@SNAP)
         LA    R1,OSNAP
         BALR  R14,R15            CALL SNAPPER
         MVC   OPBADDD,DWDDNAM    SHOW WHAT
         WTO   'MVSSUPA - OPEN FAILED FOR nnnnnnnn',ROUTCDE=11
OPBADDD  EQU   *-6-8,8,C'C'       Insert bad DD
FREEOSTO LTR   R10,R10            Should never happen
         BZ    FREEDSTO             but it did during testing
         FREEMAIN R,LV=ZDCBLEN,A=(R10),SP=SUBPOOL  Free DCB area
FREEDSTO LNR   R7,R7              Set return and reason code
         B     RETURNOP           Go return to caller with negative RC
OSNLIST  DC    A(0,0,0,0)         Snap list: R13 work; R10 ZDCB
OSNHEAD  DC    A(OSNHEAD1,OSNHEAD2+X'80000000')  LABELS
OSNHEAD1 DC    AL1(OSNHEAD2-*-1),C'AOPEN Work Area:'
OSNHEAD2 DC    AL1(OSNHEAD3-*-1),C'ZDCBAREA:'
OSNHEAD3 EQU   *                  END OF HEADERS
OSNAP    SNAP  PDATA=(PSW,REGS),LIST=OSNLIST,STRHDR=OSNHEAD,MF=L
         SPACE 1
@@BUGF   DC    F'0'          DEBUGGING FLAG
         SPACE 1
*---------------------------------------------------------------------*
*   Process for OUTPUT mode
*---------------------------------------------------------------------*
OPENWRIT MVI   OPERF,ORFBACON     Preset for invalid concatenation
         TM    DDWFLAG2,CWFDD     Concatenation other than input?
         BNZ   OPSERR               Yes, fail
         TM    ZMEM,255-X'40'     Member requested?
         BZ    WNOMEM               No
         MVI   OPERF,ORFBDMEM     Preset for invalid member
         CLI   DEVINFO+2,UCB3DACC   DASD ?
         BNE   OPSERR             Member name invalid
         TM    DS1DSORG,DS1DSGPO  See if DSORG=PO
         BZ    OPSERR             Is not PDS, fail request
         MVI   OPERF,ORFBDMOD     Preset for invalid MODE
         TM    WWORK,X'06'        ANY NON-RITE OPTION ?
         BNZ   OPSERR               not allowed for PDS
         MVC   JFCBELNM,ZMEM
         OI    JFCBIND1,JFCPDS
         OI    JFCBTSDM,JFCVSL    Just in case
         B     WNOMEM2            Go to move DCB info
         SPACE 1
WNOMEM   TM    JFCBIND1,JFCPDS    See if a member name in JCL
         BO    WNOMEM2            Is member name, go to continue OPEN
* See if DSORG=PO but no member so OPEN output would destroy directory
         TM    DS1DSORG,DS1DSGPO  See if DSORG=PO
         BZ    WNOMEM2            Is not PDS, go OPEN
         MVC   BADMEMDD,ZDDN      Identify bad DD
         WTO   'MVSSUPA - Output PDS missing member name for DD nnnnnnn*
               n',ROUTCDE=11
BADMEMDD EQU   *-6-8,8,C'C'       Insert bad DD
         WTO   'MVSSUPA - Refuses to write over PDS directory',        C
               ROUTCDE=11
         ABEND 123                Abend without a dump
         SPACE 1
WNOMEM2  OPEN  MF=(E,OPENCLOS),TYPE=J
         TM    DCBOFLGS,DCBOFOPN  Did OPEN work?
         BZ    FAILDCB            OPEN failed, go return error code -39
         SPACE 1
*---------------------------------------------------------------------*
*   Acquire one BLKSIZE buffer for our I/O; and one LRECL buffer
*   for use by caller for @@AWRITE, and us for @@AREAD.
*
*   Note that the GETMAIN allows for extra padeding of 4 bytes.
*   In BLOCK mode, the "record" buffer uses the block size.
*
*---------------------------------------------------------------------*
GETBUFF  L     R5,ZPBLKSZ         Load the input blocksize
         LA    R6,4(,R5)          Add 4 in case RECFM=U buffer
         L     R0,DDWBLOCK        Load the input blocksize
         AH    R0,=H'4'           allow for BDW
         CR    R6,R0              Buffer size OK?
         BNL   *+4+2                Yes
         LR    R6,R0              Use larger
         AIF ('&ZSYS' EQ 'S370').NOBEL4
         GETMAIN RU,LV=(R6),SP=SUBPOOL,LOC=BELOW  Get input buffer
         AGO .GETFIN4
.NOBEL4  ANOP  ,
         GETMAIN RU,LV=(R6),SP=SUBPOOL  Get input buffer
.GETFIN4 ANOP  ,
         ST    R1,ZBUFF1          Save for cleanup
         ST    R6,ZBUFF1+4           ditto
         ST    R1,BUFFADDR        Save the buffer address for READ
         XC    0(4,R1),0(R1)      Clear the RECFM=U Record Desc. Word
         LA    R14,0(R5,R1)       Get end address
         ST    R14,BUFFEND          for real
         SPACE 1
         L     R6,ZPLRECL         Get record length
         TM    IOMFLAGS,IOFBLOCK  Running in block mode?
         BZ    GETBUFC              No
         L     R6,ZPBLKSZ         Use block size instead
GETBUFC  LA    R6,4(,R6)          Insurance
         AIF ('&ZSYS' EQ 'S370').NOBEL5
         GETMAIN RU,LV=(R6),SP=SUBPOOL,LOC=BELOW  Get VBS buffer
         AGO .GETFIN5
.NOBEL5  ANOP  ,
         GETMAIN RU,LV=(R6),SP=SUBPOOL  Get VBS buffer
.GETFIN5 ANOP  ,
         ST    R1,ZBUFF2          Save for cleanup
         ST    R6,ZBUFF2+4           ditto
         LA    R14,4(,R1)
         ST    R14,VBSADDR        Save the VBS read/user write
         L     R5,PARM6           Get caller's BUFFER address
         ST    R14,0(,R5)         and return work address
         AR    R1,R6              Add size GETMAINed to find end
         ST    R1,VBSEND          Save address after VBS rec.build area
         FIXWRITE ,               Initialize BDW prior to use
         B     DONEOPEN           Go return to caller with DCB info
         SPACE 1
         PUSH  USING
*---------------------------------------------------------------------*
*   Establish ZDCBAREA for either @@AWRITE or @@AREAD processing to
*   a terminal, or SYSTSIN/SYSTERM in batch.
*---------------------------------------------------------------------*
TERMOPEN MVC   IOMFLAGS,WWORK     Save for duration
         NI    IOMFLAGS,IOFTERM+IOFOUT      IGNORE ALL OTHERS
         L     R15,=A(TERMDCB)    Point to pattern
         MVC   ZDCBAREA(TERMDCBL),0(R15)   Move DCB/IOPL, etc.
         MVC   ZIODDNM,DWDDNAM    DDNAME FOR DEBUGGING, ETC.
         TM    ZMEM,255-C' '      See if a member name
         LA    R0,ORFBDMEM
         BNZ   OPRERR             Yes; fail
         L     R14,PSATOLD-PSA    GET MY TCB
         USING TCB,R14
         ICM   R15,15,TCBJSCB  LOOK FOR THE JSCB
         LA    R0,ORFNOJFC        Bad/missing system area
         BZ    OPRERR        HUH ?
         USING IEZJSCB,R15
         ICM   R15,15,JSCBPSCB  PSCB PRESENT ?
         BZ    OPRERR        HUH ?
         L     R1,TCBFSA     GET FIRST SAVE AREA
         N     R1,=X'00FFFFFF'    IN CASE AM31
         L     R1,24(,R1)         LOAD INVOCATION R1
         USING CPPL,R1       DECLARE IT
         MVC   ZIOECT,CPPLECT
         MVC   ZIOUPT,CPPLUPT
         SPACE 1
*---------------------------------------------------------------------*
*                                                                     *
*   DCB parameter processng:                                          *
*                                                                     *
*   FILEMODE  0 and 2   when LRECL not 0, use for BLKSIZE, else 255   *
*                                                                     *
*             1         when LRECL not 0, use it +4 for BLKSIZE       *
*                       when LRECL is 0, use BLKSIZE - 4              *
*                       when both are 0, use 255/259                  *
*                                                                     *
*---------------------------------------------------------------------*
         MVC   ZRECFM,FILEMODE    Requested format 0-2
         NI    ZRECFM,3           Just in case
         TR    ZRECFM,=X'8040C0C0'    Change to F / V / U
         CLI   ZRECFM,X'40'       V ?
         BE    TERMOVAR             YES
         ICM   R1,15,ZPLRECL      Did user supply an LRECL?
         BNZ   *+8                  Yes; use it
         LA    R1,255             Set reasonable (?) default
         ICM   R6,15,ZPBLKSZ      AND GET BLOCK SIZE
         BNZ   TERMOSET             HOPE IT WORKS
         LR    R6,R1              ELSE DEFAULT TO RECORD LENGTH
         B     TERMOSET             and stash the result
TERMOVAR ICM   R1,15,ZPLRECL      Get record length
         BZ    TERMOVBL             try block size
         LA    R6,4(,R1)          block is record length + 4
         B     TERMOSET             and stash the result
TERMOVBL ICM   R6,15,ZPBLKSZ      Load the input blocksize
         BZ    TERMOVFF             none; use both defaults
         LH    R1,=H'-4'          get subtrahend
         AR    R1,R6              record length = block - 4
         B     TERMOSET             and stash the result
TERMOVFF LA    R1,255             default LRECL
         LA    R6,4(,R1)          Arbitrary non-zero size
TERMOSET ST    R6,ZPBLKSZ         Return it
         ST    R1,ZPLRECL         Return it
         LA    R6,4(,R6)          Add 4 in case RECFM=U buffer
         AIF ('&ZSYS' EQ 'S370').NOBEL6
         GETMAIN RU,LV=(R6),SP=SUBPOOL,LOC=BELOW  Get input buffer
         AGO .GETFIN6
.NOBEL6  ANOP  ,
         GETMAIN RU,LV=(R6),SP=SUBPOOL  Get input buffer
.GETFIN6 ANOP  ,
         ST    R1,ZBUFF2          Save for cleanup
         ST    R6,ZBUFF2+4           ditto
         LA    R1,4(,R1)          Allow for RDW if not V
         ST    R1,BUFFADDR        Save the buffer address for READ
         L     R5,PARM6           R5 points to ZBUFF2
         ST    R1,0(,R5)          save the pointer
         XC    0(4,R1),0(R1)      Clear the RECFM=U Record Desc. Word
         MVI   ZPPIX,ZPIXTRM      SET FOR TERMINAL I/O
*DELETED POP   USING
         SPACE 1
*   Lots of code tests DCBRECFM twice, to distinguish among F, V, and
*     U formats. We set the index byte to 0,4,8 to allow a single test
*     with a three-way branch.
DONEOPEN GAMAPP ,            Return user's data in user's AMODE
         LR    R7,R10             Return DCB/file handle address
         LA    R0,IXUND
         TM    ZRECFM,DCBRECU     Undefined ?
         BO    SETINDEX           Yes
         BM    GETINDFV           No
         TM    ZRECFM,DCBRECTO    RECFM=D
         BZ    SETINDEX           No; treat as U
         B     SETINDVD
GETINDFV SR    R0,R0              Set for F
         TM    ZRECFM,DCBRECF     Fixed ?
         BNZ   SETINDEX           Yes
SETINDVD LA    R0,4               Preset for V
SETINDEX STC   R0,RECFMIX         Save for the duration
         L     R5,PARM3           Point to RECFM
         SRL   R0,2               change to major record format
         ST    R0,0(,R5)          Pass either RECFM F or V to caller
         L     R8,PARM4           R8 POINTS TO LRECL
         L     R1,ZPLRECL         Load RECFM F or V max. record length
         ST    R1,0(,R8)          Return record length back to caller
         L     R5,PARM5           POINT TO BLKSIZE
         L     R0,ZPBLKSZ         Load RECFM U maximum record length
         ST    R0,0(,R5)          Pass new BLKSIZE
         L     R5,PARM2           POINT TO MODE
         MVC   0(4,R5),ZDVTYPE    DevTyp,RecFm,IOS+IOM flags
* Finished with all but R7 (handle) now
         MVC   ZPMODE,ZDVTYPE
         MVC   ZPORG,DDADSORG-DDATTRIB+DDWATTR  SAVE ORG
         MVC   ZPOPC,DCBOPTCD     RETURN OPTCD
         SPACE 1
*---------------------------------------------------------------------*
*   Request was made for member name to be all blanks when not used   *
*     or supplied. Instead of OR'ing blanks, we do a translate.       *
*     This preserves funny characters in name (e.g., SMP data)        *
*---------------------------------------------------------------------*
         LA    R1,255             Number of TR bytes
         LA    R2,CATWORK         use 256-byte work area
OPUPPLUP STC   R1,0(R1,R2)        start at the end
         BCT   R1,OPUPPLUP          repeat until done
         MVI   CATWORK,C' '       replace x'00' by x'40'
         TR    ZMEM,CATWORK       fix it
         SPACE 1
*---------------------------------------------------------------------*
*   Latest addition - calculate number of blocks (DCB error if none)  *
*   For RECFM=FBS opened for EXTEND in record mode, position to next  *
*     record to fill out short block.                                 *
*---------------------------------------------------------------------*
         CLI   ZPPIX,ZPIXSAM      BSAM ?
         BNE   RETURNOP             No; just return
         TM    ZPDEVT,UCB3TAPE+UCB3DACC  Tape or disk?
         BNM   RETURNOP                    neither; skip rest
         TM    DDWFLAG2,CWFDD     Concatenation ?
         BNZ   RETURNOP             Yes, can't support FBS
         GAMOS ,                  (OLD note; TRKCALC)
         NOTE  (R7)
         STCM  R1,14,ZPTTR        Save initial TTR or tape blk
         CLI   ZPDEVT,UCB3DACC    Working on DASD?
         BNE   RETURNOP             No; we're done
         L     R3,DCBDEBAD-IHADCB(,R7)   Get the DEB
         N     R3,=X'00FFFFFF'    Faster than AM change?
         L     R3,DEBBASND-DEBBASIC(,R3)  Get first UCB
         MVI   DWORK,1
         MVC   DWORK+1(1),ZPKEYL+L'ZPKEYL-1    Copy key length
         MVC   DWORK+2(2),ZPBLKSZ+L'ZPBLKSZ-2    and block size
         TRKCALC FUNCTN=TRKCAP,UCB=(R3),BALANCE=0,RKDD=DWORK,          *
               REGSAVE=YES,MF=(E,TRKLIST)  Get blocks per track
         BXH   R15,R15,OPNOFIT    SIZE TOO LARGE FOR TRACK
         STC   R0,ZPBLKPT         Remeber blocks per track
         CLI   ZPRECFM,DCBRECF+DCBRECBR+DCBRECSB  RECFM=FBS?
         BNE   RETURNOP             No; done
         OPENCALL OPFBS           Go to FBS extended code
*---------------------------------------------------------------------*
*   Common return from OPEN - R7=+ handle  R7=- error in open         *
*---------------------------------------------------------------------*
RETURNOP GAMAPP
         FUNEXIT RC=(R7)          Return to caller
         SPACE 1
*---------------------------------------------------------------------*
*   Return error code in 2000 range - set in concatenation check code *
*---------------------------------------------------------------------*
OPNOFIT  LA    R0,ORFBADCB        Error code for bad DCB(BLKSI)
OPRERR   LR    R7,R0              CODE PASSED IN R0
         B     OPSERR2
OPSERR   SR    R7,R7              CLEAR FOR IC
         IC    R7,OPERF           GET ERROR FLAG
OPSERR2  LA    R7,2000(,R7)       GET INTO RANGE
         B     FREEDCB              AND RETURN WITH ERROR
*
* This is not executed directly, but copied into 24-bit storage
ENDFILE  LA    R6,1               Indicate @@AREAD reached end-of-file
         LNR   R6,R6              Make negative
         BR    R14                Return to instruction after the GET
EOFRLEN  EQU   *-ENDFILE
*
         SPACE 1
         ENTRY @@CPPL             External pointer to the CPPL           *JOAO*
@@CPPL   DS    A                  Pointer to the CPPL                    *JOAO*
         LTORG ,
         SPACE 1
*     QSAM support has been removed
* QSAMDCB changes depending on whether we are in LOCATE mode or
* MOVE mode
*   QSAM deleted to gain addressability
*SAMDCB  DCB   MACRF=P&OUTM.M,DSORG=PS,DDNAME=QSAMDCB
*SAMDCBL EQU   *-QSAMDCB
*
*
* CAMDUM CAMLST SEARCH,DSNAME,VOLSER,DSCB+44
CAMDUM   CAMLST SEARCH,*-*,*-*,*-*
CAMLEN   EQU   *-CAMDUM           Length of CAMLST Template
         ORG   CAMDUM+4           Don't need rest
CAMLOC   CAMLST NAME,*-*,,*-*       look in catalog
         SPACE 1
         ORG   CAMLOC+4           Don't need rest
         POP   USING
         SPACE 1
*---------------------------------------------------------------------*
*   Expand OPEN options for reference
*---------------------------------------------------------------------*
ADHOC    DSECT ,
OPENREF  OPEN  (BSAMDCB,INPUT),MF=L    QSAM, BSAM, any DEVTYPE
         OPEN  (BSAMDCB,OUTPUT),MF=L   QSAM, BSAM, any DEVTYPE
         OPEN  (BSAMDCB,UPDAT),MF=L    QSAM, BSAM, DASD
         OPEN  (BSAMDCB,EXTEND),MF=L   QSAM, BSAM, DASD, TAPE
         OPEN  (BSAMDCB,INOUT),MF=L          BSAM, DASD, TAPE
         OPEN  (BSAMDCB,OUTINX),MF=L         BSAM, DASD, TAPE
         OPEN  (BSAMDCB,OUTIN),MF=L          BSAM, DASD, TAPE
         SPACE 1
DDATTRIB DSECT ,
DDADSORG DS    H             DS ORG FROM JFCB OR DSCB1 (2B)
DDARECFM DS    X             RECORD FORMAT
DDAATTR  DS    X             ATTRIBUTES (UCBRPS)
DDALRECL DS    H             RECORD LENGTH
DDABLKSI DS    H             BLOCK/BUFFER SIZE
DDASIZE  EQU   *-DDATTRIB
         CSECT ,
         SPACE 2
***********************************************************************
*                                                                     *
*    OPEN DCB EXIT - if RECFM, LRECL, BLKSIZE preset, no change       *
*                     unless forced by device (e.g., unit record      *
*                     not blocked)                                    *
*                    for PDS directory read, F, 256, 256 are preset.  *
*    a) device is unit record - default U, device size, device size   *
*    b) For VS in DSCB or JFCB, do not turn on BLOCKED (IEBCOPY unld) *
*    c) all others - default to values passed to AOPEN                *
*                                                                     *
*    For FB, if LRECL > BLKSIZE, make LRECL=BLKSIZE                   *
*    For VB, if LRECL+3 > BLKSIZE, set spanned                        *
*                                                                     *
*    So, what this means is that if the DCBLRECL etc fields are set   *
*    already by MVS (due to existing file, JCL statement etc),        *
*    then these aren't changed. However, if they're not present,      *
*    then start using the "LRECL" etc previously set up by C caller.  *
*                                                                     *
*    Note that the exit runs in the caller's AMODE, and does not      *
*    switch modes (hence return is plain BR R14)                      *
*    However, if the exit is loaded above the line, the caller must   *
*    be in AM31. For S380, when the exit is below the line, any       *
*    AMODE works.   R11 is used by the BTL stub and must be preserved *
*                                                                     *
***********************************************************************
         PUSH  USING
         DROP  ,
         USING OCDCBEX,R15
OCDCBEX  LA    R12,0(,R15)        Load and clean base
         DROP  R15
         USING OCDCBEX,R12
         LR    R10,R1        SAVE DCB ADDRESS AND OPEN FLAGS
         N     R10,=X'00FFFFFF'   NO 0C4 ON DCB ACCESS IF AM31
         USING IHADCB,R10    DECLARE OUR DCB WORK SPACE
         TM    IOPFLAGS,IOFDCBEX  Been here before ?
         BZ    OCDCBX1            No; nothing on first entry
         TM    ZDDFLAGS,CWFSEQ+CWFPDQ  SEQUENTIAL ACCESS?
         BZ    OCDCBX1            No; nothing to do
         OI    IOPFLAGS,IOFCONCT  Set unlike concatenation
         OI    DCBOFLGS,DCBOFPPC  Keep them coming
         TM    DCBRECFM,X'E0'     Any RECFM?
         BZ    OCDCBX1              No; use previous
         SLR   R0,R0
         IC    R0,DCBRECFM        Load RECFM
         SRL   R0,6               Keep format only
         STC   R0,FILEMODE        Store
         TR    FILEMODE,=AL1(1,1,0,2)  D,V,F,U
         MVC   RECFMIX,FILEMODE
         TR    RECFMIX,=AL1(0,4,8,8)   F,V,U,U
         MVC   ZPLRECL+2(2),DCBLRECL
         MVC   ZPBLKSZ+2(2),DCBBLKSI
OCDCBX1  OI    IOPFLAGS,IOFDCBEX  Show exit entered
         SR    R2,R2         FOR POSSIBLE DIVIDE (FB)
         SR    R3,R3
         ICM   R3,3,DCBBLKSI   GET CURRENT BLOCK SIZE
         SR    R4,R4         FOR POSSIBLE LRECL=X
         ICM   R4,3,DCBLRECL GET CURRENT RECORD LENGTH
         NI    FILEMODE,3    MASK FILE MODE
         MVC   ZRECFM,FILEMODE   GET OPTION BITS
         TR    ZRECFM,=X'90,40,C0,C0'  0-FB  1-V   2-U
         TM    DCBRECFM,DCBRECLA  ANY RECORD FORMAT SPECIFIED?
         BNZ   OCDCBFH       YES
         CLI   DEVINFO+2,UCB3UREC  UNIT RECORD?
         BNE   OCDCBFM       NO; USE OVERRIDE
OCDCBFU  CLI   FILEMODE,0         DID USER REQUEST FB?
         BE    OCDCBFM            YES; USE IT
         OI    DCBRECFM,DCBRECU   SET U FOR READER/PUNCH/PRINTER
         B     OCDCBFH
OCDCBFM  MVC   DCBRECFM,ZRECFM
OCDCBFH  LTR   R4,R4
         BNZ   OCDCBLH       HAVE A RECORD LENGTH
         L     R4,DEVINFO+4       SET DEVICE SIZE FOR UNIT RECORD
         CLI   DEVINFO+2,UCB3UREC   UNIT RECORD?
         BE    OCDCBLH       YES; USE IT
*   REQUIRES CALLER TO SET LRECL=BLKSIZE FOR RECFM=U DEFAULT
         ICM   R4,15,ZPLRECL SET LRECL=PREFERRED BLOCK SIZE
         BNZ   *+8
         L     R4,DEVINFO+4  ELSE USE DEVICE MAX
         IC    R5,DCBRECFM   GET RECFM
         N     R5,=X'000000C0'  RETAIN ONLY D,F,U,V
         SRL   R5,6          CHANGE TO 0-D 1-V 2-F 3-U
         MH    R5,=H'3'      PREPARE INDEX
         SR    R6,R6
         IC    R6,FILEMODE   GET USER'S VALUE
         AR    R5,R6         DCB VS. DFLT ARRAY
*     DCB RECFM:       --D--- --V--- --F--- --U---
*     FILE MODE:       F V  U F V  U F  V U F  V U
         LA    R6,=AL1(4,0,-4,4,0,-4,0,-4,0,0,-4,0)  LRECL ADJUST
         AR    R6,R5         POINT TO ENTRY
         ICM   R5,8,0(R6)    LOAD IT
         SRA   R5,24         SHIFT WITH SIGN EXTENSION
         AR    R4,R5         NEW LRECL
         SPACE 1
*   NOW CHECK BLOCK SIZE
OCDCBLH  LTR   R3,R3         ANY ?
         BNZ   *+8           YES
         ICM   R3,15,ZPBLKSZ SET OUR PREFERRED SIZE
         BNZ   *+8           OK
         L     R3,DEVINFO+4  SET NON-ZERO
         C     R3,DEVINFO+4  LEGAL ?
         BNH   *+8
         L     R3,DEVINFO+4  NO; SHORTEN
         TM    DCBRECFM,DCBRECU   U?
         BO    OCDCBBU       YES
         TM    DCBRECFM,DCBRECF   FIXED ?
         BZ    OCDCBBV       NO; CHECK VAR
         DR    R2,R4
         CH    R3,=H'1'      DID IT FIT ?
         BE    OCDCBBF       BARELY
         BH    OCDCBBB       ELSE LEAVE BLOCKED
         LA    R3,1          SET ONE RECORD MINIMUM
OCDCBBF  NI    DCBRECFM,255-DCBRECBR   BLOCKING NOT NEEDED
OCDCBBB  MR    R2,R4         BLOCK SIZE NOW MULTIPLE OF LRECL
         B     OCDCBXX       AND GET OUT
*   VARIABLE
OCDCBBV  LA    R5,4(,R4)     LRECL+4
         CLI   DCBRECFM,DCBRECV+DCBRECSB   plain VS ?
         BNE   OCDCBBW         no
         MVC   ZRECFM,DCBRECFM        no block (iebcopy unload)
         B     OCDCBXX       done
OCDCBBW  CR    R5,R3         WILL IT FIT ?
         BE    OCDCBXX         Yes; exactly - leave default V
         BNH   *+8           YES
         OI    DCBRECFM,DCBRECSB  SET SPANNED
         OI    DCBRECFM,DCBRECBR  SET BLOCKED, ALSO
         B     OCDCBXX       AND EXIT
*   UNDEFINED
OCDCBBU  LR    R4,R3         FOR NEATNESS, SET LRECL = BLOCK SIZE
*   EXEUNT  (Save DCB options for EXCP compatibility in main code)
OCDCBXX  STH   R3,DCBBLKSI   UPDATE POSSIBLY CHANGED BLOCK SIZE
         STH   R4,DCBLRECL     AND RECORD LENGTH
         ST    R3,ZPBLKSZ    UPDATE POSSIBLY CHANGED BLOCK SIZE
         ST    R4,ZPLRECL      AND RECORD LENGTH
         MVC   ZRECFM,DCBRECFM    DITTO
ODCBEXRT BR    R14           RETURN TO OPEN (or via caller)
         SPACE 1
         LTORG ,
         SPACE 1
         POP   USING
         SPACE 2
         AIF   ('&ZSYS' EQ 'S370').NOSTUB  Only S/380+90 needs a stub
***********************************************************************
*                                                                     *
*    OPEN DCB EXIT - 24 bit stub                                      *
*    This code is not directly executed. It is copied below the line  *
*    It is only needed when the program resides above the line.       *
*                                                                     *
***********************************************************************
         PUSH  USING
         DROP  ,
* This code provides pattern code that is relocated to the 24-bit
* ZDCB work area, where it can be invoked in AM24; when the exit
* is above the line, this stub sets AM31 and transfer to the DCB
* exit.
*
         USING PATSTUB,R15   DECLARE BASE
         DS    0A            ENSURE MATCHING ALIGNMENT
PATSTUB  BSM   R14,0         Save caller's AMODE
         LR    R11,R14            Preserve OS return address
         LA    R14,PATRET    Set return address
         L     R15,@OCDCBEX  Load 31-bit routine address
         BSM   0,R15              Call the open exit in AM31
PATRET   LR    R14,R11            Restore OS return address
         BSM   0,R14              Return to OS in original mode
@OCDCBEX DC    A(OCDCBEX+X'80000000')  AM31 exit address
PATSTUBL EQU   *-PATSTUB
         POP   USING
         SPACE 2
.NOSTUB  ANOP  ,        Only S/390 needs a stub
***********************************************************************
*                                                                     *
*   Low access data areas - here for addressability                   *
*                                                                     *
***********************************************************************
         DS    0D
BPAMDCB  DCB   MACRF=(R,W),DSORG=PO,DDNAME=BPAMDCB, input and output   *
               EXLST=1-1          DCB exits added later
BPAMDCBL EQU   *-BPAMDCB
         SPACE 1
         DS    0D
BSAMDCB  DCB   MACRF=(RP,WP),DSORG=PS,DDNAME=BSAMDCB, input and output *
               EXLST=1-1          DCB exits added later
BSAMDCBL EQU   *-BSAMDCB
READDUM  READ  NONE,              Read record Data Event Control Block *
               SF,                Read record Sequential Forward       *
               ,       (R10),     Read record DCB address              *
               ,       (R8),      Read record input buffer             *
               ,       (R9),      Read BLKSIZE or 256 for PDS.Directory*
               MF=L               List type MACRO
READDUML EQU   *-READDUM
         SPACE 1
         DS    0D
EXCPDCB  DCB   DDNAME=EXCPDCB,MACRF=E,DSORG=PS,REPOS=Y,BLKSIZE=0,      *
               DEVD=TA,EXLST=1-1,RECFM=U
         DC    8XL4'0'         CLEAR UNUSED SPACE
         ORG   EXCPDCB+84    LEAVE ROOM FOR DCBLRECL
         DC    F'0'          VOLUME COUNT
PATCCW   CCW   1,2-2,X'40',3-3
         ORG   ,
EXCPDCBL EQU   *-EXCPDCB     PATTERN TO MOVE
         SPACE 1
TERMDCB  PUTLINE MF=L        PATTERN FOR TERMINAL I/O
TERMDCBL EQU   *-TERMDCB     SIZE OF IOPL
***********************************************************************
*                                                                     *
*    AOPEN SUBROUTINES.                                               *
*    Code moved here to gain addressability.                          *
*                                                                     *
***********************************************************************
*   For VTOC access, we need to use OBTAIN and CAMLST for MVS 3.x     *
*     access, as CVAF services aren't available.
***********************************************************************
OPENVTOC OSUBHEAD ,          Define extended entry
         LA    R0,ORFBDMOD        Preset for bad mode
         CLI   WWORK,0            Plain input ?
         OBRAN OPRERR,OP=BNE        No; fail
         LA    R0,ORFBACON        Preset invalid concatenation
         TM    DDWFLAG2,CWFDD     Concatenation ?
         OBRAN OPSERR,OP=BNZ        Yes, fail
         USING UCBOB,R7
         L     R14,=A(CAMDUM)     GET FLAGS IN FIRST WORD
         L     R14,0(,R14)
         LA    R15,JFCBDSNM       POINT TO DSN
         LA    R0,UCBVOLI         POINT TO SERIAL
         LA    R1,DS1FMTID        POINT TO RETURN
         STM   R14,R1,CAMLST
         MVI   OPERF,ORFNODS1     PRESET FOR BAD DSCB 1
         OBTAIN CAMLST       READ DSCB1
         N     R15,=X'000000F7'   Other than 0 or 8?
         OBRAN OPSERR,OP=BNZ        Too bad
         MVC   ZVLOCCHH(L'ZVLOCCHH+L'ZVHICCHH),DS4VTOCE+2
         MVC   ZVUSCCHH,ZVLOCCHH  Set for scan start
         MVI   ZVUSCCHH+L'ZVUSCCHH-1,0 Start with fmt 4 again
         MVC   ZVHICCHH+L'ZVHICCHH-1(1),DS4DEVDT   end record
         MVC   ZVCPVOL(L'ZVCPVOL+L'ZVTPCYL),DS4DEVSZ  Sizes
         MVC   ZVHIREC,DS4DEVDT   DSCBS PER TRACK
         MVI   ZRECFM,X'80'       Set RECFM=F
         MVI   DCBRECFM,X'80'     Set RECFM=F
         LA    R0,DS1END-DS1DSNAM  DSCB size (44 key + 96 data)
         ST    R0,ZPBLKSZ         Treat key as part of data
         ST    R0,ZPLRECL
         STH   R0,DCBBLKSI
         STH   R0,DCBLRECL
         MVI   ZPPIX,ZPIXVTC      Set for VTOC I/O
         MVC   ZVSER,UCBVOLI      Remember the serial
         OSUBRET ROUTE=      Return from extended entry
         POP   USING
         SPACE 2
***********************************************************************
*   VSAM OPEN support                                                 *
***********************************************************************
OPENVSAM OSUBHEAD ,          Define extended entry
         LA    R0,ORFBDMOD        Preset for bad mode
         TM    WWORK,X'06'        Plain input or output?
         OBRAN OPRERR,OP=BNZ        No; fail for now
         MVI   ZRECFM,X'C0'       Set RECFM=U
         MVI   RECFMIX,X'C0'        and access code
         OI    IOSFLAGS,IOFVSAM   Use VSAM logic
         MVI   ZPPIX,ZPIXVSM      Set for VSAM I/O
         LA    R0,ORFBACON        Preset invalid concatenation
         TM    DDWFLAG2,CWFDD     Concatenation ?
         OBRAN OPSERR,OP=BNZ        Yes, fail
         MVC   ZAACB(VSAMDCBL),VSAMDCB BUILD ACB
         MVC   ACBDDNM-IFGACB+ZAACB(L'ACBDDNM),ZDDN
         MVC   ZARPL(VSAMRPLL),VSAMRPL BUILD ACB
         OPEN  ((R10)),MF=(E,OPENCLOS)
         SR    R7,R7
         IC    R7,ACBERFLG-IFGACB+ZAACB  LOAD ERROR FLAG
         LA    R7,1000(,R7)       Get into the 3000 range
         LTR   R15,R15            Did it open?
         OBRAN OPSERR2,OP=BNZ     NO; TOO BAD
         SHOWCB ACB=(R10),AREA=(S,ZPKEYL),LENGTH=12,                   *
               FIELDS=(KEYLEN,RKP,LRECL),                              *
               MF=(G,ZASHOCB,ZASHOCBL)
         LTR   R15,R15           Did it work?
         OBRAN FAILDCB,OP=BNZ      No
         L     R0,ZPLRECL
         ST    R0,ZPBLKSZ         Treat key as part of data
         AH    R0,=H'4'           allow for a little extra
         ST    R0,DDWBLOCK        Set the input blocksize
         XC    ZARRN,ZARRN   CLEAR, JUST IN CASE
         LA    R0,ZARRN
         ST    R0,ZAARG      SET RRN/KEY TO NULL
         LA    R2,ZARPL      Save RPL address
         MODCB RPL=(R2),ACB=(R10),OPTCD=(KEY,LOC,SEQ),                 *
               MF=(G,ZAMODCB,ZAMODCBL)
         LA    R3,ZAARG
         ICM   R15,15,ZPKEYL TEST KEY LENGTH
         BZ    OPDOVMOD      PROCEED
         MODCB RPL=(R2),ARG=(R3),        OPTCD=(KEY,SEQ),              *
               MF=(G,ZAMODCB)
OPDOVMOD TM    WWORK,1            Output?
         OBRAN GETBUFF,OP=BZ        No; get buffer
         MODCB RPL=(R2),OPTCD=(ADR,SEQ,NUP,LOC),ARG=(R3),              *
               MF=(G,ZAMODCB)     Set for write
         OSUBRET ROUTE=      Return from extended entry
VECTOR   OSUBRET ROUTE=(14)  Return from extended entry
         SPACE 1
VSAMDCB  ACB   DDNAME=VSAMDCB,EXLST=EXLSTACB,                          *
               MACRF=(SEQ)
VSAMDCBL EQU   *-VSAMDCB
VSAMRPL  RPL   ACB=VSAMDCB,OPTCD=(SEQ,LOC,SYN)  read or write
VSAMRPLL EQU   *-VSAMRPL
EXLSTACB EXLST AM=VSAM,EODAD=VSAMEOD,LERAD=VLERAD,SYNAD=VSYNAD
         SPACE 1
         POP   USING
         SPACE 2
***********************************************************************
*                                                                     *
*   Support for RECFM=FBS EXTEND in record mode.                      *
*   1) Point to the last record and read it in                        *
*   2) When the last block is full, just do normal writes             *
*   3) For a partial block, set the end address into BUFFCURR         *
*      so that the last block will be filled.                         *
*   Note that R7 = R10                                                *
*                                                                     *
*   Despite the fact that the DCB has (RP,WP), the support routines   *
*     treat a READ request as a formatting WRITE.                     *
*     To get around this, we issue an EXCP Read Count/Key/Data        *
*     instead, by clobbering the Write CCW and restoring it after.    *
*                                                                     *
***********************************************************************
OPFBS    OSUBHEAD ,          Define extended entry
         CLI   ZPMODE+3,ZPMAPP    Record mode append?
         BNE   OPFBSEX              No; return
         LA    R0,24              Preset for invalid DSORG
         TM    DDWFLAG1,CWFSEQ    True sequential not PDS(mem)
         OBRAN OPRERR,OP=BZ,EXIT=OPFBSER  Extend PDS member ?
         TM    DDWFLAG1,CWFPDQ+CWFPDS+CWFVSM+CWFVTOC  Other ?
         OBRAN OPRERR,OP=BNZ,EXIT=OPFBSER  non-sequential?
         SLR   R2,R2              Clear low byte
         ICM   R2,14,ZPTTR   Just in case - get last block TTR
         ST    R2,ZWORK           Remember for a while
         BZ    OPFBSEX              New/Old not append?
         POINT (R10),ZWORK
         GAMOS ,                  Get low on S380
         L     R5,DCBIOBA-IHADCB(,R10)  Get IOB prefix
         LA    R5,8(,R5)          Skip prfix
         USING IOBSTDRD,R5        Give assembler IOB base
         L     R8,BUFFADDR
         L     R9,ZPBLKSZ
         LA    R4,48(,R5)
         LA    R0,8
OPFBSLP  CLI   0(R4),X'1D'        WRITE CKD?
         BE    OPFBSBP
         LA    R4,8(,R4)
         BCT   R0,OPFBSLP
         B     OPFBSEX              HUH?
OPFBSBP  MVC   DWORK(16),0(R4)      MOVE WRITE CKD
         MVC   0(16,R4),=X'1E000000,80000008,00000000,20000000'
         LA    R1,DWDDNAM
         STCM  R1,7,1(R4)
         STCM  R8,7,9(R4)
         STCM  R9,3,8+6(R4)
         L     R2,IOBECBPT
         EXCP  (R5)
         L     R14,IOBCSW-1
         SH    R14,=H'8'
         MVC   0(16,R4),DWORK     MOVE WRITE CKD
         WAIT  ECB=(R2)           Wait for READ to complete
         GAMAPP ,                 Restore caller's mode
         SLR   R1,R1              Clear residual amount work register
         ICM   R1,B'0011',IOBCSW+5  Load residual count
         BZ    OPFBSFL
         SR    R9,R1              Get blocklen
         A     R9,BUFFADDR
         ST    R9,BUFFCURR
         OI    IOPFLAGS,IOFLDATA  Write pending
         B     OPFBSPT
OPFBSFL  ICM   R1,15,ZWORK        Restore TTR for full block
         AL    R1,=X'00000100'    Add one to record number
         ST    R1,ZWORK           tentatively use
         CLM   R1,2,ZPBLKPT       Legal block?
         BL    OPFBSPT              yes
         AL    R1,=X'00010000'    Go to new track
         BO    OP2BIG               more than 65K tracks
         ICM   R1,2,=X'01'        Reset to record 1
         ST    R1,ZWORK           tentatively use
OPFBSPT  POINT (R10),ZWORK
OPFBSEX  OSUBRET ,                Finish OPEN return
OP2BIG   LA    R0,ORFTOBIG        Invalid TTR
         OBRAN OPRERR,OP=B,EXIT=OPFBSER  No extensio possible
OPFBSER  OSUBRET ROUTE=(R14)      Take error return
         SPACE 1
         POP   USING
         SPACE 2
***********************************************************************
*                                                                     *
*  ALINE - See whether any more input is available                    *
*     R15=0 EOF     R15=1 More data available                         *
*                                                                     *
***********************************************************************
@@ALINE  FUNHEAD IO=YES,SAVE=(WORKAREA,WORKLEN,SUBPOOL)
*NO      FIXWRITE ,
         TM    IOMFLAGS,IOFTERM   Terminal Input?
         BNZ   ALINEYES             Always one more?
         LR    R2,R10        PASS DCB
         LA    R3,KEPTREC
         LA    R4,KEPTREC+4
         STM   R2,R4,DWORK   BUILD PARM LIST
         LA    R15,@@AREAD
         LA    R1,DWORK
         BALR  R14,R15       GET NEXT RECORD
         SR    R15,R15       SET EOF FLAG
         LTR   R6,R6         HIT EOF ?
         BM    ALINEX        YES; RETURN ZERO
         OI    IOPFLAGS,IOFKEPT   SHOW WE'RE KEEPING A RECORD
ALINEYES LA    R15,1         ELSE RETURN ONE
ALINEX   FUNEXIT RC=(R15)
         SPACE 2
***********************************************************************
*                                                                     *
*  AREAD - Read from an open data set                                 *
*                                                                     *
*    R15 = 0  Record or block read; address and size returned         *
*    R15 = -1 Encountered End-of-File - no data returned              *
*    R15 = 4  Encountered new DD in unlike concatenation. No data     *
*               returned. Next read will be from new DD.              *
*                                                                     *
***********************************************************************
@@AREAD  FUNHEAD IO=YES,SAVE=SAVEADCB,US=NO          READ / GET
         L     R3,4(,R1)  R3 points to where to store record pointer
         L     R4,8(,R1)  R4 points to where to store record length
         SR    R0,R0
         ST    R0,0(,R3)          Return null in case of EOF
         ST    R0,0(,R4)          Return null in case of EOF
         TM    IOPFLAGS,IOFLEOF   Prior EOF ?
         BNZ   READEOD            Yes; don't abend
         SR    R6,R6              No EOF
         TM    IOPFLAGS,IOFKEPT   Saved record ?
         BZ    READREAD           No; go to read
         LM    R8,R9,KEPTREC      Get prior address & length
         ST    R8,0(,R3)          Set address
         ST    R9,0(,R4)            and length
         XC    KEPTREC(8),KEPTREC Reset record info
         NI    IOPFLAGS,255-IOFKEPT   Reset flag
         B     READGOOD
         SPACE 1
READREAD SLR   R15,R15
         IC    R15,ZPPIX          Branch by read type
         B     *+4(R15)
           B   REREAD               xSAM
           B   REREAD               QSAM (reserved)
           B   VSAMREAD             VSAM
           B   VTOCREAD             VTOC read
           B   TGETREAD             Terminal
         SPACE 1
*   Return here for end-of-block or unlike concatenation
*
REREAD   ICM   R8,B'1111',BUFFCURR  Load address of next record
         BNZ   DEBLOCK            Block in memory, go de-block it
         L     R8,BUFFADDR        Load address of input buffer
         L     R9,ZPBLKSZ         Load block size to read
         CLI   RECFMIX,IXVAR      RECFM=Vxx ?
         BE    READ               No, deblock
         LA    R8,4(,R8)          Room for fake RDW
READ     DS    0H
         TM    IOMFLAGS,IOFEXCP   EXCP mode?
         BZ    READBSAM           No, use BSAM
*---------------------------------------------------------------------*
*   EXCP read
*---------------------------------------------------------------------*
READEXCP STCM  R8,7,TAPECCW+1     Read buffer
         STH   R9,TAPECCW+6         max length
         MVI   TAPECCW,2          READ
         MVI   TAPECCW+4,X'20'    SILI bit
         EXCP  TAPEIOB            Read
         WAIT  ECB=TAPEECB        wait for completion
         TM    TAPEECB,X'7F'      Good ?
         BO    EXRDOK             Yes; calculate input length
         CLI   TAPEECB,X'41'      Tape Mark read ?
         BNE   EXRDBAD            NO
         CLM   R9,3,IOBCSW+5-IOBSTDRD+TAPEIOB  All unread?
         BNE   EXRDBAD            NO
         L     R1,DCBBLKCT
         BCTR  R1,0
         ST    R1,DCBBLKCT        allow for tape mark
         OI    DCBOFLGS,X'04'     Set tape mark found
         L     R0,ZXCPVOLS        Get current volume count
         SH    R0,=H'1'           Just processed one
         ST    R0,ZXCPVOLS
         BNP   READEOD            None left - take End File
         EOV   TAPEDCB            switch volumes
         B     READEXCP           and restart
         SPACE 1
EXRDBAD  ABEND 001,DUMP           bad way to show error?
         SPACE 1
EXRDOK   SR    R0,R0
         ICM   R0,3,IOBCSW+5-IOBSTDRD+TAPEIOB
         SR    R9,R0         LENGTH READ
         BNP   BADBLOCK      NONE ?
         LTR   R6,R6              See if end of input data set
         BM    READEOD            Is end, go return to caller
         B     POSTREAD           Go to common code
         SPACE 1
*---------------------------------------------------------------------*
*   BSAM read   (also used for BPAM member read)                      *
*---------------------------------------------------------------------*
READBSAM FIXWRITE ,               For OUTIN request and UPDAT
         SR    R6,R6              Reset EOF flag
         GAMOS
         READ  DECB,              Read record Data Event Control Block C
               SF,                Read record Sequential Forward       C
               (R10),             Read record DCB address              C
               (R8),              Read record input buffer             C
               (R9),              Read BLKSIZE or 256 for PDS.DirectoryC
               MF=E               Execute a MF=L MACRO
         GAMAPP
*---------------------------------------------------------------------*
*                                                                     *
*   There is a stupid (?) error in the code. When processing unlike   *
*   PDS concatenations, the member is read correctly, but the EOF     *
*   triggers an S001 abend. To avoid this, we test prior to the CHECK *
*   and treat as an end-file.                                         *
*                                                                     *
*---------------------------------------------------------------------*
         TM    ZDDFLAGS,CWFSEQ+CWFPDQ  Sequential access ?
         BNZ   READCHEK                  Yes; handle normally
         WAIT  ECB=DECB
         CLI   DECB,X'41'         Unit exception?
         BNE   READCHEK             No; handle normally
         L     R14,DECB+16    DECIOBPT
         USING IOBSTDRD,R14       Give assembler IOB base
         CLI   IOBCSW+3,X'0D'     Unit exception?
         BE    READEOD            Treat as EOF
         CLC   =X'0C40',IOBCSW+3  Expected short?
         BNE   READCHEK             No
         MVI   DECB,X'7F'    Fake good I/O
         DROP  R14                Don't need IOB address base anymore
*                                 If EOF, R6 will be set to F'-1'
READCHEK DS    0H
         GAMOS
         CHECK DECB               Wait for READ to complete
         GAMAPP
         TM    ZPDEVT,UCB3TAPE+UCB3DACC  Tape or disk?
         BNM   READNNOT                    neither; skip rest
         GAMOS
         NOTE  (R10)              Note current position
         GAMAPP
         ST    R1,ZTTR            Save TTR0
READNNOT TM    IOPFLAGS,IOFCONCT  Did we hit concatenation?
         BZ    READUSAM           No; restore user's AM
         NI    IOPFLAGS,255-IOFCONCT   Reset for next time
         L     R5,ZPBLKSZ         Get block size
         LA    R5,4(,R5)          Alloc for any BDW
         C     R5,ZBUFF1+4        buffer sufficient?
         BNH   READUNLK             yes; keep it
         SPACE 1
*---------------------------------------------------------------------*
*   If the new concatenation requires a larger buffer, free the old
*   one and replace it by a larger one.
*---------------------------------------------------------------------*
         LM    R1,R2,ZBUFF1       Get buffer address and length
         FREEMAIN R,LV=(R2),A=(R1),SP=SUBPOOL  and free it
         L     R5,ZPBLKSZ         Load the input blocksize
         LA    R6,4(,R5)          Add 4 in case RECFM=U buffer
         AIF ('&ZSYS' EQ 'S370').NOBEL7
         GETMAIN RU,LV=(R6),SP=SUBPOOL,LOC=BELOW  Get input buffer
         AGO .GETFIN7
.NOBEL7  ANOP  ,
         GETMAIN RU,LV=(R6),SP=SUBPOOL  Get input buffer
.GETFIN7 ANOP  ,
         ST    R1,ZBUFF1          Save for cleanup
         ST    R6,ZBUFF1+4           ditto
         ST    R1,BUFFADDR        Save the buffer address for READ
         XC    0(4,R1),0(R1)      Clear the RECFM=U Record Desc. Word
         LA    R14,0(R5,R1)       Get end address
         ST    R14,BUFFEND          for real
READUNLK LA    R6,4          SET RETURN CODE FOR NEXT DS READ
         B     READEXIT           Return code 4; read next call
         SPACE 1
READUSAM DS    0H
         LTR   R6,R6              See if end of input data set
         BM    READEOD            Is end, go return to caller
         L     R14,DECB+16    DECIOBPT
         USING IOBSTDRD,R14       Give assembler IOB base
         SLR   R1,R1              Clear residual amount work register
         ICM   R1,B'0011',IOBCSW+5  Load residual count
         DROP  R14                Don't need IOB address base anymore
         SR    R9,R1              Provisionally return blocklen
         STM   R8,R9,RSNAREA      Save for SNAP
         SPACE 1
POSTREAD TM    IOMFLAGS,IOFBLOCK  Block mode ?
         BNZ   POSTBLOK           Yes; process as such
         TM    ZRECFM,DCBRECU     Also exit for U
         BNO   POSTREED
POSTBLOK ST    R8,0(,R3)          Return address to user
         ST    R9,0(,R4)          Return length to user
         STM   R8,R9,KEPTREC      Remember record info
         XC    BUFFCURR,BUFFCURR  Show READ required next call
         B     READEXIT
POSTREED LA    R6,1               Error code - bad BDW
         CLI   RECFMIX,IXVAR      See if RECFM=V
         BNE   EXRDNOTV           Is RECFM=U or F, so not RECFM=V
         ICM   R9,3,0(R8)         Get presumed block length
         C     R9,ZPBLKSZ         Valid?
         BH    BADBLOCK           No
         ICM   R0,3,2(R8)         Garbage in BDW?
         BNZ   BADBLOCK           Yes; fail
         LA    R6,2               Error code - bad RDW
         B     EXRDCOM
EXRDNOTV LA    R0,4(,R9)          Fake length
         SH    R8,=H'4'           Space to fake RDW
         STH   R0,0(0,R8)         Fake RDW
         LA    R9,4(,R9)          Up for fake RDW (F/U)
EXRDCOM  LA    R8,4(,R8)          Bump buffer address past BDW
         SH    R9,=H'4'             and adjust length to match
         BM    BADBLOCK           Oops
         ST    R8,BUFFCURR        Indicate data available
         ST    R8,0(,R3)          Return address to user
         ST    R9,0(,R4)          Return length to user
         STM   R8,R9,KEPTREC      Remember record info
         LA    R7,0(R9,R8)        End address + 1
         ST    R7,BUFFEND         Save end
         SPACE 1
         TM    IOMFLAGS,IOFBLOCK   Block mode?
         BNZ   READGOOD           Yes; exit
         TM    ZRECFM,DCBRECU     Also exit for U
         BO    READGOOD
*NEXT*   B     DEBLOCK            Else deblock
         SPACE 1
*        R8 has address of current record
DEBLOCK  CLI   RECFMIX,IXVAR      Is data set RECFM=U
         BL    DEBLOCKF           Is RECFM=Fx, go deblock it
*
* Must be RECFM=V, VB, VBS, VS, VA, VM, VBA, VBM, VSA, VSM, VBSA, VBSM
*  VBS SDW ( Segment Descriptor Word ):
*  REC+0 length 2 is segment length
*  REC+2 0 is record not segmented
*  REC+2 1 is first segment of record
*  REC+2 2 is last seqment of record
*  REC+2 3 is one of the middle segments of a record
*        R5 has address of current record
DEBLOCKV CLI   0(R8),X'80'   LOGICAL END OF BLOCK ?
         BE    REREAD        YES; DONE WITH THIS BLOCK
         LH    R9,0(,R8)     GET LENGTH FROM RDW
         CH    R9,=H'4'      AT LEAST MINIMUM ?
         BL    BADBLOCK      NO; BAD RECORD OR BAD BLOCK
         C     R9,ZPLRECL    VALID LENGTH ?
         BH    BADBLOCK      NO
         LA    R7,0(R9,R8)   SET ADDRESS OF LAST BYTE +1
         C     R7,BUFFEND    WILL IT FIT INTO BUFFER ?
         BL    DEBVCURR      LOW - LEAVE IT
         BH    BADBLOCK      NO; FAIL
         LA    R6,3          Preset for bad sdw
         SR    R7,R7         Preset for block done
DEBVCURR ST    R7,BUFFCURR        for recursion
         TM    3(R8),X'FF'   CLEAN RDW ?
         BNZ   BADBLOCK
         TM    IOPFLAGS,IOFLSDW   WAS PREVIOUS RECORD DONE ?
         BO    DEBVAPND           NO
         LH    R0,0(,R8)          Provisional length if simple
         ST    R0,0(,R4)          Return length
         ST    R0,KEPTREC+4       Remember record info
         CLI   2(R8),1            What is this?
         BL    SETCURR            Simple record
         BH    BADBLOCK           Not=1; have a sequence error
         OI    IOPFLAGS,IOFLSDW   Starting a new segment
         L     R2,VBSADDR         Get start of buffer
         MVC   0(4,R2),=X'00040000'   Preset null record
         B     DEBVMOVE           And move this
DEBVAPND CLI   2(R8),3            IS THIS A MIDDLE SEGMENT ?
         BE    DEBVMOVE           YES, PUT IT OUT
         CLI   2(R8),2            IS THIS THE LAST SEGMENT ?
         BNE   BADBLOCK           No; bad segment sequence
         NI    IOPFLAGS,255-IOFLSDW  INDICATE RECORD COMPLETE
DEBVMOVE L     R2,VBSADDR         Get segment assembly area
         SR    R1,R1              Never trust anyone
         ICM   R1,3,0(R8)         Length of addition
         SH    R1,=H'4'           Data length
         LA    R0,4(,R8)          Skip SDW
         SR    R15,R15
         ICM   R15,3,0(R2)        Get amount used so far
         LA    R14,0(R15,R2)      Address for next segment
         LA    R8,0(R1,R15)       New length
         STH   R8,0(,R2)          Update RDW
         A     R8,VBSADDR         New end address
         C     R8,VBSEND          Will it fit ?
         BH    BADBLOCK
         LR    R15,R1             Move all
         MVCL  R14,R0             Append segment
         TM    IOPFLAGS,IOFLSDW    Did last segment?
         BNZ   REREAD             No; get next one
         L     R8,VBSADDR         Give user the assembled record
         SR    R0,R0
         ICM   R0,3,0(R8)         Provisional length if simple
         ST    R0,0(,R4)          Return length
         ST    R0,KEPTREC+4       Remember record info
         B     SETCURR            Done
         SPACE 2
* If RECFM=FB, bump address by lrecl
*        R8 has address of current record
DEBLOCKF L     R7,ZPLRECL         Load RECFM=F DCB LRECL
         ST    R7,0(,R4)          Return length
         ST    R7,KEPTREC+4       Remember record info
         AR    R7,R8              Find the next record address
* If address=BUFFEND, zero BUFFCURR
SETCURR  CL    R7,BUFFEND         Is it off end of block?
         BL    SETCURS            Is not off, go store it
         SR    R7,R7              Clear the next record address
SETCURS  ST    R7,BUFFCURR        Store the next record address
         ST    R8,0(,R3)          Store record address for caller
         ST    R8,KEPTREC         Remember record info
         B     READGOOD
         SPACE 1
TGETREAD L     R6,ZIOECT          RESTORE ECT ADDRESS
         L     R7,ZIOUPT          RESTORE UPT ADDRESS
         MVI   ZGETLINE+2,X'80'   EXPECTED FLAG
         GAMOS                    S380 AM24
         GETLINE PARM=ZGETLINE,ECT=(R6),UPT=(R7),ECB=ZIOECB,           *
               MF=(E,ZIOPL)
         GAMAPP                   S380 SWITCH TO AM31
         LR    R6,R15             COPY RETURN CODE
         CH    R6,=H'16'          HIT BARRIER ?
         BE    READEOD2           YES; EOF, BUT ALLOW READS
         CH    R6,=H'8'           SERIOUS ?
         BNL   READEXNG           ATTENTION INTERRUPT OR WORSE
         L     R1,ZGETLINE+4      GET INPUT LINE
*---------------------------------------------------------------------*
*   MVS 3.8 undocumented behavior: at end of input in batch execution,
*   returns text of 'END' instead of return code 16. Needs DOC fix
*---------------------------------------------------------------------*
         CLC   =X'00070000C5D5C4',0(R1)  Undocumented EOF?
         BNE   TGETNEOF
         XC    KEPTREC(8),KEPTREC Clear saved record info
         LA    R6,1
         LNR   R6,R6              Signal EOF
         B     TGETFREE           FREE BUFFER AND QUIT
TGETNEOF L     R6,BUFFADDR        GET INPUT BUFFER
         LR    R8,R1              INPUT LINE W/RDW
         LH    R9,0(,R1)          GET LENGTH
         LR    R7,R9               FOR V, IN LEN = OUT LEN
         CLI   RECFMIX,IXVAR      RECFM=V ?
         BE    TGETHAVE           YES
         BL    TGETSKPF
         SH    R7,=H'4'           ALLOW FOR RDW
         B     TGETSKPV
TGETSKPF L     R7,ZPLRECL           FULL SIZE IF F
TGETSKPV LA    R8,4(,R8)          SKIP RDW
         SH    R9,=H'4'           LENGTH SANS RDW
TGETHAVE ST    R6,0(,R3)          RETURN ADDRESS
         ST    R7,0(,R4)            AND LENGTH
         STM   R6,R7,KEPTREC      Remember record info
         ICM   R9,8,=C' '           BLANK FILL
         MVCL  R6,R8              PRESERVE IT FOR USER
         SR    R6,R6              NO EOF
TGETFREE LH    R0,0(,R1)          GET LENGTH
         ICM   R0,8,=AL1(1)       SUBPOOL 1
         FREEMAIN R,LV=(0),A=(1)  FREE SYSTEM BUFFER
         B     READEXIT
READGOOD SR    R6,R6              Set good return
         B     READEXIT           TAKE NORMAL EXIT
         SPACE 1
READEOD  OI    IOPFLAGS,IOFLEOF   Remember that we hit EOF
READEOD2 XC    KEPTREC(8),KEPTREC Clear saved record info
         NI    IOPFLAGS,255-IOFKEPT   Reset flag
         LA    R6,1
READEXNG LNR   R6,R6              Signal EOF
READEXIT DS    0H
         FUNEXIT RC=(R6)          Return to caller (0, 4, or -1)
         SPACE 1
*---------------------------------------------------------------------*
*   VSAM read
*---------------------------------------------------------------------*
VSAMREAD LA    R8,ZARPL      GET RPL ADDRESS
         LM    R5,R6,ZBUFF1  GET AVAILABLE BUFFER
         GAMOS
         MODCB RPL=(R8),AREA=(R5),AREALEN=(R6),OPTCD=(LOC),            *
               MF=(G,ZAMODCB)
         GET   RPL=(R8)           Get a record
         GAMAPP
         TM    IOPFLAGS,IOFLEOF   EOF ?
         BNZ   READEOD              Yes; get out
         BXH   R15,R15,EXRDBAD FAIL ON ERROR
*  N.B. I TRIED SHOWCB TO GET AREA & LENGTH, AND FAILED.
         L     R5,RPLAREA-IFGRPL(,R8)  RECORD ADDRESS POINTER
         L     R5,0(,R5)          GET RECORD ADDRESS
         L     R6,RPLRLEN-IFGRPL(,R8)  GET RECORD LENGTH
         ST    R5,0(,R3)          Return the block address
         ST    R6,0(,R4)            and length
         B     READGOOD
         SPACE 1
         PUSH  USING
         DROP  ,
         USING VSAMEOD,R15
         USING ZARPL,R1
VSAMEOD  OI    IOPFLAGS,IOFLEOF   Set EOF flag
         BR    R14                  return to VSAM
         POP   USING
         SPACE 1
*---------------------------------------------------------------------*
*   VSAM LERAD AND SYNAD: SET ERROR CODE AND RETURN                   *
*---------------------------------------------------------------------*
VLERAD   DS    0H
VSYNAD   LA    R15,8         DITTO
         BR    R14           RETURN TO VSAM
         SPACE 1
*---------------------------------------------------------------------*
*   VTOC read
*---------------------------------------------------------------------*
VTOCREAD CLC   ZVUSCCHH,ZVHICCHH  At or past VTOC end?
         BNL   READEOD              Yes; quit with EOF
         L     R14,PATSEEK        Get SEEK pattern
         LA    R15,ZVUSCCHH       Requested address
         LA    R0,ZVSER           Volume serial
         L     R1,ZBUFF1          Point to buffer
         ST    R1,0(,R3)          Return the block address
         LA    R2,DS1END-DS1DSNAM Length
         ST    R2,0(,R4)
         STM   R14,R1,ZVSEEK      Complete CAMLST
         SR    R14,R14            Clear for address increae
         ICM   R14,3,ZVUSCCHH     Load cylinder
         SR    R15,R15
         ICM   R15,3,ZVUSCCHH+2   Load track
         SR    R1,R1
         IC    R1,ZVUSCCHH+L'ZVUSCCHH-1     Get current record
         LA    R1,1(,R1)          Increase
         CLM   R1,1,ZVHIREC       Valid?
         BNH   VTOCSET@             Yes
         LA    R1,1               Set for new track record
         LA    R15,1(,R15)        Space to new track
         CH    R15,ZVTPCYL        Valid?
         BL    VTOCSET@             Yes
         SLR   R15,R15            Track on next cylinder
         LA    R14,1(,R14)        New cylinder
         CLM   R14,3,ZVCPVOL      Valid?
         BNL   READEOD              No; fake EOF
VTOCSET@ STH   R14,ZVUSCCHH                 New cylinder
         STH   R15,ZVUSCCHH+2               New track
         STC   R1,ZVUSCCHH+L'ZVUSCCHH-1     New record
         CLC   ZVUSCCHH,ZVHICCHH  At or past VTOC end?
         BNL   READEOD              Yes; quit with EOF
         OBTAIN ZVSEEK            Read to VTOC block
         BXLE  R15,R15,READGOOD   Normal return
         B     EXRDBAD            Abend
PATSEEK  CAMLST SEEK,*-*,*-*,*-*
         ORG   PATSEEK+4
         SPACE 1
BADBLOCK LM    R14,R15,RSNAREA    GET START/SIZE OF BLOCK
         AR    R15,R14            END + 1
         BCTR  R15,0              END
         ST    R15,RSNAREA+4      UPDATE END
         OI    RSNAREA+4,X'80'    SET END OF LIST
         L     R15,=A(@@SNAP)
         LA    R1,RSNAP
         BALR  R14,R15            CALL SNAPPER
RSNDONE  WTO   'MVSSUPA - @@AREAD - problem processing RECFM=V(bs) file*
               ',ROUTCDE=11       Send to programmer and listing
         MVC   BADBLOT+8+13(8),ZDDN    Identify file
         L     R1,=C'SRB?'
         SLL   R6,3
         SRL   R1,0(R6)
         STC   R1,BADBLOT+8+32
BADBLOT  WTO   'MVSSUPA - DD xxxxxxxx - INVALID xDW',                  *
               ROUTCDE=11         add more useful info
         ABEND 1234,DUMP          Abend U1234 and allow a dump
RSNLIST  DC    A(RSNAREA,RSNAREA+7)   1/2
RSNAREA  DC    A(0,0)             Snap list: Bad block
RSNHEAD  DC    A(RSNHEAD1+X'80000000')  LABEL
RSNHEAD1 DC    AL1(RSNHEAD2-*-1),C'RECFM=Vxx bad block'
RSNHEAD2 EQU   *                  END OF HEADERS
RSNAP    SNAP  PDATA=(PSW,REGS),LIST=RSNLIST,STRHDR=RSNHEAD,MF=L
*
         LTORG ,                  In case someone adds literals
         SPACE 2
***********************************************************************
*                                                                     *
*  AWRITE - Write to an open data set                                 *
*                                                                     *
***********************************************************************
@@AWRITE FUNHEAD IO=YES,SAVE=SAVEADCB,US=NO          WRITE / PUT
         LR    R11,R1             SAVE PARM LIST
WRITMORE NI    IOPFLAGS,255-IOFCURSE   RESET RECURSION
         L     R4,4(,R11)         R4 points to the record address
         L     R4,0(,R4)          Get record address
         L     R5,8(,R11)         R5 points to length of data to write
         L     R5,0(,R5)          Length of data to write
         SPACE 1
WRITRITE SLR   R15,R15
         IC    R15,ZPPIX          Branch by write type
         B     *+4(R15)
           B   WRITSAM              xSAM
           B   WRITSAM              QSAM (reserved)
           B   VSAMWRIT             VSAM
           B   6      ***VTOC write not supported***
           B   TPUTWRIT             Terminal
*
WRITSAM  TM    IOMFLAGS,IOFBLOCK  Block mode?
         BNZ   WRITBLK            Yes
         CLI   OPENCLOS,X'84'     Running in update mode ?
         BNE   WRITENEW           No
         LM    R2,R3,KEPTREC      Get last record returned
         LTR   R3,R3              Any?
         BNP   WRITEEX            No; ignore (or abend?)
         CLI   RECFMIX,IXVAR      RECFM=V...
         BNE   WRITUPMV           NO
         LA    R0,4               ADJUST FOR RDW
         AR    R2,R0              KEEP OLD RDW
         SR    R3,R0              ADJUST REPLACE LENGTH
         AR    R4,R0              SKIP OVER USER'S RDW
         SR    R5,R0              ADJUST LENGTH
WRITUPMV MVCL  R2,R4              REPLACE DATA IN BUFFER
         OI    IOPFLAGS,IOFLDATA  SHOW DATA IN BUFFER
         B     WRITEEX            REWRITE ON NEXT READ OR CLOSE
         SPACE 1
WRITENEW CLI   RECFMIX,IXVAR      V-FORMAT ?
         BH    WRITBLK            U - WRITE BLOCK AS IS
         BL    WRITEFIX           F - ADD RECORD TO BLOCK
         TM    IOPFLAGS,IOFLSDW   CONTINUATION ?
         BNZ   WRITESKC             YES; SKIP CHECK
         CH    R5,0(,R4)          RDW LENGTH = REQUESTED LEN?
         BNE   WRITEBAD           NO; FAIL
WRITESKC L     R8,BUFFADDR        GET BUFFER
         ICM   R6,15,BUFFCURR     Get next record address
         BNZ   WRITEVAT
         LA    R0,4
         STH   R0,0(,R8)          BUILD BDW
         LA    R6,4(,R8)          SET TO FIRST RECORD POSITION
WRITEVAT L     R9,BUFFEND         GET BUFFER END
         SR    R9,R6              LESS CURRENT POSITION
         TM    ZRECFM,DCBRECSB    SPANNED?
         BZ    WRITEVAR           NO; ROUTINE VARIABLE WRITE
         C     R5,ZPLRECL         VALID SIZE?
         BH    WRITEBAD           NO; TAKE A DIVE
         TM    IOPFLAGS,IOFLSDW   CONTINUATION ?
         BNZ   WRITEVAW           YES; DO HERE
         CR    R5,R9              WILL IT FIT AS IS?
         BNH   WRITEVAS           YES; DON'T NEED TO SPLIT
WRITEVAW CH    R9,=H'5'           AT LEAST FIVE BYTES LEFT ?
         BL    WRITEVNU           NO; WRITE THIS BLOCK; RETRY
         LR    R3,R6              SAVE START ADDRESS
         LR    R7,R9              COPY LENGTH
         CR    R7,R5              ROOM FOR ENTIRE SEGMENT ?
         BL    *+4+2              NO
         LR    R7,R5              USE ONLY WHAT'S AVAILABLE
         MVCL  R6,R4              COPY SDW + DATA
         ST    R6,BUFFCURR        UPDATE NEXT AVAILABLE
         LR    R7,R6              SAVE SECOND COPY
         SR    R6,R8              LESS START
         STH   R6,0(,R8)          UPDATE BDW
         SR    R7,R3              GET RECORD LENGTH
         STH   R7,0(,R3)          FIX RDW LENGTH
         MVC   2(2,R3),=X'0100'   SET FLAGS FOR START SEGMENT
         OI    IOPFLAGS,IOFLDATA  SHOW WRITE DATA IN BUFFER
         TM    IOPFLAGS,IOFLSDW   DID START ?
         BZ    WRITEWAY           NO; FIRST SEGMENT
         MVI   2(R3),3            SHOW MIDDLE SEGMENT
         LTR   R5,R5              DID WE FINISH THE RECORD ?
         BP    WRITEWAY           NO
         MVI   2(R3),2            SHOW LAST SEGMENT
         NI    IOPFLAGS,255-IOFLSDW-IOFCURSE  RCD COMPLETE
         XC    KEPTREC(8),KEPTREC      Reset saved address/len
         TM    DCBRECFM,DCBRECBR  BLOCKED?
         BZ    WRITPREP             No; write it now
         B     WRITEEX            DONE
WRITEWAY LA    R9,4               ALLOW FOR EXTRA RDW
         SR    R4,R9
         AR    R5,R9
         STM   R4,R5,KEPTREC      MAKE FAKE PARM LIST
         OI    IOPFLAGS,IOFLSDW   SHOW PARTIAL RECORD IN BUFFER
         B     WRITPREP           GO FOR MORE
         SPACE 1
WRITEVAR LA    R1,4(,R5)          GET RECORD + BDW LENGTH
         C     R1,ZPBLKSZ         VALID SIZE?
         BH    WRITEBAD           NO; TAKE A DIVE
         L     R9,BUFFEND         GET BUFFER END
         SR    R9,R6              LESS CURRENT POSITION
         CR    R5,R9              WILL IT FIT ?
         BH    WRITEVNU           NO; WRITE NOW AND RECURSE
WRITEVAS LR    R7,R5              IN LENGTH = MOVE LENGTH
         MVCL  R6,R4              MOVE USER'S RECORD
         ST    R6,BUFFCURR        UPDATE NEXT AVAILABLE
         SR    R6,R8              LESS START
         STH   R6,0(,R8)          UPDATE BDW
         OI    IOPFLAGS,IOFLDATA  SHOW WRITE DATA IN BUFFER
         TM    DCBRECFM,DCBRECBR  BLOCKED?
         BNZ   WRITEEX              Yes; normal. Else write
         B     WRITPREP           Write it now
         SPACE 1
WRITEVNU OI    IOPFLAGS,IOFCURSE  SET RECURSION REQUEST
         B     WRITPREP           SET ADDRESS/LENGTH TO WRITE
         SPACE 1
WRITEBAD LA    R14,0(,R4)         Get current record address
         LA    R15,0(,R5)           and length
         ST    R14,WSNLIST        Save address
         CH    R15,=H'32'         Only need first line?
         BNH   *+8
         LH    R15,=H'32'         so truncate it
         AR    R15,R14            END + 1
         BCTR  R15,0              END
         ST    R15,WSNLIST+4      UPDATE END
         OI    WSNLIST+4,X'80'    SET END OF LIST
         L     R15,=A(@@SNAP)
         LA    R1,WSNAP
         BALR  R14,R15            CALL SNAPPER
         WTO   'MVSSUPA - @@AWRITE - invalid RECFM=Vxx request',       *
               ROUTCDE=11       Send to programmer and listing
         MVC   BADBLOW+8+13(8),ZDDN    Identify file
BADBLOW  WTO   'MVSSUPA - DD xxxxxxxx',                                *
               ROUTCDE=11         add more useful info
         ABEND 002,DUMP           INVALID REQUEST
WSNLIST  DC    A(0,0)             Snap list: Bad block
WSNHEAD  DC    A(WSNHEAD1+X'80000000')  LABEL
WSNHEAD1 DC    AL1(WSNHEAD2-*-1),C'RECFM=Vxx bad record'
WSNHEAD2 EQU   *                  END OF HEADERS
WSNAP    SNAP  PDATA=(PSW,REGS),LIST=WSNLIST,STRHDR=WSNHEAD,MF=L
         SPACE 1
WRITEFIX ICM   R6,15,BUFFCURR     Get next available record
         BNZ   WRITEFAP           Not first
         L     R6,BUFFADDR        Get buffer start
         L     R7,ZPBLKSZ         Get block size
         AR    R7,R6              Make end (fixp for RDW pad)
         ST    R7,BUFFEND         Set correct end (AREAD chg?)
WRITEFAP L     R7,ZPLRECL         Record length
         ICM   R5,8,=C' '         Request blank padding
         MVCL  R6,R4              Copy record to buffer
         ST    R6,BUFFCURR        Update new record address
         OI    IOPFLAGS,IOFLDATA  SHOW DATA IN BUFFER
         TM    DCBRECFM,DCBRECBR  BLOCKED?
         BZ    WRITPREP             No; write it now
         C     R6,BUFFEND         Room for more ?
         BL    WRITEEX            YES; RETURN
WRITPREP L     R4,BUFFADDR        Start write address
         LR    R5,R6              Current end of block
         SR    R5,R4              Current length
*NEXT*   B     WRITBLK            WRITE THE BLOCK
         SPACE 1
WRITBLK  AR    R5,R4              Set start and end of write
         STM   R4,R5,BUFFADDR     Pass to physical writer
         OI    IOPFLAGS,IOFLDATA  SHOW DATA IN BUFFER
         FIXWRITE ,               Write physical earlier block
         TM    IOPFLAGS,IOFLSDW   Partial record ?
         BZ    WRITEEX              No; just return
         LM    R4,R5,KEPTREC      Residual text & length
         B     WRITENEW             Finish record
VSAMWRIT LA    R8,ZARPL      GET RPL ADDRESS
         L     R5,ZBUFF1     GET BUFFER
         MODCB RPL=(R8),AREA=(R4),AREALEN=(R5),OPTCD=(MVE),            *
               MF=(G,ZAMODCB)
         PUT   RPL=(R8)           Get a record
         BXH   R15,R15,WRITBAD FAIL ON ERROR
         CHECK RPL=(R8)      TAKE APPLICABLE EXITS
         BXLE  R15,R15,WRITEEX    Return
WRITBAD  ABEND 001,DUMP           Or set error code?
         SPACE 1
TPUTWRIT CLI   RECFMIX,IXVAR      RECFM=V ?
         BE    TPUTWRIV           YES
         L     R1,BUFFADDR        GET MY (INPUT?) BUFFER
         LA    R2,4(,R5)          LENGTH WITH RDW
         LA    R14,4(,R1)         POINT PAST RDW
         LR    R15,R5             COPY LENGTH
         MVCL  R14,R4             MOVE USER'S RECORD TO WORK
         LR    R4,R1              MOVE BUFFER ADDRESS
         LR    R5,R2              LENGTH WITH RDW
         STCM  R5,12,2(R4)        CLEAR RDW FLAGS FIELD
TPUTWRIV STH   R5,0(,R4)          FILL RDW
         STCM  R5,12,2(R4)          ZERO REST
         L     R6,ZIOECT          RESTORE ECT ADDRESS
         L     R7,ZIOUPT          RESTORE UPT ADDRESS
         GAMOS ,
         PUTLINE PARM=ZPUTLINE,ECT=(R6),UPT=(R7),ECB=ZIOECB,           *
               OUTPUT=((R4),DATA),TERMPUT=EDIT,MF=(E,ZIOPL)
         GAMAPP ,
         SPACE 1
WRITEEX  TM    IOPFLAGS,IOFCURSE  RECURSION REQUESTED?
         BNZ   WRITMORE           PROCESS REMAINING DATA
         FUNEXIT RC=0             EXIT
         SPACE 2
***********************************************************************
*                                                                     *
*  ANOTE  - Return position saved after READ/WRITE (BSAM/BPAM only)   *
*                                                                     *
***********************************************************************
@@ANOTE  FUNHEAD IO=YES,SAVE=SAVEADCB,US=NO          NOTE position
         L     R3,4(,R1)          R3 points to the return value
         TM    ZPDEVT,UCB3TAPE+UCB3DACC  Tape or disk?
         BNM   NOTECOM                     neither; skip rest
         GAMOS ,                  SET AM24 ON S380
         TM    IOMFLAGS,IOFEXCP   EXCP mode?
         BZ    NOTEBSAM           No
         L     R4,DCBBLKCT        Return block count
         B     NOTECOM
         SPACE 1
NOTEBSAM L     R4,ZTTR            Get current position
NOTECOM  GAMAPP ,
         ST    R4,0(,R3)          Return TTR0 to user
         FUNEXIT RC=0
         SPACE 2
***********************************************************************
*                                                                     *
*  APOINT - Restore the position in the data set (BSAM/BPAM only)     *
*           Note that this does not fail; it just bombs on the        *
*           next read or write if incorrect.                          *
*           In particular, when POINT is used for unlike concatenated *
*           data sets, a POINT into a different data set will cause   *
*           errors.                                                   *
*                                                                     *
***********************************************************************
@@APOINT FUNHEAD IO=YES,SAVE=SAVEADCB,US=NO          NOTE position
         L     R3,4(,R1)          R3 points to the TTR value
         L     R3,0(,R3)          Get the TTR
         ST    R3,ZWORK           Save below the line
         TM    ZPDEVT,UCB3TAPE+UCB3DACC  Tape or disk?
         BNM   POINCOM                     neither; skip rest
         FIXWRITE ,                 Write pending data
         GAMOS ,                  SET AM24 ON S380
         TM    IOMFLAGS,IOFEXCP   EXCP mode ?
         BZ    POINBSAM           No
         L     R4,DCBBLKCT        Get current position
         SR    R4,R3              Get new position's increment
         BZ    POINCOM
         BM    POINHEAD
POINBACK MVI   TAPECCW,X'27'      Backspace
         B     POINECOM
POINHEAD MVI   TAPECCW,X'37'      Forward space
POINECOM LA    R0,1
         STH   R0,TAPECCW+6
         LPR   R4,R4
POINELUP EXCP  TAPEIOB
         WAIT  ECB=TAPEECB
         BCT   R4,POINELUP
         ST    R3,DCBBLKCT
         B     POINCOM
         SPACE 1
POINBSAM POINT (R10),ZWORK        Request repositioning
POINCOM  GAMAPP ,
         NI    IOPFLAGS,255-IOFLEOF   Valid POINT resets EOF
         XC    KEPTREC(8),KEPTREC      Also clear record data
         FUNEXIT RC=0
         SPACE 2
***********************************************************************
*                                                                     *
*  ADCBA - Report the DCB parameters for an open file.                *
*    Modified for more general information retrieval:                 *
*    Call now has only two parameters -                               *
*      Parm 1 is an integer (signed 32 bit) function code             *
*      Parm 2 is the address of a user supplied return area           *
*                                                                     *
*    Function  1 - DCB parameters for current DCB (all signed 32 bit) *
*                l=28(dvtp,RECFM,IOS,IOM flgs) RecFmIndex LRECL BLKSZ *
*                  curr.buffer address next-available end-addr        *
*              2 - position information                               *
*                l=16 blocks/track first-TTR current-TTR max-tracks   *
*              3 - DD information                                     *
*                l=94 DDN concat#/totcc# DSN mem VOLSERs              *
*                                                                     *
*                                                                     *
***********************************************************************
@@ADCBA  FUNHEAD IO=YES,SAVE=SAVEADCB,US=NO          READ / GET
         LDVAL R3,4(,R1)   Get function code
         L     R4,8(,R1)   R4 points to user's return area
         CH    R3,=H'1'    Valid function?
         BL    ADCBERR       No
         BE    DCBF001       DCB attributes
         CH    R3,=H'3'    Positioning, etc?
         BL    DCBF002       Yes
         BE    DCBF003     DD information
ADCBERR  LA    R15,1
         LNR   R15,R15     Set -1 error code
         B     ADCBEXIT    Return error
         SPACE 1
*---------------------------------------------------------------------*
*  DCB parameters                                                     *
*   0 (l=1) UCB3TBYT3  1 (1)  DCBRECFM  2 (1) IOM  3 (1) IOS flags    *
*   4 (4) record index (0-F 4-V 8-U)                                  *
*   8 (4) record length                                               *
*  12 (4) block size                                                  *
*---------------------------------------------------------------------*
DCBF001  MVC   0(4,R4),ZDVTYPE    DevTyp,RecFm,IOS+IOM flags
         SLR   R0,R0
         IC    R0,RECFMIX
         SRL   R0,2               Change to flag
         ST    R0,4(,R4)          Return RECFM
         ICM   R0,3,DCBLRECL
         ST    R0,8(,R4)          Return LRECL
         ICM   R0,3,DCBBLKSI
         ST    R0,12(,R4)         Return BLKSIZE
         MVC   16(3*4,R4),BUFFADDR
         B     ADCBGOOD
         SPACE 1
*---------------------------------------------------------------------*
*  Positioning and capacity information                               *
*   0 (4) blocks per track (current DD only if concatenated)          *
*   4 (4) first TTR at OPEN or concatenation exit                     *
*   8 (4) current TTR after last read or write                        *
*  12 (4) total tracks in current DD (not total for concatenation)    *
*---------------------------------------------------------------------*
         PUSH  USING
DCBF002  SLR   R0,R0              for non-DASD or no fit
         CLI   ZPDEVT,UCB3DACC    Working on DASD?
         BNE   DCBF002B             No; we're done
         L     R3,DCBDEBAD        Get the DEB
         N     R3,=X'00FFFFFF'    Faster than AM change?
         L     R3,DEBBASND-DEBBASIC(,R3)  Get first UCB
         MVI   ZWORK,1
         MVC   ZWORK+1(1),ZPKEYL+L'ZPKEYL-1    Copy key length
         MVC   ZWORK+2(2),ZPBLKSZ+L'ZPBLKSZ-2    and block size
         GAMOS
         TRKCALC FUNCTN=TRKCAP,UCB=(R3),BALANCE=0,RKDD=ZWORK,          *
               REGSAVE=YES,MF=(E,TRKLIST)  Get blocks per track
         GAMAPP
*NEXT*   BXH   R15,R15,DCBF002B   SIZE TOO LARGE FOR TRACK
DCBF002B STC   R0,ZPBLKPT         Remember blocks per track
         IC    R0,ZPBLKPT
         ST    R0,0(,R4)          Return RECFM
         SLR   R0,R0
         ICM   R0,14,ZPTTR        Get first TTR
         ST    R0,4(,R4)          Beginning TTR
         MVC   8(4,R4),ZTTR       Current TTR
         SLR   R0,R0              Accumulator for tracks
         CLI   ZDVTYPE,UCB3DACC   DASD ?
         BNE   DCBF#TRK             no; leave tracks at 0
         L     R3,DCBDEBAD        Get DEB
         N     R3,=X'00FFFFFF'    faster than AM switches?
         USING DEBBASIC,R3        declare start of DEB proper
         SLR   R5,R5
         IC    R5,DEBNMEXT        Get extent count
         SLR   R6,R6
         IC    R6,DEBAMLNG        Get extent count
         SLR   R15,R15
         LA    R14,DEBBASND       Point to DASD data
         USING DEBDASD,R14        Declare the mapping
DCBFXLUP ICM   R15,3,DEBNMTRK     Get tracks in this extent
         AR    R0,R15             add them in
         AR    R14,R6             Next extent
         BCT   R5,DCBFXLUP          until done
DCBF#TRK ST    R0,12(,R4)         Total tracks
         B     ADCBGOOD
         POP   USING
         SPACE 1
*---------------------------------------------------------------------*
*  DD information                                                     *
*   0 (8)  DD name (original name if concatenation, not blanks)       *
*   8 (2)  current concatenation count (relative to zero)             *
*  10 (2)  total DDs in concatenation (relative to 1)                 *
*  12 (44) data set name                                              *
*  56 (8)  member name (when none, could be 8X'00' or 8X' ')          *
*  64 (30) 0-5 six byte volume serials                                *
*---------------------------------------------------------------------*
         PUSH  USING
DCBF003  MVC   0(8,R4),ZDDN
         MVC   56(8,R4),ZMEM
         L     R7,PSATOLD-PSA     Get my TCB
         L     R7,TCBTIO-TCB(,R7)   Need later
         N     R7,=X'00FFFFFF'    clean it
         USING TIOT1,R7
         SLR   R5,R5
         ICM   R5,3,DCBTIOT
         ALR   R5,R7              Get TIOT entry
         DROP  R7
         USING TIOENTRY,R5
         ICM   R1,7,TIOEJFCB      GET JFCB ADDRESS OR TOKEN
         BZ    ADCBERR              NO JFCB ?
         L     R15,=A(LOOKSWA)    GET TOKEN CONVERSION
         BALR  R14,R15            INVOKE IT
         LTR   R6,R15             LOAD AND TEST ADDRESS
         BNP   ADCBERR              Huh ???
         USING INFMJFCB,R6
         MVC   12(44,R4),JFCBDSNM
         MVC   64(5*6,R4),JFCBVOLS
*
         SLR   R9,R9
         LA    R0,TIOENTRY-TIOT1  INCREMENT TO FIRST ENTRY
         DROP  R5
         USING TIOENTRY,R7        DECLARE IT
DCBF003L AR    R7,R0              NEXT ENTRY
         ICM   R0,1,TIOELNGH      GET ENTRY LENGTH
         BZ    DCBF003F             TOO BAD
         TM    TIOESTTA,TIOSLTYP  SCRATCHED ENTRY?
         BNZ   DCBF003L             YES; IGNORE
         CLC   TIOEDDNM,ZDDN      matches current?
         BNE   DCBF003L             not yet
DCBF003C CLR   R5,R7              Our active entry?
         BE    DCBF003T             yes; have concat number
         AL    R9,=X'00010001'    Up concatentation counts
         AR    R7,R0              space to next entry
         ICM   R0,1,TIOELNGH      GET ENTRY LENGTH
         BNZ   DCBF003C
         BZ    DCBF003F             TOO BAD
*
DCBF003N CLI   TIOEDDNM,C' '      Another concatenation?
         BNE   DCBF003F             no; done
         TM    TIOESTTA,TIOSLTYP  SCRATCHED ENTRY?
         BNZ   DCBF003F             YES; IGNORE
DCBF003T AL    R9,=X'00000001'    Up total count
         AR    R7,R0              space to next entry
         ICM   R0,1,TIOELNGH      GET ENTRY LENGTH
         BNZ   DCBF003N
*NEXT    BZ    DCBF003F             TOO BAD
DCBF003F ST    R9,8(,R4)          Concatenation # (0-n/m)
         B     ADCBGOOD
         POP   USING
         SPACE 1
*---------------------------------------------------------------------*
*   Return; R15 =0 good exit   =-1 for error                          *
*---------------------------------------------------------------------*
ADCBGOOD SLR   R15,R15            Good exit
ADCBEXIT FUNEXIT RC=(R15)         Return to caller
         SPACE 2
***********************************************************************
*                                                                     *
*   This routine provided to enable cross-assembly of OS/390 & zOS    *
*   code under MVS 3.8. For a full featured system, use SWAREQ.       *
*   Caller must be in AMODE 31 under OS/390 or zOS system.            *
*                                                                     *
*   SWA LOOK-UP SUBROUTINE  (in older systems, just skips Q header    *
*    >> CALLER IN AMODE 31 FOR ATL access <<                          *
*        R1  - REQUESTED SVA ADDRESS/TOKEN; 24-BIT, RIGHT-JUSTIFIED   *
*        R15 - RETURNED SWA ADDRESS OR 0                              *
*        R14 - RETURN                                                 *
*                                                                     *
*   REVISED BY JOAO REGINATO (NOV/2020)                               *
*        TO CALL SWAREQ IF THE SWA IS ABOVE THE 2 GB BAR              *
***********************************************************************
         PUSH  USING
         DROP  R12                                                       *JOAO*
         USING LOOKSWA,R12                                               *JOAO*
LOOKSWA  SAVE  (14,12)       SAVE REGS                                   *JOAO*
         LR    R12,R15       BASE REG                                    *JOAO*
         LR    R3,R1         SVA OF JFCB                                 *JOAO*
         CNOP  0,4           FULLWORD ALIGNMENT                          *JOAO*
         BAL   R1,LOOKINIT   BR AROUND AND SET R1                        *JOAO*
         DS    18F           SAVE AREA                                   *JOAO*
*DDWSWA  SWAREQ FCODE=RL,EPA=DDWEPA,MF=L                                 *JOAO*
DDWSWA   DS    0F            SWA MANAGER PARAMETER LIST                  *JOAO*
         DC    A(DDWEPA)     ADDR OF EPA                                 *JOAO*
         DC    A(DDWFCN)     ADDR OF FUNCTION CODE                       *JOAO*
DDWEPA   DC    A(DDWSVA)     ADDR OF SVA                                 *JOAO*
DDWSVA   DS    XL28          SVA RETURN AREA                             *JOAO*
DDWFCN   DC    CL2'RL'       FUNCTION CODE                               *JOAO*
LOOKINIT ST    R13,4(,R1)    LINKAGE                                     *JOAO*
         ST    R1,8(,R13)    CONVENTION                                  *JOAO*
         LR    R13,R1        NEW SAVE AREA                               *JOAO*
         N     R3,=X'00FFFFFF' CLEAN IT
         EX    R3,EXSWAODD   SEE WHETHER IT'S AN ODD ADDRESS
         BZ    LOOKSVA       NO; HAVE ADDRESS
         SR    R15,R15       NOTHING FOUND - RETURN 0
         L     R5,PSATOLD-PSA GET TCB
         USING TCB,R5
         L     R5,TCBJSCB
*COMP*   USING IEZJSCB,R5
*COMP*   L     R5,JSCBQMPI   (NOT IN S370/380)
         ICM   R5,15,X'0F4'(R5) GET QMPI
         BZ    LOOKSWAT      NO QMPI, SKIP IT
*COMP*   USING IOPARAMS,R5
         TM    X'010'(R5),X'04' IS QMAT 64 BITS?                         *JOAO*
         BZ    LOOKNO64      BR IF NOT                                   *JOAO*
         XC    DDWSVA,DDWSVA CLEAR SVA RETURN AREA                       *JOAO*
         STCM  R3,7,DDWSVA+4 JFCB TOKEN                                  *JOAO*
*        SWAREQ MF=(E,DDWSWA),UNAUTH=YES                                 *JOAO*
         LA    R1,DDWSWA     PARAMETER LIST                              *JOAO*
         LA    R13,0(,R13)   PREPARE R13                                 *JOAO*
         L     R15,CVTPTR    GET CVT                                     *JOAO*
         L     R15,X'0128'(,R15) GET JESCT                               *JOAO*
         L     R15,X'0064'(,R15) GET JESCT EXTENSION                     *JOAO*
         L     R15,X'0058'(,R15) GET UNAUTHORIZED SWA MANAGER            *JOAO*
         BALR  R14,R15       CALL SWA MANAGER                            *JOAO*
         L     R15,DDWSVA    LOAD JFCB ADDRESS                           *JOAO*
         B     LOOKSWAT      RETURN                                      *JOAO*
         SPACE 1
*COMP*   ICM   R4,15,QMAT    QMAT BASE
LOOKNO64 ICM   R4,15,X'018'(R5) GET QMAT
         BZ    LOOKSWAT      NO QMAT, SKIP IT
*COMP*   USING QMAT,R4
         LR    R0,R3         COPY TOKEN                                  *JOAO*
         SRL   R0,16         MOVE EXTENT TO LAST BYTE
         N     R3,=XL4'FFFF' ISOLATE SVA OFFSET
         LA    R1,X'FF'      MAX QMAT EXTENTS
         NR    R0,R1         ISOLATE QMAT COUNTER
         BZ    LOOKSWAV      ZERO; CHECK QMAT VERSION
         SPACE 1
*COMP*AP ICM   R4,15,QMATNEXT NEXT QMAT EXTENT
LOOKSWAP ICM   R4,15,X'00C'(R4) NEXT QMAT EXTENT
         BZ    LOOKSWAX      NONE?
         BCT   R0,LOOKSWAP   LOOP TO FIND THE EXTENT
         SPACE 1
*COMP*AV CLI   QMATVERS,2    IS IT AN ESA4 QMAT?
LOOKSWAV CLI   X'004'(R4),2  IS IT AN ESA4 QMAT?
         BL    LOOKSWAX      NO, USE AS IS
         LA    R4,1(,R4)     ALIGN
LOOKSWAX ALR   R3,R4         ADD QMAT BASE
         L     R3,0(,R3)     GET HEADER ADDRESS
LOOKSVA  LA    R15,16(,R3)   SKIP HEADER
LOOKSWAT L     R13,4(,R13)   PREVIOUS SAVE AREA                          *JOAO*
         ST    R15,16(,R13)  SAVE RC                                     *JOAO*
         RETURN (14,12)      RETURN TO CALLER                            *JOAO*
EXSWAODD TM    =X'01',*-*    ODD ADDRESS?
         LTORG ,                                                         *JOAO*
         POP   USING
*
***********************************************************************
*                                                                     *
*  ACLOSE - Close a data set                                          *
*                                                                     *
***********************************************************************
@@ACLOSE FUNHEAD IO=YES,SAVE=(WORKAREA,WORKLEN,SUBPOOL)  CLOSE
         TM    IOMFLAGS,IOFTERM   TERMINAL I/O MODE?
         BNZ   FREEBUFF           YES; JUST FREE STUFF
         FIXWRITE ,          WRITE FINAL BUFFER, IF ONE
FREEBUFF LM    R1,R2,ZBUFF1       Look at first buffer
         LTR   R0,R2              Any ?
         BZ    FREEDBF1           No
         FREEMAIN RC,LV=(0),A=(1),SP=SUBPOOL  Free BLOCK buffer
FREEDBF1 LM    R1,R2,ZBUFF2       Look at second buffer
         LTR   R0,R2              Any ?
         BZ    FREEDBF2           No
         FREEMAIN RC,LV=(0),A=(1),SP=SUBPOOL  Free RECRD buffer
FREEDBF2 TM    IOMFLAGS,IOFTERM   TERMINAL I/O MODE?
         BNZ   NOPOOL             YES; SKIP CLOSE/FREEPOOL
         CLOSE MF=(E,OPENCLOS)
         TM    DCBBUFCA+L'DCBBUFCA-1,1      BUFFER POOL?
         BNZ   NOPOOL             NO, INVALIDATED
         SR    R15,R15
         ICM   R15,7,DCBBUFCA     DID WE GET A BUFFER?
         BZ    NOPOOL             0-NO
         FREEPOOL ((R10))
NOPOOL   DS    0H
         FREEMAIN R,LV=ZDCBLEN,A=(R10),SP=SUBPOOL
         FUNEXIT RC=0
         SPACE 2
         PUSH  USING
         DROP  ,
*---------------------------------------------------------------------*
* FIND THE ZDCBAREA FOR A PREVIOUSLY OPENED FILE (CURRENT OR HIGHER   *
*   TCB, AND RETURN ITS ADDRESS OR ZERO IN R1                         *
* THE ADDRESS OF THE DESIRED DDNAME IS POINTED TO BY R1               *
*---------------------------------------------------------------------*
         ENTRY @@AQZDCB
@@AQZDCB L     R1,0(,R1)          R1 POINTS TO THE DESIRED DD NAME
AQZDCB   STM   R14,R12,12(R13)    SAVE A BIT
         LR    R12,R15
         USING @@AQZDCB,R12
         SLR   R15,R15            PRESET FOR NO MATCH
         SLR   R4,R4              CLEAR HIGH BYTE FOR ICM
         L     R3,PSATOLD-PSA     GET CURRENT TCB
         USING TCB,R3
AQZTCBLK L     R2,TCBDEB          POINT TO FIRST DEB
         LTR   R2,R2              ANY?
         BZ    AQZUPTCB             NO; CHECK HIGHER TCB
         B     AQZDEBL2           CHECK DEB
         USING DEBBASIC,R2
AQZDEBLP ICM   R2,7,DEBDEBB       ANOTHER DEB TO CHECK?
         BZ    AQZUPTCB
AQZDEBL2 ICM   R4,7,DEBDCBB       GET DCB
         USING IHADCB,R4
         SLR   R5,R5
         ICM   R5,3,DCBTIOT       GET TIOT OFFSET
         AL    R5,TCBTIO          POINT TO TIOT ENTRY
         USING TIOENTRY,R5
         CLC   TIOEDDNM,0(R1)     WANTED DD?
         BNE   AQZDEBLP             NO; CHECK NEXT
         LR    R15,R4             RETURN DCB ADDRESS
         B     AQZDCBEX
AQZUPTCB CL    R3,TCBJSTCB        JOB STEP TCB?
         BE    AQZDCBEX                YES; NO MORE RISING
         ICM   R3,15,TCBOTC       GET HIGHER TCB
         BNZ   AQZTCBLK             AND LOOK AT ITS DEBS
AQZDCBEX ST    R15,24(,R13)       RETURN ADDRESS OR 0 IN R1
         LM    R14,R12,12(R13)    RESTORE MOST
         BR    R14                RETURN ZDCBAREA ADDRESS OR 0
         POP   USING
         SPACE 2
         PUSH  USING
         DROP  ,
*---------------------------------------------------------------------*
*  Physical Write - called by @@ACLOSE, switch from output to input
*    mode, and whenever output buffer is full or needs to be emptied.
*  Works for EXCP and BSAM. Special processing for UPDAT mode
*---------------------------------------------------------------------*
         USING IHADCB,R10    COMMON I/O AREA SET BY CALLER
TRUNCOUT B     TRUNCBEG-TRUNCOUT(,R15)   SKIP LABEL
         DC    AL1(9),CL(9)'TRUNCOUT' EXPAND LABEL
TRUNCBEG TM    IOPFLAGS,IOFLDATA   PENDING WRITE ?
         BZR   R14           NO; JUST RETURN
         STM   R14,R12,12(R13)    SAVE CALLER'S REGISTERS
         LR    R12,R15
         USING TRUNCOUT,R12
         LA    R15,ZIOSAVE2-ZDCBAREA(,R10)
         ST    R15,8(,R13)
         ST    R13,4(,R15)
         LR    R13,R15
         LM    R4,R5,BUFFADDR  START/NEXT ADDRESS
         CLI   RECFMIX,IXVAR      RECFM=V?
         BNE   TRUNLEN5
         LH    R5,0(,R4)     Get BDW length field
         CL    R5,=F'8'      Empty or invalid ?
         BNH   TRUNPOST        Yes; ignore and reset buffer
         B     TRUNTMOD           CHECK OUTPUT TYPE
TRUNLEN5 SR    R5,R4              CONVERT TO LENGTH
         BNP   TRUNPOST           NOTHING TO DO
TRUNTMOD DS    0H
         TM    IOMFLAGS,IOFEXCP   EXCP mode ?
         BNZ   EXCPWRIT           Yes
         CLI   OPENCLOS,X'84'     Update mode?
         BE    TRUNSHRT             Yes; just rewrite as is
         CLI   RECFMIX,IXVAR      RECFM=F ?
         BNL   *+8                No; leave it alone
         STH   R5,DCBBLKSI        Why do I need this?
         GAMOS ,
         WRITE DECB,SF,(R10),(R4),(R5),MF=E  Write block
         B     TRUNCHK
TRUNSHRT DS    0H
         GAMOS ,
         WRITE DECB,SF,MF=E       Rewrite block from READ
TRUNCHK  CHECK DECB
         TM    ZPDEVT,UCB3TAPE+UCB3DACC  Tape or disk?
         BNM   TRUNPOST                    neither; skip rest
         NOTE  (R10)              Note current position
         ST    R1,ZTTR            Save TTR0
         B     TRUNPOST           Clean up
         SPACE 1
EXCPWRIT STH   R5,TAPECCW+6
         STCM  R4,7,TAPECCW+1     WRITE FROM TEXT
         NI    DCBIFLGS,255-DCBIFEC   ENABLE ERP
         OI    DCBIFLGS,X'40'     SUPPRESS DDR
         STCM  R5,12,IOBSENS0-IOBSTDRD+TAPEIOB   CLEAR SENSE
         OI    DCBOFLGS-IHADCB+TAPEDCB,DCBOFLWR  SHOW WRITE
         XC    TAPEECB,TAPEECB
         EXCP  TAPEIOB
         WAIT  ECB=TAPEECB
         TM    TAPEECB,X'7F'      GOOD COMPLETION?
         BO    TRUNPOST
*NEXT*   BNO   EXWRN7F            NO
         SPACE 1
EXWRN7F  TM    IOBUSTAT-IOBSTDRD+TAPEIOB,IOBUSB7  END OF TAPE?
         BNZ   EXWREND       YES; SWITCH TAPES
         CLC   =X'1020',IOBSENS0-IOBSTDRD+TAPEIOB  EXCEEDED AWS/HET ?
         BNE   EXWRB001
EXWREND  L     R15,DCBBLKCT
         SH    R15,=H'1'
         ST    R15,DCBBLKCT       ALLOW FOR EOF 'RECORD'
         EOV   TAPEDCB       TRY TO RECOVER
         B     EXCPWRIT
         SPACE 1
EXWRB001 LA    R9,TAPEIOB    GET IOB FOR QUICK REFERENCE
         ABEND 001,DUMP
         SPACE 1
TRUNPOST XC    BUFFCURR,BUFFCURR  CLEAR
         NI    IOPFLAGS,255-IOFLDATA  Reset it
         GAMAPP
         CLI   RECFMIX,IXVAR      RECFM=V
         BL    TRUNCOEX           F - JUST EXIT
         LA    R4,4               BUILD BDW
         L     R3,BUFFADDR        GET BUFFER
         STH   R4,0(,R3)          UPDATE
         LA    R4,0(R4,R3)
         ST    R4,BUFFCURR        SET NEXT AVAILABLE
TRUNCOEX L     R13,4(,R13)
         LM    R14,R12,12(R13)    Reload all
         BR    R14
         LTORG ,
         POP   USING
         SPACE 2
***********************************************************************
*                                                                     *
*  GETM - GET MEMORY                                                  *
*    Input:  0(R1) - Address of requested amount                      *
*                                                                     *
*    Output: R15 - address of user memory or 0 if failed/invalid      *
*                 note that this is 8 higher to allow for saved       *
*                 size prefix.                                        *
*    Memory: 0(R15) - Amount obtained                                 *
*            4(R15) - Amount requested                                *
*                                                                     *
***********************************************************************
@@GETM   FUNHEAD ,
         LDINT R3,0(,R1)          LOAD REQUESTED STORAGE SIZE
         SLR   R5,R5              PRESET IN CASE OF ERROR
         LR    R4,R3              COPY ORIGINAL VALUE
*
* To reduce fragmentation, round up size to 64 byte multiple
*
         AL    R3,=A(8+(64-1))    OVERHEAD PLUS ROUNDING
         N     R3,=X'FFFFFFC0'    MULTIPLE OF 64
         AIF   ('&ZSYS' NE 'S380' AND '&ZSYS' NE 'ZARCH').NOANY
         GETMAIN RC,LV=(R3),SP=SUBPOOL,LOC=ANY
         AGO   .FINANY
.NOANY   GETMAIN RC,LV=(R3),SP=SUBPOOL
.FINANY  LTR   R15,R15            Sucessful?
         BNZ   GETMEX               No; return 0
*
* We store the amount we requested from MVS into this address
* and just below the value we return to the caller, we save
* the amount requested.
*
         STM   R3,R4,0(R1)        Gotten and requested size
         LA    R5,8(,R1)          Skip prefix
GETMEX   FUNEXIT RC=(R5)
         SPACE 2
***********************************************************************
*                                                                     *
*  FREEM - FREE MEMORY                                                *
*                                                                     *
***********************************************************************
@@FREEM  FUNHEAD ,
         LDADD R1,0(,R1)          Address of block to be freed
         SL    R1,=F'8'           Position to prefix
         L     R0,0(,R1)          Get actual size obtained
         FREEMAIN RC,LV=(0),A=(1),SP=SUBPOOL
         FUNEXIT RC=(15)
         LTORG ,
         SPACE 2
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
***********************************************************************
*                                                                     *
*  GETTZ - Get the offset from GMT in 1.048576 seconds                *
*                                                                     *
***********************************************************************
         ENTRY @@GETTZ
@@GETTZ  L     R15,CVTPTR
         L     R15,CVTTZ-CVTMAP(,R15)  GET GMT TIME-ZONE OFFSET
         BR    R14
         SPACE 2
***********************************************************************
*                                                                     *
*    CALL @@SYSTEM,(req-type,pgm-len,pgm-name,parm-len,parm),VL       *
*                                                                     *
*    "-len" fields are self-defining values in the calling list,      *
*        or else pointers to 32-bit signed integer values             *
*                                                                     *
*    "pgm-name" is the address of the name of the program to be       *
*        executed (one to eight characters)                           *
*                                                                     *
*    "parm" is the address of a text string of length "parm-len",     *
*        and may be zero to one hundred bytes (OS JCL limit)          *
*                                                                     *
*    "req-type" is or points to 1 for a program ATTACH                *
*                               2 for TSO CP invocation               *
*                                                                     *
*---------------------------------------------------------------------*
*                                                                     *
*     Author:  Gerhard Postpischil                                    *
*                                                                     *
*     This code is placed in the public domain.                       *
*                                                                     *
*---------------------------------------------------------------------*
*                                                                     *
*     Assembly: Any MVS or later assembler may be used.               *
*        Requires SYS1.MACLIB. TSO CP support requires additional     *
*        macros from SYS1.MODGEN (SYS1.AMODGEN in MVS).               *
*        Intended to work in any 24 and 31-bit environment.           *
*                                                                     *
*     Linker/Binder: RENT,REFR,REUS                                   *
*                                                                     *
*---------------------------------------------------------------------*
*     Return codes:  when R15:0 R15:1-3 has return from program.      *
*       R15 is 04806nnn  ATTACH failed                                *
*       R15 is 1400000n  PARM list error: n= 1,2, or 3 (req/pgm/parm) *
*       R15 is 80sss000 or 80000uuu Subtask ABENDED (SYS sss/User uuu)*
*                                                                     *
***********************************************************************
@@SYSTEM FUNHEAD SAVE=(SYSATWRK,SYSATDLN,78)  ISSUE OS OR TSO COMMAND
         L     R15,4(,R13)        GET CALLER'S SAVE AREA
         LA    R11,16(,R15)       REMEMBER THE RETURN CODE ADDRESS
         LR    R9,R1              SAVE PARAMETER LIST ADDRESS
         SPACE 1
         MVC   0(4,R11),=X'14000002'  PRESET FOR PARM ERROR
         LDINT R4,0(,R9)          REQUEST TYPE
         LDINT R5,4(,R9)          LENGTH OF PROGRAM NAME
         L     R6,8(,R9)          -> PROGRAM NAME
         LDINT R7,12(,R9)         LENGTH OF PARM
         L     R8,16(,R9)         -> PARM TEXT
         SPACE 1
*   NOTE THAT THE CALLER IS EITHER COMPILER CODE, OR A COMPILER
*   LIBRARY ROUTINE, SO WE DO MINIMAL VALIDITY CHECKING
*
*   EXAMINE PROGRAM NAME LENGTH AND STRING
*
         CH    R5,=H'8'           NOT TOO LONG ?
         BH    SYSATEXT           TOO LONG; TOO BAD
         SH    R5,=H'1'           LENGTH FOR EXECUTE
         BM    SYSATEXT           NONE; OOPS
         MVC   SYSATPGM(L'SYSATPGM+L'SYSATOTL+1),=CL11' '  PRE-BLANK
         EX    R5,SYSAXPGM        MOVE PROGRAM NAME
         CLC   SYSATPGM,=CL11' '  STILL BLANK ?
         BE    SYSATEXT           YES; TOO BAD
*   BRANCH AND PROCESS ACCORDING TO REQUEST TYPE
*
         MVI   3(R11),1           SET BAD REQUEST TYPE
         CH    R4,=H'2'           CP PROGRAM ATTACH ?
         BE    SYSATCP            YES
         CH    R4,=H'1'           OS PROGRAM ATTACH ?
         BNE   SYSATEXT           NO; HAVE ERROR CODE
*   OS PROGRAM ATTACH - PREPARE PARM, ETC.
*
*   NOW LOOK AT PARM STRING
         LTR   R7,R7              ANY LENGTH ?
         BM    SYSATEXT           NO; OOPS
         STH   R7,SYSATOTL        PASS LENGTH OF TEXT
         BZ    SYSATNTX
         CH    R7,=AL2(L'SYSATOTX)  NOT TOO LONG ?
         BH    SYSATEXT           TOO LONG; TOO BAD
         BCTR  R7,0
         EX    R7,SYSAXTXT        MOVE PARM STRING
SYSATNTX LA    R1,SYSATOTL        GET PARAMETER ADDRESS
         ST    R1,SYSATPRM        SET IT
         OI    SYSATPRM,X'80'     SET END OF LIST BIT
         B     SYSATCOM           GO TO COMMON ATTACH ROUTINE
*   TSO CP REQUEST - PREPARE PARM, CPPL, ETC.
*
SYSATCP  LTR   R7,R7              ANY LENGTH ?
         BM    SYSATEXT           NO; OOPS
         LA    R1,SYSATOTX-SYSATOPL(,R7)  LENGTH WITH HEADER
         STH   R1,SYSATOPL        PASS LENGTH OF COMMAND TEXT
         LA    R1,1(,R5)          BYTE AFTER COMMAND NAME
         STH   R1,SYSATOPL+2      LENGTH PROCESSED BY PARSER
         BZ    SYSATXNO
         CH    R7,=AL2(L'SYSATOTX)  NOT TOO LONG ?
         BH    SYSATEXT           TOO LONG; TOO BAD
         BCTR  R7,0
         EX    R7,SYSAXTXT        MOVE PARM STRING
SYSATXNO LA    R1,SYSATOPL        GET PARAMETER ADDRESS
         ST    R1,SYSATPRM        SET IT
*   TO MAKE THIS WORK, WE NEED THE UPT, PSCB, AND ECT ADDRESS.
*   THE FOLLOWING CODE WORKS PROVIDED THE CALLER WAS INVOKED AS A
*   TSO CP, USED NORMAL SAVE AREA CONVENTIONS, AND HASN'T MESSED WITH
*   THE TOP SAVE AREA.
         MVI   3(R11),4           SET ERROR FOR BAD CP REQUEST
         LA    R2,SYSATPRM+8      CPPLPSCB
         EXTRACT (R2),FIELDS=PSB  GET THE PSCB
         PUSH  USING
         L     R1,PSATOLD-PSA     GET THE CURRENT TCB
         USING TCB,R1
         L     R1,TCBFSA          GET THE TOP LEVEL SAVE AREA
         N     R1,=X'00FFFFFF'    KILL TCBIDF BYTE
         POP   USING
         L     R1,24(,R1)         ORIGINAL R1
         LA    R1,0(,R1)            CLEAN IT
         LTR   R1,R1              ANY?
         BZ    SYSATEXT           NO; TOO BAD
         TM    0(R1),X'80'        END OF LIST?
         BNZ   SYSATEXT           YES; NOT CPPL
         TM    4(R1),X'80'        END OF LIST?
         BNZ   SYSATEXT           YES; NOT CPPL
         TM    8(R1),X'80'        END OF LIST?
         BNZ   SYSATEXT           YES; NOT CPPL
         CLC   8(4,R1),SYSATPRM+8   MATCHES PSCB FROM EXTRACT?
         BNE   SYSATEXT           NO; TOO BAD
         MVC   SYSATPRM+4(3*4),4(R1)  COPY UPT, PSCB, ECT
         L     R1,12(,R1)
         LA    R1,0(,R1)     CLEAR EOL BIT IN EITHER AMODE
         LTR   R1,R1         ANY ADDRESS?
         BZ    SYSATCOM      NO; SKIP
         PUSH  USING         (FOR LATER ADDITIONS?)
         USING ECT,R1        DECLARE ECT
         LM    R14,R15,SYSATPGM   GET COMMAND NAME
         LA    R0,7          MAX TEST/SHIFT
SYSATLCM CLM   R14,8,=CL11' '  LEADING BLANK ?
         BNE   SYSATLSV      NO; SET COMMAND NAME
         SLDL  R14,8         ELIMINATE LEADING BLANK
         IC    R15,=CL11' '  REPLACE BY TRAILING BLANK
         BCT   R0,SYSATLCM   TRY AGAIN
SYSATLSV STM   R14,R15,ECTPCMD
         NI    ECTSWS,255-ECTNOPD      SET FOR OPERANDS EXIST
         EX    R7,SYSAXBLK   SEE IF ANY OPERANDS
         BNE   SYSATCOM           HAVE SOMETHING
         OI    ECTSWS,ECTNOPD     ALL BLANK
         POP   USING
SYSATCOM LA    R1,SYSATPRM        PASS ADDRESS OF PARM ADDRESS
         LA    R2,SYSATPGM        POINT TO NAME
         LA    R3,SYSATECB        AND ECB
         ATTACH EPLOC=(R2),       INVOKE THE REQUESTED PROGRAM         *
               ECB=(R3),SF=(E,SYSATLST)  SZERO=NO,SHSPV=78
         LTR   R15,R15            CHECK RETURN CODE
         BZ    SYSATWET           GOOD
         MVC   0(4,R11),=X'04806000'  ATTACH FAILED
         STC   R15,3(,R11)        SET ERROR CODE
         B     SYSATEXT           FAIL
SYSATWET ST    R1,SYSATTCB        SAVE FOR DETACH
         WAIT  ECB=SYSATECB       WAIT FOR IT TO FINISH
         L     R2,SYSATTCB        GET SUBTASK TCB
         USING TCB,R2             DECLARE IT
         MVC   0(4,R11),TCBCMP    COPY RETURN OR ABEND CODE
         TM    TCBFLGS,TCBFA      ABENDED ?
         BZ    *+8                NO
         MVI   0(R11),X'80'       SET ABEND FLAG
         DETACH SYSATTCB          GET RID OF SUBTASK
         DROP  R2
         B     SYSATEXT           AND RETURN
SYSAXPGM OC    SYSATPGM(0),0(R6)  MOVE NAME AND UPPER CASE
SYSAXTXT MVC   SYSATOTX(0),0(R8)    MOVE PARM TEXT
SYSAXBLK CLC   SYSATOTX(0),SYSATOTX-1  TEST FOR OPERANDS
*    PROGRAM EXIT, WITH APPROPRIATE RETURN CODES
*
SYSATEXT FUNEXIT ,           RESTORE REGS; SET RETURN CODES
         SPACE 1             RETURN TO CALLER
*    DYNAMICALLY ACQUIRED STORAGE
*
SYSATWRK DSECT ,             MAP STORAGE
         DS    18A           OUR OS SAVE AREA
SYSATCLR DS    0F            START OF CLEARED AREA
SYSATLST ATTACH EPLOC=SYSATPGM,ECB=SYSATECB,SHSPV=78,SZERO=NO,SF=L
SYSATECB DS    F             EVENT CONTROL FOR SUBTASK
SYSATTCB DS    A             ATTACH TOKEN FOR CLEAN-UP
SYSATPRM DS    4A            PREFIX FOR CP
SYSATOPL DS    2Y     1/4    PARM LENGTH / LENGTH SCANNED
SYSATPGM DS    CL8    2/4    PROGRAM NAME (SEPARATOR)
SYSATOTL DS    Y      3/4    OS PARM LENGTH / BLANKS FOR CP CALL
SYSATZER EQU   SYSATCLR,*-SYSATCLR,C'X'   ADDRESS & SIZE TO CLEAR
SYSATOTX DS    CL247  4/4    NORMAL PARM TEXT STRING
SYSATDLN EQU   *-SYSATWRK     LENGTH OF DYNAMIC STORAGE
         CSECT ,             RESTORE
         SPACE 2
***********************************************************************
*                                                                     *
*    INVOKE IDCAMS: CALL @@IDCAMS,(@LEN,@TEXT)                        *
*                                                                     *
***********************************************************************
         PUSH  USING
         DROP  ,
@@IDCAMS FUNHEAD SAVE=IDCSAVE,US=NO  EXECUTE IDCAMS REQUEST
         LA    R1,0(,R1)          ADDRESS OF IDCAMS REQUEST (V-CON)
         ST    R1,IDC@REQ         SAVE REQUEST ADDRESS
         MVI   EXFLAGS,0          INITIALIZE FLAGS
         LA    R1,AMSPARM         PASS PARAMETER LIST
         LINK  EP=IDCAMS          INVOKE UTILITY
         FUNEXIT RC=(15)          RESTORE CALLER'S REGS
         POP   USING
         SPACE 1
***********************************************************************
*                                                                     *
* XIDCAMS - ASYNCHRONOUS EXIT ROUTINE                                 *
*                                                                     *
***********************************************************************
         PUSH  USING
         DROP  ,
XIDCAMS  STM   R14,R12,12(R13)
         LR    R12,R15
         USING XIDCAMS,R12
         LA    R9,XIDSAVE         SET MY SAVE AREA
         ST    R13,4(,R9)         MAKE BACK LINK
         ST    R9,8(,R13)         MAKE DOWN LINK
         LR    R13,R9             MAKE ACTIVE SAVE AREA
         SR    R15,R15            PRESET FOR GOOD RETURN
         LM    R3,R5,0(R1)        LOAD PARM LIST ADDRESSES
         SLR   R14,R14
         IC    R14,0(,R4)         LOAD FUNCTION
         B     *+4(R14)
         B     XIDCEXIT   OPEN           CODE IN R14 = X'00'
         B     XIDCEXIT   CLOSE          CODE IN R14 = X'04'
         B     XIDCGET    GET SYSIN      CODE IN R14 = X'08'
         B     XIDCPUT    PUT SYSPRINT   CODE IN R14 = X'0C'
XIDCGET  TM    EXFLAGS,EXFGET            X'01' = PRIOR GET ISSUED ?
         BNZ   XIDCGET4                  YES, SET RET CODE = 04
         L     R1,IDC@REQ         GET REQUEST ADDRESS
         LDINT R3,0(,R1)          LOAD LENGTH
         L     R2,4(,R1)          LOAD TEXT POINTER
         LA    R2,0(,R2)          CLEAR HIGH
         STM   R2,R3,0(R5)        PLACE INTO IDCAMS LIST
         OI    EXFLAGS,EXFGET            X'01' = A GET HAS BEEN ISSUED
         B     XIDCEXIT
XIDCGET4 LA    R15,4                     SET REG 15 = X'00000004'
         B     XIDCEXIT
XIDCPUT  TM    EXFLAGS,EXFSUPP+EXFSKIP  ANY FORM OF SUPPRESSION?
         BNZ   XIDCPUTZ           YES; DON'T BOTHER WITH REST
         LM    R4,R5,0(R5)
         LA    R4,1(,R4)          SKIP CARRIAGE CONTROL CHARACTER
         BCTR  R5,0               FIX LENGTH
         ICM   R5,8,=C' '         BLANK FILL
         LA    R14,XIDCTEXT
         LA    R15,L'XIDCTEXT
         MVCL  R14,R4
         TM    EXFLAGS,EXFMALL    PRINT ALL MESSAGES?
         BNZ   XIDCSHOW           YES; PUT THEM ALL OUT
         CLC   =C'IDCAMS ',XIDCTEXT    IDCAMS TITLE ?
         BE    XIDCEXIT           YES; SKIP
         CLC   XIDCTEXT+1(L'XIDCTEXT-1),XIDCTEXT   ALL BLANK OR SOME?
         BE    XIDCEXIT           YES; SKIP
         CLC   =C'IDC0002I',XIDCTEXT   AMS PGM END
         BE    XIDCEXIT           YES; SKIP
XIDCSHOW DS    0H            Consider how/whether to pass to user
*later   WTO   MF=(E,AMSPRINT)
XIDCPUTZ SR    R15,R15
         B     XIDCEXIT
XIDCSKIP OI    EXFLAGS,EXFSKIP    SKIP THIS AND REMAINING MESSAGES
         SR    R15,R15
*---------------------------------------------------------------------*
* IDCAMS ASYNC EXIT ROUTINE - EXIT, CONSTANTS & WORKAREAS
*---------------------------------------------------------------------*
XIDCEXIT L     R13,4(,R13)        GET CALLER'S SAVE AREA
         L     R14,12(,R13)
         RETURN (0,12)            RESTORE AND RETURN TO IDCAMS
IDCSAVE  DC    18F'0'             MAIN ROUTINE'S REG SAVEAREA
XIDSAVE  DC    18F'0'             ASYNC ROUTINE'S REG SAVEAREA
AMSPRINT DC    0A(0),AL2(4+L'XIDCTEXT,0)
XIDCTEXT DC    CL132' '
AMSPARM  DC    A(HALF00,HALF00,HALF00,X'80000000'+ADDRLIST)
ADDRLIST DC    F'2'
         DC    A(DDNAME01)
         DC    A(XIDCAMS)
IDC@REQ  DC    A(0)               ADDRESS OF REQUEST POINTER
         DC    A(DDNAME02)
         DC    A(XIDCAMS)
         DC    A(0)
HALF00   DC    H'0'
DDNAME01 DC    CL10'DDSYSIN   '
DDNAME02 DC    CL10'DDSYSPRINT'
EXFLAGS  DC    X'08'              EXIT PROCESSING FLAGS
EXFGET   EQU   X'01'                PRIOR GET WAS ISSUED
EXFNOM   EQU   X'04'                SUPPRESS ERROR WTOS
EXFRET   EQU   X'08'                NO ABEND; RETURN WITH COND.CODE
EXFMALL  EQU   X'10'                ALWAYS PRINT MESSAGES
EXFSUPP  EQU   X'20'                ALWAYS SUPPRESS MESSAGES
EXFSKIP  EQU   X'40'                SKIP SUBSEQUENT MESSAGES
EXFGLOB  EQU   EXFMALL+EXFSUPP+EXFRET  GLOBAL FLAGS
         POP   USING
         SPACE 2
***********************************************************************
*                                                                     *
*    CALL @@DYNAL,(ddn-len,ddn-adr,dsn-len,dsn-adr),VL                *
*                                                                     *
*    "-len" fields are self-defining values in the calling list,      *
*        or else pointers to 32-bit signed integer values             *
*                                                                     *
*    "ddn-adr"  is the address of the DD name to be used. When the    *
*        contents is hex zero or blank, and len=8, gets assigned.     *
*                                                                     *
*    "dsn-adr" is the address of a 1 to 44 byte data set name of an   *
*        existing file (sequential or partitioned).                   *
*                                                                     *
*    Calling @@DYNAL with a DDNAME and a zero length for the DSN      *
*    results in unallocation of that DD (and a PARM error).           *
*                                                                     *
*---------------------------------------------------------------------*
*                                                                     *
*     Author:  Gerhard Postpischil                                    *
*                                                                     *
*     This program is placed in the public domain.                    *
*                                                                     *
*---------------------------------------------------------------------*
*                                                                     *
*     Assembly: Any MVS or later assembler may be used.               *
*        Requires SYS1.MACLIB                                         *
*        Intended to work in any 24 and 31-bit environment.           *
*                                                                     *
*     Linker/Binder: RENT,REFR,REUS                                   *
*                                                                     *
*---------------------------------------------------------------------*
*     Return codes:  R15:04sssnnn   it's a program error code:        *
*     04804 - GETMAIN failed;  1400000n   PARM list error             *
*                                                                     *
*     Otherwise R15:0-1  the primary allocation return code, and      *
*       R15:2-3 the reason codes.                                     *
***********************************************************************
*  Maintenance:                                     new on 2008-06-07 *
*                                                                     *
***********************************************************************
@@DYNAL  FUNHEAD ,                DYNAMIC ALLOCATION
         LA    R11,16(,R13)       REMEMBER RETURN CODE ADDRESS
         MVC   0(4,R11),=X'04804000'  PRESET
         LR    R9,R1              SAVE PARAMETER LIST ADDRESS
         LA    R0,DYNALDLN        GET LENGTH OF SAVE AND WORK AREA
         AIF ('&ZSYS' EQ 'S370').NOBEL8
         GETMAIN RC,LV=(0),LOC=BELOW        GET STORAGE
         AGO .GETFIN8
.NOBEL8  ANOP  ,
         GETMAIN RC,LV=(0)        GET STORAGE
.GETFIN8 ANOP  ,
         LTR   R15,R15            SUCCESSFUL ?
         BZ    DYNALHAV           YES
         STC   R15,3(,R11)        SET RETURN VALUES
         B     DYNALRET           RELOAD AND RETURN
*
*    CLEAR GOTTEN STORAGE AND ESTABLISH SAVE AREA
*
DYNALHAV ST    R1,8(,R13)         LINK OURS TO CALLER'S SAVE AREA
         ST    R13,4(,R1)         LINK CALLER'S TO OUR AREA
         LR    R13,R1
         USING DYNALWRK,R13
         MVC   0(4,R11),=X'14000001'  PRESET FOR PARM LIST ERROR
         MVC   DYNLIST(ALLDYNLN),PATLIST  INITIALIZE EVERYTHING
         LDINT R4,0(,R9)          DD NAME LENGTH
         L     R5,4(,R9)          -> DD NAME
         LDINT R6,8(,R9)          DSN LENGTH
         L     R7,12(,R9)         -> DATA SET NAME
*   NOTE THAT THE CALLER IS EITHER COMPILER CODE, OR A COMPILER
*   LIBRARY ROUTINE, SO WE DO MINIMAL VALIDITY CHECKING
*
*   PREPARE DYNAMIC ALLOCATION REQUEST LISTS
*
         LA    R0,ALLARB
         STCM  R0,7,ALLARBP+1     REQUEST POINTER
         LA    R0,ALLATXTP
         ST    R0,ALLARB+8        TEXT UNIT POINTER
         LA    R0,ALLAXDSN
         LA    R1,ALLAXDSP
         LA    R2,ALLAXDDN
         O     R2,=X'80000000'
         STM   R0,R2,ALLATXTP     TEXT UNIT ADDRESSES
*   COMPLETE REQUEST WITH CALLER'S DATA
*
         LTR   R4,R4              CHECK DDN LENGTH
         BNP   DYNALEXT           OOPS
         CH    R4,=AL2(L'ALLADDN)   REASONABLE SIZE ?
         BH    DYNALEXT           NO
         BCTR  R4,0
         EX    R4,DYNAXDDN        MOVE DD NAME
         OC    ALLADDN,=CL11' '   CONVERT HEX ZEROES TO BLANKS
         CLC   ALLADDN,=CL11' '   NAME SUPPLIED ?
         BNE   DYNALDDN           YES
         MVI   ALLAXDDN+1,DALRTDDN  REQUEST RETURN OF DD NAME
         CH    R4,=AL2(L'ALLADDN-1)   CORRECT SIZE FOR RETURN ?
         BE    DYNALNDD           AND LEAVE R5 NON-ZERO
         B     DYNALEXT           NO
DYNALDDN SR    R5,R5              SIGNAL NO FEEDBACK
*  WHEN USER SUPPLIES A DD NAME, DO AN UNCONDITIONAL UNALLOCATE ON IT
         LA    R0,ALLURB
         STCM  R0,7,ALLURBP+1     REQUEST POINTER
         LA    R0,ALLUTXTP
         ST    R0,ALLURB+8        TEXT UNIT POINTER
         LA    R2,ALLUXDDN
         O     R2,=X'80000000'
         ST    R2,ALLUTXTP        TEXT UNIT ADDRESS
         MVC   ALLUDDN,ALLADDN    SET DD NAME
         LA    R1,ALLURBP         POINT TO REQUEST BLOCK POINTER
         DYNALLOC ,               REQUEST ALLOCATION
DYNALNDD LTR   R6,R6              CHECK DSN LENGTH
         BNP   DYNALEXT           OOPS
         CH    R6,=AL2(L'ALLADSN)   REASONABLE SIZE ?
         BH    DYNALEXT           NO
         STH   R6,ALLADSN-2       SET LENGTH INTO TEXT UNIT
         BCTR  R6,0
         EX    R6,DYNAXDSN        MOVE DS NAME
*    ALLOCATE
         LA    R1,ALLARBP         POINT TO REQUEST BLOCK POINTER
         DYNALLOC ,               REQUEST ALLOCATION
         STH   R15,0(,R11)        PRIMARY RETURN CODE
         STH   R0,2(,R11)         REASON CODES
         LTR   R5,R5              NEED TO RETURN DDN ?
         BZ    DYNALEXT           NO
         MVC   0(8,R5),ALLADDN    RETURN NEW DDN, IF ANY
         B     DYNALEXT           AND RETURN
DYNAXDDN MVC   ALLADDN(0),0(R5)   COPY DD NAME
DYNAXDSN MVC   ALLADSN(0),0(R7)   COPY DATA SET NAME
*    PROGRAM EXIT, WITH APPROPRIATE RETURN CODES
*
DYNALEXT LR    R1,R13        COPY STORAGE ADDRESS
         L     R9,4(,R13)    GET CALLER'S SAVE AREA
         LA    R0,DYNALDLN   GET ORIGINAL LENGTH
         FREEMAIN R,A=(1),LV=(0)  AND RELEASE THE STORAGE
         LR    R13,R9        RESTORE CALLER'S SAVE AREA
DYNALRET FUNEXIT ,           RESTORE REGS; SET RETURN CODES
         LTORG ,
         PUSH  PRINT
         PRINT NOGEN         DON'T NEED TWO COPIES
PATLIST  DYNPAT P=PAT        EXPAND ALLOCATION DATA
         POP   PRINT
*    DYNAMICALLY ACQUIRED STORAGE
*
DYNALWRK DSECT ,             MAP STORAGE
         DS    18A           OUR OS SAVE AREA
DYNLIST  DYNPAT P=ALL        EXPAND ALLOCATION DATA
DYNALDLN EQU   *-DYNALWRK     LENGTH OF DYNAMIC STORAGE
         CSECT ,             RESTORE
         SPACE 2
*
*
*
**********************************************************************
*                                                                    *
*  GETPFX - get TSO prefix                                           *
*                                                                    *
**********************************************************************
         PUSH  USING
         ENTRY @@GETPFX
@@GETPFX DS    0H
         SAVE  (14,12),,@@GETPFX
         LR    R12,R15
         USING @@GETPFX,R12
*
         LA    R15,0
         LA    R0,0    Not really needed, just looks nice
         USING PSA,R0
         ICM   R2,15,PSATOLD
         BZ    RETURNGP
         USING TCB,R2
         ICM   R3,15,TCBJSCB
         BZ    RETURNGP
         USING IEZJSCB,R3
         ICM   R4,15,JSCBPSCB
         BZ    RETURNGP
         USING PSCB,R4
         ICM   R5,15,PSCBUPT
         BZ    RETURNGP
         USING UPT,R5
         LA    R15,UPTPREFX       RETURN ADDRESS (CL7/AL1)
*
RETURNGP RETURN (14,12),RC=(15)
         POP   USING
*
*
*
***********************************************************************
*-                                                                   -*
*- TITLE - @@GETEPF                                                  -*
*-                                                                   -*
*- DESCRIPTION - PROGRAM TO RETURN THE FULL DSN WITH                 -*
*-               THE PREFIX FROM THE TSO/E PROFILE.                  -*
*-                                                                   -*
*- OPERATION - @@GETEPF IS A NON-REENTRANT PROGRAM THAT              -*
*-             PERFORMS THE FOLLOWING PROCESSING:                    -*
*-                                                                   -*
*- 1 - GETS THE DSNAME FROM PARM                                     -*
*- 2 - ESTABLISHES ADDRESSABILITY AND SAVES THE CALLER'S REGISTERS   -*
*- 3 - USES THE PARSE SERVICE ROUTINE (IKJPARS) TO DETERMINE THE     -*
*-     VALIDITY OF THE OPERANDS                                      -*
*- 4 - PROVIDES A VALIDITY CHECKING ROUTINE TO GET THE FULLY         -*
*-     QUALIFIED DSN AND ITS LENGTH                                  -*
*- 5 - RESTORES THE CALLER'S REGISTERS BEFORE RETURNING              -*
*- 6 - RETURNS TO THE CALLER WITH REGISTER 15 POINTING TO THE        -*
*-     NULL-TERMINATED FULL DSNAME WITH THE PREFIX FROM THE PROFILE  -*
*-                                                                   -*
*-     >>>>> J. REGINATO-NOV/2020 - BRAZIL <<<<<                     -*
*-     >>>>> RELEASED TO THE PUBLIC DOMAIN <<<<<                     -*
*-                                                                   -*
***********************************************************************
         ENTRY @@GETEPF            ENTRY POINT
@@GETEPF SAVE  (14,12),,@@GETEPF-NOV/2020-J.REGINATO
*                                  SAVE CALLER'S REGISTERS
         LR    R12,R15             ESTABLISH ADDRESSABILITY WITHIN
         USING @@GETEPF,R12        THIS CSECT
         CNOP  0,4                 FORCE FULLWORD ALIGNMENT
         BAL   R1,GETESTRT         BR AROUND STATIC SAVE AREA
         DS    18F                 SAVE AREA
GETESTRT ST    R1,8(,R13)          PUT THE ADDRESS OF THE NEW SAVE
*                                  AREA INTO THE CALLER'S SAVE ARE
         ST    R13,4(,R1)          PUT THE ADDRESS OF THE CALLER'S
*                                  SAVE AREA INTO THE NEW SAVE AREA
         LDVAL R4,24(,R13)         GET THE PARM FROM THE ORIGINAL R1
         LR    R13,R1              POINT TO ITS OWN SAVE AREA
*
*
* We need a once-off initialization to get the stub BTL
*
         L     R2,GETEINIT
         LTR   R2,R2
         BNZ   DONEGEI
* First we just put the routine in, which is all we need if
* the module happens to reside BTL
         L     R2,=A(@@PCLST)
         LA    R14,DSNCHK
         STCM  R14,B'0111',13(R2)
*
         AIF   ('&ZSYS' EQ 'S370').NODSNS  Only S/380+90 needs a stub
* Note that module may be above 2 GiB on an AM32 system
         TM    @DSNCHK,X'FF'     Loaded above the line?
         BZ    EXBTLDSN              No; previous store is sufficient
*
         LA    R0,DSNCLEN
         GETMAIN RU,LV=(0),LOC=BELOW
         MVC   0(DSNCLEN,R1),DSNCSTB
*
         STCM  R1,B'0111',13(R2)
*
EXBTLDSN DS    0H
.NODSNS  ANOP  ,                  Only S/380 etc needs a stub
*
         L     R2,=F'1'
         ST    R2,GETEINIT
DONEGEI  DS    0H
*
***********************************************************************
*-       GET THE DSNAME FROM PARM AND FILL IN A NEW BUFFER           -*
***********************************************************************
         XC    WKCBUF(WKCBUFT),WKCBUF CLEAR THE NEW BUFFER
         LDVAL R3,=V(@@CPPL)       LOAD THE CPPL ADDRESS
         USING CPPL,R3             AND ESTABLISH ADDRESSABILITY
         L     R5,CPPLCBUF         LOAD THE ORIGINAL COMMAND BUFFER
         LH    R1,0(,R5)           GET THE BUFFER LENGTH
         BCTR  R1,R0               -1 FOR EXECUTE
         MVC   WKCBUF(0),0(R5)     COPY THE ORIGINAL BUFFER
         EX    R1,*-6              COPY THE ORIGINAL BUFFER
         TRT   0(45,R4),WKTRT      SEARCH THE DSNAME FOR \0
         BZ    GETEEND             RETURN IF NONE FOUND
         SR    R1,R4               GET THE REAL LENGTH
         BCTR  R1,R0               -1 FOR EXECUTE
         LA    R2,WKCBUFP          POINT TO THE NEW PROMPT BUFFER
         LH    R5,WKCBUFO          GET THE OFFSET TO THE DSNAME
         AR    R2,R5               ADD THE OFFSET TO THE DSNAME
         CLI   0(R2),X'00'         ANY PARM IN THE PROMPT BUFFER?
         BNE   GETEOFFK            YES, CONTINUE
         LA    R2,1(,R2)           INCREASE POINTER BY 1
         LA    R5,1(,R5)           INCREASE OFFSET BY 1
         STH   R5,WKCBUFO          SAVE THE NEW OFFSET TO THE DSNAME
GETEOFFK ST    R2,WKDSN            SAVE THE POINTER TO THE DSNAME
         MVC   0(0,R2),0(R4)       COPY DSNAME TO THE NEW BUFFER
         EX    R1,*-6              COPY DSNAME TO THE NEW BUFFER
         LA    R0,WKCBUF           GET THE BUFFER START
         SR    R2,R0               GET THE PREFIX LENGTH
         LA    R1,1(R1,R2)         GET THE TOTAL LENGTH
         STH   R1,WKCBUFL          SAVE THE NEW BUFFER LENGTH
***********************************************************************
*-       CALL THE PARSE ROUTINE TO INCLUDE THE PROFILE PREFIX        -*
***********************************************************************
         LA    R5,WKPPL            POINT TO THE NEW PPL
         USING PPL,R5              ESTABLISH ADDRESSABILITY TO THE PPL
         MVC   PPLUPT,CPPLUPT      PUT IN THE UPT ADDRESS FROM CPPL
         MVC   PPLECT,CPPLECT      PUT IN THE ECT ADDRESS FROM CPPL
         LA    R1,WKCBUF           GET THE ADDRESS OF THE NEW BUFFER
         ST    R1,PPLCBUF          PUT IN THE BUFFER ADDRESS
*                                  FROM THE CPPL
         LA    R1,WKANS            GET THE ADDRESS OF THE PARSE
*                                  ANSWER AREA AND
         ST    R1,PPLANS           STORE IT IN THE PPL
         LA    R1,WKECB            GET THE ADDRESS OF THE ECB AND
         ST    R1,PPLECB           PUT IT IN THE PPL
         MVC   PPLPCL,=A(@@PCLST)  PUT THE PCL IN THE PPL FOR PARSE
         GAMOS ,                   SET AM24 ON S380
         CALLTSSR EP=IKJPARS,MF=(E,PPL) INVOKE PARSE
         GAMAPP ,                  SET AM31 ON S380
***********************************************************************
*-       CLEANUP AND TERMINATION PROCESSING                          -*
***********************************************************************
         ICM   R1,15,WKANS         POINT TO THE PDL
         BNP   GETEEND             BR IF NOT VALID
         IKJRLSA (R1)              FREE STORAGE THAT PARSE ALLOCATED
*                                  FOR THE PDL
GETEEND  L     R15,WKDSN           RETURN THE NULL-TERMINATED DSNAME
         L     R13,4(,R13)         CHAIN TO PREVIOUS SAVE AREA
         RETURN (14,12),RC=(15)    RETURN TO THE CALLER
*
GETEINIT DC    F'0'
*
         DROP  ,                   FREE REGISTERS
*
         AIF   ('&ZSYS' EQ 'S370').NODSNS2  Only S/380+90 needs a stub
***********************************************************************
*                                                                     *
*    DSNCSTB - 24 bit stub                                            *
*    This code is not directly executed. It is copied below the line  *
*    It is only needed when the program resides above the line.       *
*                                                                     *
***********************************************************************
         PUSH  USING
         DROP  ,
*
         DS    0A            ENSURE MATCHING ALIGNMENT
DSNCSTB  SAVE  (14,12),,DSNCSTB    SAVE CALLER'S REGISTERS
         LR    R12,R15             ESTABLISH ADDRESSABILITY WITHIN
         USING DSNCSTB,R12   DECLARE BASE
         CNOP  0,4                 FORCE FULLWORD ALIGNMENT
         BAL   R2,DSNCSTRT         BR AROUND STATIC SAVE AREA
         DS    18F                 SAVE AREA
DSNCSTRT ST    R2,8(,R13)          PUT THE ADDRESS OF THE NEW SAVE
*                                  AREA INTO THE CALLER'S SAVE AREA
         ST    R13,4(,R2)          PUT THE ADDRESS OF THE CALLER'S
*                                  SAVE AREA INTO THE NEW SAVE AREA
         LR    R13,R2              POINT TO ITS OWN SAVE AREA
*
         LA    R13,0(,R13)         Clean R13
         LA    R1,0(,R1)           Clean R1
*
         BSM   R14,0         Save caller's AMODE
         LR    R11,R14            Preserve OS return address
* Do the mode transition here so that we can have clean
* addresses, essential for AM32/64 (not 31)
         LA    R5,DSNCNEXT
         O     R5,DSNCBOA
         BSM   R0,R5
DSNCNEXT DS    0H
         L     R15,@DSNCHK   Load 31/32-bit routine address
         BALR  R14,R15            Call the open exit in AM31
         L     R13,4(,R13)        CHAIN TO PREVIOUS SAVE AREA
         LR    R14,R11            Restore OS return address
         LM    R0,R12,20(R13)
         BSM   0,R14              Return to OS in original mode
@DSNCHK  DC    A(DSNCHK)     AM31/32 main routine address
DSNCBOA  DC    A(0)
         LTORG
DSNCLEN  EQU   *-DSNCSTB
         POP   USING
         SPACE 2
.NODSNS2 ANOP  ,                  Only S/380 etc needs a stub
*
*
*
***********************************************************************
*-       DSNCHK - IKJPOSIT VALIDITY CHECKING ROUTINE                 -*
*-                                                                   -*
*-       RETURN THE DSN AND ITS LENGTH WITH RETURN CODE ALWAYS 0     -*
***********************************************************************
DSNCHK   SAVE  (14,12),,DSNCHK     SAVE CALLER'S REGISTERS
         LR    R12,R15             ESTABLISH ADDRESSABILITY WITHIN
         USING DSNCHK,R12          THIS CSECT
         L     R2,0(,R1)           GET THE ADDRESS OF THE PDE
         USING DSNDSECT,R2         AND ESTABLISH ADDRESSABILITY TO
*                                  OUR MAPPING OF THE PDE
         CNOP  0,4                 FORCE FULLWORD ALIGNMENT
         BAL   R1,DSNSTART         BR AROUND STATIC SAVE AREA
         DS    18F                 SAVE AREA
DSNSTART ST    R1,8(,R13)          PUT THE ADDRESS OF THE NEW SAVE
*                                  AREA INTO THE CALLER'S SAVE ARE
         ST    R13,4(,R1)          PUT THE ADDRESS OF THE CALLER'S
*                                  SAVE AREA INTO THE NEW SAVE AREA
         LR    R13,R1              POINT TO ITS OWN SAVE AREA
***********************************************************************
*-       RETURN THE DSNAME FROM PARSER                               -*
***********************************************************************
         L     R4,DSNPTR           POINT TO THE DSN
         LH    R1,DSNLEN           GET THE DSN LENGTH
         BCTR  R1,R0               MINUS 1
         L     R3,WKDSN            GET THE DSNAME POINTER
         XC    0(45,R3),0(R3)      CLEAR IT
         MVC   0(0,R3),0(R4)       MOVE DSN
         EX    R1,*-6              MOVE DSN WITH PROPER LENGTH
DSNOK    L     R13,4(,R13)         CHAIN TO PREVIOUS SAVE AREA
         RETURN (14,12),RC=0       RETURN TO THE CALLER WITH RC=0
***********************************************************************
*-       PARSE MACROS USED TO DESCRIBE THE COMMAND OPERANDS          -*
***********************************************************************
* Can't use VALIDCK=DSNCHK because it generates an AL3, preventing
* relocation when RMODE ANY.
@@PCLST  IKJPARM DSECT=PDL         START DEFINITION
PCLDSN   IKJPOSIT DSNAME,USID,     PARSE DSN AND APPEND TO PREFIX      X
               VALIDCK=0     * DSNCHK      VALIDITY CHECK ROUTINE
         IKJENDP ,                 END DEFINITION
*---------------------------------------------------------------------*
*-       MAPPING THE PDE BUILT BY PARSE TO DESCRIBE A DSNAME OPERAND -*
*---------------------------------------------------------------------*
DSNDSECT DSECT                     PDE MAPPING FOR THE DSN
DSNPTR   DS    F                   POINTER TO THE DSN
DSNLEN   DS    H                   LENGTH OF THE DSN EXCLUDING QUOTES
DSNFLG   DS    CL1                 FLAGS BYTE
*        0... .... THE DATA SET NAME IS NOT PRESENT
*        1... .... THE DATA SET NAME IS PRESENT
*        .0.. .... THE DATA SET NAME IS NOT CONTAINED WITHIN QUOTES
*        .1.. .... THE DATA SET NAME IS CONTAINED WITHIN QUOTES
         DS    CL1                 RESERVED
MBRPTR   DS    F                   POINTER TO THE MEMBER NAME
MBRLEN   DS    H                   LENGTH OF THE MEMBER NAME
*                                  EXCLUDING PARENTHESES
MBRFLG   DS    CL1                 FLAGS BYTE
*        0... .... THE MEMBER NAME IS NOT PRESENT
*        1... .... THE MEMBER NAME IS PRESENT
         DS    CL1                 RESERVED
PSWPTR   DS    F                   POINTER TO THE DATA SET PASSWORD
PSWLEN   DS    H                   LENGTH OF THE PASSWORD
PSWFLG   DS    CL1                 FLAGS BYTE
*        0... .... THE DATA SET PASSWORD IS NOT PRESENT
*        1... .... THE DATA SET PASSWORD IS PRESENT
         DS    CL1                 RESERVED
*---------------------------------------------------------------------*
*-       MAPPING TSO CONTROL BLOCKS                                  -*
*---------------------------------------------------------------------*
         IKJPPL                    PARSE PARAMETER LIST
LENPPL   EQU   *-PPL               LENGTH OF PPL
         CSECT
***********************************************************************
*-       DECLARES THE STATIC WORK AREA                               -*
***********************************************************************
WKPDE    DS    F                   ADDRESS OF THE PDE FROM PARSE
WKECB    DS    F                   CP'S EVENT CONTROL BLOCK
WKANS    DS    F                   PARSE ANSWER PLACE
WKPPL    DS    CL(LENPPL)          PPL
WKDSN    DS    F                   DSNAME POINTER INTO THE BUFFER
WKCBUF   DS    0F                  NEW FAKE COMMAND BUFFER
WKCBUFL  DS    H                   BUFFER LENGTH
WKCBUFO  DS    H                   OFFSET TO THE DSNAME
WKCBUFP  DS    CL(8+1)             PROMPT BUFFER PGMNAME + SPACE
         DS    CL(44+1)            NULL-TERMINATED DSNAME
WKCBUFT  EQU   *-WKCBUF            TOTAL LENGTH
WKTRT    DC    X'FF',255X'00'      SEARCH FOR X'00'
*
*
*
**********************************************************************
*                                                                    *
*  TEST31 - see if we are running in AMODE 31                        *
*                                                                    *
*  This function returns 1 if we are running in AMODE 31, else 0     *
*                                                                    *
*  This code works because in 31-bit mode, a BALR will set the       *
*  high bit of the first register to 1, with the remaining 31 bits   *
*  used for the address. While in 24-bit mode, the entire top byte   *
*  has information stored, with the remaining 3 bytes used for the   *
*  address. The first 2 bits of that top byte are the ILC, which     *
*  will be b'01' for a BALR, and b'10' for a BAL. We use BALR, so    *
*  we get b'01', hence the top bit is always 0. Note that BASR       *
*  could be used instead of BALR and we would still get the same     *
*  result. Note that the b'01' in BALR means "1 halfword", ie        *
*  the instruction (BALR) is 2 bytes long.                           *
*                                                                    *
**********************************************************************
         ENTRY @@TEST31
@@TEST31 DS    0H
         SAVE  (14,12),,@@TEST31
         LR    R12,R15
         USING @@TEST31,R12
*
         LA    R15,1
         BALR  R1,R0
         LTR   R1,R1
         BM    RETURNTS
         LA    R15,0
*
RETURNTS DS    0H
         RETURN (14,12),RC=(15)
         LTORG ,
*
*
*
**********************************************************************
*                                                                    *
*  GETAM - get the current AMODE                                     *
*                                                                    *
*  This function returns 24 if we are running in exactly AMODE 24,   *
*  31 if we are running in exactly AMODE 31, and 64 for anything     *
*  else (user-defined/infinity/16/32/64/37)                          *
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
*  they should really be saying is 24, 31 and user-defined.          *
*                                                                    *
**********************************************************************
         ENTRY @@GETAM
@@GETAM  DS    0H
         SAVE  (14,12),,@@GETAM
         LR    R12,R15
         USING @@GETAM,R12
*
         L     R2,=X'C1800000'
         LA    R2,0(,R2)
         CLM   R2,B'1100',=X'0080'
         BE    GAIS24
         CLM   R2,B'1000',=X'41'
         BE    GAIS31
         LA    R15,64
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
***********************************************************************
*                                                                     *
*  ADDNUM - Add two numbers using 80386                               *
*                                                                     *
***********************************************************************
*
         PUSH  USING
         DROP  ,
         ENTRY @@ADDNUM
@@ADDNUM DS    0H
         SAVE  (14,12),,@@ADDNUM
         LR    R12,R15
         USING @@ADDNUM,R12
         LR    R2,R1  new register for parms
         L     R0,=X'FFFFFFFD' API for execute 80386
         LR    R1,R0
         LA    R3,CODE386
         LA    R14,ANRET
         L     R4,0(R2)
         L     R5,4(R2)
         L     R6,8(R2)
         SVC   120
ANRET    DS    0H
         RETURN (14,12),RC=(15)
*
         LTORG ,
*
CODE386  DS    0D
         DC    X'55' push ebp
         DC    X'8B' mov ebp,esp
         DC    X'EC'
         DC    X'8B' mov eax, ebp + 8
         DC    X'45'
         DC    X'08'
         DC    X'03' add eas, ebp + 12
         DC    X'45'
         DC    X'0C'
         DC    X'C9' leave
         DC    X'C3' return near
         DC    X'22' eyecatcher
         DC    X'22'
         DC    X'22'
         POP   USING
         SPACE 2
***********************************************************************
*                                                                     *
*  GETMSZ - Get memory size via DIAG                                  *
*                                                                     *
***********************************************************************
*
         PUSH  USING
         DROP  ,
         ENTRY @@GETMSZ
@@GETMSZ DS    0H
         SAVE  (14,12),,@@GETMSZ
         LR    R12,R15
         USING @@GETMSZ,R12
*         DIAGNOSE X'60'
         DC    X'83',X'000060'
         LR    R15,R0
         RETURN (14,12),RC=(15)
*
         LTORG ,
         POP   USING
         SPACE 2
*
*
*
***********************************************************************
*                                                                     *
*  GOSUP - go into supervisor mode                                    *
*                                                                     *
***********************************************************************
*
         PUSH  USING
         DROP  ,
         ENTRY @@GOSUP
@@GOSUP  DS    0H
         SAVE  (14,12),,@@GOSUP
         LR    R12,R15
         USING @@GOSUP,R12
         MODESET MODE=SUP
         LA    R15,0
         RETURN (14,12),RC=(15)
*
         LTORG ,
         POP   USING
         SPACE 2
***********************************************************************
*                                                                     *
*  GOPROB - go into problem mode                                      *
*                                                                     *
***********************************************************************
*
         PUSH  USING
         DROP  ,
         ENTRY @@GOPROB
@@GOPROB DS    0H
         SAVE  (14,12),,@@GOPROB
         LR    R12,R15
         USING @@GOPROB,R12
         MODESET MODE=PROB
         LA    R15,0
         RETURN (14,12),RC=(15)
*
         LTORG ,
         POP   USING
         SPACE 2
***********************************************************************
*                                                                     *
*  CALL @@SVC99,(rb)                                                  *
*                                                                     *
*  Execute DYNALLOC (SVC 99)                                          *
*                                                                     *
*  Caller must provide a request block, in conformance with the       *
*  MVS documentation for this (which is very complicated)             *
*                                                                     *
***********************************************************************
         PUSH  USING
         DROP  ,
         ENTRY @@SVC99
@@SVC99  DS    0H
         SAVE  (14,12),,@@SVC99   Save caller's regs.
         LR    R12,R15
         USING @@SVC99,R12
         LR    R11,R1
*
         AIF ('&ZSYS' EQ 'S370').NOBEL9
         GETMAIN RU,LV=WORKLEN,SP=SUBPOOL,LOC=BELOW
         AGO .GETFIN9
.NOBEL9  ANOP  ,
         GETMAIN RU,LV=WORKLEN,SP=SUBPOOL
.GETFIN9 ANOP  ,
         ST    R13,4(,R1)
         ST    R1,8(,R13)
         LR    R13,R1
         LR    R1,R11
         USING WORKAREA,R13
*
* Note that the SVC requires a pointer to the pointer to the RB.
* Because this function (not SVC) expects to receive a standard
* parameter list, where R1 so happens to be a pointer to the
* first parameter, which happens to be the address of the RB,
* then we already have in R1 exactly what SVC 99 needs.
*
* Except for one thing. Technically, you're meant to have the
* high bit of the pointer on. So we rely on the caller to have
* the parameter in writable storage so that we can ensure that
* we set that bit.
*
         L     R2,0(,R1)
         O     R2,=X'80000000'
         ST    R2,0(,R1)
         SVC   99
         LR    R2,R15
*
RETURN99 DS    0H
         LR    R1,R13
         L     R13,SAVEAREA+4
         FREEMAIN RU,LV=WORKLEN,A=(1),SP=SUBPOOL
*
         LR    R15,R2             Return success
         RETURN (14,12),RC=(15)   Return to caller
*
         POP   USING
         SPACE 2
***********************************************************************
*                                                                     *
*    CALL @@SNAP,snaplist                                             *
*                                                                     *
*    snaplist is the expansion produced by SNAP options,MF=L          *
*    Examples of use are in AOPEN and AREAD.                          *
*                                                                     *
*    Dump data are written to the SYSTERM DD, with predetermined      *
*        DCB values (required by SVC 51).                             *
*                                                                     *
*    according to my macro manual, SNAP will operate correctly with   *
*        addresses above the line, and only the DCB must be in 24-bit *
*        storage. If the MVS version doesn't work that way, code must *
*        be added to copy the caller's parm list to the DCB work area *
*                                                                     *
*    No output is produced unless the debug flag is on.               *
*                                                                     *
*    CODE IS NON-REENTRANT, NON-REFRESHABLE, but REUSABLE.            *
*                                                                     *
*---------------------------------------------------------------------*
*                                                                     *
*     Author:  Gerhard Postpischil                                    *
*                                                                     *
*     This code is placed in the public domain.                       *
*                                                                     *
*---------------------------------------------------------------------*
*                                                                     *
*     Return codes:  as set by SNAP macro/SVC                         *
*                                                                     *
***********************************************************************
*  Maintenance:                                     new on 2014-08-31 *
*                                                                     *
***********************************************************************
         SPACE 1
         PUSH  USING
         PUSH  PRINT
         PRINT NOGEN         DON'T NEED TWO COPIES
         DROP  ,
@@SNAP   FUNHEAD SAVE=(SNAPAREA,SNAPALEN,SUBPOOL)
         L     R15,4(,R13)        GET CALLER'S SAVE AREA
         LA    R11,16(,R15)       REMEMBER RETURN CODE ADDRESS
         SLR   R0,R0
         ST    R0,0(,R11)         PRESET
         LA    R9,0(,R1)          SAVE PARAMETER LIST ADDRESS
         LTR   R9,R9         REQUEST TO CLOSE/FREE?
         BZ    SNAPCLOS        YES
         SPACE 1
         L     R6,=A(@@BUGF)      GET DEBUGGING FLAG
         TM    3(R6),X'01'        SNAP REQUESTED?
         BZ    SNAPRET              NO; RETURN
         ICM   R10,15,@SNAPDCB    PREVIOUSLY GOTTEN?
         BNZ   SNAPGOT
         USING SNAPDCB,R10   DECLARE DYNAMIC WORK AREA
SNAPGET  LA    R0,SNAPSLEN   GET LENGTH OF SAVE AND WORK AREA
         AIF ('&ZSYS' EQ 'S370').NOBELA
         GETMAIN RU,LV=(0),LOC=BELOW
         AGO .GETFINA
.NOBELA  ANOP  ,
         GETMAIN RU,LV=(0)
.GETFINA ANOP  ,
         STM   R0,R1,#SNAPDCB     SAVE FOR RELEASE
         LR    R10,R1
         MVC   SNAPDCB(PATSNAPL),PATSNAP   INIT DCB, ETC.
         OPEN  (SNAPDCB,OUTPUT),MF=(E,SNAPOCL)
         SPACE 1
         LTR   R9,R9         ANY ADDRESS ?
         BZ    SNAPCLOS        NO; CLOSE REQUEST
SNAPGOT  LA    R7,1          INCREMENT DUMP COUNTER
         AL    R7,SNAPCTR    INCREMENT DUMP COUNTER
         ST    R7,SNAPCTR    INCREMENT DUMP COUNTER
         SNAP  DCB=SNAPDCB,ID=(R7),MF=(E,(R9))
         ST    R15,0(,R11)   PROPAGATE RETUNR CODE
         B     SNAPRET
         SPACE 1
SNAPCLOS ICM   R10,15,@SNAPDCB    EVER GOTTEN STORAGE ?
         BZ    SNAPRET              NO; JUST RETURN
         TM    SNAPDCB+DCBOFLGS-IHADCB,DCBOFOPN  OPEN ?
         BZ    SNAPFREE             NO; JUST FREE STORAGE
         CLOSE MF=(E,SNAPOCL)
SNAPFREE L     R0,#SNAPDCB
         FREEMAIN R,LV=(0),A=(R10)
         XC    #SNAPDCB(L'#SNAPDCB+L'@SNAPDCB),#SNAPDCB
         SPACE 1
SNAPRET  FUNEXIT ,           RESTORE REGS; SET RETURN CODES
         SPACE 1
         LTORG ,
         SPACE 1
#SNAPDCB DC    F'0'    1/2   LENGTH OF PERSISTENT DCB WORK AREA
@SNAPDCB DC    A(0)    2/2   ADDR. OF PERSISTENT DCB WORK AREA
         SPACE 1
PATSNAP  DCB   DDNAME=SYSTERM,MACRF=(W),DSORG=PS,                      *
               RECFM=VBA,LRECL=125,BLKSIZE=1632  882
PATSNOC  DC    X'8F000000'   OPEN MF=L
*OLD*OC  OPEN  (PATSNAP,OUTPUT),MF=L
PATSCTR  DC    F'0'          DUMP ID; WRAPS AT 256->0
PATSNAPL EQU   *-PATSNAP
         SPACE 1
         SPACE 1
SNAPSAVE DSECT ,
SNAPDCB  DCB   DDNAME=SYSTERM,MACRF=(W),DSORG=PS,                      *
               RECFM=VBA,LRECL=125,BLKSIZE=1632  882
*OLDOCL  OPEN  (SNAPDCB,OUTPUT),MF=L
SNAPOCL  DC    A(0)          OPEN MF=L
SNAPCTR  DC    F'0'          DUMP ID; WRAPS AT 256->0
SNAPSLEN EQU   *-SNAPSAVE
         SPACE 1
SNAPAREA DSECT ,
         DS    18A           STANDARD SAVE AREA ONLY
SNAPALEN EQU   *-SNAPAREA    LENGTH TO GET
         POP   USING
         POP   PRINT
         CSECT ,             RESTORE CSECT
         SPACE 2
*
* Keep this code last because it makes no difference - no USINGs
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
* S/370 doesn't support switching modes so this code is useless,
* and won't compile anyway because "BSM" is not known.
*
         AIF   ('&ZSYS' EQ 'S370').NOMODE If S/370 we can't switch mode
         PUSH  USING
         DROP  ,
***********************************************************************
*                                                                     *
*  SETM24 - Set AMODE to 24                                           *
*                                                                     *
***********************************************************************
         ENTRY @@SETM24
         USING @@SETM24,R15
@@SETM24 LA    R14,0(,R14)        Sure hope caller is below the line
         BSM   0,R14              Return in amode 24
         POP   USING
*
         SPACE 1
         PUSH  USING
         DROP  ,
***********************************************************************
*                                                                     *
*  SETM31 - Set AMODE to 31                                           *
*                                                                     *
***********************************************************************
         ENTRY @@SETM31
         USING @@SETM31,R15
@@SETM31 ICM   R14,8,=X'80'       Clobber entire high byte of R14
*                                 This is necessary because if people
*                                 use BALR in 24-bit mode, the address
*                                 will have rubbish in the high byte.
*                                 People switching between 24-bit and
*                                 31-bit will be RMODE 24 anyway, so
*                                 there is nothing to preserve in the
*                                 high byte.
         BSM   0,R14              Return in amode 31
         LTORG ,
         POP   USING
*
         SPACE 1
         PUSH  USING
         DROP  ,
***********************************************************************
*                                                                     *
*  SETM64 - Set AMODE to 64                                           *
*                                                                     *
***********************************************************************
* The caller is likely to be using R12 as a base register, so we
* need to clean that too. As such, we can't use it as the base
* register here (not conveniently, anyway).
         ENTRY @@SETM64
@@SETM64 DS    0H
         SAVE  (0,11),,@@SETM64
         LR    R7,R15
         USING @@SETM64,R7
         LA    R7,0(,R7)          Clean base register
         LA    R12,0(,R12)        Clean caller's base register
         LA    R14,0(,R14)        Clean return address
         LA    R2,NXT64
         LA    R2,1(R2)           Set AM64 bit
         BSM   0,R2
NXT64    DS    0H
         LA    R15,0
         RETURN (0,11),RC=(15)
         LTORG ,
         POP   USING
*
.NOMODE  ANOP  ,                  S/370 doesn't support MODE switching
*
*
*
**********************************************************************
*                                                                    *
* DOLOOP - go into a hard loop                                       *
*                                                                    *
**********************************************************************
         ENTRY @@DOLOOP
@@DOLOOP DS    0H
         LR    R12,R15
         USING @@DOLOOP,R12
*
         LA    R3,3
         LA    R4,4
         LA    R5,5
HARDLOOP B     HARDLOOP
*
*
*
**********************************************************************
*                                                                    *
*  SETUP - do initialization. I used the word "setup" instead of     *
*  "init" in case someone imagines that "init" is some sort of       *
*  complicated compiler-generated function.                          *
*                                                                    *
*  This routine figures out the amode switching strategy given that  *
*  the operating system may require a lower amode that the           *
*  application, and this will be reflected in the fact that the      *
*  rmode will be lower than the amode, to allow this switch to       *
*  occur. It is left to the user to use a utility to set the RMODE   *
*  to something that their current operating system supports. E.g.   *
*  a future version of z/OS may allow execution of READ in AM64 in   *
*  which case the z/OS user is free to change this module from RM31  *
*  to RM64, with a view to having the 32-bit load module loaded in   *
*  the 2 GiB to 4 GiB region.                                        *
*                                                                    *
*  Note that AMODE switching is not required, and thus doesn't even  *
*  need time to be wasted, if you are targeting a "pure" environment *
*  such as S370 where everything is in AM24 and the OS can handle    *
*  that, or S390 where everything is in AM31 and the OS can handle   *
*  that, and possibly in the future there will be such a thing as    *
*  Z999 where the OS can handle being called in AM64, so there is    *
*  no need to waste time checking to see if an amode switch is       *
*  required. However, it is strongly advised that instead of coding  *
*  for such pure environments, you instead select STEPD,             *
*  which will work optimally for 32-bit applications on all          *
*  environments, ie AM24 in MVS 3.8j, switch between AM31 and AM24   *
*  on MVS/XA, remain in AM31 on late MVS/ESA and above, and switch   *
*  between AM32 (aka AM64) and AM24 on MVS/380, while attempting to  *
*  obtain RM32 memory on MVS/380.                                    *
*                                                                    *
*  Note that this function should be the last in the source file,    *
*  so that when the test is done to see where the function has been  *
*  loaded, it will err on the side of caution when e.g. the load     *
*  module spans the 2 GiB bar, and only activate step-down           *
*  processing if it finds the SETUP function itself is below the     *
*  2 GiB bar which means the other functions will succeed in         *
*  switching to AM31.                                                *
*                                                                    *
*  Exception - to work on z/PDOS which is pure AM64, if you discover *
*  you are in AM64 then no step-down will be done.                   *
*                                                                    *
**********************************************************************
         ENTRY @@SETUP
@@SETUP  DS    0H
         SAVE  (14,12),,@@SETUP
         LR    R12,R15
         USING @@SETUP,R12
*
         AIF   ('&STEPD' NE 'YES').NOSETUP
*
* If we are running in a pure 24-bit environment, where
* the AMODE and RMODE are the same, there is no need to
* ever do AMODE switching, so none of this AMODE
* switching code is required at all
*
         L     R2,=X'C1800000'
         LA    R2,0(,R2)
         CLM   R2,B'1100',=X'0080'
* If we are currently in AM24, there is nothing
* to ever do, as we will stay in that mode forever
         BE    RETURNSU
*
* Now see if we are running AM31
         CLM   R2,B'1000',=X'41'
         BNE   IS32
* We are running AM31. If we are also located in
* RM31 space we do not need to do BSM switching
         LR    R2,R12
         N     R2,=X'7F000000'
         BNZ   RETURNSU No amode switching possible
* The app is AM31 but the OS is AM24
* An OS of AM24 is default, so just go and set the
* application AMODE now
         B     COMM3164
* Note that we say "32" here, but it is actually
* any value other than 24 or 31.
IS32     DS    0H
*
* At this stage we know we are running in AM64
* aka AM32 aka AM-infinity (we don't know which one)
* First we need to know if we are running in RM32,
* highly unlikely.
*
         LR    R2,R12
         N     R2,=X'80000000'
         BNZ   RETURNSU No amode switching possible
* !!! EXCEPTION !!!
* Don't step down when we are in AM64 AND in RM31
* space so that z/PDOS works (S/380 model only)
         AIF   ('&ZSYS' NE 'S380').ZSTEPD
         LR    R2,R12
         N     R2,=X'7F000000'
         BNZ   RETURNSU     Don't switch amodes
.ZSTEPD  ANOP
* !!! EXCEPTION !!!
* Now see if we are running in RM31 space
         LR    R2,R12
         N     R2,=X'7F000000'
         BZ    COMM3164 RM24 so just set the app amode bits
* We are indeed running in RM31 so we need the high bit
* set whenever we switch to OS mode
         OI    NEEDBOO,X'80'
COMM3164 DS    0H
* We have dealt with the appropriate bits to set
* the OS mode, now we need to set the return to
* application mode. That is easy, it is the current
* amode, either AM64 or AM31
         LA    R2,0
         BSM   R2,0
         ST    R2,NEEDBOA this will be suitable for ORing
         L     R3,=A(DSNCBOA) the DSN check stub needs this too
         ST    R2,0(,R3)
         OI    NEEDBF,NEEDBANY  set flag to say we need BSM switching
         B     RETURNSU
*
.NOSETUP ANOP  ,     Mode switching only relevant to S/380-style
RETURNSU DS    0H
         LA    R15,0
         RETURN (14,12),RC=(15)
         LTORG ,
NEEDBF   DC    X'00'   flag bits for whether BSM needed
NEEDBANY EQU   X'01'   need any amode switching at all?
NEEDBOA  DC    A(0)    amode bits to be ORed in to return APP to
*                      original amode
NEEDBOO  DC    A(0)    amode bits to be ORed in to set OS amode
*
***********************************************************************
***********************************************************************
*                                                                     *
* End of functions, start of data areas                               *
*                                                                     *
***********************************************************************
***********************************************************************
         SPACE 2
*
***********************************************************************
*                                                                     *
*  The work area includes both a register save area and various       *
*  variables used by the different routines.                          *
*                                                                     *
***********************************************************************
WORKAREA DSECT
SAVEAREA DS    18F
DWORK    DS    D                  Extra work space
WWORK    DS    D                  Extra work space
DWDDNAM  DS    D                  Extra work space
WORKLEN  EQU   *-WORKAREA
PARM1    DS    A             FIRST PARM     DD NAME
PARM2    DS    A              NEXT PARM     I/O MODE
PARM3    DS    A              NEXT PARM     FORMAT (F, V, U)
PARM4    DS    A              NEXT PARM     RECORD LEN
PARM5    DS    A              NEXT PARM     BLOCK SIZE
PARM6    DS    A              NEXT PARM     opt. BUFFER
PARM7    DS    A              NEXT PARM     MEMBER NAME
SAVOSUB  DS    6A         R10-R15 Return saver for AOPEN subs
MYJFCB   DS    0F
         IEFJFCBN LIST=YES        Job File Control Block
CAMLST   DS    XL(CAMLEN)         CAMLST for OBTAIN to get VTOC entry
* Format 1 Data Set Control Block
*   N.B. Current program logic does not use DS1DSNAM, leaving 44 bytes
*     of available space
         IECSDSL1 1               Map the Format 1 DSCB
DSCBCCHH DS    CL5                CCHHR of DSCB returned by OBTAIN
         DS    CL47               Rest of OBTAIN's 148 byte work area
         ORG   DS1FMTID
         IECSDSL1 4               Redefine for VTOC
         ORG   ,
         SPACE 1
*   DEFINITIONS TO ALLOW ASSEMBLY AND TESTING OF SMS, ETC. UNDER
*   MVS 3.n
*
FM1FLAG  EQU   DS1NOBDB+1,1,C'X'  MORE FLAGS
FM1COMPR EQU   X'80'           COMPRESSABLE EXTENDED IF DS1STRP
FM1CPOIT EQU   X'40'           CHECKPOINTED D S
FM1SMSFG EQU   FM1FLAG+1,1,C'X'  SMS FLAG
FM1SMSDS EQU   X'80'           SMS D S
FM1SMSUC EQU   X'40'           NO BCS ENTRY
FM1REBLK EQU   X'20'           MAY BE REBLOCKED
FM1CRSDB EQU   X'10'           BLKSZ BY DADSM
FM1PDSE  EQU   X'08'           PDS/E
FM1STRP  EQU   X'04'           EXTENDED FORMAT D S
FM1PDSEX EQU   X'02'           HFS D S
FM1DSAE  EQU   X'01'           EXTENDED ATTRIBUTES EXISY
FM1SCEXT EQU   FM1SMSFG+1,3,C'X'  SECONDARY SPACE EXTENSION
FM1SCXTF EQU   FM1SCEXT,1,C'X'  -"- FLAG
FM1SCAVB EQU   X'80'           SCXTV IS AVG BLOCK LEN
FM1SCMB  EQU   X'40'                 IS IN MEGBYTES
FM1SCKB  EQU   X'20'                 IS IN KILOBYTES
FM1SCUB  EQU   X'10'                 IS IN BYTES
FM1SCCP1 EQU   X'08'           SCXTV COMPACTED BY 256
FM1SCCP2 EQU   X'04'                 COMPACTED BY 65536
FM1SCXTV EQU   FM1SCXTF+1,2,C'X'  SEC SPACE EXTNSION VALUE
FM1ORGAM EQU   DS1ACBM         CONSISTENT NAMING - VSAM D S
FM1RECFF EQU   X'80'           RECFM F
FM1RECFV EQU   X'40'           RECFM V
FM1RECFU EQU   X'C0'           RECFM U
FM1RECFT EQU   X'20'           RECFM T   001X XXXX IS D
FM1RECFB EQU   X'10'           RECFM B
FM1RECFS EQU   X'08'           RECFM S
FM1RECFA EQU   X'04'           RECFM A
FM1RECMC EQU   X'02'           RECFM M
*   OPTCD DEFINITIONS   BDAM    W.EFA..R
*                       ISAM    WUMIY.LR
*             BPAM/BSAM/QSAM    WUCHBZTJ
FM1OPTIC EQU   X'80'  FOR DS1ORGAM - CATLG IN ICF CAT
FM1OPTBC EQU   X'40'           ICF CATALOG
FM1RACDF EQU   DS1IND40
FM1SECTY EQU   DS1IND10
FM1WRSEC EQU   DS1IND04
FM1SCAL1 EQU   DS1SCALO,1,C'X'    SEC. ALLOC FLAGS
FM1DSPAC EQU   X'C0'         SPACE REQUEST MASK
FM1CYL   EQU   X'C0'           CYLINDER BOUND
FM1TRK   EQU   X'80'           TRACK
FM1AVRND EQU   X'41'           AVG BLOCK + ROUND
FM1AVR   EQU   X'40'           AVG BLOCK LEN
FM1MSGP  EQU   X'20'
FM1EXT   EQU   X'10'           SEC. EXTENSION EXISTS
FM1CONTG EQU   X'08'           REQ. CONTIGUOUS
FM1MXIG  EQU   X'04'           MAX
FM1ALX   EQU   X'02'           ALX
FM1DSABS EQU   X'00'           ABSOLUTE TRACK
FM1SCAL3 EQU   FM1SCAL1+1,3,C'X'  SEC ALLOC QUANTITY
         SPACE 1
DDWATTR  DS    16XL8         DS ATTRIBUTES (DSORG,RECFM,X,LRECL,BLKSI)
BLDLLIST DS    Y(1,12+2+31*2)     BLDL LIST HEADER
BLDLNAME DS    CL8' ',XL(4+2+31*2)    MEMBER NAME AND DATA
         AGO   .COMSWA  replaced SWA for cross-assembly compatibility
*COMP*   AIF   ('&ZSYS' NE 'S390').COMSWA
DDWEPA   DS    A(DDWSVA)
DDWSWA   SWAREQ FCODE=RL,EPA=DDWEPA,MF=L
DDWSVA   DS    7A                 (IBM LIES ABOUT 4A)
DDWSWAL  EQU   *-DDWSWA           LENGTH TO CLEAR
.COMSWA  SPACE 1
ZEROES   DS    F             CONSTANT
DDWBLOCK DS    F             MAXIMUM BUFFER SIZE NEEDED
DDWFLAGS DS    X             RESULT FLAGS FOR ALL
DDWFLAG1 DS    X             RESULT FLAGS FOR FIRST DD
DDWFLAG2 DS    X             RESULT FLAGS FOR ALL BUT FIRST
CWFDD    EQU   X'80'           FOUND A DD
CWFCONC  EQU   CWFDD           AFTER FLAG MERGE - CONCATENATED
CWFSEQ   EQU   X'40'           USE IS SEQUENTIAL
CWFPDQ   EQU   X'20'           DS IS PDS WITH MEMBER NAME
CWFPDS   EQU   X'10'           DS IS PDS (or PDS/E with S390)
CWFVSM   EQU   X'08'           DS IS VSAM (limited support)
CWFVTOC  EQU   X'04'           DS IS VTOC (limited support)
CWFBLK   EQU   X'02'           DD HAS FORCED BLKSIZE
OPERF    DS    X             ERROR CONDITIONS
ORFBADNM EQU   04            DD name <= blank
ORFNODD  EQU   08            DD name not found in TIOT
ORFNOJFC EQU   12            Error getting JFCB
ORFNODS1 EQU   16            Error getting DSCB 1
ORFBATIO EQU   20            Unusable TIOT entry
ORFBADSO EQU   24            Invalid or unsupported DSORG
ORFBADCB EQU   28            Invalid DCB parameters
ORFBATY3 EQU   32            Unsupported unit type (Graf, Comm, etc.)
ORFBACON EQU   36            Invalid concatenation
ORFBDMOD EQU   40            Invalid MODE for DD/function
ORFBDPDS EQU   44            PDS not initialized
ORFBDDIR EQU   48            PDS not initialized
ORFNOSTO EQU   52            Out of memory
ORFNOMEM EQU   68            Member not found (BLDL/FIND)
ORFBDMEM EQU   72            Member not permitted (seq.)
ORFTOBIG EQU   96            EXTEND to more than 64KIB tracks
         SPACE 1
TRUENAME DS    CL44               DS name for alias on DD
CATWORK  DS    ((265+7)/8)D'0'    LOCATE work area
         ORG   CATWORK            Redefine returned data
CAWCOUNT DS    H                  Number of entries returned
CAW#VOL  DS    H                  Number of volumes in this DS
CAWTYPE  DS    XL4                Unit type
CAWSER   DS    CL6                Volume serial
CAWFILE  DS    XL2                Tape file number
         ORG   ,
OPENLEN  EQU   *-WORKAREA         Length for @@AOPEN processing
         SPACE 2
***********************************************************************
*                                                                     *
* ZDCBAREA - the address of this memory is used by the C caller       *
* as a "handle". The block of memory has different contents depending *
* on what sort of file is being opened, but it will be whatever the   *
* assembler code is expecting, and the caller merely needs to         *
* provide the handle (returned from AOPEN) in subsequent calls so     *
* that the assembler can keep track of things.                        *
*                                                                     *
*   FILE ACCESS CONTROL BLOCK (N.B.-STARTS WITH DCBD DUE TO DSECT)    *
*   CONTAINS DCB, DECB, JFCB, DSCB 1, BUFFER POINTERS, FLAGS, ETC.    *
*                                                                     *
***********************************************************************
         DCBD  DSORG=PS,DEVD=(DA,TA)   Map Data Control Block
         ORG   IHADCB             Overlay the DCB DSECT
ZDCBAREA DS    0H
         DS    CL(BPAMDCBL)       Room for BPAM DCB
         READ  DECB,SF,IHADCB,2-2,3-3,MF=L  READ/WRITE BSAM
*DEFUNCT ORG   IHADCB             Only using one DCB
*DEFUNCT DS    CL(QSAMDCBL)         so overlay this one
         ORG   IHADCB             Only using one DCB
         DS    CL(BSAMDCBL)
         ORG   IHADCB             Only using one DCB
ZAACB    DS    CL(VSAMDCBL)       VSAM ACB
ZARPL    RPL   ACB=ZAACB,OPTCD=(SEQ,SYN,LOC)
ZAMODCB  DS    XL(ZAMODCBL)  MODCB WORK AREA
ZASHOCB  DS    XL(ZASHOCBL)  SHOCB WORK AREA
ZAARG    DS    A                  Pointer
ZARRN    DS    F                  Relative record number
         SPACE 2
         ORG   IHADCB             Only using one DCB
TAPEDCB  DCB   DDNAME=TAPE,MACRF=E,DSORG=PS,REPOS=Y,BLKSIZE=0,         *
               DEVD=TA                 LARGE SIZE
         ORG   TAPEDCB+84    LEAVE ROOM FOR DCBLRECL
ZXCPVOLS DC    F'0'          VOLUME COUNT
TAPECCW  CCW   1,3-3,X'40',4-4
         CCW   3,3-3,X'20',1
TAPEXLEN EQU   *-TAPEDCB     PATTERN TO MOVE
TAPEECB  DC    A(0)
TAPEIOB  DC    X'42,00,00,00'
         DC    A(TAPEECB)
         DC    2A(0)
         DC    A(TAPECCW)
         DC    A(TAPEDCB)
         DC    2A(0)
         SPACE 1
         ORG   IHADCB
ZPUTLINE PUTLINE MF=L        PATTERN FOR TERMINAL I/O
*DSECT*  IKJIOPL ,
         SPACE 1
ZIOPL    DS    0A            MANUAL EXPANSION TO AVOID DSECT
IOPLUPT  DS    A        PTR TO UPT
IOPLECT  DS    A        PTR TO ECT
IOPLECB  DS    A        PTR TO USER'S ECB
IOPLIOPB DS    A        PTR TO THE I/O SERVICE RTN PARM BLOCK
ZIOECB   DS    A                   TPUT ECB
ZIOECT   DS    A                   ORIGINATING ECT
ZIOUPT   DS    A                   UPT
ZIODDNM  DS    CL8      DD NAME AT OFFSET X'28' FOR DCB COMPAT.
ZGETLINE GETLINE MF=L             TWO WORD GTPB
         SPACE 2
*   VTOC READ ACCESS - INTERLEAVE WITH BSAM DCB
*
         ORG   IHADCB
ZVCPVOL  DS    H                  Cylinder per volume
ZVTPCYL  DS    H                  Tracks per cylinder
ZVLOCCHH DS    XL4     1/3        CCHH of VTOC start
ZVHICCHH DS    XL4     2/3        CCHH of VTOC end
ZVHIREC  DS    X       3/3        High record on track
         DS    0H                 Align for STH
ZVUSCCHH DS    XL5                Address of current record
ZVSER    DS    CL6                Volume serial
ZVSEEK   CAMLST SEEK,1-1,2-2,3-3  CAMLST to SEEK by address
         SPACE 2
         ORG   ,
OPENCLOS DS    A                  OPEN/CLOSE parameter list
DCBXLST  DS    2A                 07 JFCB / 85 DCB EXIT
EOFR24   DS    CL(EOFRLEN)
         DS    0A                 Ensure correct DC A alignment
         AIF   ('&ZSYS' EQ 'S370').NOSB   Only S/380+90 needs a stub
A24STUB  DS    CL(PATSTUBL)       DCB open exit 24-bit code
.NOSB    ANOP  ,                  Only S/390 needs a stub
ZBUFF1   DS    A,F                Address, length of buffer
ZBUFF2   DS    A,F                Address, length of 2nd buffer
KEPTREC  DS    A,F                Address & length of saved rcd
*
         MAPSUPRM DSECT=NO,PFX=ZP      MAP MODE WORK AREA
BUFFADDR DS    A     1/3          Location of the BLOCK Buffer
BUFFCURR DS    A     2/3          Current record in the buffer
BUFFEND  DS    A     3/3          Address after end of current block
VBSADDR  DS    A                  Location of the VBS record build area
VBSEND   DS    A                  Addr. after end VBS record build area
         SPACE 1
ZWORK    DS    D             Below the line work storage
ZDDN     DS    CL8           DD NAME
ZMEM     DS    CL8           MEMBER NAME or nulls
DEVINFO  DS    2F                 UCB Type / Max block size
ZTTR     DS    A             Last TTR written (BSAM, EXCP)
         SPACE 1
RECFMIX  DS    X             Record format index: 0-F 4-V 8-U
IXFIX    EQU   0               Recfm = F
IXVAR    EQU   4               Recfm = V
IXUND    EQU   8               Recfm = U
         SPACE 1
ZDVTYPE  DS    X      1/4    Device type of first/only DD
         SPACE 1
ZRECFM   DS    X      2/4    Equivalent RECFM bits
         SPACE 1
IOSFLAGS DS    X      3/4    Additional MODE related flags
IOFVSAM  EQU   X'04'           Use VSAM
         SPACE 1
IOMFLAGS DS    X      4/4    Remember open MODE
IOFTERM  EQU   X'80'           Using GETLINE/PUTLINE
IOFBPAM  EQU   X'20'           Unlike BPAM concat - special handling
IOFBLOCK EQU   X'10'           Using BSAM READ/WRITE mode
IOFEXCP  EQU   X'08'           Use EXCP for TAPE
IOFOUT   EQU   X'01'           Output mode
         SPACE 1
IOPFLAGS DS    X             Remember prior events
IOFLEOF  EQU   X'80'           Encountered an End-of-File
IOFLSDW  EQU   X'40'           Spanned record incomplete
IOFLDATA EQU   X'20'           Output buffer has data
IOFLIOWR EQU   X'10'           Last I/O was Write type
IOFCURSE EQU   X'08'           Write buffer recursion
IOFDCBEX EQU   X'04'           DCB exit entered
IOFCONCT EQU   X'02'           Reread - unlike concatenation
IOFKEPT  EQU   X'01'           Record info kept
         SPACE 1
FILEMODE DS    X             AOPEN requested record format default
FMFIX    EQU   0               Fixed RECFM (blocked)
FMVAR    EQU   1               Variable (blocked)
FMUND    EQU   2               Undefined
ZDDFLAGS DS    X             RESULT FLAGS FOR ALL
TRKLIST  TRKCALC FUNCTN=TRKCAP,UCB=(R3),BALANCE=*,RKDD=TKRKDD,         *
               REGSAVE=YES,MF=L            GET BLOCKS PER TRACK
ZIOSAVE2 DS    18F           Save area for physical write
SAVEADCB DS    18F                Register save area for PUT
ZDCBLEN  EQU   *-ZDCBAREA
*
* End of handle/DCB area
*
*
*
         SPACE 2
         PRINT NOGEN
         IHAPSA ,            MAP LOW STORAGE
         CVT DSECT=YES
         IKJTCB ,            MAP TASK CONTROL BLOCK
         IKJECT ,            MAP ENV. CONTROL BLOCK
         IKJPTPB ,           PUTLINE PARAMETER BLOCK
         IKJCPPL ,
         IKJPSCB ,
         IEZJSCB ,
         IEZIOB ,
         IEFZB4D0 ,          MAP SVC 99 PARAMETER LIST
         IEFZB4D2 ,          MAP SVC 99 PARAMETERS
MYUCB    DSECT ,
         IEFUCBOB ,
MYTIOT   DSECT ,
         IEFTIOT1 ,
         IEZDEB ,
         IHAPDS PDSBLDL=YES
         SPACE 1
         IFGACB ,
         SPACE 1
         IFGRPL ,
         IEFJESCT ,
         IKJUPT ,
R0       EQU   0             NO STANDARD REGEQU MACRO
R1       EQU   1             NO STANDARD REGEQU MACRO
R2       EQU   2             NO STANDARD REGEQU MACRO
R3       EQU   3             NO STANDARD REGEQU MACRO
R4       EQU   4             NO STANDARD REGEQU MACRO
R5       EQU   5             NO STANDARD REGEQU MACRO
R6       EQU   6             NO STANDARD REGEQU MACRO
R7       EQU   7             NO STANDARD REGEQU MACRO
R8       EQU   8             NO STANDARD REGEQU MACRO
R9       EQU   9             NO STANDARD REGEQU MACRO
R10      EQU   10            NO STANDARD REGEQU MACRO
R11      EQU   11            NO STANDARD REGEQU MACRO
R12      EQU   12            NO STANDARD REGEQU MACRO
R13      EQU   13            NO STANDARD REGEQU MACRO
R14      EQU   14            NO STANDARD REGEQU MACRO
R15      EQU   15            NO STANDARD REGEQU MACRO
         END   ,
