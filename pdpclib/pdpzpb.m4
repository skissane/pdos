//PDPMVS   JOB CLASS=C,REGION=0K,TIME=1440
//*
//PDPASM   PROC LIST='NOLIST'
//ASM      EXEC PGM=ASMA90,
//   PARM='DECK,&LIST,TERM'
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR,DCB=BLKSIZE=32720
//         DD DSN=&&MACLIB,DISP=(OLD,PASS)
//         DD DSN=SYS1.MODGEN,DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSTERM  DD SYSOUT=*
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DUMMY
//SYSGO    DD DUMMY
//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(9000,9000)),
//            DISP=(MOD,PASS)
//*
//         PEND
//CREATE   EXEC PGM=IEFBR14
//DD13     DD DSN=&&LOADLIB,DISP=(,PASS),
// DCB=(RECFM=U,LRECL=0,BLKSIZE=6144),
// SPACE=(CYL,(10,10,20)),UNIT=SYSALLDA
//DD14     DD DSN=&&MACLIB,DISP=(,PASS),
// DCB=(RECFM=FB,LRECL=80,BLKSIZE=6160),
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
//SAPSTART EXEC PDPASM
//SYSIN  DD  *
undivert(sapstart.asm)dnl
/*
//ZPBSUPA  EXEC PDPASM,LIST='LIST'
//SYSIN  DD  *
undivert(zpbsupa.asm)dnl
/*
//START    EXEC PDPASM
//SYSIN  DD *
undivert(start.s)dnl
/*
//STDIO    EXEC PDPASM
//SYSIN  DD *
undivert(stdio.s)dnl
/*
//STDLIB   EXEC PDPASM
//SYSIN  DD  *
undivert(stdlib.s)dnl
/*
//CTYPE    EXEC PDPASM
//SYSIN  DD  *
undivert(ctype.s)dnl
/*
//STRING   EXEC PDPASM
//SYSIN  DD  *
undivert(string.s)dnl
/*
//TIME     EXEC PDPASM
//SYSIN  DD  *
undivert(time.s)dnl
/*
//ERRNO    EXEC PDPASM
//SYSIN  DD  *
undivert(errno.s)dnl
/*
//ASSERT   EXEC PDPASM
//SYSIN  DD  *
undivert(assert.s)dnl
/*
//LOCALE   EXEC PDPASM
//SYSIN  DD  *
undivert(locale.s)dnl
/*
//MATH     EXEC PDPASM
//SYSIN  DD  *
undivert(math.s)dnl
/*
//SETJMP   EXEC PDPASM
//SYSIN  DD  *
undivert(setjmp.s)dnl
/*
//SIGNAL   EXEC PDPASM
//SYSIN  DD  *
undivert(signal.s)dnl
/*
//@@MEMMGR EXEC PDPASM
//SYSIN  DD  *
undivert(__memmgr.s)dnl
/*
//ZPBSUPC  EXEC PDPASM
//SYSIN  DD  *
undivert(zpbsupc.s)dnl
/*
//PDPTEST EXEC PDPASM
//SYSIN  DD  *
undivert(pdptest.s)dnl
/*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,AMODE=31,RMODE=ANY'
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//*SYSLIB not needed
//SYSLMOD  DD DSN=&&LOADLIB(PDPTEST),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSABEND DD SYSOUT=*
//SYSIN DD *
/*
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=&&LOADLIB,DISP=(OLD,PASS)
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(1,1)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=PDPTEST
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
//PDPTEST    EXEC PGM=LOADZERO,PARM='dd:in dd:out'
//STEPLIB  DD  DSN=OZPD.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY2,DISP=(OLD,DELETE)
//OUT      DD  DSN=HERC02.ZIP,DISP=(,KEEP),UNIT=TAPE,
//         LABEL=(1,SL),VOL=SER=MFTOPC,
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=8000)
//SYSPRINT DD  SYSOUT=*
//SYSTERM  DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//*
//* Running the test program requires IPLing it
//*
//
