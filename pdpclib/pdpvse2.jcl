* $$ JOB JNM=VSEJOB
* $$ LST LST=SYSLST,CLASS=A,RBM=999999
// JOB VSEJOB
*
*
* Note that the first step to get the file off tape should
* instead be changed to whatever is suitable for your site.
* Also note that this puts stuff into the system CIL.
*
* 10000,2000 SPACE-PHASE
* 12000,1000 SPACE-PUNCH
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
* First we get the main phase off tape, and into 80-byte blocks
* using objmaint
*
// UPSI 1
// ASSGN SYS004,TAPE
// DLBL UOUT,'PDPPUNCH.DAT',0,SD
// EXTENT SYS005,,,,10000,2000 SPACE-PHASE
// ASSGN SYS005,SYS000
// EXEC OBJMAINT,SIZE=AUTO
./COPY
/*
// UPSI 0
// ASSGN SYS005,SYSLST
// ASSGN SYS004,UA
*
*
*
*
*
*
*
* Then we put the phase into the CIL, using LNKEDT
*
// DLBL IJSYSLN,,0,SD
// EXTENT SYSLNK,,,,12000,2000 SPACE-LINK
ASSGN SYSLNK,SYS000
// OPTION CATAL
// DLBL IJSYSIN,'PDPPUNCH.DAT'
ASSGN SYSIPT,SYS000
 INCLUDE
CLOSE SYSIPT,READER
// EXEC LNKEDT
ASSGN SYSLNK,UA
*
*
*
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
*
*
/&
* $$ EOJ
