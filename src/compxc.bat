gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ xcopy.c
aswin -o xcopy.o xcopy.s
del xcopy.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

aswin -o support.o support.s

ldwin -o xcopy.exe ../pdpclib/w32start.o xcopy.o pos.o support.o ../pdpclib/msvcrt.a
