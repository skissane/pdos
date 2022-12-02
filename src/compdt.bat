gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ deltree.c
aswin -o deltree.o deltree.s
del deltree.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

aswin -o support.o support.s

ldwin -o deltree.exe ../pdpclib/w32start.o deltree.o pos.o support.o ../pdpclib/msvcrt.a
