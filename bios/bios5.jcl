//BIOSGEN   JOB CLASS=C,REGION=0K
//*
//BIOSCMP   PROC BIOPREF='BIOS',MEMBER='',GCCPREF='GCC',
// PDPPREF='PDPCLIB',
// COS1='-Os -S -ansi',
// COS2='-o dd:out -'
//*
//COMP     EXEC PGM=GCC,
// PARM='&COS1 &COS2'
//STEPLIB  DD DSN=&GCCPREF..LINKLIB,DISP=SHR
//SYSIN    DD DSN=&BIOPREF..SOURCE(&MEMBER),DISP=SHR
//INCLUDE  DD DSN=&BIOPREF..INCLUDE,DISP=SHR,DCB=BLKSIZE=32720
//         DD DSN=&PDPPREF..INCLUDE,DISP=SHR
//SYSINCL  DD DSN=&BIOPREF..INCLUDE,DISP=SHR,DCB=BLKSIZE=32720
//         DD DSN=&PDPPREF..INCLUDE,DISP=SHR
//OUT      DD DSN=&&TEMP1,DISP=(,PASS),UNIT=SYSALLDA,
//            DCB=(LRECL=80,BLKSIZE=6080,RECFM=FB),
//            SPACE=(6080,(500,500))
//SYSPRINT DD SYSOUT=*
//SYSTERM  DD SYSOUT=*
//*
//ASM      EXEC PGM=ASMA90,
//            PARM='DECK,NOLIST',
//            COND=(4,LT,COMP)
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR,DCB=BLKSIZE=32720
//         DD DSN=&PDPPREF..MACLIB,DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))
//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))
//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DUMMY
//SYSGO    DD DUMMY
//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(5000,0)),
//            DISP=(MOD,PASS)
//SYSIN    DD DSN=&&TEMP1,DISP=(OLD,DELETE)
//*
//         PEND
//*
//LINK     PROC BIOPREF='BIOS',PDPPREF='PDPCLIB'
//LKED     EXEC PGM=IEWL,
// PARM='MAP,LIST,SIZE=(999424,65536),AMODE=31,RMODE=ANY'
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(30,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//SYSLIB   DD DSN=&PDPPREF..NCALIB,DISP=SHR,DCB=BLKSIZE=32760
//SYSLMOD  DD DSN=&BIOPREF..LINKLIB(BIOS),DISP=SHR
//*
//         PEND
//*
//LINKW    PROC BIOPREF='BIOS'
//LKED     EXEC PGM=IEWL,
// PARM='MAP,LIST,SIZE=(999424,65536),AMODE=31,RMODE=ANY'
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(30,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//*SYSLIB   DD DSN=&PDPPREF..NCALIB,DISP=SHR,DCB=BLKSIZE=32760
//SYSLMOD  DD DSN=&BIOPREF..LINKLIB(OSWORLD),DISP=SHR
//*
//         PEND
//*
//BIOS     EXEC BIOSCMP,MEMBER=BIOS
//EXELOAD  EXEC BIOSCMP,MEMBER=EXELOAD
//*
//DOLINK1  EXEC LINK
//*
//GENSTART EXEC BIOSCMP,MEMBER=GENSTART
//OSWORLD  EXEC BIOSCMP,MEMBER=OSWORLD
//OSFUNC   EXEC BIOSCMP,MEMBER=OSFUNC
//*
//DOLINK2  EXEC LINKW
//*
//
