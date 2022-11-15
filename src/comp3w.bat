rem uses PDPCLIB's makefile.wcd

wasm -zq -zcm -Dmemodel=large support.asm
wcl -oneatx -zp1 -q -w -c -I. -ml -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib pcomm.c pos.c dostime.c
wlink File pcomm.obj,pos.obj,dostime.obj,support.obj Name pcomm.exe Form dos Library ..\pdpclib\watcom.lib Option quiet
