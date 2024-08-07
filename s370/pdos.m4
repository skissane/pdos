//PDPMVS   JOB CLASS=C,REGION=0K,TIME=1440
//*
//PDPASM   PROC MEMBER='',LIST='NOLIST'
//ASM      EXEC PGM=ASMA90,
//   PARM='DECK,&LIST'
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR
//         DD DSN=&&MACLIB,DISP=(OLD,PASS)
//         DD DSN=SYS1.MODGEN,DISP=SHR
//         DD DSN=SYS1.APVTMACS,DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DUMMY
//SYSGO    DD DUMMY
//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(2000,2000)),
//            DISP=(,PASS)
//*
//LKED     EXEC PGM=IEWL,PARM='NCAL',
//            COND=(4,LT,ASM)
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//SYSLMOD  DD DSN=&&NCALIB(&MEMBER),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//         PEND
//CREATE   EXEC PGM=IEFBR14
//DD0      DD DSN=&&HEX,DISP=(,PASS),
// DCB=(RECFM=U,LRECL=0,BLKSIZE=3200),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//DD12     DD DSN=&&NCALIB,DISP=(,PASS),
// DCB=(RECFM=U,LRECL=0,BLKSIZE=3200),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//DD13     DD DSN=&&LOADLIB,DISP=(,PASS),
// DCB=(RECFM=U,LRECL=0,BLKSIZE=18432),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//DD14     DD DSN=&&MACLIB,DISP=(,PASS),
// DCB=(RECFM=FB,LRECL=80,BLKSIZE=3120),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//*
//PDPTOP   EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPTOP),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdptop.mac)dnl
/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDPMAIN  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPMAIN),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdpmain.mac)dnl
/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDPPRLG  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPPRLG),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdpprlg.mac)dnl
/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDPEPIL  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPEPIL),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdpepil.mac)dnl
/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//SAPSTART EXEC PDPASM,MEMBER=SAPSTART,LIST=LIST
//SYSIN  DD  *
undivert(sapstart.asm)dnl
/*
//SAPSUPA  EXEC PDPASM,MEMBER=SAPSUPA,LIST=LIST
//SYSIN  DD  *
undivert(sapsupa.asm)dnl
/*
//MVSSTART EXEC PDPASM,MEMBER=MVSSTART,LIST=LIST
//SYSIN  DD  *
undivert(mvsstart.asm)dnl
/*
//MVSSUPA  EXEC PDPASM,MEMBER=MVSSUPA,LIST=LIST
//SYSIN  DD  *
undivert(mvssupa.asm)dnl
/*
//START    EXEC PDPASM,MEMBER=START
//SYSIN  DD *
undivert(start.s)dnl
/*
//STDIO    EXEC PDPASM,MEMBER=STDIO
//SYSIN  DD *
undivert(stdio.s)dnl
/*
//STDLIB   EXEC PDPASM,MEMBER=STDLIB
//SYSIN  DD  *
undivert(stdlib.s)dnl
/*
//CTYPE    EXEC PDPASM,MEMBER=CTYPE
//SYSIN  DD  *
undivert(ctype.s)dnl
/*
//STRING   EXEC PDPASM,MEMBER=STRING
//SYSIN  DD  *
undivert(string.s)dnl
/*
//TIME     EXEC PDPASM,MEMBER=TIME
//SYSIN  DD  *
undivert(time.s)dnl
/*
//ERRNO    EXEC PDPASM,MEMBER=ERRNO
//SYSIN  DD  *
undivert(errno.s)dnl
/*
//ASSERT   EXEC PDPASM,MEMBER=ASSERT
//SYSIN  DD  *
undivert(assert.s)dnl
/*
//LOCALE   EXEC PDPASM,MEMBER=LOCALE
//SYSIN  DD  *
undivert(locale.s)dnl
/*
//MATH     EXEC PDPASM,MEMBER=MATH
//SYSIN  DD  *
undivert(math.s)dnl
/*
//SETJMP   EXEC PDPASM,MEMBER=SETJMP
//SYSIN  DD  *
undivert(setjmp.s)dnl
/*
//SIGNAL   EXEC PDPASM,MEMBER=SIGNAL
//SYSIN  DD  *
undivert(signal.s)dnl
/*
//@@MEMMGR EXEC PDPASM,MEMBER=@@MEMMGR
//SYSIN  DD  *
undivert(__memmgr.s)dnl
/*
//PLOAD    EXEC PDPASM,MEMBER=PLOAD
//SYSIN  DD  *
undivert(pload.s)dnl
/*
//PLOADSUP EXEC PDPASM,MEMBER=PLOADSUP,LIST=LIST
//SYSIN  DD  *
undivert(ploadsup.asm)dnl
/*
//PDOS     EXEC PDPASM,MEMBER=PDOS
//SYSIN  DD  *
undivert(pdos.s)dnl
/*
//PDOSSUP  EXEC PDPASM,MEMBER=PDOSSUP,LIST=LIST
//SYSIN  DD  *
undivert(pdossup.asm)dnl
/*
//PDOSUTIL EXEC PDPASM,MEMBER=PDOSUTIL
//SYSIN  DD  *
undivert(pdosutil.s)dnl
/*
//PCOMM    EXEC PDPASM,MEMBER=PCOMM
//SYSIN  DD  *
undivert(pcomm.s)dnl
/*
//WORLD    EXEC PDPASM,MEMBER=WORLD
//SYSIN  DD  *
undivert(world.s)dnl
/*
//MKIPLTAP EXEC PDPASM,MEMBER=MKIPLTAP
//SYSIN  DD  *
undivert(mkipltap.s)dnl
/*
//MKIPLCRD EXEC PDPASM,MEMBER=MKIPLCRD
//SYSIN  DD  *
undivert(mkiplcrd.s)dnl
/*
//BBS      EXEC PDPASM,MEMBER=BBS
//SYSIN  DD  *
undivert(bbs.s)dnl
/*
//EXE2COM  EXEC PDPASM,MEMBER=EXE2COM
//SYSIN  DD  *
undivert(exe2com.s)dnl
/*
//PDPNNTP  EXEC PDPASM,MEMBER=PDPNNTP
//SYSIN  DD  *
undivert(pdpnntp.s)dnl
/*
//COPYCON  EXEC PDPASM,MEMBER=COPYCON
//SYSIN  DD  *
undivert(copycon.s)dnl
/*
//ZCALC    EXEC PDPASM,MEMBER=ZCALC
//SYSIN  DD  *
undivert(zcalc.s)dnl
/*
//HEX2DEC  EXEC PDPASM,MEMBER=HEX2DEC
//SYSIN  DD  *
undivert(hex2dec.s)dnl
/*
//DEC2HEX  EXEC PDPASM,MEMBER=DEC2HEX
//SYSIN  DD  *
undivert(dec2hex.s)dnl
/*
//MFEMUL   EXEC PDPASM,MEMBER=MFEMUL
//SYSIN  DD  *
undivert(mfemul.s)dnl
/*
//XYCHOP   EXEC PDPASM,MEMBER=XYCHOP
//SYSIN  DD  *
undivert(xychop.s)dnl
/*
//ASSEMBLE EXEC PDPASM,MEMBER=ASSEMBLE
//SYSIN  DD  *
undivert(assemble.s)dnl
/*
//MULTISC  EXEC PDPASM,MEMBER=MULTISC
//SYSIN  DD  *
undivert(multisc.s)dnl
/*
//WTOWORLD EXEC PDPASM,MEMBER=WTOWORLD
//SYSIN  DD  *
         CSECT
         USING *,15
         WTO   'HELLO from WTO'
         LA    15,0
         BR    14
         END
/*
//*
//*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(WTOWORLD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN    DD *
 INCLUDE SYSLIB(WTOWORLD)
/*
//*
//*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PLOAD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(SAPSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(SAPSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(PLOAD)
 INCLUDE SYSLIB(PLOADSUP)
 INCLUDE SYSLIB(PDOSUTIL)
 ENTRY @@MAIN
/*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PDOS),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(SAPSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(SAPSUPA)
 INCLUDE SYSLIB(STDIO)
 INCLUDE SYSLIB(STDLIB)
 INCLUDE SYSLIB(CTYPE)
 INCLUDE SYSLIB(STRING)
 INCLUDE SYSLIB(TIME)
 INCLUDE SYSLIB(ERRNO)
 INCLUDE SYSLIB(ASSERT)
 INCLUDE SYSLIB(LOCALE)
 INCLUDE SYSLIB(MATH)
 INCLUDE SYSLIB(SETJMP)
 INCLUDE SYSLIB(SIGNAL)
 INCLUDE SYSLIB(@@MEMMGR)
 INCLUDE SYSLIB(PDOS)
 INCLUDE SYSLIB(PDOSSUP)
 INCLUDE SYSLIB(PDOSUTIL)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PCOMM),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(PCOMM)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(WORLD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(WORLD)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(MKIPLTAP),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MKIPLTAP)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(MKIPLCRD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MKIPLCRD)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(BBS),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(BBS)
 ENTRY @@MAIN
/*
//*
//* These programs should link with standard pdpclib,
//* not the current memmgr version
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(EXE2COM),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(EXE2COM)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PDPNNTP),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(PDPNNTP)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(COPYCON),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(COPYCON)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(ZCALC),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(ZCALC)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(HEX2DEC),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(HEX2DEC)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(DEC2HEX),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(DEC2HEX)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(MFEMUL),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MFEMUL)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(XYCHOP),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(XYCHOP)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(ASSEMBLE),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(ASSEMBLE)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=PDPCLIB.NCALIB,DISP=SHR
//         DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(MULTISC),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MULTISC)
 ENTRY @@MAIN
/*
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PLOAD
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&COPY2,DISP=(NEW,PASS),SPACE=(CYL,(1,1)),
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=18452),UNIT=SYSALLDA
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PLOAD    EXEC PGM=LOADZERO,PARM='dd:in dd:out'
//STEPLIB  DD  DSN=OZPD.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY2,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PLOAD),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDOS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&COPY2,DISP=(NEW,PASS),SPACE=(CYL,(1,1)),
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=18452),UNIT=SYSALLDA
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDOS     EXEC PGM=LOADZERO,PARM='dd:in dd:out'
//STEPLIB  DD  DSN=OZPD.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY2,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDOSIMG),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDOS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDOS),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=WTOWORLD
/*
//*
//* Get WTOWORLD as a COM file
//* Note that we can't read and write from the same temporary
//* file, so we do this here instead. So wtoworld is an exception
//*
//EXE2COM  EXEC PGM=EXE2COM,PARM='dd:in dd:out'
//STEPLIB  DD  DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//IN       DD  DSN=&&COPY,DISP=(OLD,PASS)
//OUT      DD  DSN=&&HEX(WTOWCOM),DISP=(OLD,PASS)
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(WTOWORLD),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy WORLD
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=WORLD
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(WORLD),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MKIPLTAP
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MKIPLTAP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MKIPLTAP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MKIPLCRD
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MKIPLCRD
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MKIPLCRD),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy BBS
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=BBS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(BBS),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy EXE2COM
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=EXE2COM
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(EXE2COM),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy PDPNNTP
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDPNNTP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDPNNTP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy COPYCON
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=COPYCON
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(COPYCON),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy ZCALC
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=ZCALC
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(ZCALC),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy HEX2DEC
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=HEX2DEC
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(HEX2DEC),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy DEC2HEX
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=DEC2HEX
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(DEC2HEX),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy MFEMUL
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MFEMUL
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MFEMUL),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy XYCHOP
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=XYCHOP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(XYCHOP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy ASSEMBLE
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=ASSEMBLE
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(ASSEMBLE),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Copy MULTISC
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MULTISC
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MULTISC),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy DIFF.  Note that this should really be part of
//* the SEASIK package rather than here
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=DIFFUTL.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=DIFF
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(DIFF),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy UEMACS
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=UEMACS.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=UEMACS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(UEMACS),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy PDMAKE
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDMAKE.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDMAKE
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDMAKE),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy PDCC
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDCC.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDCC
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PDCC),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MicroEMACS 3.6
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=MEMACS.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MEMACS
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MEMACS),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy EDLIN
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=EDLIN.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=EDLIN
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(EDLIN),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy GCC.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=GCC.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=GCC
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(GCC),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy COPYFILE.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=COPYFILE
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(COPYFILE),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy HEXDUMP.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=HEXDUMP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(HEXDUMP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MVSENDEC.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MVSENDEC
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MVSENDEC),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//* Copy MVSUNZIP.
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=MVSUNZIP
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(MVSUNZIP),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PCOMM
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(PCOMM),DISP=(OLD,PASS)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(PCOMMIN),DISP=(OLD,PASS)
//IN       DD  *
echo off
echo welcome to autoexec.bat
echo type "help" for some example commands

echo type "type readme.txt" for some documentation
echo but that won't work very well so you need to use
echo hexdump instead (just type "hexdump")

echo on
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(SAMPLE),DISP=(OLD,PASS)
//IN       DD  *
#include <stdio.h>

int main(void)
{
    printf("hello, world\n");
    printf("maximum file size is %d\n", FILENAME_MAX);
    return (0);
}
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(MVSSAMP),DISP=(OLD,PASS)
//IN       DD  DATA,DLM=XX
/* ***************************************************************** */
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/* ***************************************************************** */
/* ***************************************************************** */
/*                                                                   */
/*  mvssamp - sample program that can be handled by multisc          */
/*                                                                   */
/*  This program just prints ABCDE using WTO                         */
/*                                                                   */
/*  compile like this:                                               */
/*  multisc mvssamp.c mvssamp.com                                    */
/*                                                                   */
/* ***************************************************************** */

