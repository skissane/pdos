gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ boot.c
aswin -o boot.o boot.s
del boot.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

aswin -o support.o support.s

ldwin -o boot.exe ../pdpclib/w32start.o boot.o pos.o support.o ../pdpclib/msvcrt.a
