rd /s /q dist
md dist
md dist\tapes

dasdinit -bz2 dist\scratch.cckd 3390 SC3390
copy \devel\mvssrc\ozpd\c\zap.c dist
copy \winpath\zap.exe dist
copy zapcons.bat dist
copy readme2.txt dist\readme.txt

echo after you get the prompt, type "quit"
call doit
copy tapes\out.aws dist\tapes
copy saved.log dist\hercules.log
copy pdos00.ckd dist
copy pdos.cnf dist
copy pdos.ins dist
copy pcomm.exe dist

echo after hercules/380 starts, run startterm then ipl 1b9
echo then mkipltap tav1c2:
echo then quit
hercules -f pdos.cnf
copy tapes\out.aws dist\tapes\ipltap.aws

echo after hercules starts the usual way do: dsk2fil 1b9 tap1c2:
hercules -f pdos.cnf
copy tapes\out.aws dist\tapes\disk3390.aws

echo after hercules start, do: mkiplmem dev1c2:
hercules -f pdos.cnf
hetget -b -n tapes\out.aws dist\pdosmem.img 1 U 0 32760

echo after hercules start, do: mkiplcrd dev1c2:
hercules -f pdos.cnf
hetget -b -n tapes\out.aws dist\tapes\iplcrd.dat 1 U 0 32760