int ret; /* must be this name */
    /* reason being that the hash is calculated as:
       ( (114-48)*100+(101-48)*10+(116-48) ) * 2 = 14396 = x'383c'
       which you can see referenced in the assembler below,
       and also in the startup code which is embedded in the compiler */
int x;

/* this will probably be in a separate file, and pdcc used to
   merge them together this first. It needs to be seen before
   "main" calls it */

void display()
{
    /* can't use r7 - points to base of code */
    /* can't use r13 - points to save area */
    /* all other registers are free to use */
    
    /* we want to load r9 with data at a fixed address (ret) from r7 */
    
    /* balr r15, 0 */
    asm 5; asm 240; /* 05 f0 */
    
/* base1: */
    
    /* bal r14, 8(,r15) */ /* call bypass1 */
    asm 69; asm 224; asm 240; asm 8; /* 45 e0 f0 08 */
    
    /* 6 * 0x10000 (DATASTART) + 0x7078 = 0x67078 */
    /* this is where the "ret" variable goes */
    asm 0; asm 6; asm 112; asm 120;

/* bypass1: */
        
    /* l r9, 0(,r14) */
    asm 88; asm 144; asm 224; asm 0; /* 58 90 e0 00 */

    /* ar r9, r7 */
    asm 26; asm 151; /* 1a 97 */
        
    /* l r9, 0(,r9) */
    asm 88; asm 144; asm 144; asm 0; /* 58 90 90 00 */


    /* now we have the data - store it where the WTO is */
    /* reestablish addressability in case someone wants to
       put more code above here */
            
    /* balr r15, 0 */
    asm 5; asm 240; /* 05 f0 */

/* base2: */
        
    /* stc r9, 12(,r15) */ 
    asm 66; asm 144; asm 240; asm 12; /* 42 90 f0 0c */
    
    /* bal r1, 18(,r15) */  /* call bypass2 */
    asm 69; asm 16; asm 240; asm 18; /* 45 10 f0 12 */
    
    /* this is the WTO parameters */
    asm 0; asm 5; /* text length of 1 requires 5 */
    asm 128; asm 0; /* mcs flags */
    asm 231; /* placeholder ('X') */
    asm 0; asm 0; /* descriptor codes */
    asm 0; asm 32; /* routing codes */
    asm 0; /* one byte padding to get 2-byte alignment because we
              output a single character */
    
/* bypass2: */
    
    asm 10; asm 35; /* svc 35 */

    /* asm 0; asm 0; */ /* dc h'0' to force a s0c1 abend - can't easily
                           use ex 0,* to get s0c3 instead */
    
}

