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
*
*
* Now assemble an assembler file
*
// DLBL IJSYSPH,'FUNC2.OBJ',0,SD
// EXTENT SYSPCH,,,,12000,500 SPACE-OBJ
ASSGN SYSPCH,SYS000
// OPTION DECK,NOEDECK
// EXEC ASSEMBLY
undivert(func2.asm)dnl
/*
CLOSE SYSPCH,PUNCH
*
*
*
*
* Dump the object code to tape
*
// DLBL SDI1,'FUNC2.OBJ'
// TLBL MTO1,'HERC01.MFTOPC',0
// ASSGN SYS011,TAPE,VOL=MFTOPC
// EXEC VSEMANIP,SIZE=AUTO,PARM='dd:sdi1 dd:mto1'
*
*
*
/&
* $$ EOJ
