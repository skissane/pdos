wasm -zq -zcm -Dmemodel=large support.asm
wcl -oneatx -ecc -zp1 -q -w -c -I. -ml -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib sys.c pos.c
wlink File sys.obj,pos.obj,support.obj Name sys.exe Form dos Library ..\pdpclib\watcom.lib Option quiet
