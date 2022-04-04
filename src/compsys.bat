rem If you want to build an executable that works under
rem MSDOS, you will need to use Watcom's version of the
rem int86 routine which compensates for flags not being
rem popped in INT 25H and 26H

rem wcl -zp1 -q -I. -ml -DWATNATIVE sys.c pos.c


rem To build an executable that works under PDOS/86 use this:

rem wasm -zq -zcm -Dmemodel=large support.asm
rem wcl -zp1 -q -w -c -I. -ml -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib sys.c pos.c
rem wlink File sys.obj,pos.obj,support.obj Name sys.exe Form dos Library ..\pdpclib\watcom.lib Option quiet



gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ sys.c
aswin -o sys.o sys.s
del sys.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

aswin -o support.o support.s

ldwin -o sys.exe ../pdpclib/w32start.o sys.o pos.o support.o ../pdpclib/msvcrt.a
