rem wasm -zq -zcm -Dmemodel=large support.asm
rem wcl -zp1 -q -w -c -I. -ml -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib boot.c pos.c
rem wlink File boot.obj,pos.obj,support.obj Name boot.exe Form dos Library ..\pdpclib\watcom.lib Option quiet


gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ boot.c
aswin -o boot.o boot.s
del boot.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

aswin -o support.o support.s

ldwin -o boot.exe ../pdpclib/w32start.o boot.o pos.o support.o ../pdpclib/msvcrt.a
