//PDPGEN   JOB CLASS=C,REGION=0K
//*
//RUNPDP   PROC PDPPREF='PDPCLIB'
//PDPTEST  EXEC PGM=PDPTEST,PARM='abc'
//STEPLIB  DD DSN=&PDPPREF..LINKLIB,DISP=SHR
//SYSIN    DD DUMMY
//SYSPRINT DD SYSOUT=*
//SYSTERM  DD SYSOUT=*
//         PEND
//*
//CLEAN    PROC PDPPREF='PDPCLIB'
//DELETE   EXEC PGM=IEFBR14
//DD1      DD DSN=&PDPPREF..ALLZIPS,DISP=(MOD,DELETE),
//       UNIT=SYSALLDA,SPACE=(TRK,(0))
//         PEND
//*
//S1 EXEC RUNPDP
//S2 EXEC CLEAN
//
