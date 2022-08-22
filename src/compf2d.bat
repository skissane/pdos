gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ fil2dsk.c
aswin -o fil2dsk.o fil2dsk.s
del fil2dsk.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

aswin -o support.o support.s

ldwin -o fil2dsk.exe ../pdpclib/w32start.o fil2dsk.o pos.o support.o ../pdpclib/msvcrt.a
