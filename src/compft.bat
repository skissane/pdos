rem wasmr -zq -zcm -Dmemodel=large support.asm
rem tcc -O -c -ml -I..\pdpclib functest.c pos.c bos.c
rem tlink -x functest+bos+pos+support,functest.exe,,..\pdpclib\borland.lib

rem wasm -zq -zcm -Dmemodel=large support.asm
rem wcl -q -w -c -I. -ml -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib functest.c pos.c bos.c
rem wlink File functest.obj,pos.obj,bos.obj,support.obj Name functest.exe Form dos Library ..\pdpclib\watcom.lib Option quiet

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ functest.c
aswin -o functest.o functest.s
del functest.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ bos.c
aswin -o bos.o bos.s
del bos.s

aswin -o support.o support.s

ldwin -o functest.exe ../pdpclib/w32start.o functest.o pos.o bos.o support.o ../pdpclib/msvcrt.a kernel32.a
