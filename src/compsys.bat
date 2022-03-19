rem Use Watcom's version of int86 routine which compensates
rem for flags not being popped in INT 25H and 26H

rem wcl -q -I. -ml -DWATNATIVE sys.c pos.c


gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ sys.c
aswin -o sys.o sys.s
del sys.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

aswin -o support.o support.s

ldwin -o sys.exe ../pdpclib/w32start.o sys.o pos.o support.o ../pdpclib/msvcrt.a