/* this is the only function you really need to read */

void main()
{
    ret = 193; /* 'A' in EBCDIC */
    while( ret < 198 ){
        display();
        ret = ret + 1;
    }
    ret = 0; /* return code to operating system */
}

/* this probably belongs in a separate file, for pdcc to
   merge last */

void _start()
{
    main();
}
XX
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(MSSUP),DISP=(OLD,PASS)
//IN       DD  DATA,DLM=XX
/* ***************************************************************** */
/*                                                                   */
/*  This code was written by Paul Edwards.                           */
/*  Released to the Public Domain                                    */
/*                                                                   */
/* ***************************************************************** */
/* ***************************************************************** */
/*                                                                   */
/*  mssup - some support routines useful to C programs using         */
/*  the multisc (subset) C compiler on z/PDOS                        */
/*                                                                   */
/*  this is seen as a header file, so it looks like the normal       */
/*  way of calling utility routines (with a separate link step)      */
/*  but it is in fact inline C code, since we don't have a separate  */
/*  linker at the moment.                                            */
/*                                                                   */
/* ***************************************************************** */

int ret; /* must be this name */
    /* reason being that the hash is calculated as:
       ( (114-48)*100+(101-48)*10+(116-48) ) * 2 = 14396 = x'383c'
       which you can see referenced in the assembler below,
       and also in the startup code which is embedded in the compiler */

