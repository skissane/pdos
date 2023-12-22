CC=wcl386
# Don't use intrinsics because we need the actual
# function for PDOS-generic
# That is why I no longer use -oneatx
COPTS=-ecc -D__OS2__ -D__32BIT__ -bt=os2 -l=os2v2 -wx -c -I. -I..\src -y -fpi87 -s -zq -3s -zm -zl -oneatblr

pdptest.exe: osstart.obj pdptest.obj stdio.obj string.obj stdlib.obj \
       start.obj time.obj errno.obj assert.obj signal.obj locale.obj \
       ctype.obj setjmp.obj math.obj fpfuncsw.obj
#Debug All
  wasm -zcm -q -DOS220 ..\src\needpdos.asm
  wlink File needpdos.obj Name needpdos.exe Form dos Option quiet,dosseg
  if exist watcom.lib del watcom.lib
  wlib -q watcom osstart.obj stdio.obj string.obj stdlib.obj start.obj time.obj errno.obj assert.obj signal.obj locale.obj ctype.obj setjmp.obj math.obj fpfuncsw.obj
  rm -f os2.lib
  rm -f temp.wat
  echo ++DosExit.DOSCALLS.DosExit.234 >>temp.wat
  echo ++DosOpen.DOSCALLS.DosOpen.273 >>temp.wat
  echo ++DosClose.DOSCALLS.DosClose.257 >>temp.wat
  echo ++DosRead.DOSCALLS.DosRead.281 >>temp.wat
  echo ++DosWrite.DOSCALLS.DosWrite.282 >>temp.wat
  echo ++DosDelete.DOSCALLS.DosDelete.259 >>temp.wat
  echo ++DosMove.DOSCALLS.DosMove.271 >>temp.wat
  echo ++DosExecPgm.DOSCALLS.DosExecPgm.283 >>temp.wat
  echo ++DosSetFilePtr.DOSCALLS.DosSetFilePtr.256 >>temp.wat
  echo ++DosGetDateTime.DOSCALLS.DosGetDateTime.230 >>temp.wat
  echo ++DosDevIOCtl.DOSCALLS.DosDevIOCtl.284 >>temp.wat
  echo ++DosAllocMem.DOSCALLS.DosAllocMem.299 >>temp.wat
  echo ++DosFreeMem.DOSCALLS.DosFreeMem.304 >>temp.wat
  echo ++DosScanEnv.DOSCALLS.DosScanEnv.227 >>temp.wat
  echo ++DosSetRelMaxFH.DOSCALLS.DosSetRelMaxFH.382 >>temp.wat
  wlib -q os2.lib @temp.wat
  wlink File pdptest.obj Name pdptest.exe Form os2 flat full Library watcom.lib Library os2.lib Option quiet,stub=needpdos.exe

osstart.obj: osstart.asm
  wasm -zq -zcm -bt=os2 osstart.asm

.c.obj:
  $(CC) $(COPTS) $<
