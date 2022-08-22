gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ secttest.c
aswin -o secttest.o secttest.s
del secttest.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

aswin -o support.o support.s

ldwin -o secttest.exe ../pdpclib/w32start.o secttest.o pos.o support.o ../pdpclib/msvcrt.a