void display()
{
    /* can't use r7 - points to base of code */
    /* can't use r13 - points to save area */
    /* all other registers are free to use */
    
    /* we want to load r9 with data at a fixed address (ret) from r7 */
    
    /* balr r15, 0 */
    asm 5; asm 240; /* 05 f0 */
    
/* base1: */
    
    /* bal r14, 8(,r15) */ /* call bypass1 */
    asm 69; asm 224; asm 240; asm 8; /* 45 e0 f0 08 */
    
    /* 6 * 0x10000 (DATASTART) + 0x7078 = 0x67078 */
    /* this is where the "ret" variable goes */
    asm 0; asm 6; asm 112; asm 120;

/* bypass1: */
        
    /* l r9, 0(,r14) */
    asm 88; asm 144; asm 224; asm 0; /* 58 90 e0 00 */

    /* ar r9, r7 */
    asm 26; asm 151; /* 1a 97 */
        
    /* l r9, 0(,r9) */
    asm 88; asm 144; asm 144; asm 0; /* 58 90 90 00 */


    /* now we have the data - store it where the WTO is */
    /* reestablish addressability in case someone wants to
       put more code above here */
            
    /* balr r15, 0 */
    asm 5; asm 240; /* 05 f0 */

/* base2: */
        
    /* stc r9, 12(,r15) */ 
    asm 66; asm 144; asm 240; asm 12; /* 42 90 f0 0c */
    
    /* bal r1, 18(,r15) */  /* call bypass2 */
    asm 69; asm 16; asm 240; asm 18; /* 45 10 f0 12 */
    
    /* this is the WTO parameters */
    asm 0; asm 5; /* text length of 1 requires 5 */
    asm 128; asm 0; /* mcs flags */
    asm 231; /* placeholder ('X') */
    asm 0; asm 0; /* descriptor codes */
    asm 0; asm 32; /* routing codes */
    asm 0; /* one byte padding to get 2-byte alignment because we
              output a single character */
    
/* bypass2: */
    
    asm 10; asm 35; /* svc 35 */

    /* asm 0; asm 0; */ /* dc h'0' to force a s0c1 abend - can't easily
                           use ex 0,* to get s0c3 instead */
    
}
XX
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(EXPROG),DISP=(OLD,PASS)
//IN       DD  DATA,DLM=XX
/* ***************************************************************** */
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/* ***************************************************************** */
/* ***************************************************************** */
/*                                                                   */
/*  exprog - example C program                                       */
/*                                                                   */
/*  This program just prints HI using WTO                            */
/*                                                                   */
/*  compile like this:                                               */
/*  pdmake -f exprog.mak                                             */
/*                                                                   */
/*  That produces exprog.com which can be executed by just           */
/*  typing "exprog".                                                 */
/*                                                                   */
/* ***************************************************************** */

