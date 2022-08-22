gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ makecd.c
aswin -o makecd.o makecd.s
del makecd.s

ldwin -o makecd.exe ../pdpclib/w32start.o makecd.o ../pdpclib/msvcrt.a
