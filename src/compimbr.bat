rem uses PDPCLIB's makefile.wcd

rem wasm -zq -zcm -Dmemodel=large support.asm
rem wcl -q -w -c -I. -ml -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib instmbr.c bos.c
rem wlink File instmbr.obj,bos.obj,support.obj Name instmbr.exe Form dos Library ..\pdpclib\watcom.lib Option quiet


gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ instmbr.c
aswin -o instmbr.o instmbr.s
del instmbr.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ bos.c
aswin -o bos.o bos.s
del bos.s

aswin -o support.o support.s

ldwin -o instmbr.exe ../pdpclib/w32start.o instmbr.o bos.o support.o ../pdpclib/msvcrt.a