/* I wanted to use angle brackets here, but that isn't working and needs
   to be investigated (pdcc on z/PDOS says it can't find mssup.h) */
#include "mssup.h"

void main()
{
    ret = 200; /* 'H' in EBCDIC */ display();
    ret = 201; /* 'I' in EBCDIC */ display();

    ret = 0; /* return code to operating system */
}
XX
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(MSSTART),DISP=(OLD,PASS)
//IN       DD  DATA,DLM=XX
/* ***************************************************************** */
/*                                                                   */
/*  This code written by Paul Edwards.                               */
/*  Released to the Public Domain                                    */
/*                                                                   */
/* ***************************************************************** */
/* ***************************************************************** */
/*                                                                   */
/*  standard startup code required for multisc to work               */
/*                                                                   */
/* ***************************************************************** */

void _start()
{
    main();
}
XX
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(MSJOIN),DISP=(OLD,PASS)
//IN       DD  DATA,DLM=XX
/* ***************************************************************** */
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/* ***************************************************************** */
/* ***************************************************************** */
/*                                                                   */
/*  msjoin - basically a dummy file designed to allow pdcc to        */
/*  merge the actual code into a single file for multisc             */
/*                                                                   */
/*  Used by a makefile, e.g. exprog.mak                              */
/*                                                                   */
/* ***************************************************************** */

