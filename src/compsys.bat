rem Use Watcom's version of int86 routine which compensates
rem for flags not being popped in INT 25H and 26H

wcl -q -I. -ml -DWATNATIVE sys.c pos.c


rem gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ sys.c
rem aswin -o sys.o sys.s
rem del sys.s

rem gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
rem aswin -o pos.o pos.s
rem del pos.s

rem aswin -o support.o support.s

rem ldwin -o sys.exe ../pdpclib/w32start.o sys.o pos.o support.o ../pdpclib/msvcrt.a
