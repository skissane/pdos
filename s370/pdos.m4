//PDPMVS   JOB CLASS=C,REGION=0K
//*
//PDPASM   PROC MEMBER=''
//ASM      EXEC PGM=ASMA90,
//   PARM='DECK,LIST'
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
undivert(pdptop.mac)/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDPPRLG  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPPRLG),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdpprlg.mac)/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//PDPEPIL  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(PDPEPIL),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(pdpepil.mac)/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//MVSMACS  EXEC PGM=IEBGENER
//SYSUT2   DD  DSN=&&MACLIB(MVSMACS),DISP=(OLD,PASS)
//SYSUT1   DD  *
undivert(mvsmacs.mac)/*
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DUMMY
//*
//SAPSTART EXEC PDPASM,MEMBER=SAPSTART
//SYSIN  DD  *
undivert(sapstart.asm)/*
//SAPSUPA  EXEC PDPASM,MEMBER=SAPSUPA
//SYSIN  DD  *
undivert(sapsupa.asm)/*
//MVSSTART EXEC PDPASM,MEMBER=MVSSTART
//SYSIN  DD  *
undivert(mvsstart.asm)/*
//MVSSUPA  EXEC PDPASM,MEMBER=MVSSUPA
//SYSIN  DD  *
undivert(mvssupa.asm)/*
//START    EXEC PDPASM,MEMBER=START
//SYSIN  DD *
undivert(start.s)/*
//STDIO    EXEC PDPASM,MEMBER=STDIO
//SYSIN  DD *
undivert(stdio.s)/*
//STDLIB   EXEC PDPASM,MEMBER=STDLIB
//SYSIN  DD  *
undivert(stdlib.s)/*
//CTYPE    EXEC PDPASM,MEMBER=CTYPE
//SYSIN  DD  *
undivert(ctype.s)/*
//STRING   EXEC PDPASM,MEMBER=STRING
//SYSIN  DD  *
undivert(string.s)/*
//TIME     EXEC PDPASM,MEMBER=TIME
//SYSIN  DD  *
undivert(time.s)/*
//ERRNO    EXEC PDPASM,MEMBER=ERRNO
//SYSIN  DD  *
undivert(errno.s)/*
//ASSERT   EXEC PDPASM,MEMBER=ASSERT
//SYSIN  DD  *
undivert(assert.s)/*
//LOCALE   EXEC PDPASM,MEMBER=LOCALE
//SYSIN  DD  *
undivert(locale.s)/*
//MATH     EXEC PDPASM,MEMBER=MATH
//SYSIN  DD  *
undivert(math.s)/*
//SETJMP   EXEC PDPASM,MEMBER=SETJMP
//SYSIN  DD  *
undivert(setjmp.s)/*
//SIGNAL   EXEC PDPASM,MEMBER=SIGNAL
//SYSIN  DD  *
undivert(signal.s)/*
//@@MEMMGR EXEC PDPASM,MEMBER=@@MEMMGR
//SYSIN  DD  *
undivert(__memmgr.s)/*
//PLOAD    EXEC PDPASM,MEMBER=PLOAD
//SYSIN  DD  *
undivert(pload.s)/*
//PLOADSUP EXEC PDPASM,MEMBER=PLOADSUP
//SYSIN  DD  *
undivert(ploadsup.asm)/*
//PDOS     EXEC PDPASM,MEMBER=PDOS
//SYSIN  DD  *
undivert(pdos.s)/*
//PDOSSUP  EXEC PDPASM,MEMBER=PDOSSUP
//SYSIN  DD  *
undivert(pdossup.asm)/*
//PDOSUTIL EXEC PDPASM,MEMBER=PDOSUTIL
//SYSIN  DD  *
undivert(pdosutil.s)/*
//PCOMM    EXEC PDPASM,MEMBER=PCOMM
//SYSIN  DD  *
undivert(pcomm.s)/*
//WORLD    EXEC PDPASM,MEMBER=WORLD
//SYSIN  DD  *
undivert(world.s)/*
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
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(PCOMM),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MVSSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(MVSSUPA)
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
 INCLUDE SYSLIB(PCOMM)
 ENTRY @@MAIN
/*
//*
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST'
//SYSLIN   DD DDNAME=SYSIN
//SYSLIB   DD DSN=&&NCALIB,DISP=(OLD,PASS)
//SYSLMOD  DD DSN=&&LOADLIB(WORLD),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 INCLUDE SYSLIB(MVSSTART)
 INCLUDE SYSLIB(START)
 INCLUDE SYSLIB(MVSSUPA)
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
 INCLUDE SYSLIB(WORLD)
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
//STEPLIB  DD  DSN=MVSSRC.OZPD.LINKLIB,DISP=SHR
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
//COPYFILE EXEC PGM=COPYFILE,PARM='-bb dd:in dd:out'
//STEPLIB  DD  DSN=PDPCLIB.LINKLIB,DISP=SHR
//IN       DD  DSN=&&COPY,DISP=(OLD,DELETE)
//OUT      DD  DSN=&&HEX(WTOWORLD),DISP=(OLD,PASS)
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
//* Copy DIFF.  Note that this should really be part of
//* the SEASIK package rather than here
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=DIFFUTIL.LINKLIB,DISP=SHR
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
//*
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
rem this is an example autoexec.bat

echo let's do a directory listing
dir

echo let's dump the IPL1 record
dumpblk 0 0 1

echo unzip header files
mvsunzip pdpclib.zip
dir

echo what version of GCC do we have here?
gcc --version

echo what options does GCC have?
gcc --help

mvsunzip hercauto.zip
hercauto

echo that's enough for now - enter further commands yourself!
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
//OUT      DD  DSN=&&HEX(PDOSIN),DISP=(OLD,PASS)
//IN       DD  *
undivert(herc.cnf)/*
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
