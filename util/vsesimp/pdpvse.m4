* $$ JOB JNM=VSEJOB
* $$ LST LST=SYSLST,CLASS=A
// JOB VSEJOB
*
*
*
* Standard assignments for C programs
*
// ASSGN SYS000,SYSLNK
// ASSGN SYS005,SYSLST
// ASSGN SYS007,SYSPCH
// OPTION DUMP
*
*
*
*
* Now assemble the main routine
*
// OPTION CATAL
 PHASE PDPTEST,S+80
// EXEC ASSEMBLY
undivert(func1.asm)dnl
/*
*
* Now assemble the subroutines
*
// EXEC ASSEMBLY
undivert(func2.asm)dnl
/*
*
* Now link the whole app
*
// EXEC LNKEDT
*
*
*
* Now run the app
*
// EXEC PDPTEST,SIZE=AUTO,PARM='PaulEdwards two Three'
*
*
*
*
*
*
* Now punch the phase
*
// DLBL IJSYSPH,'PDPPUNCH.DAT',0,SD
// EXTENT SYSPCH,,,,10000,1000 SPACE-SYSPUNCH
ASSGN SYSPCH,SYS000
// EXEC CSERV
 PUNCH PDPTEST
/*
CLOSE SYSPCH,PUNCH
*
*
* And put it to tape
*
// DLBL SDI1,'PDPPUNCH.DAT'
// TLBL MTO1,'HERC01.MFTOPC',0
// ASSGN SYS011,TAPE,VOL=MFTOPC
// EXEC VSEMANIP,SIZE=AUTO,PARM='dd:sdi1 dd:mto1'
*
*
*
*
*
/&
* $$ EOJ