#include "mstemp1.c"
#include "msstart.c"
XX
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(EXPROGM),DISP=(OLD,PASS)
//IN       DD  DATA,DLM=XX
# Written by Paul Edwards and released to the public domain
# Produce multisc z/PDOS .com file using multisc.

all: exprog.com

exprog.com:
  copyfile exprog.c mstemp1.c
  pdcc -E -N -I . -o mstemp2.c msjoin.c
  multisc mstemp2.c exprog.com
XX
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(PDOSIN),DISP=(OLD,PASS)
//IN       DD  *
undivert(pdos.cnf)dnl
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(ANTITWIT),DISP=(OLD,PASS)
//IN       DD  *
undivert(tweets.txt)dnl
/*
//*
//COPYFILE EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//OUT      DD  DSN=&&HEX(READMEE),DISP=(OLD,PASS)
//IN       DD  *
undivert(readme2.txt)dnl
/*
//*
//ZIP      EXEC PGM=MINIZIP,PARM='-0 -x .txt -l -o dd:out dd:in'
//STEPLIB  DD  DSN=MINIZIP.LINKLIB,DISP=SHR
//SYSIN    DD  DUMMY
//SYSPRINT DD  SYSOUT=*
//OUT      DD  DSN=HERC02.ZIP,DISP=(,KEEP),UNIT=TAPE,
//         LABEL=(1,SL),VOL=SER=MFTOPC,
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=8000)
//SYSTERM  DD  SYSOUT=*
//SYSUT1   DD  DSN=&&TEMP,DISP=(,DELETE),UNIT=SYSALLDA,
//         SPACE=(CYL,(10,10))
//IN       DD  DSN=&&HEX,DISP=(OLD,PASS)
//*
//
